# Implementation Summary — v2.0.67

## Changes

- Removed the Overview's extra Update/Refresh metric card; the existing global
  Refresh control remains available.
- Changed the Market Value and Day P&L eye controls to use event delegation on
  the persistent `#view` container. This avoids relying on one-time listeners
  attached to dynamically rebuilt metric buttons and handles clicks on nested
  SVG icon elements.
- Visibility state remains in memory and its local preference continues to
  apply when Overview is rendered again. `setMetricVisibility` now synchronizes
  the value, button accessibility label, pressed state, title, and icon together.

## Regression coverage

Python source-level tests verify removal of the redundant metric, retention of
the main Refresh button, delegated metric click handling, and visibility-state
synchronization. Existing UI optimization and authenticated Stock API routes
are retained.

## Login popup and security

The post-login release popup includes the v2.0.67 summary. No Stock API
credentials are added to browser state, and no authentication or backend API
route behavior is changed.
