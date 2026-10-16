# Release Notes v2.0.64

## Manual refresh recovery

- The required Upstox holdings request now receives one bounded retry for
  retryable transport failures. Authentication errors and HTTP response errors
  are not retried.
- Dashboard API requests now have a shared 50-second browser deadline. This
  leaves headroom over the maximum 40-second backend snapshot budget when the
  holdings call uses its retry and positions/news each use one bounded attempt.
- Positions and news remain optional to the holdings view; their failures do
  not discard a successfully refreshed portfolio.
- Stock API calls remain authenticated, use the existing HTTPS/host validation,
  and keep broker credentials on the server.

The post-login popup summarizes this release.
