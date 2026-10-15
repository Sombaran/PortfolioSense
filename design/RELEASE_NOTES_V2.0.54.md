# Release Notes — v2.0.54

## Upstox Market Data Feed V3 WebSocket

- Implemented the server-side Upstox V3 WebSocket connection using libcurl.
- Validated the authorized feed URL against the trusted Upstox host boundary,
  restricted transport to verified TLS, and kept the bearer token and
  authorized URL out of browser responses.
- Subscribed to up to 100 unique, validated live holdings instrument keys,
  decoded bounded binary Protobuf messages, and reconnected after transient
  connection failures.
- Added an authenticated local `/api/stream/quotes` JSON snapshot endpoint.
  The Overview reads this cached data only while it is visible and automatic
  updates are enabled, avoiding direct browser-to-Upstox connections.
- Corrected live quote rendering to update the Last-price cell without
  overwriting the Average-price column.
- Retained the responsive dashboard layout and Operations subviews, including
  the JSON view, without changing login-popup dismissal or session behavior.
- Added C++ feed validation/decoder coverage and Python route/version
  regression coverage.

The login popup shown after authentication notes the live feed implementation
and confirms that broker credentials remain server-side.
