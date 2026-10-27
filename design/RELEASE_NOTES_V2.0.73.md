# Release Notes v2.0.73

## Fix Windows web startup

The Windows launcher previously relied only on environment variables already
present in its parent shell. When credentials were saved in the standard
per-user `.upstox.env` file, the application exited before binding port 8080,
so the browser displayed `ERR_CONNECTION_REFUSED`.

- Load the approved Upstox API and dashboard login values from
  `%USERPROFILE%\.upstox.env` when not already present in the environment.
- Parse simple assignments without executing the file as PowerShell or shell
  code; explicitly set environment values continue to take precedence.
- Reject reparse-point credential files and never write credentials to the
  web log.
- Start the browser only after the launched application is listening.

Stock API access remains authenticated, read-only, and server-side. The
post-login release popup summarizes this fix.
