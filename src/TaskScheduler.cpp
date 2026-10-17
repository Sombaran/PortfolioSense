#include "TaskScheduler.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace folio {

TaskScheduler::TaskScheduler(std::size_t workerCount, std::size_t maxQueueSize)
    : maxQueueSize_(maxQueueSize), running_(true) {
    if (workerCount == 0 || maxQueueSize == 0) {
        throw std::invalid_argument("task scheduler requires workers and queue capacity");
    }
    workers_.reserve(workerCount);
    for (std::size_t i = 0; i < workerCount; ++i) {
        workers_.emplace_back(&TaskScheduler::workerLoop, this);
    }
}

TaskScheduler::~TaskScheduler() {
    shutdown();
}

std::string TaskScheduler::schedule(const std::string& type, TaskFn fn, int priority) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("task scheduler is shut down");
    }
    if (tasks_.size() >= maxQueueSize_) {
        throw std::runtime_error("task queue is full");
    }
    const std::string id = type + "-" + std::to_string(++nextId_);
    Task task{id, type, std::move(fn), priority,
              std::chrono::steady_clock::now(), nextSequence_++};
    tasks_.push_back(std::move(task));
    condition_.notify_one();
    return id;
}

void TaskScheduler::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) return;
        running_ = false;
    }
    condition_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) worker.join();
    }
}

bool TaskScheduler::running() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return running_;
}

void TaskScheduler::workerLoop() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] {
                return !running_ || !tasks_.empty();
            });
            if (!running_ && tasks_.empty()) {
                return;
            }
            const auto next = std::max_element(
                tasks_.begin(), tasks_.end(), [](const Task& lhs, const Task& rhs) {
                    if (lhs.priority != rhs.priority) return lhs.priority < rhs.priority;
                    return lhs.sequence > rhs.sequence;
                });
            task = std::move(*next);
            tasks_.erase(next);
        }

        if (task.fn) {
            try {
                task.fn();
            } catch (const std::exception&) {
                std::cerr << "Task scheduler: task " << task.id << " (" << task.type
                          << ") failed\n";
            } catch (...) {
                std::cerr << "Task scheduler: task " << task.id << " (" << task.type
                          << ") failed with an unknown exception\n";
            }
        }
    }
}

} // namespace folio
