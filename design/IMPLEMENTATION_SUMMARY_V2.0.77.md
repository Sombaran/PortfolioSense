# Implementation Summary v2.0.77

## Root cause and changes

The CMake build already injected `PORTFOLIO_HEALTH_VERSION`, but the Bazel
targets did not. As a result, Bazel-compiled code used the existing `"unknown"`
fallback. The Bazel core library and executable now receive the project
version macro; the regression test keeps that definition aligned with the
CMake, Conan, Bazel module, and documentation versions.

The market tooltip had a later UI style rule that explicitly set
`white-space:nowrap`. Long holiday descriptions therefore spilled beyond the
dark tooltip background. The versioned dashboard style now overrides that
rule, permits wrapping, breaks long tokens when needed, and constrains tooltip
width to the viewport.

The Windows batch launch/build scripts were reviewed. Their current root
handling, build selection, stale-input checks, and safe rebuild guards already
cover the relevant Windows workflow; no unrelated batch-file refactor was
needed for these source/build-metadata fixes.

The post-login release popup summarizes the Bazel version and tooltip fixes.
Stock API credentials and holiday requests remain server-side, authenticated,
and read-only. Linux runtime behavior is unchanged.

## Files and validation

- Update coordinated release metadata to `2.0.77`.
- Define `PORTFOLIO_HEALTH_VERSION` on Bazel's core and application targets.
- Add regression assertions for version alignment and tooltip wrapping.
- Validate the focused Python test, Windows-compatible Python suite, and
  `git diff --check`.
