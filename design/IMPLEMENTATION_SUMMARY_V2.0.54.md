# Implementation Summary — v2.0.54

The project now implements Upstox Market Data Feed V3 through a server-owned
WebSocket connection. The authenticated local service fetches the one-time
authorization URL, validates the host and instrument keys, verifies TLS, and
subscribes to no more than 100 holdings. Binary Protobuf messages are decoded
into a thread-safe quote cache with bounded message size and reconnect support.

The authenticated `/api/stream/quotes` route returns connection status and
cached quote data only. The browser does not receive the access token or
authorized feed URL. The Overview polls this local route only while visible
and when updates are not paused. Live quote changes update the Last-price
column and derived market value/Day P&L without disturbing other table data.

The Operations JSON view and shared responsive dashboard behavior from v2.0.53
remain in place. No login flow or session-cookie semantics were changed; the
post-login popup continues to show one versioned summary and can be dismissed
as before.

Validation includes C++ URL/key/Protobuf tests, Python route and version
regressions, and the full available CTest and Python test suites. A live broker
connection was not exercised because no user credentials were supplied.
