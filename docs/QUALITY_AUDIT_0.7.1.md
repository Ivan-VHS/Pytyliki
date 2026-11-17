# Pytyliki 0.7.1 — Repository Quality Audit

> Historical snapshot dated 2026-11-16, limited to the PR #1–#15 audit batch. It does not describe the later open integration queue (#23–#32) or its current review state.

**Audit scope:** the complete pull-request history available on 2026-11-16, the consolidated main implementation, CMake/CTest wiring, CI workflow definitions, core C11 modules, Win32 GUI, CLI, DSP path and improvement/evolution APIs.

This is a repository engineering review supported by source inspection and GitHub Actions evidence. It is not a formal security certification, silicon validation, fuzzing campaign or independent third-party code review.

## Consolidation and PR disposition

| PR | Result | Disposition |
|---:|---|---|
| #1 | Merged | Initial native C11 CPU lab |
| #2 | Closed, not merged | Hardware-rule work superseded by consolidated #9 |
| #3 | Merged | Earlier engineering modules; unrelated clinical modules subsequently removed by #9 |
| #4 | Merged | Native Win32 workstation and DSP suite |
| #5 | Closed, not merged | Processor-only refactor superseded by #9 |
| #6 | Closed, not merged | Recursive improvement engine superseded by #9 |
| #7 | Closed, not merged | Engineering foundations superseded by #9 |
| #8 | Closed, not merged | Duplicate consolidation superseded by #9 |
| #9 | Merged | Processor-only consolidation: scanner, 20-capability engine, hardware rules, policy and pipeline plan |
| #10 | Merged | Guarded 20×3 automation protocol documentation |
| #11 | Closed, not merged | Initial HyperFusion implementation superseded by #12 |
| #12 | Merged | Bounded six-module HyperFusion coordinator |
| #13 | Merged | Exact requested 20×3 capability labels and regression tests |
| #14 | Merged | Verified PR auto-merge workflow and approval freshness/token-scope hardening |
| #15 | Merged | DSP validation, scanner deduplication and JSON report fixes |

All 15 PRs are closed. Nine were merged; six were closed as superseded by consolidated implementations. No open PR remained at the end of this audit.

## Defects corrected

1. **Non-finite tone parameters:** the public tone API now rejects NaN and infinity before converting a floating-point frame count to size_t; the frame count is bounded before allocation.
2. **Non-finite audio samples:** all DSP, measurement and WAV-export entry points validate sample finiteness. Gain processing preflights output range before mutating the buffer, preventing partial updates on float overflow.
3. **Numeric parsing:** CLI parsing rejects range errors and non-finite values; Win32 GUI parameter parsing rejects non-finite values.
4. **Duplicate source findings:** generic SIMD findings no longer duplicate the more specific prefetch/streaming-store finding on the same source line; overlapping inline-assembly labels are deduplicated.
5. **JSON report contract:** JSON export validates terminated CPU strings and finite, non-negative benchmark measurements. The embedded report version now matches 0.7.0.
6. **Capability catalogue:** the 20×3 engineering/optimization/automation matrix now matches the requested terminology and has explicit regression assertions.

## CI evidence

The following pull-request runs completed successfully:

- **PR #9:** C11 CI on Windows and Ubuntu; Recursive Evolution Gate.
- **PR #12:** C11 CI on Windows and Ubuntu; Recursive Evolution Gate.
- **PR #13:** C11 CI on Windows and Ubuntu; Recursive Evolution Gate.
- **PR #14:** C11 CI on Windows and Ubuntu; Recursive Evolution Gate. The verified-auto-merge approval job failed closed because no independent approval was present at that run; the PR was later merged separately. Do not interpret that failed approval job as an auto-merge success.
- **PR #15:** C11 CI on Windows and Ubuntu; Recursive Evolution Gate.

For PR #15, both Windows and Ubuntu completed Configure, Build and CTest successfully. The additional JSON, DSP finite-input and scanner-deduplication assertions were included in that test run. PR #13 separately validated the exact capability-catalogue changes on both operating systems.

## Current implementation boundaries

- The hardware-rule engine contains **12 executable rules**. The 1,000-ID space is reserved; unimplemented IDs are not reported as executed.
- The C11/C23 pipeline is a deterministic reference plan, not a production transpiler.
- HyperFusion computes bounded advisory readiness/synergy indicators; it does not rewrite source or independently certify CI.
- CPU discovery is read-only. AVX-family CPUID flags do not prove OS vector-state support.
- No firmware, BIOS/UEFI, microcode, voltage, clock, power-limit, kernel, driver or device-register writes are performed.
- The improvement engine records evidence and requires human approval at integration. It does not self-approve or self-merge.
- The verified auto-merge workflow separates read-only validation permissions from write permissions, checks the latest independent approval against the triggering PR head SHA, and does not use the PR author's own review. PR #16 additionally hardens the gate by re-reading the live PR head before enabling auto-merge and using strict shell failure handling so API or parsing errors fail closed. GitHub branch protection must also require the relevant checks and review approval; a workflow alone cannot prevent an authorized maintainer from manually merging a PR after a failed workflow job.

## Recommended ongoing release gate

1. Require the Windows and Ubuntu CMake/CTest jobs.
2. Require Recursive Evolution Gate.
3. Require at least one approval from someone other than the PR author, with stale approvals dismissed on new commits.
4. Keep main protected; disallow force pushes and bypasses.
5. Merge only the tested head SHA; re-run checks after any update.
6. Preserve benchmark inputs, outputs and platform details; do not describe heuristic scores as measured hardware gains.

**Audit result:** the consolidated processor workstation and its meaningful follow-up PRs are integrated into main; the identified DSP, scanner and report-contract defects are fixed and covered by passing Windows/Linux CI. The remaining items above are explicit scope/governance limitations, not claims of completed hardware synthesis or autonomous source rewriting.


## APSIA planner integration addendum

The APSIA foundation is extended with a dedicated `pytyliki-apsia` CLI, explicit workload/capability inputs, Windows/Linux CTest coverage, and an evolution-lock JSON Schema syntax check. The CLI exposes the existing advisory router; it does not compile, load or execute generated modules. Cross-language compiler workers, production eBPF/APM, shadow traffic and runtime hot swapping remain future phases and must not be described as shipped.
