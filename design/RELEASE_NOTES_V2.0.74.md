# Release Notes v2.0.74

## Windows live market feed WSS support

Windows Bazel builds now install vcpkg's `curl[websockets]` feature instead of
plain curl. The build always resolves the feature into the project-local
vcpkg install root, so a previously installed plain-curl package does not
silently bypass the WebSocket-capable dependency. `run.bat --web` now loads the
runtime DLLs from that same install root.

An explicit `CURL_ROOT` remains available for custom installations; it must
contain a libcurl build with WSS enabled. Existing runtime capability checks
continue to report an actionable error if a custom library lacks WSS. The
Upstox WebSocket URL validation, TLS verification, and server-side handling of
credentials and live quotes are unchanged. Linux build and runtime behavior
are unchanged.

The post-login release popup summarizes the fix.

## Validation

- Add Windows build-script regression coverage for vcpkg's WebSocket feature
  and dependency root selection.
- Run the Python regression suite and `git diff --check`.
- A live Upstox feed still requires valid credentials and was not verified as
  part of this release.
