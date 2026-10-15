# Release Notes v2.0.56

## Upstox WebSocket protocol support

The live market feed could report that it was reconnecting indefinitely because
the system libcurl used by the build did not advertise the `wss` protocol. The
transport then failed immediately with `Unsupported protocol`, before an
Upstox WebSocket session could be established. This was a build dependency
capability issue, not an incoming market-data frame or partial-write issue.

The standard Conan build now uses libcurl 8.22.0 with WebSocket support enabled.
At runtime, the streaming client checks that the loaded libcurl advertises WSS
before attempting authorization or connecting, and returns an actionable
configuration error when it does not. Existing TLS certificate/hostname
validation and the trusted Upstox endpoint restrictions are unchanged.

The post-login popup summarizes the correction. Quotes remain served
to the browser only through the authenticated same-origin cache endpoint;
broker tokens and authorized feed URLs are not exposed to the browser.

## Validation

- Add regression coverage for the required WSS-capable runtime and for the
  pinned Conan dependency option.
- Run the C++/Python suites and verify the built executable is linked against a
  libcurl that advertises `wss`.
- A real Upstox stream still requires valid user credentials and was not
  verified as part of this release.
