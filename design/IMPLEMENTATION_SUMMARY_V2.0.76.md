# Implementation Summary v2.0.76

## Root cause and changes

The existing ignore rules covered the primary `.env` and Upstox credential
files but not environment variants such as `.env.production`, credential-file
backup names, or per-machine Windows Visual Studio and CMake state. Those
files can contain local credentials or machine-specific paths and are easy to
stage by mistake.

The ignore rules now cover `.env.*` while explicitly allowing the safe
`.env.example` and `.env.template` names, common login/Upstox backup suffixes,
Visual Studio user state, CMake user presets and generated metadata, and
standard CMake build directories. Existing explicit build outputs remain
unchanged. The documentation explains that ignore rules do not affect files
already tracked by Git.

No launcher or platform-specific runtime code changed. Linux behavior is
unchanged, and API credentials continue to be loaded and kept server-side as
before. The post-login popup includes the ignore-rule summary.

## Files and validation

- Update `.gitignore` with narrow secret and Windows-generated-file patterns.
- Add unit coverage for both ignored paths and safe environment template
  exceptions.
- Update the README, Windows run steps, design index, release notes, and
  versioned login popup to 2.0.76.
