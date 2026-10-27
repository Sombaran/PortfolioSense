# Release Notes v2.0.77

## Bazel version display and market-holiday tooltip

Bazel targets now define `PORTFOLIO_HEALTH_VERSION`, so the dashboard and CLI
report the project semver instead of falling back to `unknown`. Long market
holiday names and explanatory status text now wrap inside the dark tooltip,
keeping the light foreground readable and contained on narrow screens.

The authenticated market-holiday endpoint, data freshness behavior, and
server-side read-only Stock API access are unchanged. Linux runtime behavior
is unchanged. The post-login release popup summarizes both fixes.

## Validation

- Check the version definition in the Bazel build and release metadata.
- Run the focused Python regression tests and the Windows-compatible Python
  regression suite.
- Run `git diff --check`.
