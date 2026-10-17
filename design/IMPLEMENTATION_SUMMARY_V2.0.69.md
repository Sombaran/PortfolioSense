# Implementation summary — v2.0.69

## Root causes closed

- The market-holiday handler previously returned an error-shaped JSON body while leaving the response at HTTP 200 if the initial Upstox request failed. It now throws a controlled unavailable error when there is no validated cache, and the route returns HTTP 502 with `status: "error"`.
- When a validated calendar exists but a subsequent refresh fails, the existing cache is preserved and the response explicitly includes `stale: true` and a warning. The dashboard tooltip exposes that warning; if no calendar is available, it identifies the regular-hours estimate.
- Shared dashboard content now sets `aria-busy` during each view load. A monotonically increasing render ID rejects late API responses from older tabs, prevents their stale values from replacing newer shared-cache entries, and ensures only the latest navigation clears the busy state.

## Compatibility and safety

- Successful holiday responses retain their existing Upstox payload and 12-hour cache behavior.
- Read-only Stock API access, authenticated routes, session handling, refresh scheduling, and server-side token boundaries are unchanged.
- Error responses do not expose upstream response bodies or credentials.

## Login popup

The post-login release popup displays the v2.0.69 summary for API status clarity, stale-calendar handling, and dashboard loading feedback.
