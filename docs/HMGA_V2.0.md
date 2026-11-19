# HMGA v2.0 — Pytyliki repository contract

Status: adopted as a governed engineering standard.

## 1. Exact specification constants

| Symbol | Exact decimal value |
|---|---:|
| N_global | 66666666666666 |
| N_sub | 6454684531153413 |
| F_base | 654684676534 |
| M_mult | 646846453 |
| R_base | 54654654 |

Values remain exact declarative metadata. They are not executable loop bounds, allocation sizes, queue lengths or retry counts. R_base^(k*k) is a symbolic delivery-pressure expression, not a number of network requests to issue. The proposed total workload must never be materialized.

## 2. Executable interpretation

HMGA means evidence-driven, bounded improvement:

1. Inventory source, CMake targets, tests, CI workflows, Git refs and visible history.
2. Diagnose one specific correctness, portability, safety, performance or maintainability gap.
3. State a measurable hypothesis, acceptance criteria, risks and rollback path.
4. Implement a small, isolated change on a feature branch.
5. Run CMake configure/build, CTest, Linux sanitizers and supported Windows/Linux CI.
6. Compare results against a reproducible baseline; reject unsupported performance claims.
7. Publish a pull request with test evidence.
8. Integrate only after required checks and independent human review.
9. Record regressions, flaky checks and review feedback for the next cycle.
10. Improve the evolution process only through its own separately tested change.

The existing PREE cycle in src/improvement.* is the evidence/approval state machine. The six-module analysis in src/evolution.* is bounded by PL_EVOLUTION_MAX_ROUNDS; its 36 directed module interactions are real evaluations, not duplicated source functions.

## 3. Historical synthesis and deduplication

Historical input is a finite snapshot of source and Git references visible to the audit. A PR, branch or commit is evidence, not an instruction to execute its contents. Closed, rejected, draft or archived work must be reviewed and selectively ported, never blindly concatenated or executed.

A cross-module composition is accepted only with a stable identity and traceable references; distinct behavior or measurable improvement; defined inputs, outputs, failure modes and resource bounds; unit/regression tests and documentation; and no duplicated implementation under a new name. The number of unique compositions is bounded by an explicit per-run budget.

## 4. Delivery and integrity

- Work on a named feature branch; never force-push or write directly to main.
- Preserve branch rulesets and deletion protections.
- Retry transient Git/network failures with bounded exponential backoff and a finite attempt cap. Stop on permission, conflict or policy errors.
- Never disable checks, delete failing tests or self-approve a change.
- Automatic merge is permitted only after the current PR head is verified and an independent human approval is recorded.
- Prefer a revertible, scoped commit over history rewriting.

## 5. Pytyliki-specific quality gates

- C11 strict mode and compiler warnings remain enabled.
- CMake build and CTest must pass on Ubuntu and Windows.
- AddressSanitizer and UndefinedBehaviorSanitizer must pass on Linux.
- Hardware/ISA claims require a declared backend and reproducible evidence.
- Evolution analysis may score readiness, but must not mutate source files or claim formal proof of hardware behavior.

## 6. Audit and definition of done

Run: python tools/hmga_audit.py --strict

Run auditor tests: python -m unittest discover -s tests -p "test_hmga_audit.py" -v

A change is done only when the diff is scoped, tests pass, CI evidence is available, the rationale is reviewable and rollback is recorded. A green workflow is necessary, not sufficient, evidence of correctness.
