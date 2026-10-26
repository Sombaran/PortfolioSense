# Release Notes v2.0.71

## Windows build and launcher reliability

- Rebuild detection now includes Windows platform adapters and verifies both the
  executable and its shared core library before launching.
- `buildCode.bat --rebuild` refuses to recursively remove a path outside the
  project directory or a reparse-point build directory.
- Windows and cross-platform build scripts run Python tests from
  `tests/python`, preventing pytest from collecting tests in Bazel's generated
  external dependencies.
- Linux build behavior and existing application features remain unchanged.
- Authenticated Stock API access remains read-only, with tokens and API
  credentials kept server-side.

The post-login release popup summarizes these fixes on the next login.
