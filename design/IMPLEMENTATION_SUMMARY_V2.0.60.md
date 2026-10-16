# Implementation Summary v2.0.60

## Root cause and fix

The screenshot's error state is rendered after the initial holdings request
fails. The Retry now button was only expected to work through document-level
event delegation. In addition, `refreshView()` returned early whenever an
automatic refresh was in progress, discarding manual retry intent.

Retry and metric buttons now receive direct click handlers at creation.
`manualRefreshPending` records clicks while a poll or wake refresh is active;
`finishRefresh()` then immediately starts one forced request after the active
load completes. The periodic refresh, wake recovery, and manual path share
that finalizer. The prior 15-second AbortController deadline remains active
through response-body reading, ensuring stalled requests eventually release
the single-flight guard.

## Version and login popup

The patch release is `2.0.60`, aligned across CMake, Conan, Bazel, README,
design index, and runtime popup. The post-login popup describes the retry
handler and queued manual refresh change.

## Security and validation

The fix uses the existing authenticated same-origin endpoints and
server-enforced manual-refresh rate limit. No stock API credential is sent to
the browser. Live Upstox behavior still requires user credentials.
