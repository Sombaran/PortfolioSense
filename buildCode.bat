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
if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\msys64"
set "BUILD_SYSTEM=cmake"
set "REBUILD=0"
set "SKIP_TESTS=0"
set "INVALID_ARGUMENT="

:parse_args
if "%~1"=="" goto parsed_args
if "%~1"=="1" (
  set "BUILD_SYSTEM=cmake"
  shift
  goto parse_args
)
if "%~1"=="2" (
  set "BUILD_SYSTEM=bazel"
  shift
  goto parse_args
)
if /I "%~1"=="--cmake" (
  set "BUILD_SYSTEM=cmake"
  shift
  goto parse_args
)
if /I "%~1"=="--bazel" (
  set "BUILD_SYSTEM=bazel"
  shift
  goto parse_args
)
if /I "%~1"=="--rebuild" (
  set "REBUILD=1"
  shift
  goto parse_args
)
if /I "%~1"=="--skip-tests" (
  set "SKIP_TESTS=1"
  shift
  goto parse_args
)
if /I "%~1"=="-h" goto usage
if /I "%~1"=="--help" goto usage
set "INVALID_ARGUMENT=%~1"
goto invalid_argument

:parsed_args
if "%BUILD_SYSTEM%"=="cmake" goto cmake_build
goto bazel_build

:cmake_build
set "MSYS_CMAKE=%MSYS2_ROOT%\ucrt64\bin\cmake.exe"
set "MSYS_NINJA=%MSYS2_ROOT%\ucrt64\bin\ninja.exe"
if not exist "%MSYS_CMAKE%" (
  echo ERROR: MSYS2 UCRT64 CMake was not found at "%MSYS_CMAKE%".
  echo Install MSYS2 UCRT64 CMake and set MSYS2_ROOT if MSYS2 is elsewhere.
  goto failed
)
if not exist "%MSYS_NINJA%" (
  echo ERROR: MSYS2 UCRT64 Ninja was not found at "%MSYS_NINJA%".
  echo Install mingw-w64-ucrt-x86_64-ninja from the UCRT64 terminal.
  goto failed
)

set "PATH=%MSYS2_ROOT%\ucrt64\bin;%MSYS2_ROOT%\usr\bin;%PATH%"
if "%REBUILD%"=="1" (
  call :validate_build_dir
  if errorlevel 1 goto failed
  if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
  if exist "%BUILD_DIR%" (
    echo ERROR: Could not remove build directory "%BUILD_DIR%".
    goto failed
  )
)

echo Configuring Windows CMake build in "%BUILD_DIR%"...
call "%MSYS_CMAKE%" -S "%ROOT%" -B "%BUILD_DIR%" -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH="%MSYS2_ROOT%\ucrt64" ^
  -DBUILD_TESTING=ON
if errorlevel 1 goto failed

echo Building portfolio_health...
call "%MSYS_CMAKE%" --build "%BUILD_DIR%" --parallel
if errorlevel 1 goto failed

if "%SKIP_TESTS%"=="1" goto success

echo Running C++ regression tests...
call "%MSYS_CMAKE%" -E env "PATH=%PATH%" ctest --test-dir "%BUILD_DIR%" --output-on-failure
if errorlevel 1 goto failed

call :run_python_tests
if errorlevel 1 goto failed
goto success

:validate_build_dir
set "PORTFOLIO_BUILD_ROOT=%ROOT%"
set "PORTFOLIO_BUILD_DIR=%BUILD_DIR%"
powershell.exe -NoProfile -Command "$root=[IO.Path]::GetFullPath($env:PORTFOLIO_BUILD_ROOT).TrimEnd('\'); $target=[IO.Path]::GetFullPath($env:PORTFOLIO_BUILD_DIR).TrimEnd('\'); if ($target -eq $root -or -not $target.StartsWith($root + '\',[StringComparison]::OrdinalIgnoreCase)) { Write-Error 'BUILD_DIR must be a child of the project directory for --rebuild.'; exit 2 }; $current=$target; while ($current -and $current.StartsWith($root + '\',[StringComparison]::OrdinalIgnoreCase)) { $item=Get-Item -LiteralPath $current -Force -ErrorAction SilentlyContinue; if ($item -and ($item.Attributes -band [IO.FileAttributes]::ReparsePoint)) { Write-Error 'Refusing to recursively remove a path containing a reparse point.'; exit 2 }; $current=[IO.Path]::GetDirectoryName($current) }"
if errorlevel 1 exit /b 1
exit /b 0

:bazel_build
if defined BAZEL_EXE (
  if exist "%BAZEL_EXE%" goto bazel_found
  echo ERROR: BAZEL_EXE does not point to an executable: "%BAZEL_EXE%".
  goto failed
)
for /f "delims=" %%B in ('where bazel 2^>nul') do if not defined BAZEL_EXE set "BAZEL_EXE=%%B"
if defined BAZEL_EXE goto bazel_found
for /f "delims=" %%B in ('where bazelisk 2^>nul') do if not defined BAZEL_EXE set "BAZEL_EXE=%%B"
if defined BAZEL_EXE goto bazel_found
echo ERROR: Bazel or Bazelisk was not found on PATH.
goto failed

:bazel_found
if not exist "%ROOT%\googletest_bazel9.patch" (
  echo ERROR: Required Bazel compatibility patch is missing: "%ROOT%\googletest_bazel9.patch".
  echo Restore this file from the repository before building with Bazel.
  goto failed
)
if not exist "%ROOT%\nlohmann_json_bazel9.patch" (
  echo ERROR: Required Bazel compatibility patch is missing: "%ROOT%\nlohmann_json_bazel9.patch".
  echo Restore this file from the repository before building with Bazel.
  goto failed
)
where cl >nul 2>nul
if not errorlevel 1 goto msvc_ready
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo ERROR: MSVC compiler was not found and vswhere.exe is unavailable.
  echo Install Visual Studio C++ Build Tools or run from a Developer Command Prompt.
  goto failed
)
set "VS_INSTALL="
for /f "usebackq tokens=*" %%V in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_INSTALL=%%V"
if not defined VS_INSTALL (
  echo ERROR: Visual Studio C++ Build Tools were not found.
  echo Install the MSVC x64/x86 build tools workload.
  goto failed
)
set "VS_DEV_CMD=%VS_INSTALL%\Common7\Tools\VsDevCmd.bat"
if not exist "%VS_DEV_CMD%" (
  echo ERROR: Visual Studio environment script was not found at "%VS_DEV_CMD%".
  goto failed
)
call "%VS_DEV_CMD%" -arch=x64 -host_arch=x64 -no_logo
if errorlevel 1 goto failed
where cl >nul 2>nul
if errorlevel 1 (
  echo ERROR: MSVC compiler was not available after initializing Visual Studio.
  goto failed
)

:msvc_ready
if not defined VCPKG_TRIPLET set "VCPKG_TRIPLET=x64-windows"
if defined CURL_ROOT goto curl_root_selected
if not defined VCPKG_INSTALL_ROOT (
  if not defined LOCALAPPDATA (
    echo ERROR: LOCALAPPDATA is not defined; set VCPKG_INSTALL_ROOT or CURL_ROOT.
    goto failed
  )
  set "VCPKG_INSTALL_ROOT=%LOCALAPPDATA%\baran-capital-view\vcpkg\installed"
)
if defined VCPKG_ROOT if exist "%VCPKG_ROOT%\vcpkg.exe" set "VCPKG_EXE=%VCPKG_ROOT%\vcpkg.exe"
if not defined VCPKG_EXE for /f "delims=" %%V in ('where vcpkg 2^>nul') do if not defined VCPKG_EXE set "VCPKG_EXE=%%V"
if not defined VCPKG_ROOT if defined VCPKG_EXE for %%I in ("%VCPKG_EXE%") do set "VCPKG_ROOT=%%~dpI"
if not defined VCPKG_EXE if exist "C:\vcpkg\vcpkg.exe" (
  set "VCPKG_EXE=C:\vcpkg\vcpkg.exe"
  if not defined VCPKG_ROOT set "VCPKG_ROOT=C:\vcpkg"
)
if not defined VCPKG_ROOT (
  echo ERROR: Windows Bazel builds need MSVC-compatible libcurl.
  echo Install vcpkg or set VCPKG_ROOT to its directory.
  echo Set CURL_ROOT to an existing triplet directory to skip automatic installation.
  goto failed
)
if not defined VCPKG_BASELINE for /f "delims=" %%B in ('git -C "%VCPKG_ROOT%" rev-parse HEAD 2^>nul') do set "VCPKG_BASELINE=%%B"
if not defined VCPKG_BASELINE if exist "%VCPKG_ROOT%\vcpkg-bundle.json" for /f "usebackq delims=" %%B in (`powershell.exe -NoProfile -Command "(Get-Content -Raw -LiteralPath '%VCPKG_ROOT%\vcpkg-bundle.json' | ConvertFrom-Json).embeddedsha"`) do set "VCPKG_BASELINE=%%B"
if not defined VCPKG_BASELINE (
  echo ERROR: Could not determine the vcpkg registry baseline.
  echo Set VCPKG_BASELINE to the registry commit supported by this vcpkg instance.
  goto failed
)
if not defined CURL_ROOT if exist "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\curl\curl.h" set "CURL_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%"
if not defined CURL_ROOT set "CURL_ROOT=%VCPKG_INSTALL_ROOT%\%VCPKG_TRIPLET%"
if exist "%CURL_ROOT%\include\curl\curl.h" if exist "%CURL_ROOT%\lib\libcurl.lib" goto curl_root_selected
if not defined VCPKG_EXE (
  echo ERROR: MSVC-compatible libcurl was not found under "%CURL_ROOT%".
  echo vcpkg.exe was not found to install curl in manifest mode.
  echo Add vcpkg to PATH, set VCPKG_ROOT, or provide a complete CURL_ROOT.
  echo Or set CURL_ROOT to a triplet containing include\curl\curl.h and lib\libcurl.lib.
  goto failed
)
echo Installing MSVC-compatible libcurl for triplet %VCPKG_TRIPLET%...
set "VCPKG_MANIFEST_DIR=%TEMP%\baran-capital-view-vcpkg-%RANDOM%-%RANDOM%"
mkdir "%VCPKG_MANIFEST_DIR%" >nul 2>nul
if errorlevel 1 (
  echo ERROR: Could not create temporary vcpkg manifest directory "%VCPKG_MANIFEST_DIR%".
  goto failed
)
>"%VCPKG_MANIFEST_DIR%\vcpkg.json" echo {"name":"portfolio-health-build","version-string":"1.0.0","builtin-baseline":"%VCPKG_BASELINE%","dependencies":["curl"]}
pushd "%VCPKG_MANIFEST_DIR%" >nul
if errorlevel 1 (
  rmdir /s /q "%VCPKG_MANIFEST_DIR%"
  echo ERROR: Could not enter the temporary vcpkg manifest directory.
  goto failed
)
call "%VCPKG_EXE%" install "--triplet=%VCPKG_TRIPLET%" "--x-install-root=%VCPKG_INSTALL_ROOT%" --no-print-usage
set "VCPKG_INSTALL_RESULT=%ERRORLEVEL%"
popd
rmdir /s /q "%VCPKG_MANIFEST_DIR%"
if not "%VCPKG_INSTALL_RESULT%"=="0" (
  echo ERROR: vcpkg could not install curl:%VCPKG_TRIPLET%.
  goto failed
)
:curl_root_selected
if not defined CURL_ROOT (
  echo ERROR: Windows Bazel builds need MSVC-compatible libcurl.
  echo Install it with vcpkg and set VCPKG_ROOT or CURL_ROOT.
  goto failed
)
if not exist "%CURL_ROOT%\include\curl\curl.h" (
  echo ERROR: libcurl headers were not found under "%CURL_ROOT%".
  goto failed
)
if not exist "%CURL_ROOT%\lib\libcurl.lib" (
  echo ERROR: libcurl.lib was not found under "%CURL_ROOT%\lib".
  goto failed
)
if exist "%ROOT%\build_support\vcpkg_curl\include\curl" rmdir /s /q "%ROOT%\build_support\vcpkg_curl\include\curl"
if exist "%ROOT%\build_support\vcpkg_curl\include\curl" (
  echo ERROR: Could not clear the previous staged libcurl headers.
  goto failed
)
mkdir "%ROOT%\build_support\vcpkg_curl\include\curl"
if errorlevel 1 (
  echo ERROR: Could not create the Bazel curl header staging directory.
  goto failed
)
xcopy /E /I /Y "%CURL_ROOT%\include\curl" "%ROOT%\build_support\vcpkg_curl\include\curl" >nul
if errorlevel 1 (
  echo ERROR: Could not stage libcurl headers from "%CURL_ROOT%\include\curl".
  goto failed
)
if not exist "%ROOT%\build_support\vcpkg_curl\include\curl\curl.h" (
  echo ERROR: curl.h was not staged for the Bazel build.
  goto failed
)
if not exist "%ROOT%\build_support\vcpkg_curl\lib" mkdir "%ROOT%\build_support\vcpkg_curl\lib"
if errorlevel 1 (
  echo ERROR: Could not create the Bazel curl library staging directory.
  goto failed
)
copy /Y "%CURL_ROOT%\lib\libcurl.lib" "%ROOT%\build_support\vcpkg_curl\lib\libcurl.lib" >nul
if errorlevel 1 (
  echo ERROR: Could not stage libcurl.lib from "%CURL_ROOT%\lib".
  goto failed
)
if not exist "%ROOT%\build_support\vcpkg_curl\lib\libcurl.lib" (
  echo ERROR: libcurl.lib was not staged for the Bazel build.
  goto failed
)
set "LIB=%CURL_ROOT%\lib;%LIB%"
set "PATH=%CURL_ROOT%\bin;%PATH%"
if "%REBUILD%"=="1" (
  call "%BAZEL_EXE%" clean --expunge
  if errorlevel 1 goto failed
)

echo Building Windows Bazel target...
call "%BAZEL_EXE%" build //:portfolio_health ^
  --action_env=LIB
if errorlevel 1 goto failed
if "%SKIP_TESTS%"=="1" goto success

echo Running Bazel C++ regression tests...
call "%BAZEL_EXE%" test //:portfolio_health_tests --test_output=errors ^
  --action_env=LIB ^
  --test_env=PATH
if errorlevel 1 goto failed

call :run_python_tests
if errorlevel 1 goto failed
goto success

:run_python_tests
set "PYTHONUTF8=1"
where py >nul 2>nul
if not errorlevel 1 goto run_python_py
where python >nul 2>nul
if not errorlevel 1 goto run_python
echo ERROR: Python was not found; install Python and pytest, or use --skip-tests.
exit /b 1

:run_python_py
echo Running Python regression tests; Bash-launch integration cases are skipped on Windows.
py -3 -m pytest -q tests/python -k "not test_web_launch_captures_cpp_output_in_private_log and not test_non_web_launch_is_not_redirected and not test_fresh_shell_token_overrides_stale_env_file and not test_launcher_rebuilds_when_source_is_newer_than_the_binary and not test_launcher_rebuild_option_is_consumed_before_forwarding_arguments"
exit /b %ERRORLEVEL%

:run_python
echo Running Python regression tests; Bash-launch integration cases are skipped on Windows.
python -m pytest -q tests/python -k "not test_web_launch_captures_cpp_output_in_private_log and not test_non_web_launch_is_not_redirected and not test_fresh_shell_token_overrides_stale_env_file and not test_launcher_rebuilds_when_source_is_newer_than_the_binary and not test_launcher_rebuild_option_is_consumed_before_forwarding_arguments"
exit /b %ERRORLEVEL%

:invalid_argument
echo ERROR: Unknown argument "%INVALID_ARGUMENT%".
goto usage_error

:usage
echo Usage: buildCode.bat [1^|2] [--rebuild] [--skip-tests] [--cmake^|--bazel]
echo   1, --cmake  Build with MSYS2 UCRT64 CMake and Ninja.
echo   2, --bazel  Build with Bazel and the native MSVC toolchain.
echo   --rebuild   Remove selected output; CMake paths must stay inside the project.
echo   --skip-tests Build targets without running C++ or Python tests.
echo.
echo CMake default: build-windows
echo Override MSYS2_ROOT, BUILD_DIR, CURL_ROOT, VCPKG_ROOT, VCPKG_INSTALL_ROOT, VCPKG_TRIPLET, VCPKG_BASELINE, or BAZEL_EXE as needed.
echo Bazel requires MSVC C++ Build Tools and MSVC-compatible curl from vcpkg.
goto finished

:usage_error
echo Usage: buildCode.bat [1^|2] [--rebuild] [--skip-tests] [--cmake^|--bazel]
set "RESULT=2"
goto finish

:success
set "RESULT=0"
echo Build completed successfully.
goto finish

:failed
set "RESULT=1"
echo Build failed.
goto finish

:finished
set "RESULT=0"

:finish
popd
exit /b %RESULT%
