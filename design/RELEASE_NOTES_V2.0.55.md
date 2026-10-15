# Release Notes — v2.0.55

## Live market feed reconnect reliability

- Corrected libcurl WebSocket writes to resume partial writes and retry
  `CURLE_AGAIN` rather than treating a temporarily non-writable socket as a
  failed feed connection. Sends stop after a bounded timeout.
- Applied the same safe write handling to WebSocket ping responses.
- Added a short connection diagnostic to the authenticated local quote
  snapshot endpoint so the Overview can show why the feed is reconnecting.
  The endpoint does not return the authorized feed URL or broker credentials.
- Matched each displayed holding to its quote by instrument key rather than
  relying on symbol ordering. The Overview updates live Last price, position
  P&L, market value, and Day P&L.
- Refactored holdings rendering for clearer, safer attribute escaping while
  preserving the existing news, sorting, fallback, and responsive behavior.
- Retained the Operations JSON view, other dashboard pages, and login/session
  behavior. The post-login popup summarizes this fix.

The Upstox connection was not exercised against a live account because no
broker credentials were provided.
