# Windows release distribution

## Release workflow

The Windows x64 distribution is built by `.github/workflows/desktop-build.yml` when a `v*` tag is pushed. The workflow builds the C11 workstation and command-line tools, runs CTest, captures the desktop application, prepares `Pytyliki-windows-x64.zip`, and writes `SHA256SUMS.txt`.

The release job downloads the archive and checksum, verifies the checksum with `sha256sum --check`, and creates a **draft** GitHub Release. It runs in the GitHub Environment `release-review`.

Repository administrators must configure required reviewers for `release-review` in GitHub Settings before this can be treated as an enforced approval gate. The automation does not claim that the environment already has reviewers configured. A maintainer must inspect the draft and publish it manually.

## Integrity and provenance

The SHA-256 sidecar detects accidental or malicious changes to the archive relative to the recorded digest. It is not a digital signature and does not authenticate the publisher. This workflow does not yet provide Authenticode signing, a signed software bill of materials or an independent signing identity.

## Rollback and scope

To withdraw a candidate, close or delete its draft release. For a published version, use the previous known-good tagged archive. Pytyliki is a local desktop/CLI workstation, not a hosted service; this release workflow does not deploy a web application or mutate processor settings, firmware or drivers.
