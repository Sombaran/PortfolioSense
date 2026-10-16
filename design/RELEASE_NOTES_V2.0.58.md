# Release Notes v2.0.58

## Alerts tab crash root cause

The Alerts page registered a `MutationObserver` on the entire rendered view.
Its callback unconditionally assigned `"Portfolio news review"` to the
heading. Updating `textContent` creates a child-list mutation, which invoked
the same observer again. The repeated callback rescanned and rewrote the
heading indefinitely, consuming CPU and eventually causing the browser tab to
become unresponsive or crash.

The heading is now updated only when its text differs, making normalization
idempotent. Page-level normalizers now observe top-level view replacements
instead of every nested DOM update. This also avoids redundant full-table
rescans while live quote values change.

No Stock API route or browser-visible credential was added. Existing
authenticated same-origin API access and server-side broker credentials are
unchanged.

The login popup shown after successful authentication summarizes the
root-cause fix.

## Validation

- Add regression coverage for idempotent Alerts heading normalization and
  top-level-only observer scope.
- Run Python and C++ unit suites and rebuild the versioned executable.
- Live broker behavior requires user credentials and is not verified offline.
