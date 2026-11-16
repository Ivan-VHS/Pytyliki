# APSIA 2.0 — Autonomous Polyglot Self-Improvement Architecture

APSIA 2.0 is the evolution architecture for Pytyliki: a guarded engineering loop that observes workloads, identifies opportunities, selects an implementation strategy, generates isolated changes, verifies them, and proposes reversible integration. It extends the existing C11 workstation, Processor Recursive Evolution Engine (PREE), and HyperFusion coordinator.

> **Implementation status:** the C11 polyglot router in `src/apsia.c` is an implemented deterministic planning component. It does not generate code, invoke compilers, load shared libraries, run eBPF, or deploy modules. Those are staged capabilities, not claims about the current executable.

## 1. Unified architecture

| Layer | Responsibility | Current boundary |
|---|---|---|
| Observe | CPU/ISA inventory, bounded benchmarks, source review, future APM/eBPF adapters | Existing local diagnostics; no always-on production profiler |
| Diagnose | Detect regressions, missing capabilities, resource pressure and repeated failures | Existing advisory rules and PREE/HyperFusion planning |
| Route | Select a language from workload type, sandbox needs, target availability and FFI economics | Implemented C11 planner |
| Specify | Extract pure interfaces and emit WIT, Protobuf, FlatBuffers or C ABI contracts | Design specification; no automatic AST extraction |
| Synthesize | Generate candidate source from a pinned specification | Future isolated agent integration |
| Build | Hermetic compiler jobs, pinned dependencies, reproducible artifacts, SBOM and signatures | Future build service |
| Verify | Differential/property tests, fuzzing, sanitizers, static analysis and benchmarks | Existing CTest plus staged verification gates |
| Shadow | Compare candidate results without affecting user-visible output | Future runtime capability |
| Promote | Canary, atomic dispatch, rollback and artifact lockfile | Human-approved PR workflow; no in-process hot swap |
| Learn | Store decision evidence, outcomes, regressions and architecture choices | Existing evolution reports; persistent episodic store is future work |

## 2. Polyglot routing policy

The router uses workload semantics rather than language popularity:

| Workload | Preferred target | Rationale |
|---|---|---|
| CPU-bound parsing, tight loops, memory-safe native code | Rust | Ownership model and predictable native execution |
| Specialized native ABI / existing C++ code | C++ | Reuse established libraries and ABI boundaries |
| Small native utilities / cross compilation | Zig | Explicit build and target control |
| GPU kernels | CUDA | NVIDIA GPU execution |
| Portable untrusted plugins | WebAssembly / WASI | Capability sandbox and portable module boundary |
| Relational aggregation | SQL / DuckDB | Execute close to columnar data |
| Dynamic orchestration / scripting | Python | Fast iteration and ecosystem |
| Concurrent network services | Go | Straightforward concurrency and deployment |

Mojo, Triton, Metal, Elixir, Lua and DuckDB extensions remain optional adapters. A target is eligible only when the corresponding toolchain and capability are explicitly available.

### FFI economics

For each candidate, estimate:

`speedup = host_execution_ns / (target_execution_ns + ffi_overhead_ns)`

`ffi_cpu_cores = ffi_overhead_ns * ffi_calls_per_second / 1e9`

Reject the polyglot route when the measured speedup is below the configured threshold, the FFI CPU budget is exceeded, or required target support is absent. Batch work, use stable C ABI/WIT boundaries and prefer zero-copy buffers for high-volume data. Never assume that zero-copy is safe without explicit lifetime and ownership rules.

## 3. Interface and memory contract

1. Extract only deterministic functions with explicit inputs, outputs and error channels.
2. Prohibit implicit access to process globals, environment secrets and mutable host state.
3. Describe interfaces using WIT for WASM components, Protobuf/FlatBuffers for message boundaries, or a versioned C ABI for native modules.
4. Define ownership, allocator, alignment, lifetime, nullability and error translation in the contract.
5. Prefer Arrow, mmap or shared-memory buffers only when the host and module agree on layout, lifetime, synchronization and bounds.
6. Treat raw pointers as unsafe capabilities; do not expose them to untrusted modules.
7. Version every contract and reject incompatible module manifests before loading.

## 4. Synthesis and hermetic builds

A future synthesis service must work from a pinned task specification, repository revision, dependency lockfile and compiler image. Build jobs run without production credentials, network access by default, or write access to the source repository. Network dependency acquisition occurs in a separate verified prefetch phase.

Each candidate must record source revision, toolchain versions, build flags, dependency hashes, license inventory, SBOM, artifact digest and provenance. Release artifacts are content-addressed and signed. Reproducibility is checked by rebuilding independently and comparing digests.

Compiler diagnostics may be fed back into a bounded repair loop. Repair attempts are capped; a timeout, dependency drift, untrusted diagnostic instruction or repeated failure abandons the candidate.

## 5. Verification gates

A candidate is ineligible for promotion until it passes:

- Differential tests against the host implementation, including edge cases and numeric tolerances.
- Property-based and fuzz testing with minimized failures retained as regression cases.
- Static analysis and compiler warnings policy.
- ASan/UBSan/TSan where supported; MSan only with a compatible instrumented dependency stack.
- Concurrency, resource, descriptor and cold-start tests.
- Benchmarks on representative data with confidence intervals and a recorded baseline.
- License, dependency, SBOM, signature and sandbox-capability checks.
- Shadow comparison with zero user-visible influence.
- Independent CI checks and human review.

A speedup is never inferred from heuristic scores. Promotion requires a statistically credible gain or a separately justified functional capability, with no correctness or security regression.

## 6. Runtime evolution and rollback

The intended runtime uses versioned module handles and a dispatcher. Candidate loading happens in a separate process or WASM sandbox first. Native in-process loading is a later opt-in capability because a segmentation fault, allocator mismatch or unsafe unload can terminate the host process.

For safe promotion, use immutable module versions, atomic routing changes, reference-counted/epoch-based retirement, bounded canaries, health metrics and a known-good fallback. Never unload code while calls are active. Rollback must be tested before rollout and must not depend on the candidate module.

## 7. Immutable safety kernel

The self-improvement agent cannot change its own authorization kernel, verification policy, signing trust roots, rollback path or protected-branch rules. It cannot grant itself permissions, approve its own PR, suppress failed checks, or alter evidence after a run.

The following actions remain denied by default:

- Executing arbitrary inspected source in the host process.
- Writing firmware, microcode, BIOS/UEFI, device registers or CPU voltage/clock/power controls.
- Reading production secrets from build or synthesis jobs.
- Merging without independent approval and required CI.
- Deploying a candidate that has not passed its declared verification profile.

## 8. Repository-wide delivery model

APSIA is delivered across repositories through independent, auditable PRs:

1. Inventory repository language, build system, tests, licenses and branch protections.
2. Generate a repository-specific integration plan; do not blindly copy C11 modules into other stacks.
3. Create a feature branch from the current default branch.
4. Add adapters and tests in small, reviewable commits.
5. Run native CI plus security and compatibility checks.
6. Open a PR with evidence, risk classification, rollback instructions and artifact provenance.
7. Require an independent approval and all required checks before merge.
8. Record the merged revision and observed outcome in the evolution ledger.

No cross-repository bulk push, force push, self-approval or branch-protection bypass is permitted.

## 9. Delivery phases

- **Phase A — Planner (implemented):** deterministic workload-to-language routing, target availability checks, FFI-adjusted speedup and overhead budget.
- **Phase B — Contract tooling:** versioned C ABI/WIT schemas and interface compatibility tests.
- **Phase C — Build workers:** isolated compiler adapters, dependency pinning, SBOM and provenance.
- **Phase D — Verification workers:** differential fuzzing, sanitizers, benchmark confidence and shadow comparison.
- **Phase E — Runtime adapters:** WASM-first plugin runtime, versioned dispatch and tested rollback.
- **Phase F — Repository federation:** repository inventory, per-repository PR generation and evidence ledger.

Each phase is independently useful. A later phase cannot silently upgrade the status of an earlier prototype into a production capability.

## 10. Local validation

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

The APSIA router test validates workload mapping, sandbox precedence, missing targets, FFI speedup rejection, overhead budgets and invalid numeric inputs.
