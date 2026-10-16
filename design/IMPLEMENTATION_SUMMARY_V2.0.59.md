# Implementation Summary v2.0.59

## Root cause and fix

The Refresh action used `refreshInFlight` to prevent overlapping dashboard
loads, but dashboard API fetches had no timeout. A stalled request could
therefore leave the guard set forever; subsequent refresh clicks returned
without starting a request. Fetch timeouts alone were not sufficient because
the response body could also stall, so the new 15-second abort deadline covers
both `fetch()` and `response.text()`.

Refresh controls are now found and restored as a group in the promise's
`finally` handler. The dynamically rendered Overview button is a real DOM
button handled through event delegation, as is the error-page retry action.
Existing manual `refresh=1` behavior, server authentication, retry reporting,
and retention of the last rendered view are preserved.

## Version and login popup

The patch release is `2.0.59`, aligned across CMake, Conan, Bazel, the README,
design index, and runtime version popup. The post-login popup explains the
stalled-request root cause and the refresh recovery changes.

## Security and validation

No broker token, Stock API URL, or new server endpoint is exposed to browser
code. Existing same-origin authenticated routes and server-side credentials
remain unchanged. Regression coverage checks the timeout scope and user-action
recovery path. Live Upstox behavior still requires user credentials.
