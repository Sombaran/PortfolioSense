# Release Notes v2.0.60

## Retry button root cause

The unavailable-data panel is created dynamically after an API failure. Its
Retry now action depended on a document-level delegated click handler, and the
single-flight guard discarded a manual refresh whenever an automatic poll or
wake recovery was already loading the view. In that state, clicking Refresh
could update the status message but never start a forced snapshot request.

The error-panel retry button and the Overview metric refresh button now bind
direct click listeners when they are rendered. If an automatic request is
already in progress, the manual refresh is queued and starts as soon as that
request completes. Automatic polling, wake recovery, and manual refresh all use
the same finalizer so the pending refresh is not lost and button states are
restored consistently. The 15-second request timeout introduced in v2.0.59
still covers headers and response-body reads.

No API endpoint or browser-visible broker credential was added. Refresh uses
the existing authenticated same-origin routes and existing server-side
rate-limited `refresh=1` handling.

The login popup shown after successful authentication summarizes this fix.

## Validation

- Add regression checks for direct retry listeners and refresh queuing.
- Run Python and C++ test suites and rebuild the versioned executable.
- Live broker behavior requires user credentials and is not verified offline.
