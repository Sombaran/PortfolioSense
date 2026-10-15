# Implementation Summary v2.0.56

## Root cause and fix

The previous deployment linked against the distribution's libcurl 8.5.0,
which did not include WSS in its compiled protocol list. Although that build
exposed WebSocket frame APIs, it could not create a `wss://` transfer and
returned `Unsupported protocol`. The normal Conan build now pins libcurl
8.22.0 and enables `with_websockets=True`; the streaming client checks runtime
protocol support and surfaces a clear error if an incompatible library is
loaded.

The web UI continues to receive updates from the server-owned live feed via
the authenticated local quote cache. TLS verification, trusted Upstox hosts,
credential handling, and subscription validation remain in place. The login
popup includes the v2.0.56 fix summary.

## Files and validation

- Updated build metadata to v2.0.56 and selected the WSS-capable Conan
  dependency for the standard CMake build.
- Added runtime WSS capability regression coverage and aligned version checks.
- Updated the README and design release index.
- Live broker streaming requires valid Upstox credentials and is not claimed
  as verified by offline tests.
