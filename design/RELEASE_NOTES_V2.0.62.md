# Release Notes v2.0.62

## Overview controls

- Removed the repeated Refresh wording from the Overview metric by labeling it
  **Manual update** while retaining the **Refresh now** action.
- Added an accessible eye button beside Market value to hide and show the
  displayed amount. The preference is saved locally; the financial value itself
  is not stored in browser storage.
- Kept the value hidden across Overview rerenders and live quote updates when
  the user has hidden it.
- Preserved the existing refresh flow and authenticated, server-side Stock API
  boundary. No API credentials or financial values are sent to new endpoints.

The post-login popup includes a summary of this release.
