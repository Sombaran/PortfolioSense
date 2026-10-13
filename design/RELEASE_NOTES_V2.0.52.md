# Release Notes v2.0.52

**Version:** 2.0.52  
**Date:** October 13, 2026

## One-hour dashboard stability

- Fix the one-hour interruption by renewing the existing HttpOnly session cookie whenever a valid authenticated request is made.
- Use a sliding one-hour idle timeout: active dashboard use stays signed in; inactive sessions expire and are removed.
- Apply renewal to the dashboard page and authenticated JSON/metrics responses. A visible tab also refreshes its session every ten minutes through a lightweight local endpoint that makes no Stock API requests.
- Preserve the existing SameSite=Strict, HttpOnly cookie, authenticated route checks, and server-side-only Upstox credentials.

## Release transparency and tests

- Added this v2.0.52 fix summary to the post-login login popup.
- Aligned CMake, Conan, Bazel, README, and design metadata on `2.0.52`.
- Added regression coverage for idle expiry, authenticated cookie renewal, and version alignment.
