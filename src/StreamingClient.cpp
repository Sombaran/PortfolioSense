#include "StreamingClient.hpp"

#include "HttpClient.hpp"
#include "UpstoxClient.hpp"
#include "PlatformAdapter.hpp"

#include <curl/curl.h>
#include <curl/websockets.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <utility>

namespace folio {
namespace {

using nlohmann::json;
constexpr std::size_t kMaxSubscriptions = 100;
constexpr std::size_t kMaxFeedMessageSize = 1024 * 1024;

class ProtoReader {
public:
    explicit ProtoReader(const std::string& bytes) : bytes_(bytes) {}

    bool next(unsigned& field, unsigned& wire) {
        if (offset_ == bytes_.size()) return false;
        const std::uint64_t key = varint();
        field = static_cast<unsigned>(key >> 3);
        wire = static_cast<unsigned>(key & 7);
        if (field == 0) throw std::runtime_error("invalid protobuf field number");
        return true;
    }

    std::uint64_t varint() {
        std::uint64_t value = 0;
        for (unsigned shift = 0; shift < 64; shift += 7) {
            if (offset_ >= bytes_.size()) throw std::runtime_error("truncated protobuf varint");
            const auto byte = static_cast<unsigned char>(bytes_[offset_++]);
            if (shift == 63 && (byte & 0xfe) != 0) {
                throw std::runtime_error("protobuf varint overflow");
            }
            value |= static_cast<std::uint64_t>(byte & 0x7f) << shift;
            if ((byte & 0x80) == 0) return value;
        }
        throw std::runtime_error("invalid protobuf varint");
    }

    double fixed64() {
        if (bytes_.size() - offset_ < sizeof(std::uint64_t)) {
            throw std::runtime_error("truncated protobuf fixed64");
        }
        std::uint64_t bits = 0;
        for (unsigned index = 0; index < 8; ++index) {
            bits |= static_cast<std::uint64_t>(
                        static_cast<unsigned char>(bytes_[offset_++]))
                    << (index * 8);
        }
        double value = 0;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    std::string bytes() {
        const std::uint64_t length = varint();
        if (length > bytes_.size() - offset_) {
            throw std::runtime_error("truncated protobuf message");
        }
        const auto size = static_cast<std::size_t>(length);
        std::string result = bytes_.substr(offset_, size);
        offset_ += size;
        return result;
    }

    void skip(unsigned wire) {
        switch (wire) {
        case 0:
            (void)varint();
            return;
        case 1:
            advance(8);
            return;
        case 2: {
            const std::uint64_t length = varint();
            if (length > bytes_.size() - offset_) {
                throw std::runtime_error("truncated protobuf field");
            }
            offset_ += static_cast<std::size_t>(length);
            return;
        }
        case 5:
            advance(4);
            return;
        default:
            throw std::runtime_error("unsupported protobuf wire type");
        }
    }

private:
    void advance(std::size_t amount) {
        if (amount > bytes_.size() - offset_) {
            throw std::runtime_error("truncated protobuf field");
        }
        offset_ += amount;
    }

    const std::string& bytes_;
    std::size_t offset_ = 0;
};

void parseLtpc(const std::string& payload, StreamingClient::Quote& quote) {
    ProtoReader reader(payload);
    unsigned field = 0, wire = 0;
    while (reader.next(field, wire)) {
        if ((field == 1 || field == 4) && wire == 1) {
            const double value = reader.fixed64();
            if (field == 1) quote.lastPrice = value;
            else quote.close = value;
        } else if ((field == 2 || field == 3) && wire == 0) {
            const auto value = static_cast<std::int64_t>(reader.varint());
            if (field == 2) quote.timestamp = value;
            else quote.volume = value;
        } else {
            reader.skip(wire);
        }
    }
}

bool parseFeed(const std::string& payload, StreamingClient::Quote& quote);

bool parseMarketFullFeed(const std::string& payload, StreamingClient::Quote& quote) {
    ProtoReader reader(payload);
    unsigned field = 0, wire = 0;
    bool found = false;
    while (reader.next(field, wire)) {
        if (field == 1 && wire == 2) {
            parseLtpc(reader.bytes(), quote);
            found = true;
        } else {
            reader.skip(wire);
        }
    }
    return found;
}

bool parseFullFeed(const std::string& payload, StreamingClient::Quote& quote) {
    ProtoReader reader(payload);
    unsigned field = 0, wire = 0;
    bool found = false;
    while (reader.next(field, wire)) {
        if ((field == 1 || field == 2) && wire == 2) {
            const std::string nested = reader.bytes();
            if (field == 1) found = parseMarketFullFeed(nested, quote) || found;
            else {
                ProtoReader indexReader(nested);
                unsigned indexField = 0, indexWire = 0;
                while (indexReader.next(indexField, indexWire)) {
                    if (indexField == 1 && indexWire == 2) {
                        parseLtpc(indexReader.bytes(), quote);
                        found = true;
                    } else {
                        indexReader.skip(indexWire);
                    }
                }
            }
        } else {
            reader.skip(wire);
        }
    }
    return found;
}

bool parseFeed(const std::string& payload, StreamingClient::Quote& quote) {
    ProtoReader reader(payload);
    unsigned field = 0, wire = 0;
    bool found = false;
    while (reader.next(field, wire)) {
        if (field == 1 && wire == 2) {
            parseLtpc(reader.bytes(), quote);
            found = true;
        } else if (field == 2 && wire == 2) {
            found = parseFullFeed(reader.bytes(), quote) || found;
        } else {
            reader.skip(wire);
        }
    }
    return found;
}

void parseMapEntry(const std::string& payload,
                   std::vector<StreamingClient::Quote>& quotes) {
    ProtoReader reader(payload);
    unsigned field = 0, wire = 0;
    std::string instrumentKey;
    std::string feedPayload;
    while (reader.next(field, wire)) {
        if (field == 1 && wire == 2) instrumentKey = reader.bytes();
        else if (field == 2 && wire == 2) feedPayload = reader.bytes();
        else reader.skip(wire);
    }
    if (instrumentKey.empty() || feedPayload.empty()) return;
    StreamingClient::Quote quote;
    quote.instrumentKey = std::move(instrumentKey);
    if (parseFeed(feedPayload, quote) &&
        std::isfinite(quote.lastPrice) && quote.lastPrice > 0) {
        quotes.push_back(std::move(quote));
    }
}

std::string subscriptionMessage(const std::string& method,
                                const std::vector<std::string>& keys) {
    static std::atomic<std::uint64_t> sequence{0};
    const std::string guid = std::to_string(sequence.fetch_add(1, std::memory_order_relaxed));
    return json({{"guid", guid},
                 {"method", method},
                 {"data", {{"mode", "ltpc"}, {"instrumentKeys", keys}}}}).dump();
}

bool sendFrame(CURL* handle, const std::string& payload, unsigned flags,
               std::string& error) {
    curl_socket_t socket = CURL_SOCKET_BAD;
    if (curl_easy_getinfo(handle, CURLINFO_ACTIVESOCKET, &socket) != CURLE_OK ||
        socket == CURL_SOCKET_BAD) {
        error = "WebSocket send socket is unavailable";
        return false;
    }

    std::size_t offset = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    do {
        std::size_t sent = 0;
        const CURLcode result = curl_ws_send(
            handle, payload.data() + offset, payload.size() - offset,
            &sent, 0, flags);
        offset += sent;
        if (result == CURLE_OK && offset == payload.size()) return true;
        if (result != CURLE_OK && result != CURLE_AGAIN) {
            error = curl_easy_strerror(result);
            return false;
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            error = "WebSocket send timed out";
            return false;
        }

        const int ready = platform::waitForSocket(
            static_cast<platform::SocketHandle>(socket), true, 250);
        if (ready < 0) {
            error = "WebSocket send socket is unavailable";
            return false;
        }
        if (result == CURLE_OK && sent == 0 && ready == 0) {
            continue;
        }
        if (ready == 0) continue;
        if (result == CURLE_OK && sent == 0) {
            error = "WebSocket send made no progress";
            return false;
        }
    } while (offset < payload.size() || payload.empty());

    error = "incomplete WebSocket write";
    return false;
}

bool sendBinary(CURL* handle, const std::string& payload, std::string& error) {
    if (!sendFrame(handle, payload, CURLWS_BINARY, error)) {
        return false;
    }
    return true;
}

template <typename Value>
void setCurlOption(CURL* handle, CURLoption option, Value value) {
    const CURLcode result = curl_easy_setopt(handle, option, value);
    if (result != CURLE_OK) {
        throw std::runtime_error(std::string("market feed transport option failed: ") +
                                 curl_easy_strerror(result));
    }
}

} // namespace

class StreamingClient::Impl {};

StreamingClient::StreamingClient(std::string accessToken, std::string baseUrl)
    : authorize_([token = std::move(accessToken), base = std::move(baseUrl)] {
          const auto result = UpstoxClient(token, base).authorizeMarketDataFeed();
          if (!result.ok) throw std::runtime_error(result.error);
          return result.authorizedRedirectUri;
      }),
      impl_(std::make_unique<Impl>()) {
    CurlGlobal::ensureInit();
}

StreamingClient::StreamingClient(AuthorizationCallback authorize)
    : authorize_(std::move(authorize)), impl_(std::make_unique<Impl>()) {
    CurlGlobal::ensureInit();
}

StreamingClient::~StreamingClient() {
    disconnect();
}

std::string StreamingClient::getAuthorizedWsUrl() const {
    if (!authorize_) return {};
    return authorize_();
}

bool StreamingClient::validInstrumentKey(const std::string& key) {
    if (key.empty() || key.size() > 128) return false;
    const auto separator = key.find('|');
    if (separator == std::string::npos || separator == 0 ||
        separator + 1 == key.size() || key.find('|', separator + 1) != std::string::npos) {
        return false;
    }
    return std::all_of(key.begin(), key.end(), [](unsigned char character) {
        return std::isalnum(character) || character == '_' || character == '|' ||
               character == '&' || character == '-' || character == ' ' || character == '.';
    });
}

bool StreamingClient::supportsWebSocketProtocol() {
    const curl_version_info_data* version = curl_version_info(CURLVERSION_NOW);
    if (!version || !version->protocols) return false;
    for (const char* const* protocol = version->protocols; *protocol; ++protocol) {
        if (std::strcmp(*protocol, "wss") == 0) return true;
    }
    return false;
}

bool StreamingClient::isTrustedWebSocketUrl(const std::string& url) {
    if (url.rfind("wss://", 0) != 0) return false;
    const auto authorityStart = std::string("wss://").size();
    const auto authorityEnd = url.find_first_of("/?#", authorityStart);
    const std::string authority = url.substr(
        authorityStart, authorityEnd == std::string::npos
                            ? std::string::npos
                            : authorityEnd - authorityStart);
    if (authority.empty() || authority.find('@') != std::string::npos) return false;

    const auto colon = authority.find(':');
    const std::string host = authority.substr(0, colon);
    if (colon != std::string::npos && authority.substr(colon + 1) != "443") return false;
    constexpr char suffix[] = ".upstox.com";
    if (host.size() <= sizeof(suffix) - 1 ||
        host.compare(host.size() - (sizeof(suffix) - 1), sizeof(suffix) - 1, suffix) != 0) {
        return false;
    }
    if (url.find('#') != std::string::npos) return false;
    return std::all_of(host.begin(), host.end(), [](unsigned char character) {
        return std::islower(character) || std::isdigit(character) || character == '-' ||
               character == '.';
    });
}

std::vector<StreamingClient::Quote>
StreamingClient::decodeFeedResponse(const std::string& payload) {
    if (payload.empty() || payload.size() > kMaxFeedMessageSize) {
        throw std::runtime_error("invalid market feed message size");
    }
    ProtoReader reader(payload);
    unsigned field = 0, wire = 0;
    std::vector<Quote> quotes;
    while (reader.next(field, wire)) {
        if (field == 2 && wire == 2) parseMapEntry(reader.bytes(), quotes);
        else reader.skip(wire);
    }
    return quotes;
}

bool StreamingClient::connect(const std::vector<std::string>& instrumentKeys) {
    std::vector<std::string> keys;
    keys.reserve(instrumentKeys.size());
    for (const auto& key : instrumentKeys) {
        if (!validInstrumentKey(key)) {
            std::lock_guard<std::mutex> lock(quoteMutex_);
            lastError_ = "invalid market feed instrument key";
            return false;
        }
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) keys.push_back(key);
    }
    if (keys.empty() || keys.size() > kMaxSubscriptions) {
        std::lock_guard<std::mutex> lock(quoteMutex_);
        lastError_ = "market feed requires 1 to 100 unique instrument keys";
        return false;
    }
    if (!supportsWebSocketProtocol()) {
        std::lock_guard<std::mutex> lock(quoteMutex_);
        lastError_ = "installed libcurl lacks WSS support; rebuild with the Conan libcurl WebSocket package";
        return false;
    }
    if (!authorize_) {
        std::lock_guard<std::mutex> lock(quoteMutex_);
        lastError_ = "market feed authorization is not configured";
        return false;
    }

    std::lock_guard<std::mutex> workerLock(workerMutex_);
    if (worker_.joinable()) {
        if (workerRunning_.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(quoteMutex_);
            std::vector<std::string> added, removed;
            for (const auto& key : instrumentKeys_) {
                if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
                    removed.push_back(key);
                    quoteCache_.erase(key);
                    depthCache_.erase(key);
                }
            }
            for (const auto& key : keys) {
                if (std::find(instrumentKeys_.begin(), instrumentKeys_.end(), key) ==
                    instrumentKeys_.end()) {
                    added.push_back(key);
                }
            }
            instrumentKeys_ = std::move(keys);
            if (!removed.empty()) {
                pendingMessages_.push_back(subscriptionMessage("unsub", removed));
            }
            if (!added.empty()) {
                pendingMessages_.push_back(subscriptionMessage("sub", added));
            }
            return true;
        }
        worker_.join();
    }
    {
        std::lock_guard<std::mutex> lock(quoteMutex_);
        instrumentKeys_ = std::move(keys);
        pendingMessages_.clear();
        quoteCache_.clear();
        depthCache_.clear();
        lastError_.clear();
    }
    stopping_.store(false, std::memory_order_release);
    workerRunning_.store(true, std::memory_order_release);
    try {
        worker_ = std::thread(&StreamingClient::run, this);
    } catch (const std::exception& error) {
        workerRunning_.store(false, std::memory_order_release);
        std::lock_guard<std::mutex> lock(quoteMutex_);
        lastError_ = std::string("cannot start market feed worker: ") + error.what();
        return false;
    }
    return true;
}

void StreamingClient::disconnect() {
    std::lock_guard<std::mutex> workerLock(workerMutex_);
    stopping_.store(true, std::memory_order_release);
    if (worker_.joinable()) worker_.join();
    connected_.store(false, std::memory_order_release);
    workerRunning_.store(false, std::memory_order_release);
}

bool StreamingClient::isConnected() const {
    return connected_.load(std::memory_order_acquire);
}

std::string StreamingClient::lastError() const {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    return lastError_;
}

std::vector<StreamingClient::Quote> StreamingClient::latestQuotes() const {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    std::vector<Quote> quotes;
    quotes.reserve(quoteCache_.size());
    for (const auto& entry : quoteCache_) quotes.push_back(entry.second);
    return quotes;
}

void StreamingClient::subscribe(const std::vector<std::string>& instrumentKeys) {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    std::vector<std::string> added;
    for (const auto& key : instrumentKeys) {
        if (!validInstrumentKey(key)) {
            lastError_ = "invalid market feed instrument key";
            return;
        }
        if (std::find(instrumentKeys_.begin(), instrumentKeys_.end(), key) !=
            instrumentKeys_.end()) continue;
        if (instrumentKeys_.size() == kMaxSubscriptions) {
            lastError_ = "market feed subscription limit is 100 instruments";
            break;
        }
        instrumentKeys_.push_back(key);
        added.push_back(key);
    }
    if (!added.empty()) pendingMessages_.push_back(subscriptionMessage("sub", added));
}

void StreamingClient::unsubscribe(const std::vector<std::string>& instrumentKeys) {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    std::vector<std::string> removed;
    for (const auto& key : instrumentKeys) {
        if (!validInstrumentKey(key)) continue;
        const auto found = std::find(instrumentKeys_.begin(), instrumentKeys_.end(), key);
        if (found == instrumentKeys_.end()) continue;
        instrumentKeys_.erase(found);
        quoteCache_.erase(key);
        depthCache_.erase(key);
        removed.push_back(key);
    }
    if (!removed.empty()) pendingMessages_.push_back(subscriptionMessage("unsub", removed));
}

StreamingClient::Quote StreamingClient::getQuote(const std::string& instrumentKey) const {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    const auto found = quoteCache_.find(instrumentKey);
    return found == quoteCache_.end() ? Quote{} : found->second;
}

StreamingClient::DepthData StreamingClient::getDepth(const std::string& instrumentKey) const {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    const auto found = depthCache_.find(instrumentKey);
    return found == depthCache_.end() ? DepthData{} : found->second;
}

void StreamingClient::setQuoteCallback(QuoteCallback callback) {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    quoteCallback_ = std::move(callback);
}

void StreamingClient::setDepthCallback(DepthCallback callback) {
    std::lock_guard<std::mutex> lock(quoteMutex_);
    depthCallback_ = std::move(callback);
}

void StreamingClient::setAutoReconnect(bool enable, long delayMs) {
    autoReconnect_.store(enable, std::memory_order_release);
    reconnectDelayMs_.store(std::max(250L, delayMs), std::memory_order_release);
}

void StreamingClient::run() {
    while (!stopping_.load(std::memory_order_acquire)) {
        CURL* handle = curl_easy_init();
        if (!handle) {
            std::lock_guard<std::mutex> lock(quoteMutex_);
            lastError_ = "cannot initialize market feed transport";
        } else {
            try {
                const std::string url = authorize_();
                if (!isTrustedWebSocketUrl(url)) {
                    throw std::runtime_error("untrusted Upstox WebSocket URL");
                }
                setCurlOption(handle, CURLOPT_URL, url.c_str());
                setCurlOption(handle, CURLOPT_PROTOCOLS_STR, "wss");
                setCurlOption(handle, CURLOPT_FOLLOWLOCATION, 0L);
                setCurlOption(handle, CURLOPT_CONNECT_ONLY, 2L);
                setCurlOption(handle, CURLOPT_CONNECTTIMEOUT, 15L);
                setCurlOption(handle, CURLOPT_TIMEOUT, 30L);
                setCurlOption(handle, CURLOPT_SSL_VERIFYPEER, 1L);
                setCurlOption(handle, CURLOPT_SSL_VERIFYHOST, 2L);
                setCurlOption(handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
                setCurlOption(handle, CURLOPT_TCP_KEEPALIVE, 1L);
                const CURLcode connected = curl_easy_perform(handle);
                if (connected != CURLE_OK) {
                    throw std::runtime_error(
                        std::string("market feed WebSocket connection failed: ") +
                        curl_easy_strerror(connected));
                }

                std::vector<std::string> keys;
                {
                    std::lock_guard<std::mutex> lock(quoteMutex_);
                    keys = instrumentKeys_;
                    pendingMessages_.clear();
                }
                std::string sendError;
                if (!sendBinary(handle, subscriptionMessage("sub", keys), sendError)) {
                    throw std::runtime_error("market feed subscription failed: " + sendError);
                }
                {
                    std::lock_guard<std::mutex> lock(quoteMutex_);
                    lastError_.clear();
                }
                connected_.store(true, std::memory_order_release);

                curl_socket_t socket = CURL_SOCKET_BAD;
                if (curl_easy_getinfo(handle, CURLINFO_ACTIVESOCKET, &socket) != CURLE_OK ||
                    socket == CURL_SOCKET_BAD) {
                    throw std::runtime_error("market feed WebSocket socket is unavailable");
                }
                std::string message;
                bool streamEnded = false;
                std::array<char, 16384> buffer{};
                while (!stopping_.load(std::memory_order_acquire) && !streamEnded) {
                    std::vector<std::string> pending;
                    {
                        std::lock_guard<std::mutex> lock(quoteMutex_);
                        pending.swap(pendingMessages_);
                    }
                    for (const auto& payload : pending) {
                        if (!sendBinary(handle, payload, sendError)) {
                            throw std::runtime_error("market feed subscription failed: " + sendError);
                        }
                    }

                    const int ready = platform::waitForSocket(
                        static_cast<platform::SocketHandle>(socket), false, 1000);
                    if (ready < 0) {
                        throw std::runtime_error("market feed socket polling failed");
                    }
                    if (ready == 0) continue;

                    for (;;) {
                        std::size_t received = 0;
                        const curl_ws_frame* frame = nullptr;
                        const CURLcode result = curl_ws_recv(
                            handle, buffer.data(), buffer.size(), &received, &frame);
                        if (result == CURLE_AGAIN) break;
                        if (result != CURLE_OK || frame == nullptr) {
                            throw std::runtime_error("market feed WebSocket receive failed");
                        }
                        if ((frame->flags & CURLWS_PING) != 0) {
                            if (!sendFrame(handle, std::string(buffer.data(), received),
                                           CURLWS_PONG, sendError)) {
                                throw std::runtime_error(
                                    "market feed heartbeat response failed: " + sendError);
                            }
                            continue;
                        }
                        if ((frame->flags & CURLWS_CLOSE) != 0) {
                            streamEnded = true;
                            break;
                        }
                        if ((frame->flags & CURLWS_TEXT) != 0) {
                            throw std::runtime_error("unexpected text frame from market feed");
                        }
                        if (received > kMaxFeedMessageSize - message.size()) {
                            throw std::runtime_error("market feed message exceeds size limit");
                        }
                        message.append(buffer.data(), received);
                        if (frame->bytesleft == 0) {
                            const auto quotes = decodeFeedResponse(message);
                            message.clear();
                            for (const auto& quote : quotes) onQuoteUpdate(quote);
                        }
                    }
                }
            } catch (const std::exception& error) {
                std::lock_guard<std::mutex> lock(quoteMutex_);
                lastError_ = error.what();
            }
            connected_.store(false, std::memory_order_release);
            curl_easy_cleanup(handle);
        }
        if (!autoReconnect_.load(std::memory_order_acquire) ||
            stopping_.load(std::memory_order_acquire)) break;
        const long delay = reconnectDelayMs_.load(std::memory_order_acquire);
        for (long waited = 0; waited < delay &&
             !stopping_.load(std::memory_order_acquire); waited += 100) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                std::min(100L, delay - waited)));
        }
    }
    connected_.store(false, std::memory_order_release);
    workerRunning_.store(false, std::memory_order_release);
}

void StreamingClient::onQuoteUpdate(const Quote& quote) {
    QuoteCallback callback;
    {
        std::lock_guard<std::mutex> lock(quoteMutex_);
        if (std::find(instrumentKeys_.begin(), instrumentKeys_.end(),
                      quote.instrumentKey) == instrumentKeys_.end()) {
            return;
        }
        quoteCache_[quote.instrumentKey] = quote;
        callback = quoteCallback_;
    }
    if (callback) callback(quote);
}

void StreamingClient::onDepthUpdate(const std::string& instrumentKey,
                                   const DepthData& depth) {
    DepthCallback callback;
    {
        std::lock_guard<std::mutex> lock(quoteMutex_);
        depthCache_[instrumentKey] = depth;
        callback = depthCallback_;
    }
    if (callback) callback(instrumentKey, depth);
}

} // namespace folio
