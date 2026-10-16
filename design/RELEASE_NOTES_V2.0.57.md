# Release Notes v2.0.57

## Dashboard stability and performance

The live Overview polled the authenticated quote-cache endpoint every three
seconds without preventing another request from starting while the previous
one was still pending. If a response stalled, requests could overlap and
consume browser and server resources, putting avoidable pressure on the tab
and matching the reported instability.

Quote polling now permits only one request at a time, aborts after eight
seconds, and cancels an in-flight request when the browser tab is hidden. Live
quote updates use a precomputed instrument-to-holding map rather than
repeatedly searching all holdings, and every changed quote is applied so
updates are not silently dropped.

The fix does not add any Stock API route. The browser still reads live quotes
from the authenticated same-origin cache; Stock API credentials and the
authorized feed URL remain on the server. Existing TLS and trusted-host checks
are unchanged.

The post-login release popup summarizes this fix.

## Validation

- Add regression checks for single-flight polling, request cancellation,
  bounded request time, quote lookup, version alignment, and the login popup.
- Run the focused Python web regression tests and C++ unit test suite.
- A live Upstox feed still requires valid credentials and is not claimed as
  verified by offline tests.
