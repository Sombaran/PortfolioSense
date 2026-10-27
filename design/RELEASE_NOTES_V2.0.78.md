# Release Notes v2.0.78

## PortfolioSense naming and Windows launcher cleanup

Updated the project-facing README and coverage report title to PortfolioSense
and aligned CMake, Conan, Bazel, and documentation metadata on version 2.0.78.
The Windows launcher no longer prepends the libcurl DLL directory to `PATH` a
second time after the build helper has already configured it. Temporary vcpkg
manifest directories now use the PortfolioSense name.

Existing vcpkg cache, web-log, and browser preference locations remain
unchanged for compatibility. Stock API requests, credentials, and endpoint
validation are unchanged; requests remain authenticated, server-side, and
read-only. The post-login release popup summarizes this release.

## Validation

- Verify the version is aligned across CMake, Conan, Bazel, and the READMEs.
- Run the focused Python regression tests and C++ regression tests.
- Run `git diff --check`.
