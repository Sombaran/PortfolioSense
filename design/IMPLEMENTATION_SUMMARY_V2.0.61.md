# Implementation Summary v2.0.61

## Root cause and fix

The earlier UI recovery changes did not bound the work performed after the
browser sent a refresh. A snapshot request could keep the server's shared
`refreshMutex_` locked through three sequential Upstox calls, each with a
30-second timeout and transport retries. The browser's 15-second abort only
closed its side of the request; it did not cancel the synchronous backend
work. A subsequent retry then encountered the occupied refresh lock.

The HTTP and Upstox client methods now accept optional timeout and retry
limits, retaining their previous defaults for other callers. Snapshot
generation passes a 10-second timeout and one attempt for holdings, positions,
and news. The browser dashboard requests allow 35 seconds for the full
snapshot (up to 30 seconds of upstream waits plus local processing), covering
both response headers and body reading.

The v2.0.60 direct button handlers, queued manual refresh, and common refresh
finalizer remain active.

## Version and login popup

The patch release is `2.0.61`, aligned across CMake, Conan, Bazel, README,
design index, and runtime popup. The post-login popup explains why the
backend lock remained busy and how bounded upstream requests restore refresh.

## Security and validation

Only server-side snapshot calls use the shorter explicit timeouts and
single-attempt policy. Other API callers preserve existing retry behavior.
TLS verification, trusted-host validation, session authentication, and
server-only bearer credentials are unchanged. Live Upstox behavior still
requires user credentials.
