# Implementation Summary v2.0.72

## Root cause

`run.bat --web` printed the web URL but did not launch a browser. Starting a
browser immediately would also race the server's socket bind.

## Changes

- Added `tools/open_web_ui.ps1` to poll the IPv4 loopback socket with a bounded
  startup timeout, then open `http://localhost:8080/` using the user's default
  browser.
- The helper reports a clear error when the server does not start or the
  browser cannot be opened.
- `run.bat` starts the helper only in web mode, retains foreground server
  lifetime and private log redirection, and forwards all remaining arguments.
- Added regression coverage for the Windows web-launch contract.
- Aligned build metadata, documentation, and the post-login summary at
  semantic version `2.0.72`.

## Security and compatibility

The helper connects only to `127.0.0.1` to detect readiness and opens the
loopback UI at `localhost`. It does not accept a remote host, handle API
credentials, or change Stock API behavior. Credentials remain server-side;
Upstox access stays authenticated and read-only. Linux launcher behavior is
unchanged.

## Validation

The Windows CMake build, C++ test, and scoped Python regression tests should be
run from the Windows build environment.
