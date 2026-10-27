import os
import shutil
import stat
import subprocess
from pathlib import Path


def prepare_fake_project(tmp_path):
    project = tmp_path / "project"
    build = project / "build"
    build.mkdir(parents=True)
    shutil.copy2(Path(__file__).parents[2] / "run.sh", project / "run.sh")
    build_script = project / "buildCode.sh"
    build_script.write_text(
        "#!/bin/sh\nprintf 'build marker: %s\\n' \"$*\"\n"
    )
    build_script.chmod(0o755)
    executable = build / "portfolio_health"
    executable.write_text(
        "#!/bin/sh\nprintf 'cpp stdout marker\\n'\nprintf 'cpp stderr marker\\n' >&2\nprintf 'token marker: %s\\n' \"${UPSTOX_ACCESS_TOKEN:-unset}\"\n"
    )
    executable.chmod(0o755)
    home = tmp_path / "home"
    home.mkdir()
    (home / ".upstox.env").write_text("")
    return project, home


def test_web_launch_captures_cpp_output_in_private_log(tmp_path):
    project, home = prepare_fake_project(tmp_path)
    state_home = tmp_path / "state"
    environment = os.environ.copy()
    environment.update({"HOME": str(home), "XDG_STATE_HOME": str(state_home)})

    result = subprocess.run(
        ["bash", str(project / "run.sh"), "--web"],
        cwd=project,
        env=environment,
        capture_output=True,
        text=True,
        check=True,
    )

    log_file = state_home / "baran-capital-view" / "web-ui.log"
    contents = log_file.read_text()
    assert "cpp stdout marker" in contents
    assert "cpp stderr marker" in contents
    assert "token marker: unset" in contents
    assert stat.S_IMODE(log_file.stat().st_mode) == 0o600
    assert str(log_file) in result.stdout
    assert "http://127.0.0.1:8080" in result.stdout
    assert "cpp stdout marker" not in result.stdout


def test_non_web_launch_is_not_redirected(tmp_path):
    project, home = prepare_fake_project(tmp_path)
    state_home = tmp_path / "state"
    environment = os.environ.copy()
    environment.update({"HOME": str(home), "XDG_STATE_HOME": str(state_home)})

    result = subprocess.run(
        ["bash", str(project / "run.sh"), "--version"],
        cwd=project,
        env=environment,
        capture_output=True,
        text=True,
        check=True,
    )

    assert "cpp stdout marker" in result.stdout
    assert "cpp stderr marker" in result.stderr
    assert not (state_home / "baran-capital-view" / "web-ui.log").exists()


def test_fresh_shell_token_overrides_stale_env_file(tmp_path):
    project, home = prepare_fake_project(tmp_path)
    (home / ".upstox.env").write_text(
        "export UPSTOX_API_KEY=file-key\n"
        "export UPSTOX_API_SECRET=file-secret\n"
        "export UPSTOX_ACCESS_TOKEN=stale-file-token\n"
    )
    state_home = tmp_path / "state"
    environment = os.environ.copy()
    environment.update({
        "HOME": str(home),
        "XDG_STATE_HOME": str(state_home),
        "UPSTOX_ACCESS_TOKEN": "fresh-shell-token",
    })

    subprocess.run(
        ["bash", str(project / "run.sh"), "--web"],
        cwd=project,
        env=environment,
        capture_output=True,
        text=True,
        check=True,
    )

    log_file = state_home / "baran-capital-view" / "web-ui.log"
    contents = log_file.read_text()
    assert "token marker: fresh-shell-token" in contents
    assert "stale-file-token" not in contents


def test_launcher_rebuilds_when_source_is_newer_than_the_binary(tmp_path):
    project, home = prepare_fake_project(tmp_path)
    source = project / "src"
    source.mkdir()
    source_file = source / "WebServer.cpp"
    source_file.write_text("// updated source\n")
    executable = project / "build" / "portfolio_health"
    source_time = executable.stat().st_mtime_ns + 1_000_000_000
    os.utime(source_file, ns=(source_time, source_time))
    environment = os.environ.copy()
    environment.update({"HOME": str(home)})

    result = subprocess.run(
        ["bash", str(project / "run.sh"), "--version"],
        cwd=project,
        env=environment,
        capture_output=True,
        text=True,
        check=True,
    )

    assert "build marker: --skip-tests" in result.stdout
    assert "cpp stdout marker" in result.stdout


def test_launcher_rebuild_option_is_consumed_before_forwarding_arguments(tmp_path):
    project, home = prepare_fake_project(tmp_path)
    environment = os.environ.copy()
    environment.update({"HOME": str(home)})

    result = subprocess.run(
        ["bash", str(project / "run.sh"), "--rebuild", "--version"],
        cwd=project,
        env=environment,
        capture_output=True,
        text=True,
        check=True,
    )

    assert "build marker: --rebuild --skip-tests" in result.stdout
    assert "cpp stdout marker" in result.stdout


def test_operations_workspace_keeps_existing_subviews():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    operations = source.split("const char* operationsWorkspace()", 1)[1].split(
        "const char* responsiveDashboardNavigation()", 1
    )[0]

    assert "operationsButton.dataset.tab='operations'" in operations
    assert 'data-operation="positions"' in operations
    assert 'data-operation="health"' in operations
    assert 'data-operation="config"' in operations
    assert 'data-operation="json"' in operations
    assert "raw('positions','/api/positions','Open positions')" in operations
    assert "await health()" in operations
    assert "await config()" in operations


def test_operations_json_view_reuses_authenticated_holdings_payload():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    operations = source.split("const char* operationsWorkspace()", 1)[1].split(
        "const char* operationsDownload()", 1
    )[0]
    page = source.split("const char* page()", 1)[1].split(
        "std::unordered_set<std::string> csvSymbols", 1
    )[0]

    assert "nav.querySelector('[data-tab=\"json\"]')?.remove()" in operations
    assert "else if(next==='json')await raw('holdings','/api/holdings','Holdings JSON')" in operations
    assert "JSON.stringify(p,null,2)" in page
    assert "'+esc(text)+'" in page
    assert "grid-template-columns:repeat(3,minmax(0,1fr))" in operations
    assert "data-tab=\"json\"" in page


def test_operations_download_uses_authenticated_holdings_and_safe_csv():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    operations = source.split("const char* operationsWorkspace()", 1)[1].split(
        "const char* operationsDownload()", 1
    )[0]
    exporter = source.split("const char* operationsDownload()", 1)[1].split(
        "const char* responsiveDashboardNavigation()", 1
    )[0]

    assert 'data-operation="download"' in operations
    assert "data-download-portfolio" in operations
    assert "get('holdings','/api/holdings')" in exporter
    assert "text.replace(/\"/g,'\"\"')" in exporter
    assert "typeof value==='string'&&!numeric" in exporter
    assert "URL.revokeObjectURL(url)" in exporter


def test_market_news_uses_one_tab_and_keeps_legacy_route():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    consolidation = source.split("const char* consolidatedMarketNewsTab()", 1)[1].split(
        "const char* requestedDashboardReleaseNotice()", 1
    )[0]
    renderer = source.split("async function news()", 1)[1].split(
        "\nasync function alerts()", 1
    )[0]

    assert "nav.querySelector('[data-tab=\"global-news\"]')?.remove()" in consolidation
    assert "newsTab.textContent='Market news'" in consolidation
    assert "get('news','/api/news')" in renderer
    assert "path == \"/api/global-news\"" in source
    assert "client_.getNews(\"holdings\")" in source
    assert "releaseNoticeV2043Addendum()" in source
    assert "dashboardPageOptimization()" in source


def test_dashboard_snapshot_retries_are_bounded_and_failures_are_isolated():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    snapshot = source.split(
        "std::shared_ptr<const WebServer::Snapshot> WebServer::snapshot(bool forceRefresh) const",
        1,
    )[1].split("\nint WebServer::run()", 1)[0]
    overview = source.split("async function holdings()", 1)[1].split(
        "\nasync function news()", 1
    )[0]
    market_news = source.split("async function news()", 1)[1].split(
        "\nasync function alerts()", 1
    )[0]

    assert "std::chrono::seconds(30)" in snapshot
    assert "snapshotRetryAfter_" in snapshot
    assert "std::chrono::minutes(5)" in snapshot
    assert '"upstox-stale"' in snapshot
    assert '"snapshot_age_seconds"' in snapshot
    assert "Renew UPSTOX_ACCESS_TOKEN and restart the dashboard." in snapshot
    assert "isAccessTokenStale(snapshotRefreshError_)" in snapshot
    assert "constexpr long snapshotRequestTimeoutSeconds = 10" in snapshot
    assert "constexpr int snapshotHoldingsTransportAttempts = 2" in snapshot
    assert "client_.getHoldings(\n        snapshotRequestTimeoutSeconds, snapshotHoldingsTransportAttempts)" in snapshot
    assert "client_.getPositions(snapshotRequestTimeoutSeconds, 1)" in snapshot
    assert 'client_.getNews("holdings", snapshotRequestTimeoutSeconds, 1)' in snapshot
    assert snapshot.index("fresh->created =") > snapshot.index(
        'client_.getNews("holdings", snapshotRequestTimeoutSeconds, 1)'
    )
    assert "last successful news snapshot" in snapshot
    assert "Portfolio news is temporarily unavailable." in snapshot
    assert "catch(error){newsUnavailable=true}" in overview
    assert "portfolio holdings are still shown" in overview
    assert "Cached news · live refresh unavailable" in market_news
    assert "releaseNoticeV2044Addendum()" in source


def test_snapshot_failure_keeps_last_good_data_and_resolves_fallback_from_project():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    snapshot = source.split(
        "std::shared_ptr<const WebServer::Snapshot> WebServer::snapshot(bool forceRefresh) const",
        1,
    )[1].split("\nint WebServer::run()", 1)[0]
    local_data = source.split("std::filesystem::path projectFile(", 1)[1].split(
        "\nstd::string localNews", 1
    )[0]
    constructor = source.split("WebServer::WebServer(", 1)[1].split(
        "\nstd::string WebServer::fundamentalsAnalysis", 1
    )[0]

    assert snapshot.index("if (current) return staleSnapshot(") < snapshot.index(
        "const json fallback = localHoldings(holdingsFile_)"
    )
    assert "std::filesystem::path projectRoot()" in source
    assert "projectRoot() / configured" in local_data
    assert "holdingsFile_(projectFile(holdingsFile).string())" in constructor
    assert "UPSTOX_ACCESS_TOKEN and restart the dashboard" in snapshot
    assert 'releaseNoticeV2048Addendum()' in source


def test_web_server_handles_slow_snapshots_concurrently_with_a_connection_limit():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    snapshot = source.split(
        "std::shared_ptr<const WebServer::Snapshot> WebServer::snapshot(bool forceRefresh) const",
        1,
    )[1].split("\nint WebServer::run()", 1)[0]
    server = source.split("int WebServer::run()", 1)[1]

    assert "std::unique_lock<std::mutex> refreshLock(refreshMutex_, std::defer_lock)" in snapshot
    assert "const bool refreshInProgress = !refreshLock.try_lock()" in snapshot
    assert "if (refreshInProgress && forceRefresh)" in snapshot
    assert "refreshLock.lock()" in snapshot
    assert "else if (refreshInProgress)" in snapshot
    assert "if (current) return staleSnapshot(current, tokenExpired)" in snapshot
    assert "maxConcurrentConnections = 8" in server
    assert "std::thread([this, connection, &activeConnections]" in server
    assert '"503 Service Unavailable"' in server
    assert "activeConnections.fetch_sub(1, std::memory_order_acq_rel)" in server
    assert "releaseNoticeV2049Addendum()" in server


def test_manual_refresh_waits_for_inflight_initial_snapshot_and_reuses_it():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    snapshot = source.split(
        "std::shared_ptr<const WebServer::Snapshot> WebServer::snapshot(bool forceRefresh) const",
        1,
    )[1].split("\nint WebServer::run()", 1)[0]
    forced_contention = snapshot.split(
        "if (refreshInProgress && forceRefresh)", 1
    )[1].split("else if (refreshInProgress)", 1)[0]

    assert "refreshLock.lock()" in forced_contention
    assert "MutexGuard lock(snapshotMutex_)" in forced_contention
    assert "snapshot_ && snapshot_->created >= now" in forced_contention
    assert "return snapshot_" in forced_contention
    assert "portfolio snapshot refresh is in progress" not in forced_contention


def test_background_refresh_failure_retains_last_rendered_dashboard():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    failure = source.split("function fail(e)", 1)[1].split("\nfunction rows", 1)[0]

    assert "hasRenderedView=view.childElementCount>0&&!view.querySelector('.loading,[role=\"alert\"]')" in failure
    assert "+msg+' The last displayed data is retained.'" in failure
    assert "Live refresh unavailable · review the warning above" in failure
    assert "view.prepend(warning)" in failure
    assert "view.innerHTML='<section class=\"panel error\"" in failure
    assert "Live refresh unavailable · review the warning above" in failure


def test_refresh_errors_explain_upstream_auth_and_dns_failures_safely():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    safe_error = source.split("std::string userSafeSnapshotError(", 1)[1].split(
        "\nbool validAnalysisSymbol", 1
    )[0]
    fetcher = source.split("function apiFailure(name,detail,statusCode)", 1)[1].split(
        "\nconst DASHBOARD_REQUEST_TIMEOUT_MS", 1
    )[0]
    server = source.split("} catch (const std::exception& e) {", 1)[-1].split(
        "\n                }\n            } else if (path == \"/api/config\")", 1
    )[0]

    assert "Upstox rejected the access token (HTTP 401/403)" in safe_error
    assert "DNS or network failure" in safe_error
    assert "Check the private dashboard log for details" in safe_error
    assert "upstreamNetwork" in fetcher
    assert "Could not reach the dashboard server" in fetcher
    assert "userSafeSnapshotError(e.what())" in server


def test_manual_refresh_bypasses_server_backoff_with_a_rate_limit():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    header = (Path(__file__).parents[2] / "include" / "WebServer.hpp").read_text()
    refresh_ui = source.split("function refreshView()", 1)[1].split(
        "const baseSafeJsonFetch", 1
    )[0]
    fetcher = source.split("async function get(name,url)", 1)[1].split(
        "\nasync function safeJsonParse", 1
    )[0]
    snapshot = source.split(
        "std::shared_ptr<const WebServer::Snapshot> WebServer::snapshot(bool forceRefresh) const",
        1,
    )[1].split("\nint WebServer::run()", 1)[0]

    assert "manualRefreshRequested=true" in refresh_ui
    assert "refreshControls(true)" in refresh_ui
    assert "manualRefreshRequested&&['/api/holdings','/api/positions','/api/news'].includes(url)" in fetcher
    assert "requestUrl=url+'?refresh=1'" in fetcher
    assert "snapshot(bool forceRefresh = false)" in header
    assert "const bool manualRefreshQuery = queryStart != std::string::npos" in source
    assert "path == \"/api/holdings\" || path == \"/api/positions\" || path == \"/api/news\"" in source
    assert "snapshot(forceRefresh)" in source
    assert source.index("else if (!authenticated(requestText))") < source.index("snapshot(forceRefresh)")
    assert "manualRefreshLimit = std::chrono::seconds(10)" in snapshot
    assert "if (!forceRefresh && checkedAt < snapshotRetryAfter_)" in snapshot
    assert "if (checkedAt < manualRefreshAfter_)" in snapshot
    assert "releaseNoticeV2046Addendum()" in source


def test_stalled_dashboard_requests_release_manual_refresh_controls():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    fetcher = source.split("async function get(name,url)", 1)[1].split(
        "\nasync function safeJsonParse", 1
    )[0]
    safe_fetch = source.split("async function safeJsonFetch(url,fallback)", 1)[1].split(
        "\nfunction filterView", 1
    )[0]
    refresh = source.split("function refreshView()", 1)[1].split(
        "const baseSafeJsonFetch", 1
    )[0]
    metric = source.split("function normalizeRefreshMetric()", 1)[1].split(
        "\nfunction normalizeDeeperAnalysis", 1
    )[0]
    failure = source.split("function fail(e)", 1)[1].split("\nfunction rows", 1)[0]

    assert "new AbortController()" in fetcher
    assert "const DASHBOARD_REQUEST_TIMEOUT_MS=50000" in source
    assert "setTimeout(()=>controller.abort(),DASHBOARD_REQUEST_TIMEOUT_MS)" in fetcher
    assert "signal:controller.signal" in fetcher
    assert fetcher.index("response=await fetch") < fetcher.index(
        "text=await response.text()"
    ) < fetcher.index("clearTimeout(timeout)")
    assert "new AbortController()" in safe_fetch
    assert "setTimeout(()=>controller.abort(),DASHBOARD_REQUEST_TIMEOUT_MS)" in safe_fetch
    assert "signal:controller.signal" in safe_fetch
    assert "const payload=await safeJsonParse(response,effectiveFallback)" in safe_fetch
    assert "finally{clearTimeout(timeout)}" in safe_fetch
    assert "return openTab(activeTab,false).finally(finishRefresh)" in refresh
    assert "function refreshControls(disabled){document.querySelectorAll('#refresh-view,[data-refresh-dashboard]')" in source
    assert "manualRefreshPending=true" in refresh
    assert "Refresh queued; waiting for the current request to finish..." in refresh
    assert "function finishRefresh(){refreshInFlight=false;if(manualRefreshPending){manualRefreshPending=false;refreshView();return}" in source
    assert "metric-refresh" not in metric
    assert "Refresh now" not in metric
    assert "onclick=" not in metric
    assert "view.querySelector('[data-refresh-dashboard]').addEventListener('click',refreshView)" in failure
    assert "data-refresh-dashboard" in failure
    assert "onclick=" not in failure
    assert "openTab(activeTab,false).finally(finishRefresh)" in source
    assert "refreshInFlight=true;openTab('overview').then(sortOverviewRows).finally(finishRefresh)" in source
    assert "if(updatesPaused||refreshInFlight)return" in source


def test_overview_refresh_label_and_market_value_visibility_control():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    overview = source.split("async function holdings()", 1)[1].split(
        "\nasync function news()", 1
    )[0]
    visibility = source.split("function setMetricVisibility(metric,hidden", 1)[1].split(
        "\nfunction normalizeDeeperAnalysis", 1
    )[0]
    optimization = source.split("const char* dashboardPageOptimization()", 1)[1].split(
        "const char* releaseNoticeV2036Addendum()", 1
    )[0]

    assert '<span class="label">Update</span>' not in overview
    assert "Refresh now" not in overview
    assert ".metric-refresh{display:block;margin-top:8px" not in optimization
    assert 'id="refresh-view"' in source
    assert 'class="market-value-toggle" type="button"' in overview
    assert 'data-visibility-toggle="market-value"' in overview
    assert 'aria-label="Hide market value"' in overview
    assert 'data-visibility-toggle="day-pnl"' in overview
    assert 'aria-label="Hide Day P&amp;L"' in overview
    assert "data-visible-value" not in overview
    assert "aria-pressed" in visibility
    assert "baran-capital-view-hide-market-value" in source
    assert "baran-capital-view-hide-day-pnl" in source
    assert "amount.textContent!==text" in visibility
    assert "setMetricVisibility('day-pnl',dayPnlHidden)" in visibility
    assert "setMetricVisibility('market-value',marketValueHidden)" in visibility
    assert "setMarketValueText('INR '+value.toLocaleString" in source
    assert "setDayPnlText(dayPnl.toLocaleString" in source
    assert "metric?.querySelector('.market-value-amount')" in optimization
    assert "setMarketValueText(unavailable)" in optimization
    assert "view.addEventListener('click',event=>{const toggle=event.target.closest('[data-visibility-toggle]')" in source
    assert "document.addEventListener('click',event=>{const symbol=event.target.closest('.holding-symbol')" in source
    assert "toggle.dataset.visibilityToggle" in source
    assert "setMetricVisibility(metric,!marketValueHidden,true)" in source
    assert "setMetricVisibility(metric,!dayPnlHidden,true)" in source
    assert "button.dataset.hidden=String(hidden)" in visibility
    assert "button.dataset.ready" not in visibility


def test_query_routes_preserve_parameters_and_fundamentals_popup_wraps():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    routing = source.split("int WebServer::run()", 1)[1]
    fundamental_ui = source.split("function addFundamentalSummary()", 1)[1].split(
        "\nfunction fundamentalValue", 1
    )[0]
    popup_styles = source.split("id=\"fundamental-popup-style\"", 1)[1].split(
        "</style>", 1
    )[0]

    assert "if (forceRefresh) path.resize(queryStart)" in routing
    assert "if (queryStart != std::string::npos) path.resize(queryStart)" not in routing
    assert 'path.rfind("/api/fundamentals?symbol=", 0) == 0' in routing
    assert 'path.rfind("/api/stock-analysis?symbol=", 0) == 0' in routing
    assert 'path.rfind("/api/market-quotes?instrument_key=", 0) == 0' in routing
    assert 'path.rfind("/api/rsi?closes=", 0) == 0' in routing
    assert 'path.rfind("/api/market-quote/ohlc?instrument_key=", 0) == 0' in routing
    assert "table-layout:fixed" in popup_styles
    assert "overflow-wrap:anywhere" in popup_styles
    assert "releaseNoticeV2047Addendum()" in source


def test_deeper_analysis_deduplicates_symbols_and_keeps_ui_categories_consistent():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    analysis = source.split("std::string WebServer::runDeeperAnalysis() const", 1)[1].split(
        "\nbool WebServer::authenticated", 1
    )[0]
    renderer = source.split("const char* dashboardUiOptimization()", 1)[1].split(
        "\nconst char* dashboardResponsiveLayout()", 1
    )[0]
    page_composition = source.split("html.replace(html.find(marker)", 1)[1].split(
        "const std::string head", 1
    )[0]

    assert "uniqueAnalysisSymbols(liveSymbols)" in analysis
    assert 'platform::runPython(' in analysis
    assert '{inputPath, outputPath, "--json"}, false' in analysis
    assert "json::parse(resultText.str())" in analysis
    assert 'recommendations.emplace(symbol, recommendation)' in analysis
    assert '\"category\", category' in analysis
    assert "categoryStocks[category].push_back(symbol)" in analysis
    assert "marketValue += holding->marketValue()" in analysis
    assert "stocks.forEach(item=>{const label=order.includes(item.category)" in renderer
    assert "counts[label]++" in renderer
    assert "window.deeperCategories={counts,stocks:categoryStocks}" in renderer
    assert "<th>Category</th>" in renderer
    assert "names=categoryStocks[label]" in renderer
    assert "categoryEnhancements()" not in page_composition
    assert "releaseNoticeV2045Addendum()" in source


def test_operations_download_uses_safe_csv_from_authenticated_holdings():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    operations = source.split("const char* operationsWorkspace()", 1)[1].split(
        "const char* operationsDownload()", 1
    )[0]
    exporter = source.split("const char* operationsDownload()", 1)[1].split(
        "const char* responsiveDashboardNavigation()", 1
    )[0]

    assert 'data-operation="download"' in operations
    assert "data-download-portfolio" in operations
    assert "get('holdings','/api/holdings')" in exporter
    assert "text.replace(/\"/g,'\"\"')" in exporter
    assert "formula" in exporter or "numeric" in exporter
    assert "URL.revokeObjectURL(url)" in exporter


def test_refresh_distinguishes_expired_tokens_from_offline_fallback():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    optimization = source.split("const char* dashboardPageOptimization()", 1)[1].split(
        "\nconst char* releaseNoticeV2036Addendum()", 1
    )[0]
    refresh = source.split("function refreshView()", 1)[1].split(
        "const baseSafeJsonFetch", 1
    )[0]
    release = source.split("const char* releaseNoticeV2050Addendum()", 1)[1].split(
        "\nstd::string dashboardVersion()", 1
    )[0]
    session_recovery = source.split("const char* dashboardSessionRecovery()", 1)[1].split(
        "\nstd::string dashboardVersion()", 1
    )[0]
    snapshot = source.split(
        "std::shared_ptr<const WebServer::Snapshot> WebServer::snapshot(bool forceRefresh) const",
        1,
    )[1].split("\nint WebServer::run()", 1)[0]

    assert "'Unavailable · renew token':'Unavailable · live data offline'" in optimization
    assert "const originalFail=fail" in optimization
    assert "Renew UPSTOX_ACCESS_TOKEN and restart the dashboard." in optimization
    assert "window.summaryDashboard=async function()" in optimization
    assert ".metric-refresh" not in optimization
    assert "function refreshControls(disabled)" in source
    assert "button.textContent=button.id==='refresh-view'?'Refresh'" in source
    assert "Reliable refresh and token guidance · v2.0.51" in release
    assert "Refresh cannot renew the server-side credential" in release
    assert "response.redirected&&new URL(response.url,window.location.origin).pathname==='/login'" in session_recovery
    assert "window.location.replace('/login')" in session_recovery
    assert "dashboardSessionRecovery()" in source
    assert "Upstox access token expired or unauthorized; displaying the last successful portfolio snapshot." in snapshot


def test_patch_release_version_is_aligned_across_build_and_readmes():
    root = Path(__file__).parents[2]
    assert "VERSION 2.0.77" in (root / "CMakeLists.txt").read_text()
    assert 'version = "2.0.77"' in (root / "conanfile.py").read_text()
    assert 'version = "2.0.77"' in (root / "MODULE.bazel").read_text()
    assert r'PORTFOLIO_HEALTH_VERSION=\\\"2.0.77\\\"' in (root / "BUILD.bazel").read_text()
    assert "Version: 2.0.77" in (root / "README.md").read_text()
    assert "**Version:** `2.0.77`" in (root / "design" / "README.md").read_text()
    release77 = (root / "design" / "RELEASE_NOTES_V2.0.77.md").read_text()
    assert "unknown" in release77
    assert "market-holiday tooltip" in release77
    implementation77 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.77.md").read_text()
    assert "PORTFOLIO_HEALTH_VERSION" in implementation77
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "Windows dashboard display · v2.0.77" in source
    assert "white-space:normal!important;overflow-wrap:anywhere!important" in source
    release76 = (root / "design" / "RELEASE_NOTES_V2.0.76.md").read_text()
    assert ".env.production" in release76
    assert "tracked remain tracked" in release76
    implementation76 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.76.md").read_text()
    assert ".env.example" in implementation76
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "Windows ignore and credential hygiene · v2.0.76" in source
    assert "releaseNoticeV2076Addendum() + releaseNoticeV2077Addendum()" in source
    release75 = (root / "design" / "RELEASE_NOTES_V2.0.75.md").read_text()
    assert "denied for the executable" in release75.lower()
    implementation75 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.75.md").read_text()
    assert "output executable still running" in implementation75
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "Windows build diagnostics · v2.0.75" in source
    assert "releaseNoticeV2074Addendum() + releaseNoticeV2075Addendum()" in source
    release74 = (root / "design" / "RELEASE_NOTES_V2.0.74.md").read_text()
    assert "curl[websockets]" in release74
    assert "post-login release popup" in release74
    implementation74 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.74.md").read_text()
    assert "plain-curl installation" in implementation74
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "Windows live market feed · v2.0.74" in source
    assert "releaseNoticeV2073Addendum() + releaseNoticeV2074Addendum()" in source
    release73 = (root / "design" / "RELEASE_NOTES_V2.0.73.md").read_text()
    assert "ERR_CONNECTION_REFUSED" in release73
    assert "post-login release popup" in release73
    implementation73 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.73.md").read_text()
    assert "evaluating shell expressions" in implementation73
    assert "Windows web startup credentials · v2.0.73" in source
    assert "releaseNoticeV2072Addendum() + releaseNoticeV2073Addendum()" in source
    release72 = (root / "design" / "RELEASE_NOTES_V2.0.72.md").read_text()
    assert "run.bat --web" in release72
    assert "post-login release popup" in release72
    implementation72 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.72.md").read_text()
    assert "127.0.0.1" in implementation72
    assert "server-side" in implementation72
    assert "Windows web launcher · v2.0.72" in source
    assert "releaseNoticeV2071Addendum() + releaseNoticeV2072Addendum()" in source
    assert "# Implementation Summary v2.0.71" in (
        root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.71.md"
    ).read_text()
    release71 = (root / "design" / "RELEASE_NOTES_V2.0.71.md").read_text()
    assert "Windows build and launcher reliability" in release71
    assert "post-login release popup" in release71
    implementation71 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.71.md").read_text()
    assert "reparse points" in implementation71
    assert "read-only" in implementation71
    assert "Windows build and launcher reliability · v2.0.71" in source
    assert "releaseNoticeV2070Addendum() + releaseNoticeV2071Addendum()" in source
    release70 = (root / "design" / "RELEASE_NOTES_V2.0.70.md").read_text()
    assert "Windows, Linux, and macOS" in release70
    assert "login popup" in release70
    implementation70 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.70.md").read_text()
    assert "CreateProcessW" in implementation70
    assert "argument-based process creation" in implementation70
    release69 = (root / "design" / "RELEASE_NOTES_V2.0.69.md").read_text()
    assert "HTTP 502" in release69
    assert "login popup" in release69
    implementation69 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.69.md").read_text()
    assert "aria-busy" in implementation69
    assert "HTTP 502" in implementation69
    release68 = (root / "design" / "RELEASE_NOTES_V2.0.68.md").read_text()
    assert "task-scheduler" in release68
    assert "login popup" in release68
    implementation68 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.68.md").read_text()
    assert "cryptographic randomness" in implementation68
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "releaseNoticeV2068Addendum()" in source
    assert "Reliability and fail-closed safety · v2.0.68" in source
    assert "releaseNoticeV2067Addendum() + releaseNoticeV2068Addendum()" in source
    assert "releaseNoticeV2068Addendum() + releaseNoticeV2069Addendum()" in source
    assert "releaseNoticeV2069Addendum()" in source
    assert "Dashboard reliability and API error clarity · v2.0.69" in source
    assert "releaseNoticeV2069Addendum() + releaseNoticeV2070Addendum()" in source
    assert "Cross-platform runtime adapters · v2.0.70" in source
    assert "releaseNoticeV2052Addendum()" in source.split("html.replace(", 1)[1].split(
        "marker);", 1
    )[0]


def test_windows_batch_build_and_launcher_safeguards():
    root = Path(__file__).parents[2]
    build = (root / "buildCode.bat").read_text()
    launcher = (root / "run.bat").read_text()
    shell_build = (root / "buildCode.sh").read_text()

    assert "call :validate_build_dir" in build
    assert "BUILD_DIR must be a child of the project directory" in build
    assert "ReparsePoint" in build
    assert "while ($current" in build
    assert "pytest -q tests/python" in build
    assert "pytest -q tests/python" in shell_build
    assert '"name":"curl","features":["websockets"]' in build
    assert "curl[websockets]:%VCPKG_TRIPLET%" in build
    assert "curl[websockets]:x64-windows" in shell_build
    assert "WSS-enabled libcurl" in build
    assert "if not defined CURL_ROOT if defined VCPKG_ROOT if exist" not in launcher
    assert "PORTFOLIO_BAZEL_OUTPUT_DIR" in build
    assert "Get-Process -Name 'portfolio_health'" in build
    assert "Close the running application and retry." in build
    assert "never terminates" not in build
    assert "PORTFOLIO_STALE_CORE" in launcher
    assert "'src','include','adapters'" in launcher
    assert "-ExecutionPolicy Bypass -File" in launcher


def test_gitignore_protects_local_credentials_and_windows_build_state():
    root = Path(__file__).parents[2]
    ignored_paths = (
        ".env.production",
        ".env.local",
        ".upstox.env.backup",
        ".folio_login_code.bak",
        ".vs/WorkspaceState.json",
        "CMakeUserPresets.json",
        "cmake-build-debug/CMakeCache.txt",
        "CMakeFiles/TargetDirectories.txt",
    )
    for path in ignored_paths:
        result = subprocess.run(
            ["git", "check-ignore", "-q", "--", path],
            cwd=root,
            check=False,
        )
        assert result.returncode == 0, f"{path} should be ignored"

    for path in (".env.example", ".env.template"):
        result = subprocess.run(
            ["git", "check-ignore", "-q", "--", path],
            cwd=root,
            check=False,
        )
        assert result.returncode == 1, f"{path} should remain trackable"


def test_windows_web_launcher_waits_for_server_and_opens_browser():
    root = Path(__file__).parents[2]
    launcher = (root / "run.bat").read_text()
    web_runner = (root / "tools" / "run_web_ui.ps1").read_text()

    assert 'call powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT%\\tools\\run_web_ui.ps1"' in launcher
    assert "UPSTOX_API_KEY" in web_runner
    assert "UPSTOX_API_SECRET" in web_runner
    assert "UPSTOX_ACCESS_TOKEN" in web_runner
    assert "Import-PortfolioEnvironmentFile" in web_runner
    assert "$name -notin $allowedNames" in web_runner
    assert "ReparsePoint" in web_runner
    assert "127.0.0.1', 8080" in web_runner
    assert "Start-Process 'http://localhost:8080/'" in web_runner
    assert "within 20 seconds" in web_runner
    assert "RedirectStandardOutput = $true" in web_runner


def test_market_holiday_api_reports_unavailable_and_stale_data_explicitly():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    holidays = source.split("std::string WebServer::marketHolidays() const", 1)[1].split(
        "\nstd::shared_ptr<const WebServer::Snapshot>", 1
    )[0]
    route = source.split('path == "/api/market-holidays"', 1)[1].split(
        'path.rfind("/api/market-quotes?', 1
    )[0]
    calendar_ui = source.split("let marketHolidayData=[];", 1)[1].split(
        "\nfunction esc", 1
    )[0]

    assert 'staleCalendar["stale"] = true' in holidays
    assert 'throw std::runtime_error("Market holiday calendar is temporarily unavailable")' in holidays
    assert '{"status", "error"}' in route
    assert 'responseStatus = "502 Bad Gateway"' in route
    assert "Holiday calendar unavailable; regular NSE hours are estimates." in calendar_ui
    assert "payload.stale" in calendar_ui


def test_dashboard_views_expose_loading_state_without_stale_navigation_clearing_it():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    open_tab = source.split("async function openTab", 1)[1].split("\n", 1)[0]
    view_fetch = source.split("const getForDashboardView=get;", 1)[1].split(
        "\nasync function safeJsonFetch", 1
    )[0]

    assert "let activeRenderId=0;" in source
    assert "const renderId=activeRenderId,previous=cache[name],payload=await getForDashboardView(name,url)" in view_fetch
    assert "if(cache[name]===payload)" in view_fetch
    assert "throw Error('Dashboard view changed while data was loading')" in view_fetch
    assert "const renderId=++activeRenderId" in open_tab
    assert "view.setAttribute('aria-busy','true')" in open_tab
    assert "if(renderId===activeRenderId)fail(e)" in open_tab
    assert "if(renderId===activeRenderId)view.setAttribute('aria-busy','false')" in open_tab
    assert "if(renderId===activeRenderId)enhanceSortableTables()" in open_tab


def test_login_and_http_error_paths_fail_closed():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()

    assert "security::generateRandomString(32)" in source
    assert "if (session.empty())" in source
    assert "loginFailures_.size() >= 5" in source
    assert "429 Too Many Requests" in source
    assert "Retry-After: 60\\r\\n" in source
    assert '"404 Not Found"' in source
    assert '"405 Method Not Allowed"' in source
    assert '"502 Bad Gateway"' in source
    assert '"500 Internal Server Error"' in source
    assert "Allow: GET\\r\\n" in source


def test_unimplemented_integrations_do_not_claim_success():
    root = Path(__file__).parents[2]
    orders = (root / "src" / "OrdersAPI.cpp").read_text()
    notifications = (root / "src" / "NotificationService.cpp").read_text()
    models = (root / "src" / "MLModels.cpp").read_text()

    assert "Live order submission is not implemented; no order was sent." in orders
    assert "return false;" in orders
    assert "No configured notification channel" in notifications
    assert "return false;" in models
    release67 = (root / "design" / "RELEASE_NOTES_V2.0.67.md").read_text()
    assert "Removed the extra Overview Update/Refresh metric card" in release67
    assert "login popup" in release67
    implementation67 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.67.md").read_text()
    assert "event delegation" in implementation67
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "releaseNoticeV2067Addendum()" in source
    assert "Overview controls · v2.0.67" in source
    release66 = (root / "design" / "RELEASE_NOTES_V2.0.66.md").read_text()
    assert "HTTP 401" in release66
    assert "login popup" in release66
    implementation66 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.66.md").read_text()
    assert "DNS" in implementation66
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "releaseNoticeV2066Addendum()" in source
    assert "Refresh diagnostics · v2.0.66" in source
    release65 = (root / "design" / "RELEASE_NOTES_V2.0.65.md").read_text()
    assert "initial portfolio snapshot" in release65
    assert "login popup" in release65
    implementation65 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.65.md").read_text()
    assert "refreshLock.lock()" in implementation65
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "releaseNoticeV2065Addendum()" in source
    assert "Concurrent refresh recovery · v2.0.65" in source
    release64 = (root / "design" / "RELEASE_NOTES_V2.0.64.md").read_text()
    assert "retryable transport failures" in release64
    assert "login popup" in release64
    implementation64 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.64.md").read_text()
    assert "snapshotHoldingsTransportAttempts" in implementation64
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "releaseNoticeV2064Addendum()" in source
    assert "Refresh recovery · v2.0.64" in source
    release63 = (root / "design" / "RELEASE_NOTES_V2.0.63.md").read_text()
    assert "eye control beside Day P&L" in release63
    assert "login popup" in release63
    implementation63 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.63.md").read_text()
    assert "baran-capital-view-hide-day-pnl" in implementation63
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "releaseNoticeV2063Addendum()" in source
    assert "Overview privacy and layout · v2.0.63" in source
    release62 = (root / "design" / "RELEASE_NOTES_V2.0.62.md").read_text()
    assert "eye button" in release62
    assert "login popup" in release62
    implementation62 = (root / "design" / "IMPLEMENTATION_SUMMARY_V2.0.62.md").read_text()
    assert "localStorage" in implementation62
    source = (root / "src" / "WebServer.cpp").read_text()
    assert "releaseNoticeV2062Addendum()" in source
    assert "Overview privacy and controls · v2.0.62" in source
    release61 = (root / "design" / "RELEASE_NOTES_V2.0.61.md").read_text()
    assert "single 10-second transport attempt" in release61
    assert "login popup" in release61
    assert "releaseNoticeV2061Addendum()" in source
    assert "Snapshot request recovery · v2.0.61" in source
    release60 = (root / "design" / "RELEASE_NOTES_V2.0.60.md").read_text()
    assert "queued" in release60
    assert "login popup" in release60
    release59 = (root / "design" / "RELEASE_NOTES_V2.0.59.md").read_text()
    assert "response-body reading" in release59
    assert "login popup" in release59
    assert "releaseNoticeV2060Addendum()" in source
    assert "Refresh and retry reliability · v2.0.60" in source
    assert "releaseNoticeV2059Addendum()" in source
    assert "Manual refresh recovery · v2.0.59" in source
    release = (root / "design" / "RELEASE_NOTES_V2.0.58.md").read_text()
    assert "Updating `textContent` creates a child-list mutation" in release
    assert "login popup" in release
    conan_recipe = (root / "conanfile.py").read_text()
    assert 'self.requires("libcurl/8.22.0")' in conan_recipe
    assert '"libcurl/*:with_websockets": True' in conan_recipe


def test_alerts_observer_does_not_trigger_itself_repeatedly():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    alerts_ui = source.split("function normalizeAlertsControls()", 1)[1].split(
        "function normalizeDeeperCategories()", 1
    )[0]
    shared_view_observer = (
        "new MutationObserver(()=>{addSerialNumbers();normalizeRefreshMetric();"
        "normalizeDeeperAnalysis()}).observe(view,{childList:true})"
    )

    assert "heading&&heading.textContent.trim()!=='Portfolio news review'" in alerts_ui
    assert "new MutationObserver(normalizeAlertsControls).observe(view,{childList:true})" in alerts_ui
    assert "normalizeAlertsControls).observe(view,{childList:true,subtree:true})" not in alerts_ui
    assert shared_view_observer in source
    assert "releaseNoticeV2058Addendum()" in source


def test_live_quote_polling_is_bounded_and_retains_all_quote_updates():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    quotes_ui = source.split("const char* dashboardLiveFeedUi()", 1)[1].split(
        "std::string dashboardVersion()", 1
    )[0]
    composition = source.split("html.replace(html.find(marker)", 1)[1].split(
        "const std::string head", 1
    )[0]

    assert "if(quoteRefreshInFlight)return" in quotes_ui
    assert "setTimeout(()=>controller.abort(),8000)" in quotes_ui
    assert "signal:controller.signal" in quotes_ui
    assert "quoteRefreshController?.abort()" in quotes_ui
    assert "holdingsByKey.get(key)" in quotes_ui
    assert "updates.length>200" not in quotes_ui
    assert "releaseNoticeV2057Addendum()" in source
    assert "releaseNoticeV2052Addendum()" in composition


def test_stream_quotes_are_local_and_authorized_feed_url_is_not_returned():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    authorization = source.split('path == "/api/market-feed/authorize"', 1)[1].split(
        'path == "/api/stream/quotes"', 1
    )[0]
    quotes_ui = source.split("const char* dashboardLiveFeedUi()", 1)[1].split(
        "std::string dashboardVersion()", 1
    )[0]

    assert '"authorizedRedirectUri"' not in authorization
    assert '{"status", "success"}' in authorization
    assert 'fetch(\'/api/stream/quotes\'' in quotes_ui
    assert 'row.cells[row.cells.length-2]' in quotes_ui
    assert """view.querySelector('[data-instrument-key="'+CSS.escape(key)+'"]')""" in quotes_ui
    assert "Live market feed reconnecting" in quotes_ui
    assert '{"error", std::move(lastError)}' in source
    assert "if (lastError.size() > 180)" in source
    assert "data-instrument-key=\"'+esc(key)+'\">" in source
    assert "pnlCell.textContent" in quotes_ui
    streaming = (Path(__file__).parents[2] / "src" / "StreamingClient.cpp").read_text()
    sender = streaming.split("bool sendFrame(", 1)[1].split(
        "bool sendBinary(", 1
    )[0]
    assert "result != CURLE_AGAIN" in sender
    assert "platform::waitForSocket(" in sender
    assert "true, 250" in sender
    assert "WebSocket send timed out" in sender
    assert "supportsWebSocketProtocol()" in streaming
    assert 'installed libcurl lacks WSS support' in streaming
    assert "vcpkg: curl[websockets]" in streaming
    assert "releaseNoticeV2056Addendum()" in source


def test_web_server_has_no_unreachable_ui_code_and_consumes_json_parse_result():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    dashboard_optimization = source.split(
        "const char* dashboardUiOptimization()", 1
    )[1].split("\nconst char* dashboardResponsiveLayout()", 1)[0]
    validation = source.split("std::string ensureValidJson(", 1)[1].split(
        "\nstd::string response(", 1
    )[0]

    assert dashboard_optimization.count('return R"JS(') == 1
    assert "(void)json::parse(body);" in validation


def test_windows_bazel_build_detects_a_locked_output_without_stopping_it():
    build = (Path(__file__).parents[2] / "buildCode.bat").read_text()

    assert build.index("Get-Process -Name 'portfolio_health'") < build.index(
        'call "%BAZEL_EXE%" clean --expunge'
    )
    assert "Cannot rebuild the Bazel output while portfolio_health.exe is running" in build
    assert "Close the running application and retry." in build


def test_authenticated_requests_renew_the_sliding_session_cookie():
    root = Path(__file__).parents[2]
    source = (root / "src" / "WebServer.cpp").read_text()
    header = (root / "include" / "WebServer.hpp").read_text()
    authentication = source.split("bool WebServer::authenticated(", 1)[1].split(
        "\nstd::string WebServer::newSession()", 1
    )[0]
    new_session = source.split("std::string WebServer::newSession()", 1)[1].split(
        "\nvoid WebServer::removeSession", 1
    )[0]
    response = source.split("std::string response(const std::string& body,", 1)[1].split(
        "\nstd::string responseWithStatus", 1
    )[0]

    assert "std::unordered_map<std::string, std::chrono::steady_clock::time_point> sessions_" in header
    assert "if (found->second <= now)" in authentication
    assert "found->second = now + std::chrono::hours(1)" in authentication
    assert "sessions_[session] = now + std::chrono::hours(1)" in new_session
    assert "it = sessions_.erase(it)" in new_session
    assert "Max-Age=3600; HttpOnly; SameSite=Strict; Path=/" in source
    assert "extraHeaders" in response
    assert "responseStatus, body, \"application/json\", allowHeader + sessionCookie" in source
    assert 'path == "/api/session"' in source
    assert "json({{\"status\", \"ready\"}})" in source
    assert "setInterval(()=>{if(document.visibilityState==='visible')fetch('/api/session',{cache:'no-store'})" in source
    assert "10*60*1000" in source
    assert "releaseNoticeV2052Addendum()" in source
    assert "Long-running session stability · v2.0.52" in source


def test_raw_view_truncates_large_payload():
    source = (Path(__file__).parents[2] / "src" / "WebServer.cpp").read_text()
    # The raw JSON view now truncates large payloads in-browser and offers a download
    assert 'Download full payload' in source
