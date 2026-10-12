# baran-capital-view v2.0.51 - Implementation Summary

## Root cause

The dashboard labeled every non-live holdings fallback as requiring token renewal, including data retained during ordinary offline/network failures. That gave users misleading recovery guidance. Separately, the one-hour browser session could expire while the dashboard stayed open; API fetch followed the server's login redirect, tried to parse the login HTML as JSON, and left the page stuck. An expired Upstox token also cannot be corrected by repeatedly pressing Refresh: the API client keeps its credential server-side from process startup.

## Fix

- Derive the Overview's unavailable-value label from the retained snapshot's actual token warning rather than assuming every fallback is an expired credential.
- Reuse an expired-token warning when a later page refresh fails, while continuing to retain the last rendered data.
- Detect API fetch redirects to `/login` and navigate the browser to the login page when its session expires.
- Style the in-panel refresh action consistently and disable it while a request is running.
- Document the token-renewal and server-restart requirement in the README and post-login release popup.
- Preserve authenticated Stock API access, existing request limits, and server-side-only credentials.

## Validation

- Add focused regression coverage for expired-token versus offline fallback messaging and release-version wiring.
- Run Python regression tests and the existing C++ test/build path.
