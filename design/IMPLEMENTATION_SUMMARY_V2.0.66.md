# Implementation Summary — v2.0.66

## Confirmed failure causes

The private dashboard log recorded repeated `HTTP 401 from Upstox holdings`
responses and DNS resolution failures (`Could not resolve hostname`). The
Refresh UI collapsed failures to a generic warning whenever it retained an
already-rendered page, hiding the distinction between an invalid/expired
access token, upstream connectivity, and local dashboard request failures.

## Changes

- The backend now maps snapshot exceptions to safe user-facing categories:
  rejected Upstox access token, DNS/network unavailable, or an unspecified
  snapshot failure requiring a private log check.
- The browser maps dashboard transport timeouts and connection failures to
  actionable messages and includes the sanitized failure message above the
  retained view.
- Raw backend exception text and API credentials are not sent to the browser.
- Added regression coverage for error classification, retained-view messaging,
  version alignment, and the login popup note.

## Required operator action

An HTTP 401/403 is an Upstox OAuth credential failure. This application receives
only the configured access token and does not have a refresh-token exchange
flow. Renew `UPSTOX_ACCESS_TOKEN` through Upstox OAuth, restart the dashboard,
and then retry. For DNS/network errors, restore host connectivity and retry.

Authenticated dashboard endpoints and server-side token handling are unchanged.
