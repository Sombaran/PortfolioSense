# Implementation Summary v2.0.57

## Root cause and fix

The Overview refreshed live quotes every three seconds using `setInterval`
without an in-flight guard. When a fetch took longer than the polling interval,
additional requests could accumulate, increasing browser memory and DOM work
and contributing to the attached Firefox tab-crash symptom.

Quote refreshes are now single-flight and have an eight-second abort timeout.
The request is cancelled when the document becomes hidden, and expected aborts
are not logged as refresh failures. Holdings are indexed by instrument key
once per refresh, removing repeated linear searches. The arbitrary 200-update
break was removed so every changed quote is applied.

The UI continues to use the authenticated local quote-cache endpoint. The fix
does not expose Stock API credentials or create a new external API path.

## Version and login popup

The patch release is `2.0.57` in x.x.x semantic-version format. CMake, Conan,
Bazel, the README, design index, and runtime popup use the same release
version. The post-login popup describes the crash root cause, safeguards, and
unchanged server-side Stock API security boundary.

## Validation

Regression coverage checks the live polling safeguards and consistent release
metadata. Offline tests do not verify a live Upstox connection, which requires
user credentials.
