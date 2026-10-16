# Release Notes v2.0.63

## Overview privacy and layout

- Fixed the Refresh now control overlapping its card label by removing the
  `Manual` wording and placing the button on its own line beneath `Update`.
- Added an accessible eye control beside Day P&L to hide or show its amount,
  alongside the existing Market value control.
- Saved only the independent hide/show preferences in browser storage; no
  financial values are persisted there.
- Preserved both hidden states across Overview rerenders and live quote updates.
- Left refresh behavior, authenticated Stock API requests, and server-side
  credential handling unchanged.

The post-login popup summarizes this release.
