# Implementation Summary v2.0.73

## Root cause

`run.sh` loads credentials from the per-user `.upstox.env` file, but
`run.bat` did not. The C++ web mode requires `UPSTOX_API_KEY`,
`UPSTOX_API_SECRET`, and `UPSTOX_ACCESS_TOKEN`; without them it exits before
opening the listening socket. Consequently the browser receives
`ERR_CONNECTION_REFUSED`.

## Changes

- Route Windows web startup through `tools/run_web_ui.ps1`.
- Apply PowerShell's `Bypass` execution policy only to the helper process;
  no machine or user execution-policy setting is changed.
- Import only `UPSTOX_API_KEY`, `UPSTOX_API_SECRET`, `UPSTOX_ACCESS_TOKEN`, and
  `FOLIO_LOGIN_CODE` from `%USERPROFILE%\.upstox.env`.
- Support simple `NAME=value` and `export NAME=value` records without
  evaluating shell expressions. Inherited process variables take precedence.
- Reject reparse-point environment files, and do not print credentials or
  include them in the web log.
- Launch the executable with its argument list and private log redirection;
  open the browser only while that child process is alive and port 8080 is
  listening. Missing credentials now produce a direct diagnostic instead of
  a misleading browser failure.
- Align build, popup, and documentation versions at `2.0.73`.

## Security and compatibility

Credential data is inherited only by the application process and is never
passed on the command line. The Upstox API remains authenticated, read-only,
and server-side. Linux launch behavior is unchanged.
