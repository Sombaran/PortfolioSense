@echo off
setlocal EnableExtensions

set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
pushd "%ROOT%" >nul
if errorlevel 1 (
  echo ERROR: Could not enter the project directory.
  exit /b 1
)

if not defined BUILD_DIR set "BUILD_DIR=build-windows"
if not defined BAZEL_BIN_DIR set "BAZEL_BIN_DIR=bazel-bin"
set "BUILD_SYSTEM=cmake"
set "BUILD_SYSTEM_EXPLICIT=0"
set "WEB_MODE=0"
set "REBUILD=0"

:parse_launcher_options
if /I "%~1"=="--cmake" (
  set "BUILD_SYSTEM=cmake"
  set "BUILD_SYSTEM_EXPLICIT=1"
  shift
  goto parse_launcher_options
)
if "%~1"=="1" (
  set "BUILD_SYSTEM=cmake"
  set "BUILD_SYSTEM_EXPLICIT=1"
  shift
  goto parse_launcher_options
)
if /I "%~1"=="--bazel" (
  set "BUILD_SYSTEM=bazel"
  set "BUILD_SYSTEM_EXPLICIT=1"
  shift
  goto parse_launcher_options
)
if "%~1"=="2" (
  set "BUILD_SYSTEM=bazel"
  set "BUILD_SYSTEM_EXPLICIT=1"
  shift
  goto parse_launcher_options
)
if /I "%~1"=="--rebuild" (
  set "REBUILD=1"
  shift
  goto parse_launcher_options
)

for %%A in (%*) do if /I "%%~A"=="--web" set "WEB_MODE=1"
if "%WEB_MODE%"=="1" if "%BUILD_SYSTEM_EXPLICIT%"=="0" call :select_web_build

if "%BUILD_SYSTEM%"=="bazel" goto build_bazel

if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\msys64"
if not exist "%MSYS2_ROOT%\ucrt64\bin" (
  echo ERROR: MSYS2 UCRT64 runtime directory was not found at "%MSYS2_ROOT%\ucrt64\bin".
  echo Set MSYS2_ROOT to the MSYS2 installation directory.
  goto failed
)
set "PATH=%MSYS2_ROOT%\ucrt64\bin;%MSYS2_ROOT%\usr\bin;%PATH%"
set "EXECUTABLE=%BUILD_DIR%\portfolio_health.exe"
set "CORE_LIBRARY=%BUILD_DIR%\libportfolio_health_core.dll"
set "NEEDS_BUILD=0"

if not exist "%EXECUTABLE%" (
  set "NEEDS_BUILD=1"
) else if not exist "%CORE_LIBRARY%" (
  set "NEEDS_BUILD=1"
) else if "%REBUILD%"=="1" (
  set "NEEDS_BUILD=1"
) else (
  call :check_stale
  if errorlevel 1 set "NEEDS_BUILD=1"
)

if "%NEEDS_BUILD%"=="1" (
  if "%REBUILD%"=="1" (
    call "%ROOT%\buildCode.bat" --cmake --rebuild --skip-tests
  ) else (
    echo Building portfolio_health because source files changed or the executable is missing.
    call "%ROOT%\buildCode.bat" --cmake --skip-tests
  )
  if errorlevel 1 goto failed
)
goto launcher_ready

:build_bazel
if not defined VCPKG_TRIPLET set "VCPKG_TRIPLET=x64-windows"
if not defined VCPKG_INSTALL_ROOT if defined LOCALAPPDATA set "VCPKG_INSTALL_ROOT=%LOCALAPPDATA%\baran-capital-view\vcpkg\installed"
if not defined CURL_ROOT if defined VCPKG_ROOT if exist "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\curl\curl.h" if exist "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\lib\libcurl.lib" set "CURL_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%"
if defined CURL_ROOT set "PATH=%CURL_ROOT%\bin;%PATH%"
set "EXECUTABLE=%BAZEL_BIN_DIR%\portfolio_health.exe"
if "%REBUILD%"=="1" (
  call "%ROOT%\buildCode.bat" --bazel --rebuild --skip-tests
) else (
  call "%ROOT%\buildCode.bat" --bazel --skip-tests
)
if errorlevel 1 goto failed
if not defined CURL_ROOT if defined VCPKG_INSTALL_ROOT set "CURL_ROOT=%VCPKG_INSTALL_ROOT%\%VCPKG_TRIPLET%"
if defined CURL_ROOT set "PATH=%CURL_ROOT%\bin;%PATH%"
if not exist "%EXECUTABLE%" (
  echo ERROR: Bazel build succeeded but the executable was not found at "%EXECUTABLE%".
  echo Set BAZEL_BIN_DIR if your Bazel configuration uses a different output path.
  goto failed
)

:launcher_ready
if not defined FOLIO_LOGIN_CODE if exist "%ROOT%\.folio_login_code" (
  <"%ROOT%\.folio_login_code" set /p "FOLIO_LOGIN_CODE="
)

for %%A in (%*) do if /I "%%~A"=="--web" goto web_mode
goto launch

:select_web_build
if defined BAZEL_EXE (
  if exist "%BAZEL_EXE%" (
    set "BUILD_SYSTEM=bazel"
    exit /b 0
  )
)
for /f "delims=" %%B in ('where bazel 2^>nul') do if not defined BAZEL_EXE set "BAZEL_EXE=%%B"
if defined BAZEL_EXE (
  set "BUILD_SYSTEM=bazel"
  exit /b 0
)
for /f "delims=" %%B in ('where bazelisk 2^>nul') do if not defined BAZEL_EXE set "BAZEL_EXE=%%B"
if defined BAZEL_EXE set "BUILD_SYSTEM=bazel"
exit /b 0

:web_mode
if not defined PORTFOLIO_WEB_LOG (
  if not defined LOCALAPPDATA (
    echo ERROR: LOCALAPPDATA is not defined; set PORTFOLIO_WEB_LOG to a log path.
    goto failed
  )
  set "PORTFOLIO_WEB_LOG=%LOCALAPPDATA%\baran-capital-view\web-ui.log"
)
for %%I in ("%PORTFOLIO_WEB_LOG%") do set "WEB_LOG_DIR=%%~dpI"
if not exist "%WEB_LOG_DIR%" mkdir "%WEB_LOG_DIR%"
if errorlevel 1 (
  echo ERROR: Could not create the web log directory "%WEB_LOG_DIR%".
  goto failed
)
echo Capturing web logs in "%PORTFOLIO_WEB_LOG%"
echo Starting Portfolio Health Web UI at http://127.0.0.1:8080
set "PYTHONUNBUFFERED=1"
call "%EXECUTABLE%" %1 %2 %3 %4 %5 %6 %7 %8 %9 >>"%PORTFOLIO_WEB_LOG%" 2>&1
set "RESULT=%ERRORLEVEL%"
goto finish

:launch
call "%EXECUTABLE%" %1 %2 %3 %4 %5 %6 %7 %8 %9
set "RESULT=%ERRORLEVEL%"
goto finish

:check_stale
set "PORTFOLIO_STALE_EXE=%EXECUTABLE%"
set "PORTFOLIO_STALE_CORE=%CORE_LIBRARY%"
powershell.exe -NoProfile -Command "$outputs=@($env:PORTFOLIO_STALE_EXE,$env:PORTFOLIO_STALE_CORE); if ($outputs | Where-Object { -not (Test-Path -LiteralPath $_) }) { exit 1 }; $threshold=($outputs | ForEach-Object { Get-Item -LiteralPath $_ } | Measure-Object -Property LastWriteTimeUtc -Minimum).Minimum; $paths=@('run.bat','buildCode.bat','CMakeLists.txt','conanfile.py','MODULE.bazel','BUILD.bazel','stock_alert_nlp.py'); $files=@($paths | Where-Object { Test-Path -LiteralPath $_ } | ForEach-Object { Get-Item -LiteralPath $_ }); foreach ($dir in @('src','include','adapters')) { if (Test-Path -LiteralPath $dir) { $files+=Get-ChildItem -LiteralPath $dir -File -Recurse } }; if ($files | Where-Object { $_.LastWriteTimeUtc -gt $threshold } | Select-Object -First 1) { exit 1 }; exit 0" >nul
exit /b %ERRORLEVEL%

:failed
set "RESULT=1"

:finish
popd
exit /b %RESULT%
