# PortfolioSense

Version: 2.0.78

PortfolioSense is a C++17 portfolio analysis and monitoring application for live and saved market data. It blends portfolio health scoring, fundamental analysis, C++/Python analytics, and browser-based reporting while keeping stock API access constrained and secure.

## Highlights

- Portfolio health scoring for holdings, concentration, and diversification
- Live Upstox integration with hardened HTTPS and trusted-host validation
- Server-side Upstox Market Data Feed V3 WebSocket with live Overview quotes
- Upstox market-holiday and special-session awareness in the IST market status
- Python-backed Deeper analysis for recent company/news context and recommendations
- Browser UI for overview, news, alerts, analysis, fundamentals, and an Operations workspace
- Native platform adapters for Windows, Linux, and macOS runtime operations
- One Market news tab for the portfolio-matched Upstox news feed
- CSV download of the authenticated Som Baran Portfolio holdings from Operations
- Release-notes popup summarizing the current code-change version and fixes
- **[v2.0.78 release notes](design/RELEASE_NOTES_V2.0.78.md)** — rename the project, align release metadata, and streamline the Windows launcher
- **[v2.0.78 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.78.md)** — preserve the existing Stock API boundary and add the post-login release note
- **[v2.0.77 release notes](design/RELEASE_NOTES_V2.0.77.md)** — display the correct Bazel version and keep market-holiday tooltip text readable
- **[v2.0.77 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.77.md)** — fix Bazel version metadata and bound tooltip text to its dark background
- **[v2.0.76 release notes](design/RELEASE_NOTES_V2.0.76.md)** — ignore local credential variants and Windows IDE/build state
- **[v2.0.76 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.76.md)** — reduce accidental secret and generated-file staging risk
- **[v2.0.75 release notes](design/RELEASE_NOTES_V2.0.75.md)** — fix Windows Bazel output locking and C++ warnings
- **[v2.0.75 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.75.md)** — diagnose the running executable and remove unreachable code
- **[v2.0.74 release notes](design/RELEASE_NOTES_V2.0.74.md)** — enable WSS for Windows Bazel live-market feeds
- **[v2.0.74 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.74.md)** — configure vcpkg libcurl WebSocket support for Windows
- **[v2.0.73 release notes](design/RELEASE_NOTES_V2.0.73.md)** — load Windows web credentials safely before starting the server
- **[v2.0.73 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.73.md)** — root cause of the Windows localhost connection refusal
- **[v2.0.72 release notes](design/RELEASE_NOTES_V2.0.72.md)** — open the Windows web UI after the server is ready
- **[v2.0.72 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.72.md)** — readiness-checked localhost launch with private logs
- **[v2.0.71 release notes](design/RELEASE_NOTES_V2.0.71.md)** — harden Windows build/launcher behavior and scope Python tests to project tests
- **[v2.0.71 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.71.md)** — fix stale Windows builds, guard rebuild paths, and preserve read-only Stock API access
- **[v2.0.70 release notes](design/RELEASE_NOTES_V2.0.70.md)** — add cross-platform adapters and launch Python analysis without a shell
- **[v2.0.70 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.70.md)** — isolate OS-specific runtime operations and preserve the authenticated, read-only Stock API boundary
- **[v2.0.69 release notes](design/RELEASE_NOTES_V2.0.69.md)** — explicit holiday API failure statuses, stale-calendar visibility, and accessible dashboard loading state
- **[v2.0.69 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.69.md)** — closed the remaining partial HTTP/UI feedback gaps
- **[v2.0.68 release notes](design/RELEASE_NOTES_V2.0.68.md)** — fix scheduler races, correct HTTP statuses, harden login sessions, and disable success-shaped stubs
- **[v2.0.68 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.68.md)** — fail-closed safety for incomplete order, notification, and ML integrations
- **[v2.0.67 release notes](design/RELEASE_NOTES_V2.0.67.md)** — remove the duplicate Overview Refresh card and make privacy eye controls reliable
- **[v2.0.67 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.67.md)** — delegated metric visibility events and live value masking
- **[v2.0.66 release notes](design/RELEASE_NOTES_V2.0.66.md)** — display actionable Upstox authorization and network causes when Refresh fails
- **[v2.0.66 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.66.md)** — retain existing data and expose safe refresh diagnostics
- **[v2.0.65 release notes](design/RELEASE_NOTES_V2.0.65.md)** — prevent manual Refresh from failing when it overlaps an initial portfolio snapshot
- **[v2.0.65 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.65.md)** — wait for and reuse an in-flight snapshot during concurrent manual refresh
- **[v2.0.64 release notes](design/RELEASE_NOTES_V2.0.64.md)** — retry transient holdings transport failures and provide adequate refresh request time
- **[v2.0.64 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.64.md)** — bounded upstream retry and browser/backend timeout alignment
- **[v2.0.63 release notes](design/RELEASE_NOTES_V2.0.63.md)** — add Day P&L visibility and fix Overview refresh control overlap
- **[v2.0.63 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.63.md)** — synchronize privacy toggles with live quote updates and improve metric layout
- **[v2.0.62 release notes](design/RELEASE_NOTES_V2.0.62.md)** — remove duplicate Refresh wording and add Market value visibility control
- **[v2.0.62 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.62.md)** — accessible value masking, preference persistence, and regression coverage
- **[v2.0.61 release notes](design/RELEASE_NOTES_V2.0.61.md)** — bound server-side snapshot calls so manual refresh recovers reliably
- **[v2.0.61 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.61.md)** — align backend request budgets with dashboard timeouts
- **[v2.0.60 release notes](design/RELEASE_NOTES_V2.0.60.md)** — make Retry now reliable and queue manual refresh during automatic loading
- **[v2.0.60 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.60.md)** — direct retry handlers and single-flight refresh recovery
- **[v2.0.59 release notes](design/RELEASE_NOTES_V2.0.59.md)** — restore manual refresh after stalled dashboard API responses
- **[v2.0.59 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.59.md)** — bounded requests, reliable refresh controls, and regression coverage
- **[v2.0.58 release notes](design/RELEASE_NOTES_V2.0.58.md)** — fix Alerts mutation loop and reduce dashboard observer workload
- **[v2.0.58 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.58.md)** — root cause, observer scope changes, and regression coverage
- **[v2.0.57 release notes](design/RELEASE_NOTES_V2.0.57.md)** — live refresh crash prevention and dashboard rendering optimization
- **[v2.0.57 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.57.md)** — bounded quote polling, rendering performance, and regression coverage
- **[v2.0.56 release notes](design/RELEASE_NOTES_V2.0.56.md)** — root cause and fix for unsupported WSS protocol
- **[v2.0.56 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.56.md)** — WebSocket-enabled libcurl and live-feed validation
- **[v2.0.55 release notes](design/RELEASE_NOTES_V2.0.55.md)** — live-feed send reliability and actionable connection status
- **[v2.0.55 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.55.md)** — market feed diagnostics, UI updates, and validation
- **[v2.0.54 release notes](design/RELEASE_NOTES_V2.0.54.md)** — Upstox V3 WebSocket feed and live Overview quotes
- **[v2.0.54 implementation summary](design/IMPLEMENTATION_SUMMARY_V2.0.54.md)** — streaming architecture, security boundaries, and validation
- **[v2.0.53 release notes](design/RELEASE_NOTES_V2.0.53.md)** — JSON view grouped under Operations and responsive dashboard optimization
- **[v2.0.52 release notes](design/RELEASE_NOTES_V2.0.52.md)** — sliding session renewal for long-running dashboard tabs
- **[v2.0.51 release notes](design/RELEASE_NOTES_V2.0.51.md)** — refresh recovery and token guidance
- Security-first handling around API tokens and external HTTP endpoints
- Local monolith architecture with internal async task scheduling for background work
- Safe extension path for future distributed workers only when real operational need exists

## Versioning format

The project follows semantic versioning in x.x.x format:

- **MAJOR**: breaking architecture or incompatible changes
- **MINOR**: new features or major enhancements  
- **PATCH**: fixes, hardening, and stability improvements

Each release includes a versioned right-side popup summarizing the fix set. The browser UI, CLI, and build metadata remain aligned with the shipped code version.

Current release: 2.0.78

Last updated: October 27, 2026

Version 2.0.78 updates the project name and release metadata and avoids
duplicating the libcurl DLL directory in the Windows launcher PATH. The
post-login release popup summarizes the changes. Existing Stock API behavior
remains authenticated, server-side, and read-only; legacy local cache and log
locations remain unchanged.

Version 2.0.75 removes two MSVC warnings in the web server and improves the
Windows Bazel link-failure diagnostics. Before building, the helper now checks
whether the exact `portfolio_health.exe` output is still running and reports
its PID so it can be closed before retrying; it never terminates a live
dashboard. Linux behavior and existing WebSocket, TLS, and Stock API security
checks are unchanged. The post-login popup summarizes the fix.

Version 2.0.71 hardens Windows build and launcher behavior: CMake launch now
detects stale platform-adapter sources and missing core libraries, and rebuild
refuses unsafe build-directory paths. Windows Python regression tests are scoped
to `tests/python`, avoiding accidental collection from Bazel's generated
dependencies. Linux build behavior is unchanged. The post-login popup
summarizes the fix. Stock API access remains authenticated, read-only, and
server-side; credentials and tokens are not exposed to browser code.

Version 2.0.70 adds Windows, Linux, and macOS adapters for sockets, secure
temporary files, Python process launching, executable-path discovery,
environment cleanup, and operating-system randomness. Python analysis is
started without a shell, and temporary analysis files are removed on success
or failure. The portfolio/business logic and authenticated, read-only Upstox
API behavior remain unchanged. The post-login popup summarizes the update.
See the v2.0.70 release notes and implementation summary for details.

Version 2.0.69 makes the market-holiday endpoint return HTTP 502 with an
explicit error payload when no valid calendar cache exists. A cached calendar
is marked stale and the status tooltip explains when regular NSE hours are
only estimates. Dashboard views expose loading state accessibly and discard
outdated responses without replacing newer cached data. The post-login popup summarizes
these changes; Upstox access remains read-only with credentials kept server-side.
See the v2.0.69 release notes and implementation summary for details.

Version 2.0.68 fixes task scheduling synchronization and priority ordering,
returns appropriate HTTP statuses for unsupported methods, missing routes,
and unavailable portfolio snapshots, and rate-limits invalid local login
attempts. Session IDs now use operating-system cryptographic randomness.
Placeholder ML inference, broker order operations, and external notification
channels no longer report fake successes; order-risk checks fail closed when
sector exposure cannot be verified. Upstox credentials remain server-side.
Version 2.0.67 removes the redundant Overview Update/Refresh metric card;
the dashboard's main Refresh control remains available. Market Value and Day
P&L eye controls now use delegated click handling on the stable dashboard
container, so they continue to work after the Overview is re-rendered. Hidden
values remain masked through refreshes and the preference is retained locally.
Other page optimizations, authenticated endpoints, and server-side Stock API
credential handling are unchanged. The post-login popup summarizes the fix.

Version 2.0.66 makes Refresh failures actionable instead of masking them with
the same generic warning. HTTP 401/403 is identified as a rejected Upstox
access token and advises obtaining a replacement through Upstox; DNS/network
failures advise checking connectivity. The existing view remains visible.
Recent private dashboard logs showed both HTTP 401 responses and DNS lookup
failures, so this release clarifies that a button retry cannot renew a rejected
token or repair DNS. The post-login popup summarizes the change; credentials
remain server-side.

Version 2.0.65 fixes a race in manual Refresh during an initial portfolio load.
If another request is already building the server snapshot and no prior
snapshot or local CSV fallback exists, Refresh now waits for that request and
reuses the newly generated snapshot instead of showing a refresh-in-progress
failure. This coalesces duplicate requests while preserving stale/local
fallback behavior for non-manual requests. The post-login popup summarizes the
fix; authentication and server-side Stock API credential handling are
unchanged.

Version 2.0.64 reduces transient manual Refresh failures by retrying the
required Upstox holdings request once on transport errors. The browser allows
50 seconds for a dashboard API request, covering the bounded backend snapshot
budget of up to 40 seconds for holdings, positions, and news. Optional positions
and news failures remain isolated. The post-login popup summarizes the fix;
authentication and server-side Stock API credential handling are unchanged.

Version 2.0.63 fixes the Overview refresh action overlapping its label by
removing the `Manual` wording and placing the button beneath the `Update`
heading. An accessible eye control now also hides or shows Day P&L. Both
visibility preferences are stored locally without storing financial values,
and remain respected during live updates. Existing refresh behavior and the
authenticated server-side Stock API boundary are unchanged. The post-login
popup summarizes the fix.

Version 2.0.62 added an accessible eye button to hide or show Market value.
The hidden/shown preference is stored locally without storing the financial
value. Live quote updates continue to respect the chosen visibility setting.
Existing controls and the authenticated server-side Stock API boundary are
unchanged. The post-login popup summarizes the UI changes.

Version 2.0.61 fixes the backend cause of Refresh remaining unavailable:
server-side snapshot requests previously could outlast the browser's timeout
while holding the shared refresh lock through multiple upstream retries. Each
snapshot API call now has a 10-second single-attempt bound, while the browser
allows 35 seconds for the complete snapshot. Direct Retry handlers and queued
manual refresh from v2.0.60 remain in place. The post-login popup summarizes
the fix; Stock API access remains authenticated and server-side.

Version 2.0.57 prevents live quote polling from creating overlapping browser
requests when a response stalls. Requests now have a single-flight guard and
an eight-second timeout, and are cancelled when the tab is hidden. Quote-to-
holding lookups use a map, avoiding repeated linear searches. The release
popup shown after login summarizes the fix. Stock API requests remain behind
the authenticated server-side boundary; no broker credentials are sent to the
browser.

Version 2.0.56 fixes the root cause of the Overview's reconnect status:
the system libcurl used by the prior build did not include the WSS protocol,
so it rejected both the `wss` protocol restriction and the feed connection
with `Unsupported protocol`. Builds now use the pinned Conan libcurl package
with WebSocket support enabled. Startup also checks WSS capability explicitly
and reports a clear configuration error rather than repeatedly reconnecting
with an incompatible system library. TLS verification and the trusted Upstox
host allowlist remain enforced. The post-login popup summarizes the fix.

Version 2.0.55 fixes the Overview's generic live-feed reconnect state by
retrying partial/non-blocking WebSocket writes until the complete subscription
or heartbeat frame is sent, with a bounded timeout. The authenticated local
quotes response includes a short connection diagnostic. Quote updates are
matched by instrument key, and live Last, position P&L, market value, and Day
P&L update consistently. The post-login release popup summarizes the fix.

Version 2.0.54 implements the Upstox Market Data Feed V3 WebSocket in the
server. It authorizes and validates the Upstox `wss://` endpoint, verifies TLS,
subscribes to at most 100 live holdings, decodes bounded Protobuf updates, and
serves cached prices through the authenticated local `/api/stream/quotes`
route. The Overview refreshes prices without exposing broker credentials or
the one-time authorized WebSocket URL to browser JavaScript. Live updates are
limited to the visible Overview and pause with the existing dashboard pause
control. The post-login popup summarizes the change.

Version 2.0.53 moves the existing read-only holdings JSON view from the
top-level navigation into Operations. It continues to use the authenticated
holdings endpoint and safely escapes the displayed payload; it adds no Stock
API route or browser-visible credential. Shared responsive layout and rendering
optimizations continue across dashboard pages. The post-login release popup
summarizes the change.

Version 2.0.52 fixes the one-hour dashboard interruption: the login cookie
expired exactly 60 minutes after login even while the user was actively
refreshing live pages. Successful authenticated requests now slide the
one-hour idle timeout and renew the HttpOnly cookie. A visible dashboard also
renews its session every ten minutes through a lightweight local endpoint that
does not call Upstox. Hidden/inactive sessions still expire and are removed.
This keeps long-running dashboard tabs authenticated without exposing
credentials to browser JavaScript. The login popup summarizes the fix.

Version 2.0.51 distinguishes an expired Upstox access token from an ordinary
offline/local fallback, so users are not told to renew credentials for a
temporary network problem. Expired browser sessions now return to the login
page instead of getting stuck on a followed login redirect. Refresh failures
use the retained snapshot's authentication warning when available, and the
in-dashboard refresh control has consistent styling and a clear disabled
state. Automatic Upstox token renewal is not supported: after renewing
`UPSTOX_ACCESS_TOKEN`, restart `./run.sh --web` so the server loads the new
credential. The post-login release popup summarizes these fixes.

Version 2.0.50 fixes stale application binaries being reused by `./run.sh`.
Normal launches now rebuild when application sources or build metadata are
newer than `build/portfolio_health`, so the UI version reflects the fixes in
the current checkout. Explicit `--rebuild` continues to force a clean build;
normal auto-rebuilds use the existing incremental build. The login popup
summarizes this change.

Version 2.0.49 prevents a slow Stock API refresh from stalling the dashboard
after it has been running for a while. The local web server now handles a
bounded number of requests concurrently, and snapshot readers use the last
successful data or local fallback while another refresh is underway. A
transient refresh failure leaves the last rendered page visible with a warning
instead of replacing it. Existing refresh backoff, server-side API credentials,
and authenticated routes remain in place. The post-login popup summarizes the
fix.

Version 2.0.48 improves long-running data availability. A failed Upstox refresh
now preserves the last successful live snapshot before considering the local
CSV fallback. Project config paths are resolved from the application directory,
so fallback holdings and saved news remain available when the server starts
from another working directory. Token-expired fallback data is clearly labeled
with renewal guidance; the login popup summarizes these fixes.

Version 2.0.47 restores query-string routing for fundamentals, stock analysis,
market quote, and RSI APIs while keeping the manual-refresh query limited to
the authenticated snapshot endpoints. It also prevents the Fundamentals
summary table from overflowing its popup on smaller screens. The login
release popup summarizes these fixes.

Version 2.0.46 fixes manual dashboard refresh during an upstream retry cooldown.
The Refresh and Retry now controls can request one immediate authenticated
snapshot refresh, while the server limits forced refresh attempts to one per
10 seconds. The toolbar displays refresh progress and prevents duplicate clicks.
The post-login release popup summarizes this fix.

Version 2.0.45 fixes duplicate and inconsistent Deeper analysis rows by
normalizing and deduplicating live symbols, reading Python recommendations
from structured JSON, and building category totals/lists from the unique
rendered rows. Duplicate instrument positions for the same symbol have their
market values combined. The login release popup summarizes this fix.

Version 2.0.44 improves long-running dashboard reliability. Portfolio snapshots
are cached to reduce repeated Stock API traffic; transient upstream outages keep
the last successful data visibly marked as cached, with bounded retry backoff.
Overview holdings remain available when only news is unavailable. The post-login
release popup summarizes these changes.

Version 2.0.43 merges the duplicate News and Global market news tabs into one
Market news view. The dashboard uses its existing authenticated,
holdings-scoped news snapshot and does not add a redundant Stock API request.
Shared responsive layout and deferred offscreen rendering improve dashboard
pages; the versioned fix summary appears in the post-login popup.

Version 2.0.42 fixes credential precedence in `run.sh`: shell-provided
credentials now override `~/.upstox.env`, which only fills missing values. This
prevents an older file token from replacing a fresh shell token and causing
repeated Upstox 401 errors.
The running C++ process keeps its startup credentials, so restart `./run.sh --web`
after rotating the access token.

Version 2.0.41 adds an Operations > Download subview that exports the current
Som Baran Portfolio holdings to CSV from the existing authenticated holdings
payload. Export values are CSV-escaped and formula-leading text is neutralized
for spreadsheet safety; no new Stock API route or credential exposure is added.

Version 2.0.40 groups Positions, Data health, and Configuration under one
Operations tab with in-page subviews. Existing endpoints and view behavior are
preserved; automatic live-data refresh is skipped while Configuration is
selected.

Version 2.0.39 captures C++ stdout and stderr for `./run.sh --web` in a
per-user log at `${XDG_STATE_HOME:-~/.local/state}/baran-capital-view/web-ui.log`.
This legacy state-directory name is retained for existing installations.
Set `PORTFOLIO_WEB_LOG` to choose a different path. The log is created with
owner-only permissions; non-web commands keep their normal console output.
Automatic dashboard data polling also pauses while the browser tab is hidden
and resumes through the existing visibility recovery when it becomes visible.

Version 2.0.38 fixes a response-envelope mismatch that could display failed
live holdings requests as a valid zero-holding portfolio. Failed API payloads
now show an actionable data/authentication state; automatic retries back off
for transient failures and expired tokens, while manual Refresh remains
available. Error details stay server-side.

Version 2.0.37 adds an authenticated Upstox market-holidays endpoint and uses
the validated calendar to mark NSE trading holidays and special-session hours.
The server caches the last valid calendar for 12 hours; the existing
weekday/time calculation remains the fallback when the holiday service is
unavailable. Holiday credentials stay on the server.

Version 2.0.36 replaces serial-number `#` headers with `S.No` across dashboard
tables while preserving refresh and sorting behavior. Version 2.0.35 fixed the
Overview Day P&L when live holdings omit `day_change`
and prevents the local CSV fallback from multiplying an already-total P&L by
quantity. Shared responsive sizing improves metric grids, news lists, and wide
tables across all dashboard pages. The post-login release popup summarizes
these changes.

The Deeper analysis page is organized as a review queue: positive, risk, and
hold signals are summarized first, signal categories reveal their stocks, and
the evidence table keeps saved news, fresh NLP analysis, action, and rationale
together. The same responsive rendering and status behavior is used across all
dashboard pages.

Deeper analysis keeps navigation responsive while loading: the RSI panel is
repositioned only when necessary, preventing repeated DOM mutations from
freezing tab changes.

Dashboard navigation is compact and adaptive: desktop keeps tabs in one
horizontal row with overflow support, while mobile uses a balanced two-column
layout. Long tab labels remain readable without creating an oversized empty
row.

The dashboard navigation uses balanced responsive rows for all tabs, with
stable button sizing on desktop and mobile so long labels do not create an
isolated or visually broken second row.

Market news uses the authenticated Upstox news snapshot matched to configured
holdings. The legacy Global market news endpoint remains available for
compatibility, but the duplicate tab and its redundant browser request have
been removed. The Summary Dashboard aggregates the existing holdings and
portfolio-news snapshots without additional broker requests.

Deeper analysis retries transient wake or network failures before showing an
error, while the server worker remains asynchronous and can rebuild an
expired or failed result on the next request.

The browser dashboard recovers after system sleep or tab suspension by
clearing interrupted view requests and refreshing the active page when the
window becomes visible again. Deeper analysis can therefore restart after a
transient wake-related failure.

Long-running Upstox requests retry transient DNS, connection, and timeout
failures before falling back to local data. Authentication failures such as
HTTP 401 remain visible and require token renewal.

When Upstox returns HTTP 401, the Overview no longer presents local CSV
values as live market data. It keeps the holdings list available, marks the
market value unavailable, and logs that the access token must be renewed.

The holdings parser derives live market value from Upstox `last_price`,
quantity, and multiplier when the API does not provide an explicit valuation.
If the live request fails, the UI uses a clearly marked local fallback instead
of silently treating `config/holding.csv` as live data.

Deeper analysis uses the transformer sentiment model by default for the most
accurate available classification. If the model or a news provider is
unavailable, the existing keyword-based fallback keeps the page usable and
labels the result as fallback analysis.

The build and the browser popup both read the same version identifier from the CMake project definition so the release notes, UI banner, and runtime binary stay aligned with the shipped code change set.

## Shared library layout

The C++ build now exposes a shared library target named `portfolio_health_core` for the service implementation, while the CLI executable remains a thin runtime wrapper. This keeps the runtime entry point small, avoids duplicate translation units, and preserves the same operational behavior for the web UI and CLI.

Build targets:

- `portfolio_health_core` — shared library containing the portfolio, API, notification, and analytics logic
- `portfolio_health` — executable entry point for the app and CLI commands
- `portfolio_health_tests` — gtest regression target for the secure stock API and portfolio logic

## Build

The project supports the standard CMake/Conan workflow and a Bazel build path
for compatibility with alternative CI pipelines. Use CMake/Conan for live
Upstox streaming: that build pins libcurl with WSS support. A Bazel build that
links a system libcurl without WSS reports a clear runtime diagnostic and
cannot connect to the live feed.

```bash
cd /home/ritup2404/PortfolioSense

# 1 = CMake build with Conan integration
./buildCode.sh 1 --rebuild

# 2 = Bazel build (requires Bazel or Bazelisk to be installed locally)
./buildCode.sh 2 --rebuild

# legacy explicit flags remain supported
./buildCode.sh --cmake --rebuild
./buildCode.sh --bazel --rebuild
```

Use either the numeric selector (`1` or `2`) or the explicit flags for build backend selection in automation. If Bazel is not installed, prefer the CMake path with `1` to keep the default workflow functional.

## Dependencies

All dependencies are managed through **Conan** (`conanfile.py`) for reproducible builds across platforms:

- **nlohmann_json 3.11.3** — JSON serialization/deserialization (managed via Conan since v2.0.10)
- **libcurl** — Conan-managed HTTP/WebSocket client with WSS support enabled
- **OpenSSL 3.2.1** — TLS/HTTPS support, managed through Conan
- **GTest** — C++ testing framework (automatically integrated)

The `conanfile.py` file defines the standard CMake build requirements. Conan
automatically generates CMake toolchain files and dependency metadata during
the build process.

Add the local login secret to your shell environment or a protected local file before starting the browser UI:

```bash
printf '%s\n' "export FOLIO_LOGIN_CODE='070923'" >> ~/.upstox.env
chmod 600 ~/.upstox.env
source ~/.upstox.env
./run.sh --web
```

## Testing

The project includes regression checks for both C++ and Python paths. Run the
full local regression check after building:

```bash
# C++ unit tests with gtest
cmake --build build --target portfolio_health_tests
ctest --test-dir build --output-on-failure

# Python unit tests with pytest
pytest -q
```

`pytest.ini` limits discovery to `tests/python` and excludes generated Bazel
trees, so the command does not collect third-party or symlinked build files.
The current suite contains 1 C++ test target and 14 Python tests.

The C++ tests cover secure order validation, portfolio calculations, and live Day P&L aggregation. The Python tests cover sentiment fallback and path resolution in the Deeper analysis workflow.

### Coverage reports

Coverage is opt-in so normal Release builds remain unchanged. Install the
reporting tools first:

```bash
# Debian / Ubuntu
sudo apt install -y lcov

# Fedora / RHEL
sudo dnf install -y lcov
```

Configure an instrumented CMake build and generate both report formats:

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug \
	-DPORTFOLIO_HEALTH_ENABLE_COVERAGE=ON
cmake --build build-coverage --target coverage
cmake --build build-coverage --target coverage-gcov
```

The same workflows are available through `buildCode.sh`:

```bash
./buildCode.sh 1 --coverage
./buildCode.sh 1 --coverage-gcov
./buildCode.sh 1 --coverage --coverage-build-dir /tmp/baran-coverage
```

The lcov percentage summary is printed by the `coverage` target, with the
HTML report at `build-coverage/coverage/html/index.html`. Raw gcov files are
written to `build-coverage/coverage/gcov`. The coverage targets run the C++
CTest suite; run `pytest --cov` separately when Python coverage is needed.

## Run web UI

```bash
cd /home/ritup2404/PortfolioSense
./run.sh --web
```

## Security notes

- API traffic is restricted to HTTPS-only access for stock and broker endpoints
- Host allowlisting prevents credential forwarding to untrusted destinations
- Secrets are read from environment variables or local protected files only
- Browser session login uses a private secret code outside version-controlled files
- Redirection, stale data, and unauthorized responses fail safely without exposing sensitive details

## Architecture recommendation

The best architecture for PortfolioSense is a local monolith first, with an internal task scheduler and async worker queue for non-blocking background work.

Recommended pattern:

- Local monolith: the main C++ web/UI service remains the single operational boundary
- Internal task scheduler: scheduled refresh, analysis, and notification tasks run in-process
- Async worker queue: long-running jobs are handled without blocking the main request loop
- Distributed version later: only if there is a real need for multi-node scaling or horizontal isolation

This keeps the broker API and stock data behind one trusted boundary and avoids exposing new attack surfaces.

## Recent fix summary

Version 2.0.17 fixes the empty News fallback and stale Overview market value
issues. Saved portfolio news now degrades gracefully when the Upstox token is
stale or when holdings are temporarily unavailable, and the live broker value is
preferred before any derived fallback. The backend and browser both log a clear
stale-token warning when the access token has expired.

Version 2.0.16 optimizes Deeper analysis category controls. Hovering or
focusing `No recent news`, `Neutral news`, `going good`, `invest more`, or
`sell it off` shows the associated stock names without another API request.

Version 2.0.15 optimizes Alerts with a meaningful `Portfolio news review`
heading, removes duplicate confidence sorting, and shows the stocks in each
decision group on hover or keyboard focus.

Version 2.0.14 adds automated regression execution to `buildCode.sh`. CMake
runs CTest and pytest, while Bazel runs its native C++ test target and pytest.
Use `--skip-tests` only for build-only workflows.

Version 2.0.13 hardens holdings and dashboard API parsing. Empty, truncated, or
non-JSON responses now produce valid shape-correct fallbacks, so transient Stock
API or transport failures do not break the dashboard.

Version 2.0.12 removes the duplicate Overview confidence selector. The table
header arrows remain the single sorting control, while the shared filter now
filters data rows and cards without hiding table headers or contacting the
Stock API.

Version 2.0.11 adds optimized client-side arrow sorting to the Overview, Alerts,
Deeper analysis, and Fundamentals tables. Sorting uses the already loaded rows,
keeps serial numbers stable, and does not send new requests to the Stock API.

Version 2.0.10 included:

### Dependency Management (v2.0.10+)
- **Removed vendored nlohmann header**: Eliminated `third_party/nlohmann/json.hpp` vendored copy
- **Added Conan dependency**: `nlohmann_json/3.11.3` now managed through `conanfile.py`
- **Simplified CMakeLists.txt**: Reduced dependency resolution from 35+ lines to 2 lines using `find_package(nlohmann_json REQUIRED)`
- **Single source of truth**: CMake dependencies are managed through Conan; the legacy Bazel path uses its configured system libraries

### API resilience and error handling
- **Fixed JSON parsing errors** that showed "unexpected character at line 1 column 1" on long-running sessions by validating all API responses before sending to the browser
- **Added robust error handling** for empty, malformed, or incomplete JSON payloads at both the C++ server and JavaScript client layers
- **Implemented fallback JSON** for all API endpoints (`/api/holdings`, `/api/news`, `/api/positions`, `/api/config`) to ensure valid responses even when data sources fail
- **Enhanced error messages** with detailed context displayed in a dedicated error panel instead of raw parse exceptions

### Browser UI optimization
- **Data health tab** now shows operational diagnostics: holdings loaded, news articles, alert decisions, and missing price checks
- **JSON tab** displays raw server payload with a clear "Runtime payload" label and explanatory context
- **Config tab** summarizes the secure configuration boundary: holdings source, news source, auth mode, and HTTPS-only model
- **Error display** improved with actionable guidance and browser console logging for debugging

### Security and code quality
- **Login flow hardening** ensures `FOLIO_LOGIN_CODE` values are trimmed, URL-decoded, and compared safely before session creation
- **Environment-backed secrets** can come from `~/.upstox.env` or protected local fallback files without exposing credentials in source control
- **Repository hygiene** keeps Bazel artifacts, local caches, generated files, and secret data out of GitHub
- **Single canonical tab render path** prevents duplicate UI logic and stale data conflicts in Overview, Alerts, and Deeper analysis views

### Operational stability
- **Existing features preserved**: browser cache behavior, saved-news fallback path, and local-first architecture remain unchanged
- **Version alignment** across release popup, documentation, CMake build, and runtime UI using x.x.x semantic versioning
- **All regression tests pass** with 100% success rate under the gtest suite

## Project structure

A simple industry-style project layout for this service is:

- app/ — entry points and runtime bootstrapping for the web UI and CLI
- src/ — C++ implementation, API clients, and service logic
- include/ — public headers and shared interfaces
- config/ — runtime configuration and portfolio data files
- design/ — architecture decisions, design docs, and release notes
- docs/ — operator and developer documentation
- scripts/ — build, run, and deployment helper scripts
- tests/ — unit and integration checks for stability and security
- stock_alert_nlp.py — Python analysis and news scoring utility

This structure keeps the stock API boundary, executable surface, and configuration clearly separated so operational changes stay easier to reason about and audit.
