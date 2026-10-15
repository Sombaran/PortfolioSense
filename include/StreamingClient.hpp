#ifndef FOLIO_STREAMING_CLIENT_HPP
#define FOLIO_STREAMING_CLIENT_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <atomic>
#include <thread>

namespace folio {

/**
 * @brief Real-time streaming client for Upstox Market Data Feed V3 (WebSocket).
 * 
 * Provides live quote streaming for subscribed instruments through the
 * authenticated Upstox V3 feed. A thread-safe cache serves latest prices.
 * Handles automatic reconnection on network failure.
 * 
 * v2.0.56+: Requires a libcurl build with WSS protocol support, verified
 * server-side WebSocket transport and bounded V3 feed
 * decoding; the web server exposes only its local quote cache to the browser.
 */
class StreamingClient {
public:
    using AuthorizationCallback = std::function<std::string()>;

    struct Quote {
        std::string instrumentKey;
        double lastPrice = 0.0;
        double bidPrice = 0.0;
        double bidQty = 0.0;
        double askPrice = 0.0;
        double askQty = 0.0;
        double open = 0.0;
        double high = 0.0;
        double low = 0.0;
        double close = 0.0;
        long volume = 0;
        long oiValue = 0;
        long timestamp = 0;
    };

    struct DepthData {
        std::vector<std::pair<double, long>> bids;  // (price, qty) pairs
        std::vector<std::pair<double, long>> asks;  // (price, qty) pairs
    };

    explicit StreamingClient(std::string accessToken,
                             std::string baseUrl = "https://api.upstox.com");
    explicit StreamingClient(AuthorizationCallback authorize);
    
    ~StreamingClient();

    /**
     * @brief Get authorization URL for WebSocket feed.
     * @return Authorized WebSocket URL or error string.
     */
    std::string getAuthorizedWsUrl() const;

    /**
     * @brief Connect to WebSocket and start streaming.
     * @param instrumentKeys Symbols to subscribe to.
     * Starts the feed worker for a valid subscription. Connection is
     * asynchronous; use isConnected() to observe the live connection state.
     */
    bool connect(const std::vector<std::string>& instrumentKeys);

    /**
     * @brief Disconnect from WebSocket stream.
     */
    void disconnect();

    /**
     * @brief Check if currently connected.
     */
    bool isConnected() const;
    std::string lastError() const;
    std::vector<Quote> latestQuotes() const;

    static bool validInstrumentKey(const std::string& instrumentKey);
    static bool supportsWebSocketProtocol();
    static bool isTrustedWebSocketUrl(const std::string& url);
    static std::vector<Quote> decodeFeedResponse(const std::string& payload);

    /**
     * @brief Subscribe to additional instruments.
     * @param instrumentKeys Symbols to add to subscription.
     */
    void subscribe(const std::vector<std::string>& instrumentKeys);

    /**
     * @brief Unsubscribe from instruments.
     * @param instrumentKeys Symbols to remove from subscription.
     */
    void unsubscribe(const std::vector<std::string>& instrumentKeys);

    /**
     * @brief Get latest quote for an instrument.
     * @param instrumentKey Symbol identifier.
     * @return Quote struct if available, empty Quote if not.
     */
    Quote getQuote(const std::string& instrumentKey) const;

    /**
     * @brief Get depth data for an instrument.
     * @param instrumentKey Symbol identifier.
     * @return Depth with bid/ask levels, empty if not available.
     */
    DepthData getDepth(const std::string& instrumentKey) const;

    /**
     * @brief Set callback for quote updates.
     * Called whenever a new quote arrives from the stream.
     */
    using QuoteCallback = std::function<void(const Quote&)>;
    void setQuoteCallback(QuoteCallback callback);

    /**
     * @brief Set callback for depth updates.
     */
    using DepthCallback = std::function<void(const std::string&, const DepthData&)>;
    void setDepthCallback(DepthCallback callback);

    /**
     * @brief Enable/disable automatic reconnection.
     * @param enable true to auto-reconnect on disconnect.
     * @param delayMs delay between reconnection attempts.
     */
    void setAutoReconnect(bool enable, long delayMs = 5000);

private:
    AuthorizationCallback authorize_;

    mutable std::mutex quoteMutex_;
    std::map<std::string, Quote> quoteCache_;
    std::map<std::string, DepthData> depthCache_;
    std::string lastError_;
    std::vector<std::string> instrumentKeys_;
    std::vector<std::string> pendingMessages_;
    std::atomic<bool> connected_{false};
    std::atomic<bool> stopping_{false};
    std::atomic<bool> workerRunning_{false};
    std::atomic<bool> autoReconnect_{true};
    std::atomic<long> reconnectDelayMs_{5000};
    std::thread worker_;
    mutable std::mutex workerMutex_;

    QuoteCallback quoteCallback_;
    DepthCallback depthCallback_;

    // WebSocket handling (implementation-specific)
    class Impl;
    std::unique_ptr<Impl> impl_;

    void run();
    void onQuoteUpdate(const Quote& quote);
    void onDepthUpdate(const std::string& instrumentKey, const DepthData& depth);
};

}  // namespace folio

#endif  // FOLIO_STREAMING_CLIENT_HPP
