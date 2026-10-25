# Release notes — v2.0.70

## Cross-platform runtime adapters

- Added native adapters for Windows, Linux, and macOS networking, secure temporary-file creation, executable-path discovery, Python process launching, environment cleanup, and secure random bytes.
- Kept portfolio calculations, Upstox request logic, dashboard routes, and business rules outside the platform adapter layer.
- Replaced direct POSIX socket calls in the web server with adapter calls.

## Process and temporary-file safety

- Python-backed analysis now passes script and data paths as process arguments instead of constructing shell commands.
- Temporary analysis files are created in the operating system's temporary directory and are removed when each analysis operation exits, including error paths.
- OS-provided cryptographic randomness remains the source for generated session values when the OpenSSL fallback is used.

## Compatibility and security

- The web server remains bound to loopback (`127.0.0.1`); login/session handling and read-only Stock API behavior are unchanged.
- No Stock API credentials or authorized WebSocket data are exposed to browser code.
- Windows and macOS require native CI/build verification before platform releases can be considered fully validated.

## Dashboard login popup

The post-login **What is new** popup summarizes the cross-platform adapter layer, shell-free Python process launch, and temporary-file cleanup. Existing login and authenticated session behavior is unchanged.

## Validation

Unit tests cover secure temporary-file uniqueness and permissions where supported, as well as OS-backed random byte generation. Linux build and test results are recorded with the implementation summary.
