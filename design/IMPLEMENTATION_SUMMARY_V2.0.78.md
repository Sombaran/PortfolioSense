# Implementation Summary v2.0.78

## Changes

- Renamed project-facing README and coverage-report references to
  PortfolioSense and aligned patch-version metadata on `2.0.78`.
- Removed a redundant Windows launcher PATH update. The build helper already
  adds the selected libcurl DLL directory before returning to the launcher, so
  adding it again duplicated the same entry.
- Updated the Windows temporary vcpkg manifest prefix to PortfolioSense.
- Added the v2.0.78 release summary to the existing post-login popup chain.
- Kept existing vcpkg cache and web-log locations and browser preference keys
  intact to avoid disrupting existing installations.

## Stock API and compatibility

No Stock API client, request, credential, or authorization behavior changed.
Stock data remains authenticated, server-side, and read-only. CMake/Bazel
targets, Linux behavior, existing dashboard functionality, and guarded Windows
rebuild behavior remain unchanged.

## Validation

The regression test checks release-version alignment, the new popup entry, and
the Windows batch safeguards. Run Python and C++ regression tests and
`git diff --check` before release.
