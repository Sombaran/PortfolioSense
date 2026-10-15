# Implementation Summary — v2.0.55

The live feed reconnect issue was caused by treating libcurl's non-blocking
WebSocket send result as a terminal error. A temporarily unavailable socket or
partial write could therefore close a valid feed before the subscription or
heartbeat frame was fully sent. The streaming client now resumes writes,
waits for socket writability when libcurl returns `CURLE_AGAIN`, and applies a
bounded timeout to prevent a stuck worker.

The authenticated `/api/stream/quotes` route now reports a short error
diagnostic in addition to connection state and cached prices. The Overview
renders that diagnostic as text, preserving HTML injection protection and
allowing users to distinguish authorization, transport, and reconnect states.
No authorized WebSocket URL or access token is included in the response.

Holdings rows now carry their safely escaped instrument key, so live quote
updates are matched independently of the visible symbol sort order. Last price,
position P&L, market value, and Day P&L are recalculated from live data where
available, while existing snapshot values remain available for holdings with
no current quote. Dashboard pause/visibility behavior, responsive rendering,
Operations JSON, login, and session behavior remain unchanged.

Validation includes C++ build and CTest, Python regressions for the v2.0.55
version contract and quote route/UI behavior, JavaScript syntax validation,
and `git diff --check`. Live broker connectivity still requires the operator's
valid Upstox credentials and was not used for testing.
