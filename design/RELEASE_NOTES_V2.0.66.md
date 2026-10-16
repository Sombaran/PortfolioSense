# Release Notes — v2.0.66

## Actionable Refresh failures

The prior generic warning concealed the reason a manual Refresh failed even
when the server had logged an upstream error. Refresh now keeps the displayed
data and shows a safe explanation:

- HTTP 401/403: Upstox rejected the access token. Renew it through Upstox OAuth
  and restart the dashboard.
- DNS/network failure: Check internet access, DNS, and proxy connectivity.
- Other snapshot failure: Check the private dashboard log for details.

Recent dashboard logs contained HTTP 401 responses and DNS lookup failures.
Retrying the button cannot renew an expired/rejected token or repair DNS; those
environmental causes must be corrected first. Raw backend exception strings
and credentials are not exposed to the browser. Authentication and the
server-side Stock API boundary are unchanged. The post-login popup summarizes
the change.
