# Release Notes — v2.0.67

## Overview controls

Removed the extra Overview Update/Refresh metric card, which duplicated the
main dashboard Refresh control. The main Refresh button remains available in
the command bar.

Fixed Market Value and Day P&L eye controls by handling clicks through event
delegation on the stable dashboard view instead of binding handlers to metric
buttons that are repeatedly recreated. Visibility preferences continue to be
stored locally and masked values remain hidden after a view rerender.

Existing page optimizations, authentication, and server-side Stock API
credential handling are unchanged. The post-login popup summarizes the fix.
