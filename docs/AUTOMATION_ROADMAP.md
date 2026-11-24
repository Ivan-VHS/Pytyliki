# Automation Roadmap — Pytyliki

**Contract:** `anatidae.automation-roadmap.v1`  
**Repository:** `Ivan-VHS/Pytyliki`  
**Default branch:** `main`

## Scope

Processor and ISA discovery, bounded compute and memory measurements, local DSP, and embedded reference tooling.

Stack: C11, CMake, Python 3.

This roadmap turns the 20 requested directions into traceable workstreams. It distinguishes existing automation from work that still needs to be built: **active** means a relevant control already exists; **extend** means a control exists but does not yet deliver the full target; **planned** means it is not enabled and must not be described as implemented.

## Execution phases

1. **Baseline and contract.** Keep the roadmap, schema, documentation, and evidence references synchronized. The contract workflow checks the registry on pull requests, relevant pushes, a weekly schedule, and manual dispatch.
2. **Continuous verification.** Extend existing CI, architecture, quality, branch-synchronization, and verified auto-merge controls without duplicating them or weakening required checks.
3. **Recovery, scale, and release gates.** Introduce bounded recovery and resource-aware validation only after tests. Deployment and publication remain disabled until a protected target, rollback procedure, and human approval are defined.

## Workstream register

| ID | Direction | Priority | Status | Current evidence |
|---|---|---:|---|---|
| `01-plan-governance` | Запланировать | P0 | active | `roadmap-governance.v1.json`, `roadmap-governance.v1.schema.json`, `validate_delivery_contracts.py`, `test_delivery_contracts.py` |
| `02-organize-triggers` | Организовать | P0 | active | `.github/workflows/verified-auto-merge.yml`, `.github/workflows/update-behind-pr-branches.yml` |
| `03-systematize-sync` | Систематизировать | P0 | active | `project-contract.json`, `federation/octa-pyramid.v1.json` |
| `04-prioritize-change-flow` | Приоритизировать | P1 | active | `improvement-conflict.yml`, `quality-remediation.yml`, `.github/ISSUE_TEMPLATE/engineering_improvement.yml` |
| `05-refresh-project-config` | Актуализировать | P0 | active | `project-contract.json`, `.architecture/resilience-policy.json` |
| `06-optimize-toolchain` | Оптимизировать | P1 | extend | `.architecture/resilience-policy.json`, `.architecture/resilience-policy.schema.json` |
| `07-synchronize-branches` | Синхронизировать | P0 | active | `.github/workflows/update-behind-pr-branches.yml` |
| `08-integrate-projects` | Интегрировать | P0 | active | `project-contract.json`, `federation/octa-pyramid.v1.json` |
| `09-modernize-quality-flow` | Модернизировать | P0 | active | `.github/workflows/quality-remediation.yml`, `quality-remediation.v1.json` |
| `10-automate-diagnostics` | Автоматизировать | P0 | active | `.github/workflows/quality-remediation.yml`, `.github/workflows/update-behind-pr-branches.yml` |
| `11-structure-test-updates` | Структурировать | P0 | active | `.github/workflows/quality-repeatability-native.yml`, `tests/test_core.c` |
| `12-scale-validation` | Масштабировать | P1 | active | `ci-scale-policy.v1.json`, `ci-scale-policy.v1.schema.json`, `validate_delivery_contracts.py`, `test_delivery_contracts.py`, `.github/workflows/ci.yml` |
| `13-implement-verification` | Реализовать | P0 | active | `.github/workflows/quality-commercial.yml`, `.github/workflows/quality-remediation.yml` |
| `14-coordinate-deployment` | Координировать | P2 | not_applicable | `.github/workflows/verified-auto-merge.yml`, `project-contract.json` |
| `15-consolidate-test-matrix` | Консолидировать | P1 | active | `.github/workflows/quality-repeatability-native.yml`, `tests/test_core.c` |
| `16-improve-recovery` | Усовершенствовать | P1 | active | `safe-ci-recovery.yml`, `automation-rerun-policy.v1.json`, `validate_safe_rerun.py`, `test_safe_rerun.py` |
| `17-test-controls` | Протестировать | P0 | active | `.github/workflows/quality-remediation.yml`, `tests/test_core.c` |
| `18-verify-observability` | Верифицировать | P1 | active | `verified-auto-merge.yml`, `update-behind-pr-branches.yml`, `safe-ci-recovery.yml` |
| `19-document-automation` | Документировать | P1 | active | `docs/README.md`, `docs/DOCUMENTATION_CONTROL.md`, this roadmap |
| `20-release-publication` | Развернуть | P2 | extend | `.github/workflows/desktop-build.yml`, `docs/RELEASE.md`, `project-contract.json` |

## Operating rules

- The new roadmap workflow is read-only and uses `contents: read`.
- The roadmap validator is also run inside the existing verified merge gate (or the CI gate for VentuSample).
- Existing verified auto-merge remains conditional on successful checks and repository protection; this roadmap does not bypass either.
- Branch synchronization must use an expected head SHA, operate only on eligible same-repository pull requests, and leave conflicts for maintainers.
- Diagnostics, schema checks, and scheduled audits must be deterministic, bounded, and non-destructive.
- Automated code correction may produce a reviewable proposal; it must not silently rewrite source or commit directly to `main`.
- This roadmap does **not** authorize production deployment, external publication, secret rotation, destructive cleanup, firmware flashing, or driver installation.
- A workstream marked `planned` or `extend` is not complete. Update its status only with implementation and passing evidence.

## Validation

Run locally:

```bash
python scripts/validate_automation_roadmap.py --strict
```

The validator uses only the Python standard library and checks all 20 identifiers, project metadata, safety settings, documentation coverage, and the existence of every referenced evidence file.

## Additional execution controls

- **Prioritization (04):** new improvement issues require P0/P1/P2, impact, reproduction evidence, acceptance criteria, risk/dependency notes, verification and rollback.
- **Recovery (16):** `.github/workflows/safe-ci-recovery.yml` is manual-only, runs from protected `main`, permits failed/timed-out allow-listed validation runs from this repository only, reruns failed jobs only, and enforces two total attempts (one retry). Fork runs, cancellations, successful runs, unlisted workflows and exhausted attempts are rejected.
- **Observability (18):** the recovery decision is written to the GitHub Actions step summary with workflow, run ID, branch, commit and attempt number. Every recovery request is linked to its original Actions run.
- **Documentation (19):** policy, implementation, tests, issue intake and workflow are linked by the roadmap registry and its evidence validator.

## Deliberate release boundaries

Workstream 14 is `not_applicable`: this repository is a local desktop/CLI product without a hosted production service or remote control plane. Desktop archive distribution belongs to workstream 20, not production deployment. Re-open this direction only after an approved product decision defines a real deployable target, owner, threat model, protected environment and rollback plan. Workstream 20 is now `extend`: the tag workflow verifies the archive checksum and creates a draft release behind the `release-review` environment. It remains incomplete until required reviewers are configured in that environment, a maintainer verifies the approval path, and a publisher-signing identity/provenance process exists. The recovery workflow does not deploy or publish artifacts.

## Governance and CI scale controls

- **Governance (01):** the versioned roadmap-governance policy defines status-evidence requirements, priority review, change control and release boundaries. Its Draft 2020-12 schema and negative-case tests are enforced by `validate_delivery_contracts.py` and the main roadmap validator.
- **Bounded validation (12):** the CI scale policy caps the native matrix at three jobs and 35 minutes per runner job, and the focused quality contract at one job and 15 minutes. Both workflows require explicit concurrency and cancellation behavior; future matrices must declare `max-parallel`. Required Windows/Linux/sanitizer coverage is retained.
- **Toolchain optimisation (06):** remains `extend` until a reproducible before/after toolchain benchmark is captured; no default compiler or linker settings are changed without measured evidence.
