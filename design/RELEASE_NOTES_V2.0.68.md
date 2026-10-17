# Release notes — v2.0.68

## Reliability and security

- Fixed task-scheduler lifecycle synchronization and now execute queued work by descending priority, preserving FIFO order for equal priorities.
- Generate local session IDs using cryptographic randomness and fail closed if the operating-system random source is unavailable.
- Limit invalid login attempts to five per rolling minute and return `429 Too Many Requests` with `Retry-After` when the limit is reached.
- Return meaningful HTTP error statuses for invalid input, unsupported methods, missing routes, and failed upstream data requests.
- Reject invalid/non-finite portfolio risk inputs instead of accepting an unverifiable simulation.
- Keep the Upstox token on the server and do not expose it to browser code.

## Honest unavailable states

- Live order submission, cancellation, lookup, 2FA authorization, trained ML inference, and external webhook/Firebase/email/SMS delivery are not implemented. Those paths now fail closed or return explicit unavailable/undelivered results rather than success-shaped placeholders.
- Order simulation rejects when sector exposure cannot be verified.
- Upstox OAuth callback and token-refresh handling are not implemented. Supply a valid access token through the documented environment variable and replace it after expiry.

## Dashboard login popup

The post-login **What is new** popup includes this release summary, including scheduler and session hardening, HTTP error handling, and the explicit fail-closed status of incomplete integrations.

## Validation

Regression coverage was added/updated for scheduler priority, fail-closed integrations, random session material, risk inputs, HTTP error semantics, and version alignment.
