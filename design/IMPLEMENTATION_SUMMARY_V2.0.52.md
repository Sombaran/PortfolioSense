# baran-capital-view v2.0.52 - Implementation Summary

## Root cause

The browser session cookie had a fixed `Max-Age=3600`. It expired exactly one hour after login, regardless of ongoing dashboard use. The UI then encountered unauthenticated redirects while its background refresh was still active.

## Fix

- Make the existing one-hour lifetime an idle timeout: a valid authenticated request extends the server-side session expiry.
- Refresh the existing HttpOnly/SameSite cookie on authenticated dashboard, JSON, and metrics responses. A visible tab also uses a ten-minute local session endpoint, so low-traffic views remain signed in without Stock API traffic.
- Expire inactive sessions and clean up expired entries when new sessions are created.
- Keep login secrets, session cookies, and Upstox tokens out of browser JavaScript.

## Validation

- Add focused regression coverage for session expiry/renewal and release metadata.
- Run the Python test suite, C++ build, and CTest.
