# Implementation summary — v2.0.70

## Changes

- Added a common adapter contract in `adapters/include/PlatformAdapter.hpp` and platform-specific implementations under `adapters/linux`, `adapters/macos`, and `adapters/windows`.
- Routed web-server listener, accepted connection, timeout, receive, send, and close operations through the adapter.
- Routed secure temporary-file creation, executable-path lookup, environment-variable cleanup, file-permission checks, and the non-OpenSSL random-byte fallback through the adapter.
- Replaced shell-based Python invocation with argument-based process creation (`posix_spawnp` on Linux/macOS and `CreateProcessW` on Windows).
- Added scoped temporary-file cleanup so generated portfolio-analysis inputs and outputs are removed on both success and failure.
- Updated CMake/Bazel source wiring and aligned project metadata to `2.0.70`.
- Added adapter regression tests for unique private temporary files and cryptographic random bytes.

## Security and behavior

- The HTTP listener still binds only to `127.0.0.1`.
- Python invocation no longer interprets script or file paths as shell syntax.
- Temporary files use OS-managed unique creation; POSIX implementations request owner-only permissions.
- Portfolio calculations, Upstox endpoint validation, TLS configuration, authentication, and read-only API behavior were not changed.
- The login popup is keyed to the build version and appends the v2.0.70 adapter summary after login.

## Validation

- Linux CMake build passed; the executable reports version `2.0.70`.
- All 40 Python tests passed.
- All 44 C++ tests outside `NotificationService` passed, including the adapter tests. The full CTest run has one failure in `NotificationService.SendAlertQueuesNotification`, in code outside this change.
- Native Windows and macOS builds/tests remain required; this Linux environment cannot establish those platform builds.
