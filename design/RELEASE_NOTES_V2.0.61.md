# Release Notes v2.0.61

## Refresh lockup root cause

The dashboard aborted its holdings request after 15 seconds, but the server
could continue holding the shared snapshot refresh lock while calling Upstox.
The snapshot was fetched synchronously and called holdings, positions, and
news sequentially; each request used a 30-second timeout with up to three
transport attempts. A browser timeout therefore did not stop the server-side
work. Repeated retries could collide with the still-running snapshot and
return an unavailable response instead of starting a fresh request.

Snapshot generation now performs each holdings, positions, and news request
with a single 10-second transport attempt. The maximum expected upstream wait
for a complete snapshot is therefore 30 seconds. Dashboard data requests now
allow 35 seconds for the response, including body reading, so the browser does
not abandon a normal bounded snapshot before the backend releases the lock.
Manual refresh remains single-flight, queued during active work, and subject
to the existing server-side rate limit.

The request timeout/retry parameters are optional on `HttpClient` and
`UpstoxClient`; all other CLI and API callers retain their existing 30-second,
three-attempt defaults. TLS verification, trusted-host validation, same-origin
authentication, and server-side broker credentials are unchanged.

The login popup shown after successful authentication summarizes this fix.

## Validation

- Add regression checks for bounded snapshot calls and request deadline
  alignment.
- Run Python and C++ test suites and rebuild the versioned executable.
- Live Upstox behavior requires user credentials and is not verified offline.
