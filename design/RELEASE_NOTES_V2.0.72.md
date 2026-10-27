# Release Notes v2.0.72

## Windows web launcher

- `run.bat --web` now waits until the local web server is listening before
  opening `http://localhost:8080/` in the default browser.
- Application output and startup errors remain in the configured web log.
- All remaining application arguments are forwarded to the executable.
- The launcher continues running the server in the current console.
- Linux launch behavior and authenticated, read-only Stock API access are
  unchanged; credentials remain server-side.

The post-login release popup summarizes this fix.
