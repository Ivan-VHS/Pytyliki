# Consolidated integration audit — 2026-11-19

This document records how the open Pytyliki work was reconciled against the current `main` snapshot. It is an inventory, not a claim that every historical proposal has been merged unchanged.

## Baseline

- Base branch: `main`
- Base commit: `a06ebc12c4063c2d0d0cc7e4a28223cbfb5eeda6`
- Integration policy: preserve the current main tree, port compatible changes, retain source PRs for review, and never force-push or weaken branch protection.

## Incorporated in this integration

| Source PR | Change | Integration |
|---|---|---|
| #40 | Independent audio measurements per channel | Ported into the DSP API, CLI, Win32 measurement log and unit tests |
| #43 | WAV/RIFF boundary validation | Ported into the WAV reader with explicit container/chunk extent checks and adversarial fixtures |
| #45 | HMGA v2 governed evolution | Added the read-only repository auditor, tests and policy document |
| #46 | Engineering standard and PR hygiene | Added the engineering standard and PR template; audit workflow is additive |
| #47 | HMGA v3 operator matrix | Added the deterministic 20-axiom/400-descriptor catalogue, 64-round cap and CTest coverage |
| #49 | Shared monotonic millisecond clock for processor and benchmark timing, with a finite/non-decreasing timing contract test |
| #50 | Readability-only CPU Suite Pro catalog lookup cleanup; existing positive/negative lookup coverage retained |
| #48 | Native Win32 white-red-blue theme | Added the palette and themed owner-drawn controls |

## Already represented by current main

The current main tree already contains the processor-native DSP foundation, Forge66, CPU Suite Pro, TriMatrix AutoForge, the 20-system registry, their associated tests/docs, the Anatidae engineering profile, four localized READMEs, and three SVG visual assets. Replaying the stale snapshots of PRs #25, #26, #27, #29, #30, #35, #36 or #37 wholesale would overwrite newer integrated code or duplicate existing features. Those changes are therefore treated as already represented, not re-imported.

PR #34 targets an older integration branch rather than current main; its policy material is already represented by the current architecture/constitution documents. Its stale branch is not used as a new source tree.

## Visual documentation

The README contains two refreshed, source-aligned vector previews: the white-red-blue Win32 workstation layout and the stereo metering panel. They are explicitly marked as illustrative previews. This repository snapshot does not contain an actual screenshot captured from a running Windows executable; genuine screenshots require running the built GUI on Windows.

## Verification boundaries

The new tests cover stereo channel-statistics validation, WAV/RIFF container bounds, and HMGA v3 descriptor/budget enumeration. The HMGA auditor is read-only. CI is expected to provide the authoritative compiler, platform and test results for the pull request. No local build result is claimed by this inventory.
