# Release Notes v2.0.59

## Manual refresh root cause

Dashboard data requests for holdings, positions, and news had no deadline.
If the server stopped responding while sending headers or the response body,
the request could remain pending indefinitely. The refresh flow kept
`refreshInFlight` set until that request completed, so later clicks on
**Refresh** were ignored. That made the visible manual-refresh controls appear
broken after a stalled live request.

These requests now use an abort timeout of 15 seconds that remains active
through response-body reading. The existing error path reports the unavailable
data while retaining the last rendered view, and the refresh promise always
restores the buttons. The Overview refresh control and error-page retry control
are handled with delegated click events rather than inline handlers.

No Stock API credentials or new API endpoints are exposed to the browser.
Stock API access remains authenticated and server-side.

The login popup shown after successful authentication summarizes this fix.

## Validation

- Add regression coverage for the bounded request and refresh recovery path.
- Run the focused Python web regression tests and C++ tests.
- Live broker behavior requires user credentials and is not verified offline.
