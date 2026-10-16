# Implementation Summary v2.0.63

## Changes

- Corrected the refresh card layout: removed the `Manual` label and made
  `Refresh now` a block-level action below `Update`.
- Added an accessible Day P&L visibility button with an `aria-pressed` state,
  matching the existing Market value control.
- Kept separate visibility preferences for Market value and Day P&L; browser
  storage contains only booleans (`baran-capital-view-hide-day-pnl` and
  `baran-capital-view-hide-market-value`), never the underlying financial
  amounts.
- Routed both initial renders and live quote updates through mask-aware display
  helpers so hidden amounts remain hidden.
- Added regression assertions for visibility, live updates, refresh layout,
  version metadata, and the post-login popup.

## Security and compatibility

These are client-side presentation controls only. They do not alter portfolio
data, refresh flow, authentication, or the server-side Stock API integration.
Broker credentials remain server-side.
