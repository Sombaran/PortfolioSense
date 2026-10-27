# Implementation Summary v2.0.74

## Root cause and fix

The Windows Bazel helper installed vcpkg's default `curl` dependency, which
does not enable the optional `websockets` feature. The application correctly
detected that the linked libcurl did not advertise WSS and retained snapshot
prices, but the live feed could not connect. In addition, the launcher could
select an existing vcpkg `installed` directory before the helper had an
opportunity to install the feature-enabled dependency.

The helper now installs `curl[websockets]` into the configured
`VCPKG_INSTALL_ROOT` on each Bazel build, allowing vcpkg to upgrade an existing
plain-curl installation. The launcher no longer pre-selects vcpkg's other
`installed` directory; after a successful build, it loads the runtime DLL from
the feature-enabled install root. An explicit `CURL_ROOT` is still honored as
a user override and is documented as requiring WSS support.

No WebSocket security checks were weakened: the client still validates the
Upstox endpoint and verifies TLS. Linux build scripts and runtime behavior are
unchanged. The login popup describes the Windows dependency fix.

## Files and validation

- Update the Windows Bazel vcpkg manifest to request `curl[websockets]`.
- Use the same WebSocket-enabled dependency root for building and launching.
- Add regression checks for the manifest feature, error guidance, and versioned
  popup.
- Windows native build execution and a real Upstox connection require Windows
  build tools and user credentials, which were unavailable in this run.
