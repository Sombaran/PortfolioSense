# Implementation Summary v2.0.64

## Root cause and fix

The dashboard's manual refresh relies on a new live holdings request. Snapshot
generation had only one 10-second transport attempt for that required request,
so a transient DNS, connect, or transport timeout could fail an otherwise
recoverable refresh. The browser's 35-second timeout also left only five
seconds beyond the backend's maximum 30-second sequential snapshot budget.

The snapshot now allows two transport attempts for holdings via
`snapshotHoldingsTransportAttempts`, preserving the 10-second per-attempt
bound. Positions and news keep their existing one-attempt
10-second bounds and remain failure-isolated. A shared 50-second browser
deadline covers the resulting maximum 40-second upstream budget with additional
response-processing headroom.

## Compatibility and security

HTTP/authentication failures are not retried, avoiding repeated invalid-token
requests. Existing 10-second manual refresh rate limiting, stale-snapshot
fallbacks, HTTPS and trusted-host checks, and server-side credential handling
remain unchanged. The post-login popup summarizes the fix.
