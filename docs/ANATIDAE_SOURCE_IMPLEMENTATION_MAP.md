# Anatidae source-document implementation map — Pytyliki

**Scope:** processor systems workstation, bounded numerical/measurement kernels, source inspection, embedded reference tooling and native desktop UX.

This document maps the uploaded *Constitution of the Ducks / AETHER-ANAS / 66 Engineering Laws / 12 Laws and Formulas / 666 Rules* into Pytyliki's actual domain. It is a traceability contract, not a claim that every proposed language-runtime feature is implemented in this C11 workstation.

## 1. Domain adaptation

Pytyliki owns local hardware discovery, measurement, bounded numerical operations, DSP, memory primitives, source diagnostics and opt-in ABI interoperability. It does not become a distributed programming language, a driver installer or an unrestricted hardware-control plane. The source's architecture is applied where it has a concrete workstation analogue; language-only ambitions remain in Reaper.

## 2. Ten renunciations translated into this repository

| Source doctrine | Pytyliki implementation mapping | Status / boundary |
|---|---|---|
| Three-medium existence: land / water / sky | `src/three_medium.*` defines a bounded execution-plan contract for local, stream and distributed media. Capability bits distinguish what a host has from what a plan requests. | **Implemented as a reference planner.** It does not launch GPU jobs, create remote sessions or claim a distributed runtime. |
| Hydrodynamic calm and transparent internals | Native UI, CLI, TRACE provenance, metric registry, memory statistics and explicit plan fields expose what a measurement or operation means. | **Partial / existing.** Not every subsystem has instruction-level or cache-level introspection. |
| Ontological duck typing / behavioral resonance | C headers expose narrow, versioned contracts; FFI consumers use explicit ABI declarations. | **Partial.** No general-purpose structural type checker is claimed. |
| Feather-oil / explicit effects | `policy.*` uses deny-by-default role/action checks and scoped, expiring consent for report export, sharing and telemetry. | **Implemented for listed actions.** This does not create a universal algebraic-effects runtime. |
| Flock / collective aerodynamics | Stream plans are capacity-bounded by both the caller's in-flight limit and memory budget; non-local media explicitly require their capability. | **Implemented as planning and bounds.** No scheduler, worker pool or remote transport is silently enabled. |
| Rounded, human-first interface | Existing Anatidae desktop design, keyboard/focus/accessibility and reduced-motion rules remain the UI baseline. | **Existing design contract.** Visual polish never overrides status or warnings. |
| Semantic hypergraph instead of text | Not a fit for a processor workstation's C sources, CMake build and review workflow. `pipeline.*` remains an explicit compiler-stage reference plan. | **Not applicable as a source-storage replacement.** Git-readable text remains authoritative. |
| Content-addressed semantic identity | `trace.*` binds measurements to source digests; this is evidence provenance, not a semantic identity system for arbitrary functions. | **Partial, purpose-specific.** |
| Habitats / FBIP instead of GC or manual allocation | `memory.*` provides typed views, overflow-checked slices, arenas, checkpoints, rewind and fixed-block pools. | **Implemented subset.** FBIP and compiler-proven ownership are language/backend work, not claimed here. |
| Immune inputs and thermodynamic cost | Existing source-review and measurement boundaries remain read-only; typed views and bounded memory plus the new medium planner expose finite budgets. | **Partial and domain-specific.** No universal taint type or formal energy proof is claimed. |

## 3. Twelve Creator commandments — operational reading

1. **Uniqueness:** keep Pytyliki focused on processor/hardware measurement rather than duplicating Reaper's compiler or OPlague's driver inspection.
2. **Zero ambiguity:** every plan names its medium, required capabilities, budget and readiness result.
3. **Orthogonality:** a plan is a small value contract; it does not perform execution as a side effect.
4. **Human-first diagnostics:** return stable status codes and readable status names.
5. **Instant feedback:** keep planning O(1), and measure UI response instead of claiming an unconditional 16 ms guarantee.
6. **Self-hosting:** not a workstation requirement; the build stays conventional CMake/C11.
7. **Seamless bridge:** preserve the versioned C ABI; optional integrations remain explicit.
8. **Scalable learning:** expose simple CLI/GUI paths before advanced profiling or embedded models.
9. **Computational ecology:** cap allocations, workloads and stream queue capacity.
10. **Stable core:** evolve through versioned headers/contracts and tests.
11. **Open tooling:** maintain machine-readable reports, catalogs and provenance.
12. **Joy of creation:** the Anatidae visual language stays calm, legible and accessible.

## 4. Sixty-six engineering laws — five enforceable clusters

The 66 source laws are retained as five review clusters; each cluster must be considered for new code and tests:

- **1–14 — thermodynamics and state:** account for memory/time/I/O costs; minimize hidden state and coupling; make unsafe access explicit; define resource ownership; avoid races and alias ambiguity. Pytyliki anchors: `memory.*`, `policy.*`, `trace.*`, C11 module boundaries and sanitizer-enabled builds.
- **15–26 — mechanics and execution:** immutable/report-first behavior, explicit resource cleanup, bounded stream queues, simple composable APIs, cache-aware data layouts and platform-specific boundaries. Anchors: `compute_ops.*`, `memory.*`, `pipeline.*`, `three_medium.*`.
- **27–40 — mathematics and types:** validate dimensions, ranges, finite values, units and overflow; prefer explicit status results over ambiguous sentinel values; document algorithmic behavior. Anchors: `metric_geometry.*`, `compute_ops.*`, `memory.*`, `trace.*`.
- **41–50 — evolution and adaptation:** incubate new capabilities behind stable contracts, isolate failures, document deprecation and keep IDE/UI feedback human-readable. Anchors: `evolution.*`, feature catalogs, tests and desktop design documents.
- **51–66 — systems engineering:** expose parallelism and bottlenecks without making unsupported claims; make network/FFI boundaries explicit; prefer deterministic reports and fast regression checks. Anchors: `pipeline.*`, `systems_accel.*`, `interop.*`, `trace.*`, CI workflows.

## 5. Twelve special law/formula models

The source's Trauman, Baus–Bayes, Gause, Strauss, Bergmann, Dilman, Tilman, Spillman, Besse, Hess, Messe and Little–Tillman–Yoshia models are treated as engineering analogies and decision aids. Their practical crosswalk is: budget accounting; resilient input/error handling; no duplicate APIs; versioned evolution; small public surfaces; regression control; optimization of the actual bottleneck; limiting low-value feature growth; balanced library granularity; deterministic transformations; isolation of dependencies; and queue-capacity/backpressure design. The three-medium planner implements the final queue/budget crosswalk in a testable, conservative way.

## 6. Six hundred sixty-six rules — complete domain coverage by source range

| Source range | Domain | Pytyliki mapping |
|---|---|---|
| 1–111 | Three-medium existence and computation ontology | CPU inventory and explicit local/stream/distributed plan modes; remote and stream capabilities are declared, not assumed. |
| 112–222 | Hydrodynamic calm and transparency | native UI/CLI, TRACE provenance, measurement units, bounded memory statistics and inspectable capability plans. |
| 223–333 | Duck typing and type system | fixed-width C types, explicit data-type enums, typed memory views, validated ABI contracts and metric-domain checks. |
| 334–444 | Effects and immune barrier | deny-by-default policy, consent scopes, read-only source inspection, explicit FFI path and bounded input handling. |
| 445–555 | Flock concurrency | bounded stream planning, backpressure requirement flags, existing parallel-capability inventory and no unbounded implicit queue. |
| 556–666 | Human ergonomics, freedom and creator code | Anatidae design system, keyboard/accessibility rules, localized docs, open build tooling and transparent feature claims. |

The 666 source prescriptions are not 666 independent C functions. Their intent is captured through these traceable domain ranges, concrete APIs and acceptance tests. Features that require a compiler, distributed runtime, GPU scheduler or formal ownership checker remain outside Pytyliki's product boundary.

## 7. Strong/weak types and the 150 maxim crosswalk

The source's strong/weak type discussion is applied through fixed-width data enums, typed views, explicit read-only flags and validation before numerical operations. The Chinese maxim compendium's epistemology, systems dynamics, strategy, social coordination and health/prevention themes are used as review heuristics: verify evidence at source, isolate cascade failures, conserve scarce resources, make authority/consent explicit, and prefer prevention over repair. They do not replace empirical measurement or domain-specific professional standards.

## 8. Tri-Color Duck UI

The existing Anatidae desktop design system remains the source of truth for the requested white / blue / red family: porcelain white (`#FFFFFF`, `#F4F7FB`), cobalt/azure (`#0052CC`, `#0A1F44`) and signal red (`#E62E2D`, `#FF4D4F`). Color must never be the sole status signal; pair it with text, icons and accessible contrast. Motion must respect reduced-motion preferences and must not hide errors.

## 9. Acceptance and honesty rules

- `pl_medium_plan_build` must reject invalid budgets and report unsupported capabilities without silently selecting another medium.
- Queue capacity must never exceed `memory_budget_bytes / item_size_bytes` or `max_in_flight`.
- A distributed plan is not proof of a distributed executor; a GPU capability bit is not proof that a GPU backend exists.
- Measurement provenance must retain source digest, metric, unit, context, exact window, sample count and quality.
- Reports remain read-only and privacy/consent gates remain separate from analysis.
- The CMake/CTest suite is the authority for implemented behavior; documentation is not a substitute for tests.

## 10. Related implementation anchors

`src/three_medium.*`, `src/pipeline.*`, `src/memory.*`, `src/trace.*`, `src/policy.*`, `src/compute_ops.*`, `src/interop.*`, `docs/AETHER_ANAS_INTEGRATION.md`, `docs/ANATIDAE_DESKTOP_DESIGN_SYSTEM.md`, `docs/ANATIDAE_ENGINEERING_PROFILE.md`, `docs/ARCHITECTURE_66.md`, `docs/CONSTITUTION_666_PROFILE.md`.
