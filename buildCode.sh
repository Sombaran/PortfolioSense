#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

host_os="$(uname -s)"
case "$host_os" in
  MINGW*|MSYS*|CYGWIN*)
    windows_host=1
    default_build_dir="build-windows"
    ;;
  *)
    windows_host=0
    default_build_dir="build"
    ;;
esac

build_dir="${BUILD_DIR:-$default_build_dir}"
build_system="${BUILD_SYSTEM:-cmake}"
rebuild=0
skip_tests=0
coverage=0
coverage_target="coverage"
coverage_build_dir_set=0
build_jobs="${BUILD_JOBS:-}"

if [[ -z "$build_jobs" ]]; then
  if command -v nproc >/dev/null 2>&1; then
    build_jobs="$(nproc)"
  elif command -v getconf >/dev/null 2>&1; then
    build_jobs="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
  else
    build_jobs=2
  fi
fi

usage() {
  cat <<'EOF'
Usage: ./buildCode.sh [1|2] [--rebuild] [--skip-tests] [--coverage|--coverage-gcov] [--cmake|--bazel]
  1 = CMake build
  2 = Bazel build
  --rebuild = remove stale build artifacts before building
  --skip-tests = build only; do not run regression tests
  --coverage = instrument a Debug build and generate the lcov HTML report
  --coverage-gcov = instrument a Debug build and generate raw gcov reports
  --coverage-build-dir <dir> = override the default build-coverage directory
EOF
}

run_python_tests() {
  project_python_path="$PWD"
  if [[ "$windows_host" -eq 1 ]] && command -v cygpath >/dev/null 2>&1; then
    project_python_path="$(cygpath -m "$project_python_path")"
    python_bin="$(command -v python || command -v python3 || true)"
    if [[ -z "$python_bin" ]]; then
      echo "Python and pytest are required to run the Python regression tests." >&2
      return 1
    fi
    python_bin="$(cygpath -w "$python_bin")"
    echo "Skipping Python tests that launch Bash scripts; those integration tests run on Linux."
    PYTHONUTF8=1 PYTHONPATH="$project_python_path${PYTHONPATH:+:$PYTHONPATH}" \
      "$python_bin" -m pytest -q tests/python \
      -k "not test_web_launch_captures_cpp_output_in_private_log and not test_non_web_launch_is_not_redirected and not test_fresh_shell_token_overrides_stale_env_file and not test_launcher_rebuilds_when_source_is_newer_than_the_binary and not test_launcher_rebuild_option_is_consumed_before_forwarding_arguments"
    return
  fi
  export PYTHONPATH="$project_python_path${PYTHONPATH:+:$PYTHONPATH}"
  if command -v pytest >/dev/null 2>&1; then
    pytest -q tests/python
  elif command -v python >/dev/null 2>&1; then
    python -m pytest -q tests/python
  elif command -v python3 >/dev/null 2>&1; then
    python3 -m pytest -q tests/python
  else
    echo "Python and pytest are required to run the Python regression tests." >&2
    return 1
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    1)
      build_system="cmake"
      ;;
    2)
      build_system="bazel"
      ;;
    --rebuild)
      rebuild=1
      ;;
    --skip-tests)
      skip_tests=1
      ;;
    --coverage)
      coverage=1
      coverage_target="coverage"
      build_system="cmake"
      ;;
    --coverage-gcov)
      coverage=1
      coverage_target="coverage-gcov"
      build_system="cmake"
      ;;
    --coverage-build-dir)
      shift
      build_dir="${1:-}"
      if [[ -z "$build_dir" ]]; then
        echo "--coverage-build-dir requires a directory" >&2
        usage
        exit 2
      fi
      coverage_build_dir_set=1
      ;;
    --cmake)
      build_system="cmake"
      ;;
    --bazel)
      build_system="bazel"
      ;;
    --build-system)
      shift
      case "${1:-}" in
        1|cmake)
          build_system="cmake"
          ;;
        2|bazel)
          build_system="bazel"
          ;;
        *)
          echo "Unsupported build system: ${1:-}"
          usage
          exit 2
          ;;
      esac
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1"
      usage
      exit 2
      ;;
  esac
  shift
done

if [[ "$coverage" -eq 1 && "$coverage_build_dir_set" -eq 0 ]]; then
  if [[ "$windows_host" -eq 1 ]]; then
    build_dir="${COVERAGE_BUILD_DIR:-build-coverage-windows}"
  else
    build_dir="${COVERAGE_BUILD_DIR:-build-coverage}"
  fi
fi

if [[ "$build_system" == "bazel" ]]; then
  if ! command -v bazel >/dev/null 2>&1 && ! command -v bazelisk >/dev/null 2>&1; then
    echo "Bazel is not installed or not on PATH. Install Bazel or Bazelisk, or run: ./buildCode.sh 1 --rebuild" >&2
    exit 1
  fi

  bazel_bin="$(command -v bazel || command -v bazelisk)"
  bazel_windows_args=()
  if [[ "$windows_host" -eq 1 ]]; then
    export MSYS_NO_PATHCONV=1
    curl_root="${CURL_ROOT:-}"
    if [[ -z "$curl_root" && -n "${VCPKG_ROOT:-}" ]]; then
      curl_root="$VCPKG_ROOT/installed/x64-windows"
    fi
    if [[ -n "$curl_root" ]] && command -v cygpath >/dev/null 2>&1; then
      curl_root="$(cygpath -u "$curl_root")"
    fi
    if [[ -z "$curl_root" || ! -f "$curl_root/include/curl/curl.h" ||
          ! -f "$curl_root/lib/libcurl.lib" ]]; then
      echo "Windows Bazel builds require MSVC-compatible libcurl with WebSocket support." >&2
      echo "Install curl[websockets] with vcpkg (vcpkg install 'curl[websockets]:x64-windows'), then set VCPKG_ROOT or CURL_ROOT." >&2
      exit 1
    fi
    if command -v cygpath >/dev/null 2>&1; then
      curl_root="$(cygpath -m "$curl_root")"
    fi
    bazel_windows_args+=("--copt=/I${curl_root}/include")
    bazel_windows_args+=("--linkopt=/LIBPATH:${curl_root}/lib")
  fi

  if [[ "$rebuild" -eq 1 ]]; then
    "$bazel_bin" clean --expunge >/dev/null 2>&1 || true
  fi
  "$bazel_bin" build "${bazel_windows_args[@]}" //:portfolio_health
  if [[ "$skip_tests" -eq 0 ]]; then
    "$bazel_bin" test "${bazel_windows_args[@]}" //:portfolio_health_tests --test_output=errors
    run_python_tests
  fi
  exit 0
fi

cmake_build_type="Release"
if [[ "$coverage" -eq 1 ]]; then
  cmake_build_type="Debug"
fi

if [[ "$rebuild" -eq 1 ]]; then
  rm -rf "$build_dir"
fi

if [[ "$windows_host" -eq 1 ]]; then
  if [[ -z "${MSYSTEM:-}" ]]; then
    echo "Run the Windows CMake build from an MSYS2 MinGW terminal (for example UCRT64)." >&2
    exit 1
  fi
  msys_prefix="${MSYS2_PREFIX:-/${MSYSTEM,,}}"
  if command -v cygpath >/dev/null 2>&1; then
    msys_prefix="$(cygpath -m "$msys_prefix")"
  fi
  cmake -S . -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE="$cmake_build_type" \
    -DCMAKE_PREFIX_PATH="$msys_prefix" \
    -DBUILD_TESTING=ON \
    -DPORTFOLIO_HEALTH_ENABLE_COVERAGE="$([[ "$coverage" -eq 1 ]] && echo ON || echo OFF)"
else
  if ! conan profile show default >/dev/null 2>&1; then
    conan profile detect --force
  fi

  conan install . --output-folder="$build_dir/conan" --build=missing \
    -s build_type="$cmake_build_type"
  toolchain_dir="$build_dir/conan/build/$cmake_build_type/generators"
  cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE="$cmake_build_type" \
    -DCMAKE_TOOLCHAIN_FILE="$toolchain_dir/conan_toolchain.cmake" \
    -DPORTFOLIO_HEALTH_ENABLE_COVERAGE="$([[ "$coverage" -eq 1 ]] && echo ON || echo OFF)"
fi

if [[ "$coverage" -eq 1 ]]; then
  if [[ "$coverage_target" == "coverage" ]] && {
    ! command -v lcov >/dev/null 2>&1 || ! command -v genhtml >/dev/null 2>&1;
  }; then
    echo "Coverage HTML requires lcov and genhtml. Install lcov, then retry." >&2
    exit 1
  fi
  cmake --build "$build_dir" --target "$coverage_target" --parallel "$build_jobs"
else
  cmake --build "$build_dir" --parallel "$build_jobs"
fi

if [[ "$skip_tests" -eq 0 ]]; then
  ctest --test-dir "$build_dir" --output-on-failure
  run_python_tests
fi