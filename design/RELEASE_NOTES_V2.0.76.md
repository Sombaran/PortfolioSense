# Release Notes v2.0.76

## Local secret and Windows build-file hygiene

The repository now ignores common local `.env.*` variants such as
`.env.production` and `.env.local`, plus backup copies of the login and Upstox
environment files. Safe `.env.example` and
`.env.template` files remain eligible for version control. Windows Visual
Studio user state, local CMake presets, and generated CMake build metadata are
also ignored, alongside the existing Bazel and build directories.

These rules reduce accidental staging of credentials and machine-specific
generated files. `.gitignore` is not a secret-removal mechanism: files already
tracked remain tracked and must be removed from Git history separately if they
contain secrets. Linux build and runtime behavior are unchanged. The
post-login release popup summarizes the change.

## Validation

- Add regressions that check ignored credential/build paths and allowlisted
  safe environment templates using `git check-ignore`.
- Run the Python regression suite and `git diff --check`.
