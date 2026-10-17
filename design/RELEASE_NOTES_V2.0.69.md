# Release notes — v2.0.69

## API error semantics

- The market-holiday route now returns `502 Bad Gateway` with an explicit error payload when Upstox is unavailable and no previously validated calendar is cached.
- If Upstox is unavailable after a successful calendar load, the route retains that calendar, marks it as stale, and includes a safe warning rather than presenting it as current.
- The browser market-status tooltip now identifies unavailable or stale holiday data and clarifies when regular NSE hours are estimates.

## Dashboard usability

- Every dashboard view marks its shared content region busy while loading, ignores responses and cache updates from outdated tab navigations, and clears the busy state only when the latest navigation finishes.
- Existing responsive layouts, refresh behavior, visibility controls, and authenticated view routing remain unchanged.

## Security and scope

- Upstox access remains read-only; access tokens stay on the server and are not included in browser responses.
- No OAuth callback, token refresh, or order execution capability is claimed by this release.

## Dashboard login popup

The post-login **What is new** popup summarizes the holiday API status behavior, stale-calendar warning, and accessible view-loading state.

## Validation

Regression checks cover the no-cache HTTP 502/error response, stale cached-calendar marker, frontend warning fallback, shared view loading state, and version/popup alignment.
