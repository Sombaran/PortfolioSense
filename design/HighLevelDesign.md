# High-Level Design — Portfolio Health Web Service

## 1. Purpose

Give an Indian retail trader an at-a-glance answer to a single question —
**"Is my portfolio healthy right now?"** — through a local C++ web dashboard
or the terminal interface. The current application is read-only and advisory;
it does not submit broker orders or automate trade execution.

The service pulls positions and long-term holdings from Upstox (both via REST 
and real-time WebSocket streaming), fetches news for the account holdings, 
filters it to symbols in `config/holding.csv`, and exposes analytics, advisory 
sentiment signals, technical indicators, and Prometheus metrics. ML inference is
unavailable until a real model backend is supplied. The Alerts view uses long-term holdings as its authoritative row set,
so a holding without a matching article is still represented with a neutral 
review state.

### v2.0.4 Security hardening and release transparency

**Stock API hardening** — The live Upstox integration now validates the host,
rejects non-HTTPS requests, blocks unsafe redirects, and keeps outbound traffic
inside the trusted Upstox domains. This reduces credential leakage and prevents
malicious host or proxy paths from being used as a route into the trading API.

**Versioned release patching** — The application version follows the semver
pattern (`x.x.x`), and the right-side release popup summarizes the fix set in
plain language so operators can see what changed without opening the docs. The
project is tagged as `2.0.4` for this security and release-tracking patch.

### v2.0.3 Runtime reliability, async scheduling, and browser UX hardening

The 2.0.3 patch makes the backend Python workflow portable across working
directories by resolving the project-local script path and running it with
absolute file references, so the Deeper analysis CLI remains reliable even when
launched outside the repository root. The application now also includes a safe
local-first internal task scheduler and async worker queue for back-end refresh
and analysis work, which reduces UI blocking without introducing a distributed
attack surface around the stock API. The web UI now treats empty or stale news
payloads as recoverable states instead of failing the page, the fundamentals
popup layout is tightened for consistent viewport alignment, and expired or
unauthorized API responses are surfaced as secure redirect or retry states
instead of exposing raw HTTP failures.

### v2.0.2 Runtime TLS consistency (historical)

In v2.0.2 the build let system libcurl own its verified HTTPS stack to avoid
mixing Conan OpenSSL headers with a different system OpenSSL ABI. Since v2.0.56,
the standard CMake build uses the pinned Conan libcurl/OpenSSL dependency set,
including WebSocket protocol support; authenticated API redirects remain
disabled so bearer credentials cannot be forwarded outside approved Upstox
hosts.

### v2.0.0 Enhancements

**Real-time Streaming** — The web server connects to Upstox Market Data Feed V3
for authenticated holdings, decodes live quote updates, and serves cached
prices to the authenticated dashboard through a same-origin JSON endpoint.
Broker credentials and the authorized WebSocket URL remain server-side.

**Technical Indicators** — Expanded indicator suite includes RSI, MACD, 
Bollinger Bands, Stochastic Oscillator, and ATR, all computed in C++ for 
low-latency analysis. Indicators can inform advisory signal generation; no
model inference is active.

**Machine Learning Models** — The current `MLModels` implementation does not
load a model or produce predictions; its inference methods fail closed until a
real backend is integrated.

**Signal Generation Engine** — Local technical indicators and news-derived
signals are advisory. The UI does not claim a trained prediction model or
authorize trades.

**Mobile Notifications & Alerts** — Browser/dashboard visibility is available.
Webhook, Firebase, email, and SMS delivery are not configured integrations and
return explicit undelivered status rather than success.

**Orders API Integration** — Live submission, cancellation, lookup, and
two-factor authorization are not implemented. Order requests are rejected
explicitly; no broker order is sent. Simulation fails closed when required
portfolio or sector exposure data is not available.
The Alerts page renders only text and table content; article thumbnails or other
attached images are intentionally excluded from that view.
The raw instrument-key Stock column is also excluded from the Alerts table; the
serial number and company name identify each holding instead.
The service exports review-action gauges for Prometheus. Prometheus rules can
forward threshold crossings to Alertmanager or Grafana notification channels;
these triggers are advisory and do not place broker orders.
This integration is exposed through the web service's `/metrics` route; the
browser Alerts tab does not directly fire Prometheus or Grafana notifications.
The dashboard also provides a read-only Data health view and browser-local
acknowledgement state for Alerts, without adding write calls to Upstox.
The RSI tool provides guided chronological input, sample values, and readable
interpretation feedback while continuing to use the local calculation endpoint.
Deeper analysis distinguishes missing news (`No recent news`) from genuine
neutral news (`Neutral news`) so a data gap is never presented as a hold signal.
It iterates the live holdings universe, so all 51 holdings remain visible, and
identifies saved portfolio news as a fallback when fresh Python news is absent.
The Web UI displays the CMake project version, and News presents articles in
descending publication-time order.
The CLI exposes the same build identifier through `--version`, and Web startup
logs include the version so the running binary is unambiguous.
Credentials are supplied through the environment or an external secret
manager; repository scripts and JSON configuration contain no live secrets.
The client restricts authenticated traffic to approved Upstox HTTPS hosts and
the web service remains bound to loopback.
The baran-capital-view Python analyzer uses configured API providers first and falls back to
Google News RSS when providers are rate-limited, rejected, or unavailable;
all provider queries include the configured market query suffix.
Its implementation and dependencies are stored under baran-capital-view, so Deeper
analysis has no runtime dependency on the sibling `market` directory.
Deeper analysis refreshes its live comparison at most every five minutes,
preventing an open-session result from being reused after the market closes.
An explicit post-session CLI job atomically updates the report-shaped
`config/holding.csv` from the latest Upstox holdings.
News filtering normalizes whitespace and case for both the portfolio CSV and
live holding symbols, ensuring that only Sombaran Portfolio holdings are shown.
After authentication, the dashboard shows a concise, dismissible release popup
once per version; its seen marker is kept in browser local storage.
The Overview dashboard displays a manual Refresh action alongside its summary
metrics and hides the implementation refresh interval from users while
allowing holdings to render when optional news or positions data is unavailable,
with the partial state surfaced in the status line.
Dashboard DOM observers update metric content only when values change, avoiding
browser main-thread loops that can block tab rendering.
Market Value uses the broker-reported holding value when available and shows
two decimal places for INR precision.
For live mode, the server derives each holding's `current_value` from the
authenticated Upstox holding and sends that normalized value to the browser.
The same live total is exposed to Deeper Analysis so its values stay aligned
with the portfolio snapshot.
Deeper Analysis groups recommendations into category counts and lets the user
select a category to inspect its stock names. News is limited to portfolio
instrument keys and articles published in the previous 30 days.
Fallback snapshots apply the same filtering boundary and never expose the raw
saved news export.
Holdings responses identify whether data came from `upstox-live` or the local
fallback, and API responses are marked non-cacheable.
retaining cached synchronization to reduce unnecessary API traffic. If an
overnight token expiry or transient Upstox news failure prevents refresh, the
server serves the filtered saved-news export until live news is available
again.

Non-goals:

* Historical / EOD analytics — Upstox news is limited to its recent-news window.
* Full risk model. The health score is a heuristic, deliberately simple.
* Direct position modification or live order placement; the application is
  read-only and advisory.

---

## 2. Component view (v2.0.0)

```
             ┌─────────────────────────────────────────┐
             │                CLI (main)               │
             │  argv → Options { file, raw, json, … }  │
             └────────────┬──────────────┬─────────────┘
                          │              │
        offline (--file)  │              │ live (default)
                          ▼              ▼
                ┌───────────────┐  ┌───────────────────┐
                │ file reader   │  │  UpstoxClient     │
                │ + JSON parse  │  │  (Bearer auth)    │
                └───────┬───────┘  │  + StreamingClient│
                        │          │  (WebSocket auth) │
                        │          └─────────┬─────────┘
                        │                    │
                        ▼                    ▼
                ┌───────────────────────────────────────┐
                │           std::vector<Position>       │
                │         + Real-time quotes            │
                └────────────────────┬──────────────────┘
                                     ▼
                     ┌──────────────────────────────────┐
                     │    TechnicalIndicators::         │
                     │  Calculate(RSI, MACD, BB, etc)   │
                     └───────────────┬──────────────────┘
                                     ▼
                     ┌──────────────────────────────────┐
                     │       MLModels                   │
                     │   (inference unavailable)        │
                     └───────────────┬──────────────────┘
                                     ▼
                     ┌──────────────────────────────────┐
                     │   SignalGenerator::Generate()    │
                     │  (Consensus Buy/Sell/Hold)       │
                     └───────────────┬──────────────────┘
                                     ├─────────────────┐
                                     ▼                 ▼
                    ┌──────────────────┐   ┌─────────────────────┐
                    │ PortfolioHealth::│   │ NotificationService │
                    │   analyze()      │   │   ::Send()          │
                    └────────┬─────────┘   └──────────┬──────────┘
                             │                        │ WebSocket/HTTP
                             ▼                        ▼
                    ┌──────────────────┐   Mobile/Desktop Clients
                    │  OrdersAPI::     │
                    │  Execute()       │   (if signal >= threshold)
                    │ Upstox Orders    │
                    └─────────┬────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │   WebServer      │
                    │  JSON routes     │
                    │ Prometheus /     │
                    │  /metrics        │
                    └──────────────────┘
```

All modules use thread-safe design patterns and async I/O where applicable.
Real-time streaming starts when the web server receives a live Upstox holdings
snapshot and is bounded to 100 subscribed instruments.
Order execution requires explicit user authorization with configurable thresholds.

The current deployment also starts `WebServer` with `--web`. It serves the
embedded dashboard and JSON routes on `127.0.0.1:8080`; `/metrics` exposes
portfolio gauges for Prometheus and Grafana. The browser talks only to this
local server, so the Upstox bearer token remains server-side.

The Deeper analysis tab is intentionally on demand. Its authenticated API
route reads `config/portfolio_news.json`, invokes `stock_alert_nlp.py`
with `config/holding.csv`, and combines the Python recommendation with the
saved news signal by mapping Upstox instrument keys through live holdings.

## 3. End-to-end flow (v2.0.0)

1. **Entry point** — `--web` starts `WebServer`; no arguments starts the
  terminal menu, while explicit flags remain available for automation.
  
2. **Payload acquisition** — either
   * `--file` → read local JSON, or
   * live → `UpstoxClient` performs authenticated GETs via `HttpClient` and 
     `StreamingClient` establishes a verified WebSocket for live quote updates
     when a live Upstox holdings snapshot is available.
   
3. **Deserialisation** — the JSON `data` array is mapped 1:1 into
   `std::vector<Position>`. For live Upstox holdings, `StreamingClient`
   separately updates the quote cache, which the Overview reads through the
   authenticated local quote endpoint (see [LowLevelDesign](LowLevelDesign.md)).
   
4. **Technical Analysis** — `TechnicalIndicators` module computes:
   * RSI (Relative Strength Index) for momentum
   * MACD for trend detection
   * Bollinger Bands for volatility
   * Stochastic Oscillator for overbought/oversold
   * ATR for volatility measurement
   
5. **Machine Learning** — `MLModels` currently reports inference unavailable;
   it does not fabricate price, trend, or volatility predictions.
   
6. **Signal Generation** — `SignalGenerator` combines:
   * Technical indicator consensus
   * News sentiment analysis
   * Portfolio risk metrics
   → Produces advisory review signals and confidence scores
   
7. **Risk Scoring** — Review metrics include:
   * Position concentration risk
   * Sector concentration risk
   * Volatility risk
   * Correlation-based diversification risk
   → No live order execution guard is currently implemented
   
8. **News processing** — live holdings are matched to `config/holding.csv`; each
  article receives a keyword sentiment score and advisory signal.
  
9. **Notifications** — the dashboard provides review visibility. External
   webhook, mobile, email, and SMS delivery adapters are unavailable.
   
10. **Order Operations** — `OrdersAPI` currently rejects submission and
    reports live order lookup/cancellation/2FA unavailable. Simulation fails
    closed when required portfolio-risk inputs are unknown.
    
11. **Rendering** — terminal reports, JSON, browser tabs, and Prometheus
  metrics are generated from the live responses with signal overlays.
  
12. **Deeper analysis** — only when selected, the server runs the Python news
  analyzer and compares its result with the saved portfolio news.
  
13. **Fundamentals** — the user selects a holding and the server retrieves its
  balance sheet, income statement, cash flow, and corporate actions by ISIN.
  
14. **End-of-day update** — a scheduled `--update-holding` command fetches
  long-term holdings and atomically replaces the latest broker values in the
  configured holdings CSV.

## 4. Cross-cutting concerns

| Concern         | Approach                                                            |
| --------------- | ------------------------------------------------------------------- |
| Auth            | Upstox credentials exported by `run.sh` and read from environment; never sent to the browser. |
| Secrets         | Token is read from env only. Not echoed. `--raw` prints response, never the request. |
| Network errors  | Surfaced as `PositionsResult::error` — non-zero exit, no stack trace. |
| JSON drift      | Numeric parsers tolerate string-encoded numbers; `trading_symbol` and `tradingsymbol` are both accepted. |
| Offline testing | `--file` mode; ships `config/sample_positions.json`.                 |
| Portability     | POSIX + libcurl. Build tested on Linux; CMake also targets MSVC / MinGW paths. |
| Currency        | INR fixed-width formatting is done in-tree — no locale dependency.   |

## 5. Deployment shape

Single C++ binary (`portfolio_health`) with a localhost-only web server. It
uses `UPSTOX_ACCESS_TOKEN` for authentication and writes only
the explicit `config/portfolio_news.json` export when `--news` is used.

## 6. Extension points

* **New endpoint** — add a method to `UpstoxClient` and expose a route in
  `WebServer`.
* **Real-time streaming** — `StreamingClient` establishes WebSocket connections
  using the market data authorization from Upstox API.
* **Technical indicators** — expand `TechnicalIndicators` module with new
  indicators; integrate with signal generation.
* **Machine learning** — replace or augment ML models in `MLModels`; train on
  your own historical data or integrate external prediction services.
* **Signal customization** — modify `SignalGenerator` thresholds, weights, and
  consensus logic to match your risk tolerance.
* **Order execution** — implement and independently validate the broker order
  lifecycle before enabling any live execution; it is not available today.
* **Notification channels** — implement authenticated delivery adapters;
  webhook, Firebase, email, and SMS delivery are not available today.
* **Richer sentiment** — replace the keyword scorer with ONNX Runtime or a
  separate model service while keeping signals advisory.
* **Monitoring** — add metrics to `WebServer` for new Grafana panels.
* **Alternate broker** — implement a sibling client that yields the
  common `Position` struct; `analyze()` stays untouched.
* **Different score** — `PortfolioHealth::analyze` is the only place the
  scoring formula lives. See [PortfolioHealthModel](PortfolioHealthModel.md).
