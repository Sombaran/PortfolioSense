# Implementation Summary — v2.0.65

## Root cause

The server deliberately used a non-blocking refresh lock to keep ordinary
dashboard requests responsive. When a manual Refresh arrived during the first
snapshot build, there was no cached snapshot to return and no local CSV data
for the fallback, so the request threw an in-progress error. The browser
surfaced that exception as the generic live-refresh warning.

## Fix

Forced refresh requests now wait for the active snapshot build to finish. Once
they acquire the lock, they check whether another request created a snapshot
after their initial cache check; if so, they return that result rather than
making duplicate upstream API calls. Ordinary non-forced requests retain the
non-blocking stale/local fallback behavior.

The forced path blocks on `refreshLock.lock()` and checks the snapshot creation
time against the request's initial timestamp before returning the shared
snapshot.

## Security and login popup

The change does not expose Stock API credentials or alter route authentication,
HTTPS validation, trusted-host checks, or redirect handling. The versioned
post-login popup now includes a v2.0.65 summary of the refresh recovery.

## Regression coverage

The Python web-server source regression tests verify that forced contention
waits on the refresh lock and reuses a newly-created snapshot, while the
non-forced contention branch continues to preserve its fallback behavior.
