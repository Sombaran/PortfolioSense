# Release Notes v2.0.75

## Windows Bazel build reliability

MSVC identified two code-quality warnings in the web server: an unreachable
duplicate dashboard script return and an ignored `[[nodiscard]]` JSON parse
result. The duplicate return has been removed without changing the active
dashboard implementation, and JSON validation now explicitly discards its
parse result.

The Bazel helper now checks whether the exact `portfolio_health.exe` output
is still running before invoking Bazel clean or build. If the running
application is locking the executable, the helper reports its PID and asks
the user to close it before retrying. It does not kill or disrupt a live
dashboard. A follow-up hint is also printed if the linker reports permission
denied for the executable.

Existing Windows WSS dependency configuration, Linux behavior, and the
authenticated, read-only Stock API boundary remain unchanged. The post-login
release popup summarizes the changes.

## Validation

- Add regression tests for the MSVC warning fixes and running-output preflight.
- Run the Python regression suite and `git diff --check`.
- A native MSVC/Bazel build must be verified on a Windows machine with the
  project toolchain installed.
