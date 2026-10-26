#include "PlatformAdapter.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdexcept>

#ifdef FOLIO_PLATFORM_MACOS
#include <mach-o/dyld.h>
#endif

extern char** environ;

namespace folio::platform {
namespace {

std::string errorText(const char* operation) {
    return std::string(operation) + ": " + std::strerror(errno);
}

} // namespace

bool initializeNetworking(std::string&) {
    return true;
}

SocketHandle createLoopbackServer(std::uint16_t port, std::string& error) {
    const SocketHandle server = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server == invalidSocket) {
        error = errorText("socket");
        return invalidSocket;
    }

    int reuse = 1;
    ::setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#ifdef FOLIO_PLATFORM_MACOS
    ::setsockopt(server, SOL_SOCKET, SO_NOSIGPIPE, &reuse, sizeof(reuse));
#endif

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (::bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 ||
        ::listen(server, 8) < 0) {
        error = errorText("bind/listen");
        ::close(server);
        return invalidSocket;
    }
    return server;
}

SocketHandle acceptConnection(SocketHandle server) {
    return ::accept(server, nullptr, nullptr);
}

std::ptrdiff_t readSocket(SocketHandle socket, char* buffer, std::size_t size) {
    return static_cast<std::ptrdiff_t>(::read(socket, buffer, size));
}

std::ptrdiff_t writeSocket(SocketHandle socket, const char* data, std::size_t size) {
#ifdef FOLIO_PLATFORM_MACOS
    return static_cast<std::ptrdiff_t>(::send(socket, data, size, 0));
#else
    return static_cast<std::ptrdiff_t>(::send(socket, data, size, MSG_NOSIGNAL));
#endif
}

int waitForSocket(SocketHandle socket, bool waitForWrite, unsigned int timeoutMilliseconds) {
    pollfd descriptor{};
    descriptor.fd = socket;
    descriptor.events = waitForWrite ? POLLOUT : POLLIN;
    const int result = ::poll(&descriptor, 1, static_cast<int>(timeoutMilliseconds));
    if (result < 0) return -1;
    if (result == 0) return 0;
    if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) return -1;
    return (descriptor.revents & descriptor.events) != 0 ? 1 : 0;
}

bool socketOperationInterrupted() {
    return errno == EINTR;
}

void setSocketTimeouts(SocketHandle socket, unsigned int seconds) {
    const timeval timeout{static_cast<decltype(timeval{}.tv_sec)>(seconds), 0};
    ::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    ::setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#ifdef FOLIO_PLATFORM_MACOS
    int noSignal = 1;
    ::setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &noSignal, sizeof(noSignal));
#endif
}

void closeSocket(SocketHandle socket) {
    if (socket != invalidSocket) ::close(socket);
}

std::string createSecureTemporaryFile(const std::string& prefix,
                                      const std::filesystem::path& directory) {
    const auto targetDirectory = directory.empty()
        ? std::filesystem::temp_directory_path() : directory;
    std::string pattern = (targetDirectory / (prefix + "XXXXXX")).string();
    std::vector<char> writable(pattern.begin(), pattern.end());
    writable.push_back('\0');
    const int descriptor = ::mkstemp(writable.data());
    if (descriptor < 0) throw std::runtime_error(errorText("mkstemp"));
    ::close(descriptor);
    return writable.data();
}

void replaceFileAtomically(const std::string& temporary,
                           const std::string& destination) {
    if (::rename(temporary.c_str(), destination.c_str()) != 0) {
        throw std::runtime_error(errorText("rename"));
    }
}

std::filesystem::path executablePath() {
#ifdef FOLIO_PLATFORM_MACOS
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) return {};
    return std::filesystem::weakly_canonical(buffer.data());
#else
    std::error_code error;
    return std::filesystem::canonical("/proc/self/exe", error);
#endif
}

int runPython(const std::string& script, const std::vector<std::string>& arguments,
              bool suppressOutput) {
    std::vector<std::string> values;
    values.reserve(arguments.size() + 2);
    values.emplace_back("python3");
    values.push_back(script);
    values.insert(values.end(), arguments.begin(), arguments.end());

    std::vector<char*> argv;
    argv.reserve(values.size() + 1);
    for (auto& value : values) argv.push_back(value.data());
    argv.push_back(nullptr);

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_t* actionsPointer = nullptr;
    if (suppressOutput) {
        const int initResult = posix_spawn_file_actions_init(&actions);
        if (initResult != 0) {
            throw std::runtime_error(std::string("cannot configure Python output: ") +
                                     std::strerror(initResult));
        }
        actionsPointer = &actions;
        const int stdoutResult = posix_spawn_file_actions_addopen(
            &actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
        const int stderrResult = posix_spawn_file_actions_addopen(
            &actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
        if (stdoutResult != 0 || stderrResult != 0) {
            posix_spawn_file_actions_destroy(&actions);
            throw std::runtime_error("cannot suppress Python process output");
        }
    }

    pid_t process = 0;
    const int spawnResult = posix_spawnp(&process, values.front().c_str(), actionsPointer,
                                         nullptr, argv.data(), environ);
    if (actionsPointer) posix_spawn_file_actions_destroy(&actions);
    if (spawnResult != 0) {
        throw std::runtime_error(std::string("cannot start Python: ") +
                                 std::strerror(spawnResult));
    }

    int status = 0;
    while (::waitpid(process, &status, 0) < 0) {
        if (errno == EINTR) continue;
        throw std::runtime_error(errorText("waitpid"));
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return 128;
}

bool secureRandomBytes(std::size_t size, std::string& output) {
    output.assign(size, '\0');
    const int descriptor = ::open("/dev/urandom", O_RDONLY);
    if (descriptor < 0) return false;
    std::size_t offset = 0;
    while (offset < size) {
        const ssize_t count = ::read(descriptor, output.data() + offset, size - offset);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) {
            ::close(descriptor);
            output.clear();
            return false;
        }
        offset += static_cast<std::size_t>(count);
    }
    ::close(descriptor);
    return true;
}

void clearEnvironmentVariable(const std::string& name) {
    ::unsetenv(name.c_str());
}

bool checkFilePermissions(const std::string& path, unsigned int expectedMode) {
    struct stat fileStat {};
    if (::stat(path.c_str(), &fileStat) != 0) return false;
    return static_cast<unsigned int>(fileStat.st_mode & 0777) == expectedMode;
}

} // namespace folio::platform
