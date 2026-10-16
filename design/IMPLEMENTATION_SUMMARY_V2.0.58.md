# Implementation Summary v2.0.58

## Root cause and fix

The crash was not caused by quote-fetch overlap alone. The Alerts page had a
self-triggering `MutationObserver`: every observed descendant mutation caused
`normalizeAlertsControls()` to set the heading text. That text assignment
created another child-list mutation, which re-entered the observer and
repeatedly scanned the Alerts view.

The heading now changes only if it is not already normalized. Alerts,
Overview, and Deeper analysis page-level normalizers observe top-level view
replacement only. Other view normalizers that process links, serial headers,
or shared page state likewise run on view replacement, not on each nested
live-price update. Existing quote polling bounds from v2.0.57 remain in place.

## Version and login popup

The patch release is `2.0.58`, aligned across CMake, Conan, Bazel, the README,
design index, and runtime version popup. The post-login popup calls out the
Alerts mutation loop and the observer scope correction.

## Security and validation

Stock API requests remain authenticated and server-side; no broker token or
authorized WebSocket URL is exposed to browser code. Regression tests assert
that heading normalization is idempotent and the Alerts observer is not
subtree-wide. Live Upstox behavior still requires user credentials.
