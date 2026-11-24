# Roadmap governance and bounded CI scale

**Repository:** `Ivan-VHS/Pytyliki`  
**Policy contracts:** `.architecture/roadmap-governance.v1.json` and `.architecture/ci-scale-policy.v1.json`  
**Validation:** `tools/validate_delivery_contracts.py`  
**Status:** enforced by C11 CI, quality-repeatability CI, automation roadmap CI and quality-remediation CI.

## Purpose

This document defines how roadmap status, engineering priority, CI execution limits and release boundaries are decided. It is a control contract, not a declaration that every proposed capability is implemented.

The JSON policies and schemas are normative for their machine-readable fields. The validator checks the policies against the repository contract and the named workflow files. Regression tests deliberately feed invalid status controls, workflow timeouts, matrix settings and repository identities to ensure unsafe changes fail.

## Capability maturity

| Status | Required evidence | Permitted claim |
|---|---|---|
| `active` | Existing implementation/control, valid evidence paths, regression verification and an executable acceptance gate | The scoped control exists and its stated checks run; do not generalize beyond the tested surface |
| `extend` | Existing partial control, explicit gap and a next measurable acceptance gate | The target is only partially delivered |
| `planned` | Explicit blocker, owner/dependency and exit criteria | Not implemented; no shipped-capability claim is permitted |
| `not_applicable` | Explicit product-scope rationale, owner and a trigger that would change the product boundary | The capability is outside this product's current scope; it is not an unfinished feature |

A `not_applicable` status is valid only when the machine-readable roadmap includes a specific reason and the validator tests that reason. Use it for capability areas outside the approved product boundary; do not use it to hide incomplete implementation.

A Markdown checkbox, catalog entry, schema, interface declaration or passing documentation validator is not runtime implementation evidence. Contract validation proves the declaration and paths; it does not prove hardware access, timing guarantees, release provenance or production deployment.

## Priority and change control

- **P0** covers safety, data integrity, compatibility breaks and release blockers. A maintainer must review the rationale and evidence before merge.
- **P1** covers core workflow correctness, maintainability and measurable capability gaps. Peer or maintainer review is required.
- **P2** covers bounded improvements with a supported workaround. Normal review and documented acceptance evidence still apply.

Semantic status changes are versioned in a pull request. Each behavior-changing proposal must include affected workflows, reproduction evidence, acceptance criteria, risks/dependencies, verification commands and a rollback path. Branch updates use an expected head SHA; stale heads and merge conflicts are left for review.

## CI execution budget

The current policy sets a global ceiling of four runner jobs per workflow, 45 minutes per job and three parallel matrix workers. Named workflows use tighter project-specific budgets:

| Workflow | Runner jobs | Per-job ceiling | Matrix parallelism |
|---|---:|---:|---:|
| C11 CI | 3 | 35 min | 3 |
| Quality, Repeatability and Native Contract | 1 | 15 min | 1 |
| Automation Roadmap Contract | 1 | 10 min | 1 |
| Quality remediation matrix | 1 | 15 min | 1 |
| Quality and commercial readiness | 1 | 15 min | 1 |
| Improvement and conflict contract | 1 | 10 min | 1 |
| Safe CI Recovery | 1 | 5 min | 1 |
| Update Behind Pull Request Branches | 1 | 15 min | 1 |
| Verified Mainline Auto-Merge | 3 | 35 min | 2 |

The validator requires workflow-level concurrency groups and an explicit cancellation policy. Each runner job must declare `timeout-minutes`. Every strategy matrix must declare `max-parallel` and remain within the budget. Required Windows, Linux and sanitizer jobs are not removed to meet a budget.

The budget is an upper bound, not a request to consume the entire allowance. Test selection may become more focused only when equivalent coverage is demonstrated and retained as a required gate.

## Safe recovery and release boundaries

CI recovery is defined separately by `automation-rerun-policy.v1.json`: manual dispatch only, protected `main`, same-repository workflow allowlist, failed/timed-out conclusions only, failed jobs only and one retry maximum. It cannot retry a fork run, successful/cancelled/action-required run, unlisted workflow or a run that has already consumed the retry. It does not rerun deterministic DSP outputs or perform production operations.

Production deployment and public release remain disabled until a real target, protected environment, human approval, rollback process, signing identity and provenance evidence are configured. Pytyliki is a local processor-systems workstation; its CI artifact is not automatically a production deployment.

Hardware-changing operations remain forbidden in CI: no driver/firmware installation, flashing, physical MMIO, clock/voltage mutation or destructive cleanup.

## Toolchain optimization

Workstream 06 remains `extend`. CMake/compiler/linker defaults will not be changed on intuition. Before optimization is accepted, record a reproducible baseline (compiler/version, host/runner, build flags, clean/incremental build time, peak memory, artifact identity), compare the proposed change under the same conditions, and run the full native and sanitizer gates. Performance regressions or platform coverage loss reject the change.

## Verification

Run from repository root:

```sh
python tools/validate_delivery_contracts.py --self-test
python -m unittest discover -s tests -p "test_delivery_contracts.py" -v
python scripts/validate_automation_roadmap.py --strict
python -m unittest discover -s tests -p "test_safe_rerun.py" -v
```

These commands validate contracts and negative cases. The C11, Windows, Linux and sanitizer workflows remain the authoritative build/test qualification.
