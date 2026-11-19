# APSIA 2.0 — Autonomous Polyglot Self-Improvement Architecture

## Status and implementation contract

APSIA 2.0 is the engineering control plane for bounded, evidence-driven evolution of Pytyliki and compatible repositories. This document distinguishes implemented policy primitives from future execution capabilities.

**Implemented in this change:** deterministic workload-to-language routing, FFI-cost rejection, host-language optimization preference, least-privilege capability declarations, and a promotion-evidence gate in the native C11 core.

**Not implemented by this change:** LLM code synthesis, native compiler orchestration, production eBPF collection, automatic dependency installation, arbitrary plugin execution, dynamic library hot swapping, RCU reclamation, automatic deployment, or unattended merging. These require separately reviewed components and CI evidence; documentation must never imply they already exist.

## 1. Unified system

The four workstreams are delivered as one architecture:

1. **Pytyliki specification:** APSIA contracts, lifecycle, threat model, and module boundaries.
2. **Polyglot runtime foundation:** a C11 policy core with stable interfaces; optional Rust/C++/Zig/WASM/GPU backends are adapters, not dependencies of the core.
3. **Repository fleet coordinator:** common manifests, repository discovery, isolated branches, PR queues, test evidence, and audit trails.
4. **Verified integration:** immutable main policy, independent review, tested-head-SHA checks, sequential merge, and rollback records.

The coordinator must treat every repository as a separately authorized target. A repository is not modified merely because it is discoverable.

## 2. Architecture layers

| Layer | Responsibility | Initial implementation |
|---|---|---|
| Observe | Collect profiling, build, test, crash, and feature-gap evidence | Existing local processor diagnostics; external APM/eBPF is future work |
| Diagnose | Normalize bottlenecks and missing capabilities | C11 policy and analysis interfaces |
| Route | Select a target language from workload and platform constraints | apsia.c |
| Contract | Extract a pure boundary and define versioned IDL | Specification; WIT/Protobuf/C ABI adapters are future work |
| Synthesize | Generate source from a constrained specification | Future isolated agent |
| Build | Compile in hermetic, credential-free environments | Existing CMake CI for C11; polyglot builders are future work |
| Verify | Differential tests, fuzzing, sanitizers, static analysis, benchmarks | CTest foundation; additional gates are future work |
| Shadow | Compare old and candidate implementations without serving candidate output | Future runtime integration |
| Promote | Require complete evidence and independent authorization | apsia.c promotion gate |
| Integrate | Open PR, verify exact head SHA, merge sequentially | Existing guarded PR workflow; fleet coordinator is future work |
| Learn | Store decisions, evidence, failures, and measured outcomes | Future versioned local decision store |

## 3. Workload routing policy

Routing is deterministic and policy-based, never a free-form model choice:

- CPU-bound parsing, memory-safe systems code, and low-latency primitives: Rust; C++ or Zig may be selected by a later measured backend.
- Tensor/GPU workloads: CUDA only when a compatible GPU backend is explicitly available; otherwise retain the host implementation.
- Untrusted extensions: WebAssembly/WASI with explicit capability grants.
- High-concurrency network services: Go or Elixir, subject to deployment/runtime availability.
- Analytical aggregation: SQL or DuckDB when data locality makes it beneficial.
- Small embedded scripting: Lua; Python is an orchestration/reference option, not an automatic hot-path replacement.

A route is rejected when the expected gain does not exceed estimated FFI and data-conversion costs, when host-language optimization can capture at least 70% of the estimated opportunity, or when required hardware/runtime capabilities are absent. The C11 router is an advisory decision function; it does not compile or execute generated modules.

## 4. Stable boundary and data movement

Every candidate module must have:
- A versioned, explicit input/output contract.
- No hidden global state or undeclared side effects.
- Bounded input sizes and execution budgets.
- Explicit ownership and lifetime rules.
- Typed error mapping across the boundary.
- A declared capability set, defaulting to none.

Use a C ABI, WIT, Protobuf, or FlatBuffers only where the host boundary warrants it. Prefer fixed-layout buffers, offsets, and validated shared-memory views for high-frequency paths. Zero-copy is not assumed safe by default: lifetime, alignment, aliasing, bounds, and synchronization must be proven. JSON remains suitable for control-plane messages, not presumed suitable for hot-path payloads.

## 5. Synthesis and hermetic build

A future synthesis worker may generate source only from a reviewed contract and pinned repository context. It must:
1. Create an ephemeral branch/worktree and hermetic build environment.
2. Run without production credentials, deployment secrets, or write access to protected branches.
3. Pin compiler versions, dependencies, flags, and build inputs.
4. Emit source, tests, compiler logs, SBOM, provenance, and artifact hashes.
5. Retry compiler-directed repair only within a fixed budget.
6. Stop on unresolved diagnostics, license conflicts, unsupported targets, or resource exhaustion.

Generated artifacts are data until verified. The build worker cannot approve or merge its own PR.

## 6. Verification and promotion gate

Required evidence for a candidate:
- Contract and API compatibility tests.
- Differential/property tests against the host oracle.
- Fuzzing of boundary and malformed inputs.
- Relevant sanitizers and static analysis.
- Reproducible build metadata and dependency/license inventory.
- Statistically meaningful benchmark improvement after FFI costs, or a justified memory/reliability gain.
- Cold-start, concurrency, resource-leak, and failure-path tests where applicable.
- Shadow comparison with no user-visible candidate output.
- Independent human review and approval.

Promotion must fail closed when evidence is missing, stale, or tied to a different source SHA. Canary rollout and hot swapping are future capabilities and require safe reclamation, active-call tracking, rollback verification, and platform-specific loader tests before they can be enabled.

## 7. Immutable safety kernel

The autonomous agent cannot modify:
- The authorization and capability enforcement kernel.
- The verification and evidence-validation code.
- The rollback path and protected-branch policy.
- Its own permission boundary or signing trust roots.

No agent may self-approve, bypass branch protection, force-push, rewrite protected history, or merge unreviewed code. Firmware, microcode, BIOS/UEFI, voltage, clock, power-limit, kernel, driver, and device-register writes remain outside Pytyliki's current product scope.

## 8. Repository fleet and sequential integration

Each repository requires an explicit allowlist entry with owner, repository, default branch, allowed paths, required checks, and reviewer policy. The fleet coordinator:
- Reads repository state before proposing work.
- Creates one isolated branch per repository and change set.
- Deduplicates overlapping proposals before PR creation.
- Runs the repository's declared validation matrix.
- Rechecks PR head SHA, branch base, required checks, and latest independent approval immediately before merge.
- Integrates one PR at a time; dependent PRs are rebased/revalidated after each merge.
- Records merge SHA, test evidence, reviewer identity, and rollback reference.

A failure blocks the queue; it never triggers a force merge. A successful merge in one repository does not authorize changes in another.

## 9. Evolution manifest

The future apsia.lock.json manifest records schema version, repository identity, module contract version, selected backend, source and artifact hashes, compiler/toolchain versions, dependency hashes, SBOM reference, test evidence, benchmark baseline, approval record, and rollback target. Secrets and credentials must never be stored in the manifest.

## 10. Delivery roadmap

- **Phase A — Foundation:** C11 routing and promotion gates, architecture specification, tests, and CI.
- **Phase B — Contract layer:** stable C ABI/WIT schema, ownership rules, contract compatibility tests.
- **Phase C — Build adapters:** one backend at a time (Rust first; then WASM; then C++/Zig), hermetic builds and SBOM.
- **Phase D — Verification:** differential fuzzing, sanitizers, benchmark confidence intervals, artifact signing.
- **Phase E — Runtime:** shadow execution, dispatcher, safe unload and rollback; no production activation until platform tests pass.
- **Phase F — Fleet:** repository allowlist, deduplicated PR queue, sequential SHA-verified merge workflow.

Each phase ships as a separate, testable PR. A roadmap item is not an implemented feature until source, tests, CI evidence, and documentation agree.

## 11. Acceptance criteria

APSIA is ready for a production capability only when its contract, implementation, tests, operational telemetry, security review, rollback, and documentation are all present. Performance claims require reproducible measurements on named hardware and workloads. Heuristic scores and model-generated estimates are not benchmark results.


## Local planner CLI

The deterministic routing and promotion-evidence APIs are exposed by `pytyliki-apsia` for local policy inspection:

```sh
pytyliki-apsia cpu 2.0 0.1 0.2
pytyliki-apsia gpu 2.0 0.1 0.0 --gpu
pytyliki-apsia plugin 1.0 0.0 0.0
```

Arguments are workload, expected speedup, FFI overhead fraction and the fraction of the optimization opportunity achievable in the host language. Untrusted-plugin workloads require the explicit `--wasm` sandbox declaration. Optional flags declare GPU availability, deterministic output, sandbox requirements and requested capabilities. Routing never grants capabilities; the decision reports zero granted capabilities and requires human review.

The `apsia/evolution-lock.schema.json` schema defines evidence metadata for future immutable module artifacts. Parsing the schema does not validate an artifact signature or authorize module execution.
