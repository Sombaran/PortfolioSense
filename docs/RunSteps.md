# Run Steps

Step-by-step walkthrough — from a fresh checkout to a live health
report.

---

## 1. Install prerequisites

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential cmake libcurl4-openssl-dev
```

### RHEL / CentOS / Fedora

```bash
sudo dnf install -y gcc-c++ cmake libcurl-devel
```

### macOS (Homebrew)

```bash
brew install cmake curl
```

### Windows (MSYS2 UCRT64)

Open the **UCRT64** terminal from MSYS2, then install the matching native
toolchain and libraries:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-openssl \
  mingw-w64-ucrt-x86_64-nlohmann-json mingw-w64-ucrt-x86_64-gtest
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/ucrt64 -DBUILD_TESTING=OFF
cmake --build build
```

The native Windows batch helper can run from Command Prompt or PowerShell.
It uses MSYS2 UCRT64 CMake/Ninja for option 1; set `MSYS2_ROOT` if MSYS2 is
installed somewhere other than `C:\msys64`:

```bat
buildCode.bat 1
```

CMake uses the MSYS2 compiler and installed UCRT64 packages directly, without
Conan. GoogleTest is optional; install its UCRT64 package to build the C++
tests. Five Python integration tests that spawn Bash are skipped on Windows;
the Linux workflow continues to run the complete Python suite. The Bazel build
uses the native Windows MSVC toolchain, so it also
requires Visual Studio C++ Build Tools and an MSVC-compatible libcurl. The
helper initializes the x64 MSVC environment automatically when Visual Studio
is installed. When vcpkg is on `PATH`, configured in `VCPKG_ROOT`, or installed
at `C:\vcpkg`, the helper installs `curl[websockets]` in manifest mode,
including for vcpkg distributions that disable classic mode. This also upgrades
an existing plain-curl installation to the WSS-enabled feature. Installed
packages go under `%LOCALAPPDATA%\baran-capital-view\vcpkg\installed` by
default, avoiding permission errors when vcpkg itself is under Visual Studio.
The temporary manifest uses the vcpkg checkout commit or Visual Studio
vcpkg bundle's embedded registry commit as `builtin-baseline`; set
`VCPKG_BASELINE` only when your vcpkg instance needs a specific registry
baseline. Set `VCPKG_INSTALL_ROOT` to choose another writable install
location, `VCPKG_TRIPLET` to use a different triplet, or `CURL_ROOT` to use
an existing triplet directory containing WSS-enabled `include\curl\curl.h`
and `lib\libcurl.lib`. Bazel or Bazelisk must be on `PATH`; alternatively, set
`BAZEL_EXE` to its full executable path:

```bat
buildCode.bat 2
```

The Bazel module declares `platforms` directly for its OS-specific target
selection. Checked-in Bazel 9 compatibility patches for GoogleTest and
nlohmann/json load C++ rules through `rules_cc` and declare that dependency in
each patched module, so the rules are resolvable from those modules.
The Bazel batch helper stages vcpkg's curl headers and import library into the
ignored `build_support/vcpkg_curl` workspace directory, where Bazel declares
them as target inputs. This avoids passing external absolute paths to compiler
and linker flags, which Bazel rejects outside the execution root.
The current Bazel target produces `bazel-bin\portfolio_health.exe`; it does
not define a project DLL target. The CMake build does create
`libportfolio_health_core.dll` because that build declares the core as a shared
library. With vcpkg's dynamic triplet, libcurl's own DLLs are runtime
dependencies and are not the project's DLL output.

CMake selects the native runtime adapter from `adapters/windows`,
`adapters/linux`, or `adapters/macos`. The Bash-based `run.sh` and
`buildCode.sh` remains the Linux/macOS helper. Use `buildCode.bat` from
Command Prompt or PowerShell on Windows. `run.bat` builds when the executable
is missing or stale, then launches it and forwards command-line arguments.
The CMake launcher also checks for the core DLL and watches platform-adapter
sources when deciding whether to rebuild. `buildCode.bat --rebuild` only
removes a build directory inside the project and refuses paths that cross a
Windows reparse point. Python regression collection is limited to
`tests\python`, so Bazel's generated dependency trees are not collected.
Python must be available on `PATH` for the Python-backed analysis
(`python.exe` on Windows, `python3` on Linux/macOS).

---

## 2. Build

### Option A — one-shot script

```bash
cd /home/ritup2404/baran-capital-view
./run.sh
```

`run.sh` builds and then picks an input source in this order:

1. Explicit CLI args (`./run.sh --portfolio path.csv`).
2. `MY_PORTFOLIO` env var pointing at a file.
3. Local `config/my_portfolio.csv` if present (default when you first clone).
4. `UPSTOX_ACCESS_TOKEN` → live account.
5. Fallback: offline demo against `config/sample_positions.json`.

On Windows, use the matching batch launcher from Command Prompt or PowerShell.
It defaults to CMake. For `--web`, it automatically uses Bazel when Bazel or
Bazelisk is available; use `--cmake` to force CMake or `--bazel` to force
Bazel. The web server output is appended to the per-user log:

```bat
run.bat
run.bat --web
run.bat --cmake --web
run.bat --portfolio config\my_portfolio.csv
run.bat --rebuild --web
```

`run.bat` uses `build-windows\portfolio_health.exe` for CMake and
`bazel-bin\portfolio_health.exe` for Bazel by default. Set `BUILD_DIR` or
`BAZEL_BIN_DIR` to select another output directory. For Bazel, `VCPKG_ROOT` or
`CURL_ROOT` must be set as described above so the libcurl runtime DLLs are on
`PATH`. Web logs go to `%LOCALAPPDATA%\baran-capital-view\web-ui.log`; set
`PORTFOLIO_WEB_LOG` to choose another path. Set `FOLIO_LOGIN_CODE`,
`UPSTOX_API_KEY`, `UPSTOX_API_SECRET`, and `UPSTOX_ACCESS_TOKEN` in the
environment when needed. With `--web`, the launcher waits for the local server
to accept connections before opening `http://localhost:8080/` in the default
browser; keep the launcher console open while using the web UI.
On Windows, credentials can be set in the environment or in
`%USERPROFILE%\.upstox.env` using simple `NAME=value` or `export NAME=value`
lines. The batch launcher reads only `UPSTOX_API_KEY`, `UPSTOX_API_SECRET`,
`UPSTOX_ACCESS_TOKEN`, and `FOLIO_LOGIN_CODE`; it does not execute the file.
Keep that per-user file protected from access by other Windows accounts.

### Option B — manual CMake

```bash
cd /home/ritup2404/baran-capital-view
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Artefact: `build/portfolio_health`.

### Optional CMake flags

| Flag                                     | Effect                                              |
| ---------------------------------------- | --------------------------------------------------- |
| `-DCMAKE_BUILD_TYPE=Debug`               | Debug symbols, no optimisations.                     |
| `-DPORTFOLIO_HEALTH_ENABLE_COVERAGE=ON`  | GCC/Clang gcov instrumentation for coverage reports. |
| `-DPORTFOLIO_HEALTH_FETCH_JSON=ON`       | Ignore the vendored header, pull from GitHub.        |
| `-DCMAKE_TOOLCHAIN_FILE=<vcpkg.cmake>`   | Use vcpkg for libcurl on Windows.                    |

### Coverage reports

Install `lcov` for the percentage summary and HTML report. On Debian or
Ubuntu, use `sudo apt install -y lcov`; on Fedora or RHEL, use
`sudo dnf install -y lcov`. Then configure a separate instrumented build:

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug \
   -DPORTFOLIO_HEALTH_ENABLE_COVERAGE=ON
cmake --build build-coverage --target coverage
```

The `coverage` target runs CTest, prints the lcov coverage percentage, and
writes the browsable report to
`build-coverage/coverage/html/index.html`. To generate raw per-source gcov
files without lcov, run:

```bash
cmake --build build-coverage --target coverage-gcov
```

Those files are written to `build-coverage/coverage/gcov`.

The equivalent script options are:

```bash
./buildCode.sh 1 --coverage
./buildCode.sh 1 --coverage-gcov
./buildCode.sh 1 --coverage --coverage-build-dir /tmp/baran-coverage
```

---

## 3. Try it offline first

Verify the binary is happy against the bundled sample payload:

```bash
./build/portfolio_health --file config/sample_positions.json
```

Expected: a fixed-width report with score `54 / 100`, four open
positions, and one high-concentration alert.

JSON mode for tooling / dashboards:

```bash
./build/portfolio_health --file config/sample_positions.json --json | jq .score
# → 54
```

---

## 3.5 Analyse **your own** portfolio

1. Open `config/my_portfolio.csv` (or copy it somewhere else) and describe
   your holdings:

   ```csv
   symbol,quantity,average_price,last_price
   INFY,50,1500.00,1525.00
   RELIANCE,30,2450.00,2510.00
   ITC,200,420.00,445.00
   ```

   * `symbol`, `quantity`, `average_price` are mandatory.
   * `last_price` is optional — if you skip it, P&L for that row will
     read as 0 (see §3.6 to fetch it automatically).
   * Full column list & JSON variant:
     [docs/API_Reference.md#3-portfolio-file-format](API_Reference.md#3-portfolio-file-format).

2. Run the analysis:

   ```bash
   ./build/portfolio_health --portfolio config/my_portfolio.csv
   ./build/portfolio_health --portfolio config/my_portfolio.json --json
   ```

3. Or point `run.sh` at a file anywhere on disk:

   ```bash
   MY_PORTFOLIO=/data/prod_book.csv ./run.sh
   ```

## 3.6 Auto-fill last prices from Upstox

If you'd rather not maintain `last_price` by hand, add an
`instrument_key` column (Upstox instrument key, e.g.
`NSE_EQ|INE009A01021`) and pass `--fetch-ltp`:

```bash
export UPSTOX_ACCESS_TOKEN="eyJhbGciOi..."
./build/portfolio_health --portfolio config/my_portfolio.csv --fetch-ltp
```

The tool issues a single batched
`GET /v2/market-quote/ltp?instrument_key=...` call and updates the LTP
of every returned row before analytics run. Rows with no
`instrument_key`, or where the API returns no price, keep whatever
`last_price` you supplied (or the average-price default).

---

## 4. Get an Upstox access token

1. Sign in to the [Upstox developer console](https://account.upstox.com/developer/apps)
   and create an app. Note the **API Key** and **API Secret**.
2. Follow the
   [OAuth 2.0 flow](https://upstox.com/developer/api-documentation/authentication):
   1. Direct the user to
      `https://api.upstox.com/v2/login/authorization/dialog?client_id=<API_KEY>&redirect_uri=<REDIRECT>&response_type=code`.
   2. Exchange the returned `code` for an access token via
      `POST https://api.upstox.com/v2/login/authorization/token`.
3. The response contains `access_token`. Access tokens are valid until
   03:30 IST on the next trading day. This application reads a supplied
   access token but does not implement the OAuth callback or refresh-token
   exchange; obtain and replace the token through Upstox after it expires.

Export the token in the shell where you'll run the CLI:

```bash
export UPSTOX_ACCESS_TOKEN="eyJhbGciOi..."
```

**Never commit the token, never pass it on the CLI, never log it.**
This tool reads it from the environment only.

---

## 5. Run live

Formatted report:

```bash
./build/portfolio_health
```

Raw JSON body (useful when reporting issues):

```bash
./build/portfolio_health --raw > positions_$(date +%F).json
```

Machine-readable analytics:

```bash
./build/portfolio_health --json | tee health_$(date +%F).json
```

Behind a custom base URL (sandbox, corporate proxy, …):

```bash
./build/portfolio_health --base https://api-sandbox.upstox.com
```

---

## 6. Cron / systemd example

Snapshot the health report every 15 minutes during market hours:

```cron
*/15 9-15 * * 1-5   . $HOME/.upstox.env && \
                    /opt/portfolio_health/build/portfolio_health --json \
                    >> /var/log/portfolio_health.jsonl
```

Update the report-shaped holdings CSV after the NSE session closes:

```cron
30 15 * * 1-5 cd /opt/portfolio_health && . $HOME/.upstox.env && ./build/portfolio_health --update-holding config/holding.csv >> /var/log/portfolio_health-holdings.log 2>&1
```

`$HOME/.upstox.env` should be a `chmod 600` file that does
`export UPSTOX_ACCESS_TOKEN=...` and is updated whenever the access token
expires. No refresh-token automation is implemented by this application.

---

## 7. Troubleshooting

| Symptom                                            | Likely cause & fix                                                                                     |
| -------------------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| `UPSTOX_ACCESS_TOKEN is not set`                   | Export the token (see §4) — or use `--portfolio <file>` / `--file <file>` (no token needed).           |
| `--fetch-ltp requires UPSTOX_ACCESS_TOKEN`         | Export the token before passing `--fetch-ltp`.                                                          |
| `CSV: required columns missing ...`                | Add `symbol,quantity,average_price` header row to your CSV.                                             |
| `CSV row N: stod: no conversion` (or similar)      | Non-numeric value in a numeric column. Check quotes / decimal separator (use `.`).                      |
| `warning: row N: last_price missing ...`           | Fill `last_price` in the file, or add `instrument_key` + pass `--fetch-ltp`.                            |
| `LTP fill failed: HTTP 401 from Upstox LTP`        | Token expired / wrong. Rerun OAuth.                                                                     |
| `HTTP 401 from Upstox: ...`                        | Token expired or wrong. Rerun the OAuth flow.                                                          |
| `HTTP 403 from Upstox: ...`                        | The account doesn't have API access, or the app is disabled in the developer console.                  |
| `network error: Could not resolve host: ...`       | Offline / firewalled machine. Use `--portfolio` or `--file` mode instead.                              |
| `network error: SSL certificate problem`           | System CA bundle is stale. Update `ca-certificates`.                                                    |
| Build error: `CURL not found`                      | Install the `-dev` / `-devel` package (see §1).                                                        |
| Build error: `nlohmann/json.hpp: No such file`     | Vendored copy missing — reclone, or pass `-DPORTFOLIO_HEALTH_FETCH_JSON=ON`.                           |
| Numbers look wrong (locale)                        | Not a locale issue — INR grouping is hand-rolled. File a bug with the raw payload attached (`--raw`).   |

---

## 8. Clean up

```bash
rm -rf build
```

The tool writes no files, no config, no cache. There is nothing else to
clean up.
