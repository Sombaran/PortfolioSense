# Release Notes v2.0.53

**Version:** 2.0.53  
**Date:** October 15, 2026

## Simplified Operations navigation

- Moved the existing read-only Holdings JSON view into Operations, alongside Positions, Data health, Configuration, and Download.
- Removed JSON from the top-level navigation and kept it available as a responsive Operations subview.
- Reused the existing authenticated `/api/holdings` endpoint and the existing escaped JSON renderer; no Stock API route or credential path was added.
- Kept the shared responsive layout and rendering optimizations in place across dashboard pages.

## Compatibility and security

- Existing holdings data, authentication, refresh behavior, and Stock API access remain unchanged.
- JSON remains read-only, response content is HTML-escaped, and API credentials stay server-side.

## Release transparency and tests

- Added the v2.0.53 fix summary to the post-login release popup.
- Added regression coverage for the Operations JSON route, payload escaping, responsive layout, and aligned release metadata.
- Aligned CMake, Conan, Bazel, README, and design metadata on `2.0.53`.
