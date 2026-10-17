# Implementation summary — v2.0.68

## Root causes addressed

- `TaskScheduler` read its running state outside the scheduling lock and ignored task priority. State checks and queue operations are now synchronized; the worker selects the highest priority and preserves insertion order for ties.
- Local session IDs previously relied on a non-cryptographic fallback when OpenSSL was unavailable. Session creation now requires operating-system cryptographic randomness and fails closed on failure.
- Invalid login attempts were not bounded. The loopback server now limits invalid attempts to five per rolling minute, with a `429` response and retry hint after the limit.
- Request failures were returned as successful HTTP responses. Invalid inputs, unsupported methods, missing routes, and upstream failures now receive appropriate 4xx/5xx statuses.
- Incomplete order, model, and outbound notification integrations produced misleading success-shaped results. Their unavailable states are now explicit; no broker order or external message is falsely reported as delivered.
- Risk simulation now rejects invalid numeric inputs and fails closed when sector-exposure data cannot be verified.

## Compatibility and security

- Existing read-only holdings, positions, quote, news, WebSocket, and dashboard functionality remains available.
- The web server remains loopback-only; the authenticated session cookie remains `HttpOnly` and `SameSite=Strict`.
- The Upstox access token remains server-side. OAuth callback and token refresh are not claimed as supported.
- Live order placement and external notifications are intentionally unavailable until real, verified implementations are added.

## Release UX

The v2.0.68 login popup explains the reliability changes and identifies integrations that are unavailable, avoiding any implication that a trade, prediction, or notification was completed.

## Test coverage

Updated C++ and Python regression tests cover scheduler ordering/configuration, random-string generation, fail-closed order/notification/model behavior, risk-input validation, HTTP status handling, and release-version consistency.
