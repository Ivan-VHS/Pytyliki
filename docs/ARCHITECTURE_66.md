# Architecture 66 — Pytyliki autonomy and modularity contract

Status: normative engineering contract; not a claim that all 66 capabilities already exist.

## Architectural decision

Pytyliki is a modular C11 workstation presented as one product. The shared pytyliki_core library is the common execution boundary; the CLI and Win32 GUI are front ends. Keep the implementation internally modular. Do not collapse modules into one source file, introduce services for their own sake, or turn advisory catalogues into privileged hardware controls.

The contract distinguishes three states:

- Implemented — present in the inspected default-branch source and covered by a test or workflow.
- Partial — a foundation or documented policy exists, but the complete capability is not yet present.
- Planned — an acceptance requirement for future work; do not describe it as shipped.

The inspected default branch contains a C11 core, CLI, Windows GUI, CMake/CTest suite, read-only processor/source inspection, bounded workloads, a policy module, improvement planning and APSIA routing planning. Windows/Linux are the CI targets. Claims outside these boundaries require separate evidence.

## 66 rules

### A. One product, modular core

1. Keep one product identity and one primary user-facing workstation.
2. Put shared processor operations behind pytyliki_core; front ends must not fork domain logic.
3. Keep modules separated by responsibility: core, processor inventory, DSP, improvement planning, policy, pipeline and routing.
4. Treat public headers as module contracts; expose the smallest necessary API.
5. Route sensitive operations through an explicit policy decision; default to deny.
6. Keep CLI and GUI behavior semantically aligned by calling the same core functions.
7. Keep platform-specific code behind narrow platform adapters; do not leak Win32 details into portable modules.
8. Add a module to the core only when it has a defined purpose, owner, dependencies, failure mode and tests.

### B. Bounded autonomy

9. Detect capabilities from the actual host and distinguish unavailable telemetry from a negative result.
10. Keep processor discovery read-only.
11. Keep source inspection read-only; never compile or execute scanned source.
12. Make benchmark and DSP workloads bounded by iterations, memory, samples and time.
13. Preserve useful offline operation; network access must not be a runtime prerequisite for core workflows.
14. Keep user settings explicit and local; do not silently infer or rewrite configuration.
15. Make startup report missing optional components without preventing unrelated core features from running.
16. Make errors actionable and source-aware; never replace a failed check with a fabricated success.
17. Add recovery only where a reversible state transition exists; do not automatically alter hardware or user files.
18. Keep every automatic operation observable through a result, diagnostic or structured report.

### C. Automation and iterative improvement

19. Use one documented lifecycle for discover → analyze → plan → verify → promote.
20. Treat improvement catalogues as proposals, not proof of speedup or correctness.
21. Bound every iteration count, recursion depth, queue and generated plan size.
22. Prefer iterative, allocation-aware algorithms for potentially deep or repeated work.
23. Make repeated runs deterministic where inputs and platform evidence are unchanged.
24. Record the inputs and assumptions behind every recommendation.
25. Require explicit authorization before any operation that changes source, configuration or repository state.
26. Keep planning separate from execution; a plan must not acquire execution authority implicitly.
27. Abort safely when resource limits, validation, policy or evidence checks fail.
28. Promote a change only after tests and independent review; automation may request Auto-Merge but may not bypass GitHub protections. A human maintainer remains responsible for approval and promotion; CI evidence alone is not authorization.

### D. Independent modules and integrations

29. Keep each subsystem independently understandable and testable.
30. Keep module dependencies acyclic; document every cross-module dependency.
31. Use stable C interfaces rather than shared mutable globals.
32. Version or deliberately review breaking changes to public interfaces.
33. Use adapters for external projects; do not copy unknown repositories directly into the core.
34. Verify source identity, license, provenance, dependencies and build hooks before importing external code.
35. Pin imported revisions and record checksums for downloaded artifacts.
36. Isolate optional integrations so their absence or failure does not break processor discovery.
37. Test adapters against contract tests and the core regression suite.
38. Do not claim a project has been integrated until its source and runtime behavior have been inspected and validated.

### E. Resource governance

39. Measure CPU, memory and elapsed time where the host exposes reliable telemetry.
40. Give every workload a documented upper bound and a way to stop it.
41. Avoid unbounded allocation, uncontrolled thread creation and indefinite waits.
42. Keep foreground user operations ahead of optional background analysis.
43. Report unavailable telemetry explicitly rather than substituting guessed values.
44. Release temporary buffers and operating-system resources on every success and error path.
45. Use timeouts and bounded retries for operations that can block.
46. Keep normal operation useful on machines with limited resources; expensive analysis must be opt-in or explicitly bounded.

### F. Security, integrity and recovery

47. Apply least privilege; ordinary workstation features must not request elevation.
48. Do not write firmware, microcode, BIOS/UEFI, registers, voltage, clocks, power limits, kernels or drivers.
49. Validate numeric inputs, sizes, paths and externally supplied data at module boundaries.
50. Treat imported code, files and generated artifacts as untrusted until checked.
51. Record provenance for source revisions and generated reports.
52. Use temporary output plus atomic replacement for future file-writing features; preserve the original on failure.
53. Add backup and rollback before introducing any destructive persistent operation.
54. Fail closed when policy, integrity or required evidence cannot be established.

### G. Engineering quality and evolution

55. Keep C11 as the explicit language baseline and use portable, warning-clean code.
56. Register every executable regression test with CTest.
57. Require Windows and Linux configure, build and CTest jobs for changes to shared code.
58. Run sanitizers and static analysis on supported toolchains where available; report their scope accurately.
59. Review every PR for overlap, conflicting assumptions, duplicate implementations and stale base commits.
60. Enable Squash Auto-Merge only through the verified workflow and exact-head review policy.
61. Keep README editions and architecture documents aligned with shipped behavior, not roadmap wording.
62. Make build and test instructions reproducible from a clean checkout.
63. Keep architectural decisions and validation evidence in version control.
64. Add a regression test for each corrected defect and a boundary test for each new resource limit.
65. Track unimplemented capabilities explicitly; do not label a catalogue, planner or mock as a working backend.
66. Treat this contract as a living gate: every new module or subsystem must map to these rules and provide tests or a documented reason why a rule does not apply.

## Current conformance snapshot

| Area | Current evidence | State |
|---|---|---|
| Common C11 core and CLI | CMakeLists.txt, src/core.* and src/cli.c | Implemented |
| Win32 front end | src/win_gui.c and Windows target | Implemented |
| CTest regression suite | tests/ and CMake test registration | Implemented |
| Read-only inspection and bounded workloads | docs/ARCHITECTURE.md and core/processor modules | Implemented foundation |
| Deny-by-default policy | src/policy.* | Implemented foundation; each new operation still needs review |
| Autonomous repair / source rewriting | Not part of current runtime | Planned and intentionally disabled |
| General plugin host / external repository fleet | Integration policy only | Planned; source verification is mandatory |
| Crash recovery / transactional persistent updates | No general persistent mutation subsystem | Planned; not applicable to current read-only core |
| Windows/Linux CI | .github/workflows/ci.yml | Implemented |
| Independent approval and exact-head Auto-Merge | .github/workflows/verified-auto-merge.yml | Implemented workflow; repository protections must also be configured |

## Change acceptance checklist

A change is ready for review only when:

- its module boundary and public contract are documented;
- resource limits and failure behavior are explicit;
- unsafe or privileged behavior is absent or separately reviewed and denied by default;
- CMake/CTest registration is complete;
- Windows and Linux CI pass on the exact PR head;
- documentation distinguishes implemented behavior from plans;
- no merge protection or independent-review requirement is bypassed.

This document defines architecture requirements. It does not certify hardware behavior, performance, security or platform support by itself.
