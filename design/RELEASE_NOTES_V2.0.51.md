# Release Notes v2.0.51

**Version:** 2.0.51  
**Date:** October 12, 2026

## Dashboard refresh and data availability

- Distinguish expired or unauthorized Upstox credentials from ordinary network/offline fallback data. Temporary connectivity failures no longer incorrectly tell users to renew their token.
- When a refresh fails while fallback data is displayed, use the retained snapshot's authentication warning to give consistent recovery guidance.
- Detect authenticated API requests redirected to the login page after the browser session expires and navigate to login, rather than treating the HTML page as invalid JSON and leaving the dashboard stuck.
- Keep the current page and last displayed data available on refresh failure; style the Overview's in-panel refresh button consistently with the dashboard toolbar.
- Clarify that Upstox access tokens are server-side and are not renewed by the browser. Set the renewed `UPSTOX_ACCESS_TOKEN` in the environment and restart `./run.sh --web`.
- Preserve authenticated routes, server-side credential handling, snapshot retry limits, and existing Stock API request boundaries.

## Release transparency and tests

- Added this v2.0.51 fix summary to the post-login release popup.
- Aligned CMake, Conan, Bazel, README, and design metadata on `2.0.51`.
