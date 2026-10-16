# Implementation Summary v2.0.62

## Changes

- Replaced the Overview metric label `Refresh` with `Manual update`, so it no
  longer repeats the Refresh wording in the `Refresh now` button.
- Added an accessible `type="button"` eye control with an updated accessible
  name and `aria-pressed` state for hiding/showing the Market value.
- Persisted only the hide/show preference in `localStorage`; market amounts are
  not written to browser storage.
- Kept masking synchronized with Overview rerenders, stale/offline display
  states, and live quote updates.
- Added source regression checks for these controls and release/version
  alignment. Included the v2.0.62 summary in the post-login popup.

## Security and compatibility

The control is presentation-only. It does not change portfolio values, refresh
requests, authentication, or the server-side Stock API integration. Existing
credentials remain server-side and inaccessible to the browser.
