# Implementation Summary v2.0.71

## Root cause

The Windows CMake launcher compared source timestamps only with the executable
and omitted the `adapters` directory. A platform-adapter change could therefore
leave the application using a stale DLL. It also did not detect a missing core
DLL before launch. Separately, the build scripts invoked pytest from the
repository root, allowing collection to descend into Bazel's generated
external-workspace tree.

## Changes

- The Windows launcher now requires both the executable and
  `libportfolio_health_core.dll`, and checks source timestamps in `src`,
  `include`, and `adapters`.
- Stale-check paths are passed to PowerShell through environment variables
  rather than interpolated into the PowerShell command.
- CMake rebuild validates the canonical target path and refuses targets outside
  the repository or paths that traverse reparse points.
- Both build scripts restrict Python regression collection to `tests/python`.
- Version metadata, documentation, and the post-login popup are aligned at
  semantic version `2.0.71`.

## Security and compatibility

The stock API request and authentication paths were not changed. Upstox access
remains authenticated, read-only, and server-side; API credentials and access
tokens are not passed to browser code. The `2.0.71` summary is shown in the
post-login popup. Linux build and runtime code paths are unchanged.

## Validation

- Windows Python regression selection: 35 passed, 5 Bash-only integration
  tests deselected.
- `git diff --check` passed.
- Windows batch execution and native Bazel/CMake builds require Windows build
  tools and should be run on the target Windows environment.
