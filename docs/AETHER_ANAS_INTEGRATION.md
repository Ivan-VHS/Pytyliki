# AETHER-ANAS Integration Profile — Pytyliki

**Status:** architecture proposal for staged adoption  
**Target:** Pytyliki C11 processor-systems workstation  
**Scope:** shared core, CLI, Win32 adapter, DSP and engineering workflows  
**Normative boundary:** this profile does not claim that proposed AETHER-ANAS facilities are already implemented.

## 1. Product interpretation

Pytyliki is a native C11 processor-systems workstation. It is not a general-purpose programming language, compiler, operating system, distributed runtime or unrestricted hardware controller. AETHER-ANAS is applied here as an engineering architecture profile: preserve the coherent C11 product and adopt only principles that improve its APIs, resource model, evidence chain, portability and safety.

The existing C11 source tree, CMake build and CTest suite remain the implementation authority. A semantic hypergraph may become a derived architecture/provenance view; it must not replace C source files, the build system or ordinary version-control review.

## 2. Evidence-backed baseline on the integration branch

The current Pytyliki integration branch provides the following visible foundations:

- A shared `pytyliki_core` library with separate command-line and Win32 GUI adapters.
- A CMake project that requires ISO C11 and registers component-level CTest executables.
- Processor inventory and ISA capability reporting, bounded local workloads and source diagnostics.
- DSP modules with caller-owned state/buffers and bounded block processing.
- APSIA routing/promotion logic, policy and other deterministic planning/catalogue modules.
- Platform validation documentation and a 666-rule conformance profile with explicit Implemented / Partial / Planned / Not applicable states.

These foundations do not imply native graph storage, global static type inference, algebraic effect handlers, lock-free concurrency, distributed execution, GPU offload, FPGA synthesis or automatic hardware control.

## 3. AETHER-ANAS crosswalk

| Principle | Pytyliki state | Correct application |
|---|---|---|
| One coherent product with modular internals | Implemented / Partial | Preserve the shared C11 core; keep CLI and GUI as thin adapters |
| Deterministic, bounded work | Partial | Keep explicit workload, frame, count and iteration limits; validate before allocation or mutation |
| Strong type boundaries | Partial | Use opaque structs, enums, tagged unions, fixed-width integers and checked constructors |
| ADTs and exhaustive matching | Partial | Model variants with tagged unions and switch statements that handle every enum case; enable compiler warnings |
| Refinement / physical-unit types | Planned | Use distinct wrapper structs and validating constructors for frames, samples, bytes, time and frequencies |
| Pure core and explicit effects | Partial | Keep portable logic free of OS I/O; inject narrow platform/capability adapters |
| Semantic hypergraph | Not implemented | Start with a generated module/dependency/evidence graph, not a graph database replacement for source |
| Content-addressed identity | Partial / Planned | Hash build inputs and test artifacts for provenance; do not treat a hash as proof of semantic equivalence |
| Arena ownership / FBIP | Partial | Preserve caller-owned DSP buffers; add scoped arenas only for measured temporary workloads |
| Structured concurrency / backpressure | Planned | Keep synchronous behavior by default; any future worker pipeline must use bounded queues and cooperative cancellation |
| Time-travel debugging | Not implemented | Add structured, deterministic event traces before considering replay or reverse execution |
| CPU/GPU/distributed execution | Partial / Planned / N/A | Gate every backend on capability detection, safe fallback and target-specific evidence |
| Open governance | Implemented / Partial | Preserve the current MIT license and record provenance for external code and generated artifacts |

## 4. C11 architecture rules derived from AETHER-ANAS

### 4.1 Product unity and module boundaries

- Keep `pytyliki_core` as the owner of portable domain logic.
- Keep CLI parsing, Win32 presentation and OS-specific discovery in adapters.
- Keep dependencies directed toward the core; avoid cycles between core modules and presentation layers.
- Give each public module an explicit contract: purpose, inputs, outputs, ownership, valid ranges, failure behavior and tests.
- Do not split the product into services or add a database merely to imitate the hypergraph concept.

### 4.2 Strong contracts in C11

C11 does not provide Hindley–Milner inference, dependent types, linear ownership or native algebraic data types. Apply their engineering intent with ordinary C contracts:

- Prefer opaque public types where representation should remain private.
- Use fixed-width integer types for stable external data and `size_t` for in-process sizes.
- Use enums for closed status/variant sets and tagged unions when values have distinct payloads.
- Use distinct wrapper structs for quantities that must not be mixed, such as frame counts, sample counts, byte counts and frequencies.
- Validate values at construction and API boundaries; document whether zero, empty and sentinel values are legal.
- Enable exhaustive-switch and conversion warnings where supported; test invalid enum values at public boundaries.
- Return explicit status/error codes and define output-parameter validity on both success and failure.

These techniques improve safety but are not a claim that C11 enforces a proof-carrying type system.

### 4.3 Ownership and memory

- Mark pointers as borrowed input, mutable borrowed input, or owned output in API documentation.
- Define who allocates and frees every buffer and state object.
- Check multiplication/addition overflow before computing allocation sizes.
- Keep real-time-sensitive DSP paths allocation-free where their current contracts promise it.
- Keep scratch arenas local to a clearly bounded operation; measure them before adopting them.
- Do not add a global allocator, hidden ownership transfer or implicit lifetime extension.
- Run sanitizers and boundary tests; C has no garbage collector, but that does not make leaks, use-after-free or double-free impossible.

### 4.4 Explicit effects and capability boundaries

Represent OS interactions as narrow adapter interfaces rather than hidden calls from portable algorithms. Candidate capability interfaces include clock access, file access, logging, processor discovery and accelerator dispatch. Pass only the operations required by a subsystem.

Read-only processor inspection remains the default. Firmware, microcode, register, voltage, clock, power, kernel, driver and privileged device writes remain outside the current product scope. Missing permissions or unavailable capabilities must fail closed.

### 4.5 Streaming, bounded queues and cancellation

The current bounded DSP block contract is a useful foundation, but a maximum frame count is not itself a complete backpressure system. Before introducing asynchronous workers:

- Define a fixed queue capacity and overflow policy.
- Define producer behavior when the consumer is slower.
- Make cancellation cooperative and bounded.
- Define buffer ownership while a block is queued or in flight.
- Specify shutdown, partial-block and error propagation behavior.
- Test queue saturation, cancellation, slow consumers and cleanup.

Do not introduce detached workers, unbounded channels or lock-free structures without a measured requirement and a clear memory-ordering proof.

### 4.6 Semantic evidence graph

The first useful graph for Pytyliki is a generated, reviewable manifest:

`module → public API → dependencies → resource limits → tests → CI evidence`

Give each record a stable logical ID and include source path, build configuration and toolchain metadata. Hash canonical manifest/build inputs for provenance and cache keys. Generate the view from C headers, CMake registration and test metadata; do not manually maintain a second, drifting architecture database.

A later visual graph may help inspect dependencies, ownership and verification coverage. It must remain reproducible from repository sources and must not become the only editable copy of architecture information.

## 5. Portability and accelerator policy

- Distinguish hardware-advertised ISA features from features usable under the current OS-enabled vector state.
- Dispatch an optimized kernel only after runtime prerequisites are checked; keep a scalar reference path.
- Verify numerical equivalence and boundary behavior between scalar and accelerated implementations.
- Do not infer GPU, FPGA, ARM64, distributed or hard-real-time support from a catalogue entry or an architecture diagram.
- Keep platform claims aligned with CI, source-level guards, runtime probes and reproducible measurements.

## 6. Scientific and mathematical review gate

The laws and equations in the AETHER-ANAS master document can inspire engineering hypotheses, but physical or biological analogies do not automatically establish software guarantees.

Before a formula becomes a normative rule for allocation, scheduling, type inference or queue sizing, record its traceable source, variable definitions, units, assumptions, domain of validity, dimensional analysis, limiting cases and a reproducible experiment.

Custom attributions and indices without a supplied derivation remain unverified proposals. In particular, standard Little's Law is `L = λW`; the additional multiplicative factor in the supplied queue expression requires a separately derived queue model and explicit definitions of utilization `ρ` and waiting time `W`. Do not encode it as a runtime law without that derivation.

## 7. Staged implementation sequence

### Stage A — Evidence inventory
- Map each module's public API, ownership rules, dependencies, resource bounds and tests.
- Generate the first architecture/evidence manifest from the CMake graph and test registration.
- Acceptance: deterministic output, no undocumented module edges and CI checks for stale/missing evidence.

### Stage B — C11 type and resource hardening
- Add wrapper types and checked constructors for high-risk sizes and units.
- Audit integer overflow, allocation arithmetic, pointer validity and error-code handling.
- Preserve caller-owned DSP buffer semantics.
- Acceptance: negative/boundary tests, Windows/Linux CTest and Linux ASan/UBSan.

### Stage C — Capability/effect adapters
- Separate platform discovery and OS I/O from portable algorithms.
- Define explicit capability structs and deny-by-default failure behavior.
- Acceptance: tests with missing, partial and denied capabilities; portable core tests without platform services.

### Stage D — Evidence graph and provenance
- Generate a semantic module graph with links to headers, tests, CI checks and resource contracts.
- Hash canonical inputs and record toolchain/build configuration.
- Acceptance: identical input produces identical manifest and hash; source changes invalidate dependent evidence.

### Stage E — Bounded streaming concurrency
- Prototype fixed-capacity queues and structured cancellation only where a measured workload requires them.
- Keep a synchronous reference path.
- Acceptance: saturation, shutdown, cancellation, race and buffer-lifetime tests; sanitizer coverage.

### Stage F — Optimized backends
- Add one backend at a time behind a stable interface and validated capability probe.
- Keep scalar/reference behavior as the conformance oracle.
- Acceptance: reproducible Windows/Linux tests, differential numerical tests and benchmark reports for named hardware/toolchains.

No stage is complete because a catalogue, diagram or document exists. Require implementation, tests, CI evidence and an updated conformance state.

## 8. Compatibility, licensing and governance

- Preserve the ISO C11 baseline unless a separate architecture decision justifies a language-standard change.
- Keep public APIs source-compatible where practical; document deliberate breaking changes and version them.
- Keep generated metadata deterministic and disposable.
- The repository currently declares the **MIT License**. This integration profile does not replace it with CC0 or add a patent covenant. Any licensing change requires a separate explicit decision and provenance/legal review.
- Keep human review and repository branch protection in force. Automation may report, plan and validate; it must not bypass authorization or silently mutate hardware.

## 9. Acceptance gate

A change applying AETHER-ANAS to Pytyliki is ready only when:

1. The design decision and scope are documented.
2. Public API ownership, units, bounds and failure behavior are explicit.
3. Windows and Linux CMake/CTest pass on the exact commit.
4. Linux ASan/UBSan and the architecture/recursive-evolution guards pass where applicable.
5. New tests cover normal, invalid, boundary, failure and cleanup paths.
6. README translations describe the same implemented state.
7. No performance, accelerator or hardware-control claim exceeds the available evidence.

This profile is a staged engineering plan, not a claim that Pytyliki has become a new language or a distributed execution platform.
