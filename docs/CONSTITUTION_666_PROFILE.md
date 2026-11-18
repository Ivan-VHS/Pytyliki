# Constitution of the Ducks — 666-rule implementation profile for Pytyliki

Status: normative project profile; not a claim that all 666 capabilities are implemented.

## Product boundary

Pytyliki is a portable C11 processor-workstation application with a shared core, CLI and Win32 GUI, CMake/CTest, bounded analysis/planning and read-only host inspection. This profile applies the Constitution's engineering intent to that product. It does not turn Pytyliki into a new programming language, operating system, cloud orchestrator, firmware writer or unrestricted autonomous agent.

The six sections of the source Constitution remain authoritative. This profile maps their intent to Pytyliki and defines the evidence required before a capability may be called implemented.

## Conformance states

- Implemented — present on the reviewed branch and covered by an automated test or CI check.
- Partial — a limited foundation exists; the complete capability is absent.
- Planned — a future requirement with a defined implementation boundary.
- Not applicable — outside this product's safe or useful scope; reconsider only through an explicit architecture decision.

Never convert a planned or not-applicable item into a shipped claim in README, GUI, CLI output or release notes.

## Six-part crosswalk

| Constitution section | Rules | Pytyliki interpretation | Initial status |
|---|---:|---|---|
| I. Three-medium computing and execution ontology | 1–111 | Portable host abstraction, CPU capability discovery, explicit accelerator adapters, deterministic local simulation and offline-first operation. Cloud migration, FPGA, kernel, smart-NIC and distributed-memory semantics are not implied by processor inventory. | Partial / Planned / N/A |
| II. Hydrodynamic clarity and transparency | 112–222 | Small stable C APIs, explicit resource costs, bounded workloads, actionable errors, reproducible CMake builds, profiling evidence, testable documentation and no hidden I/O. | Partial |
| III. Duck typing and type system | 223–333 | C11-compatible contracts: opaque structs, enums, validated constructors, explicit result/error codes, fixed-width integers, ownership documentation and boundary validation. Dependent types, global inference and compiler-enforced linear types are not available in C11. | Partial |
| IV. Effects and immune barrier | 334–444 | Read-only discovery, deny-by-default policy, explicit authorization, bounded parsing, safe path handling, no privileged hardware writes, deterministic cleanup, sanitizer coverage and provenance. | Partial |
| V. Flock concurrency | 445–555 | Keep current operations bounded and deterministic; add concurrency only behind measured, cancellable, resource-limited APIs with tests for ordering, cancellation and failure. | Planned / Partial |
| VI. Freedom, ergonomics and creator's code | 556–666 | One coherent GUI/CLI product, low cognitive load, offline docs, reproducible builds, clear module map, dependency/license provenance, accessible diagnostics and human-controlled promotion. | Partial |

## Mandatory engineering interpretation

### Execution and hardware boundary
- The shared C11 core is the sole owner of portable processor-domain logic. CLI and GUI are adapters, not parallel implementations.
- Host and processor reports distinguish supported, detected, usable, unavailable and not measured. Never infer an accelerator or performance capability from a CPU model string alone.
- CPU features are reported as usable only after checking hardware support and operating-system enabled state where required.
- GPU, SIMD, FPGA, ARM, RISC-V, firmware, microcode and device-control features require a dedicated backend, capability probe, safe fallback and platform-specific tests before being described as active.
- Processor discovery and source inspection remain read-only. No firmware, BIOS/UEFI, microcode, registers, voltage, clocks, power limits, kernel, driver or user files are modified by the current workstation.

### Module and API contract
- Keep one shared core and narrow adapters. No duplicated domain logic in CLI and GUI.
- Each module declares purpose, public API, dependencies, resource bounds, failure behavior, ownership rules and tests.
- Keep dependency direction acyclic. Shared mutable global state is prohibited unless an architecture decision documents why it is unavoidable and how it is synchronized.
- Use fixed-width integer types for externally visible data and validate sizes before allocation, indexing or arithmetic.
- Public APIs document ownership, lifetime, mutability, units, error values and whether an operation can block.

### Resource and effect contract
- Every potentially expensive operation has explicit upper bounds for iterations, samples, bytes, collection length and elapsed time where measurable.
- Every long-running operation exposes cancellation or documents why it cannot be cancelled.
- No unbounded retry, queue, recursion, allocation or wait.
- Keep I/O and platform calls at adapter boundaries. Core planning and analysis functions are deterministic for identical inputs.
- Missing telemetry is unknown, not zero. Failed probes are errors, not negative capability reports.
- Any future persistent write requires validation, temporary output, atomic replacement, backup/rollback and an explicit policy decision.

### Quality and release gate
- C11 remains explicit; compiler extensions stay disabled unless a reviewed platform adapter requires them.
- Every executable regression test is registered with CTest.
- Shared-core changes require Windows and Linux configure/build/CTest on the exact PR head.
- Run ASan/UBSan and static analysis on supported toolchains; report exactly which targets were covered.
- Add a regression test for each fixed defect and boundary tests for every new quota.
- Documentation describes inspected behavior, not the aspirational catalogue.
- Required checks and independent approval remain mandatory. This profile does not authorize a merge or a branch-protection bypass.

## Prioritized implementation sequence

1. P0 — maintain a rule-to-evidence ledger; classify every rule as implemented, partial, planned or not applicable.
2. P1 — enforce module boundaries, fixed-width public data, explicit ownership, checked arithmetic, input validation and deterministic error handling.
3. P1 — audit allocations, loops, buffers and waits; keep limits consistent across CLI, GUI and library APIs.
4. P1 — keep Windows/Linux CMake/CTest, sanitizer and architecture-contract gates green on the exact commit.
5. P2 — add structured diagnostics, capability provenance, workload timing and memory accounting without fabricated measurements.
6. P2 — maintain GUI/CLI parity, consistent errors, clear unavailable states and cancellable expensive work.
7. P3 — add SIMD/GPU adapters only after scalar-reference equivalence, OS-state checks, runtime dispatch and reproducible benchmarks.
8. P3 — introduce bounded task execution only when measured workloads benefit and deterministic tests cover cancellation, ordering and shutdown.
9. P4 — import external projects only after source/license/provenance review, pinned revisions, isolated adapters and contract tests.

## Rule-range coverage ledger

| Rule IDs | Required evidence before claiming conformance |
|---|---|
| 1–111 | Platform adapter, capability probe, fallback test, deterministic simulation or explicit N/A decision |
| 112–222 | API contract, resource-cost documentation, reproducible build/test, diagnostic or profiling evidence |
| 223–333 | C11 type/API contract, fixed-width representation, validation and negative tests; language-only features marked N/A |
| 334–444 | Policy review, boundary validation, cleanup/error-path tests, sanitizer/security evidence |
| 445–555 | Concurrency design, bounded queues/tasks, cancellation/order/failure tests; unsupported distributed features marked Planned/N/A |
| 556–666 | UX/documentation review, reproducible/offline workflow, dependency provenance and protected promotion evidence |

The six ranges cover every rule exactly once. Maintain a finer-grained issue/test mapping for any rule promoted to Implemented.

## Change acceptance checklist

- [ ] Rule IDs and affected section are recorded in the PR.
- [ ] The change improves a real Pytyliki workflow and does not add architecture for its own sake.
- [ ] Public API, ownership, effects, limits and failure behavior are documented.
- [ ] No unsupported hardware, performance, security or platform claim is introduced.
- [ ] Tests include normal, invalid, boundary and failure cases as applicable.
- [ ] Windows/Linux CI and required sanitizer checks pass on the exact PR head.
- [ ] README and GUI/CLI claims match the implementation.
- [ ] Independent review and repository protection remain in force.

A rule is not implemented because it appears in this profile. Implementation requires code, tests and verifiable CI evidence.
