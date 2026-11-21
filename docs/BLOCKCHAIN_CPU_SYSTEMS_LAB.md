# Blockchain and processor systems lab

## Status and integration boundary

The lab adds a C11 API surface for the 33 functions in the engineering proposal. It is a local library and simulator-facing extension of Pytyliki, not a full blockchain node, EVM implementation, hypervisor, JIT compiler, FPGA toolchain or privileged CPU tuner.

The implementation deliberately separates four modes:

- **Native:** bounded algorithms execute locally in the Pytyliki process.
- **Simulated:** operations mutate only caller-owned VM, simulator, memory or test state.
- **Adapter required:** cryptographic protocols, ZK proof systems, relay proof formats, signature schemes and vendor FPGA synthesis are supplied through explicit callbacks.
- **Planning only:** the API computes an execution plan or profile. Applying it requires a separate, documented platform adapter and authorization.

Discover all entries with:

    pytyliki --systems-lab

The public interface is `src/systems_accel.h`; its implementation is `src/systems_accel.c`. The module is linked into `pytyliki_core`, and CTest runs `tests/test_systems_accel.c`.

## Function coverage

| # | API | Delivered behavior | Mode |
|---:|---|---|---|
| 1 | `pl_unlock_light_client_state_proofs` | Merkle path verification using a caller-supplied 32-byte hash function, leaf index and trusted root. | Adapter required |
| 2 | `pl_bypass_rpc_rate_throttling` | Round-robin endpoint selection with per-endpoint token buckets. It respects the configured provider quota; it does not evade a provider's controls. | Native |
| 3 | `pl_unlock_mempool_simulation_hooks` | Copies a caller-owned state snapshot into a fork buffer and invokes a transaction simulator. Failure restores the fork buffer. | Simulated |
| 4 | `pl_extract_abi_and_bytecode_signatures` | Extracts unique EVM `PUSH4` selectors and skips PUSH immediate payloads. It does not recover Solidity function names or full ABI metadata. | Native |
| 5 | `pl_unlock_cross_chain_message_relay` | Checks a caller-supplied proof verifier and finalized-block confirmation threshold, then builds a bounded relay packet. It does not broadcast transactions. | Adapter required |
| 6 | `pl_deobfuscate_storage_layout_slots` | Aggregates SLOAD/SSTORE reads, writes and observed key fingerprints; slot-kind guesses remain heuristics. | Native |
| 7 | `pl_unlock_deterministic_replay_snapshot` | Folds an explicit initial-state digest and canonical event fields into a repeatable fingerprint. The supplied hash adapter must be deterministic. | Adapter required |
| 8 | `pl_enable_offchain_zk_proof_aggregation` | Validates bounds and invokes a protocol-specific aggregation callback. The core does not claim to implement a proving system. | Adapter required |
| 9 | `pl_unlock_batch_transaction_bundling` | Preflights estimated gas and orchestrates begin/apply/commit/rollback callbacks. Atomicity depends on the caller's executor. | Native |
| 10 | `pl_inspect_call_trace_and_revert_reasons` | Summarizes caller-collected call traces, nested depth, gas, reverts and the first revert message. | Native |
| 11 | `pl_unlock_custom_vm_opcode_hook` | Registers and dispatches opcode callbacks in a caller-owned VM hook table. | Simulated |
| 12 | `pl_virtual_boost_clock_scaling` | Produces a thermal-capped virtual clock and CPU-quota profile. No physical frequency changes occur. | Simulated |
| 13 | `pl_turbo_scheduler_pinning` | Intersects desired and available logical-core masks and returns a NUMA/group plan. It does not change OS affinity. | Planning only |
| 14 | `pl_speculative_trace_prewarming` | Produces deduplicated prefetch target hints from a bounded profile. It does not prefetch hardware caches. | Planning only |
| 15 | `pl_jit_hot_loop_tier_promotion` | Advances a caller-owned profile tier when invocation/backedge counters pass a threshold. An external JIT must perform compilation. | Planning only |
| 16 | `pl_lockless_ring_buffer_pipeline` | Bounded single-producer/single-consumer ring buffer with C11 acquire/release atomics. | Native |
| 17 | `pl_adaptive_batch_vectorization` | Selects a lane width from declared capabilities and reports vector groups plus scalar tail. A scalar fallback remains mandatory. | Planning only |
| 18 | `pl_zero_copy_memory_arena_boost` | Allocates aligned views from caller-owned storage without copying payload data. | Native |
| 19 | `pl_dynamic_voltage_frequency_governor` | Recommends a virtual quota from latency, utilization and temperature telemetry. It does not write DVFS controls. | Planning only |
| 20 | `pl_optimize_cache_line_layout` | Creates stable, alignment-aware field placements grouped by heat and cache-line boundary. | Native |
| 21 | `pl_eliminate_redundant_bounds_checks` | Proves bounds for a simple monotonic loop range. It does not rewrite compiler IR or remove checks in a binary. | Native |
| 22 | `pl_fuse_compute_kernels` | Groups adjacent dependency-compatible nodes and keeps side-effect barriers separate. It does not generate fused machine code. | Native |
| 23 | `pl_tune_branch_predictor_hints` | Computes branch probability and likely/unlikely/neutral classification from counters. | Native |
| 24 | `pl_defragment_heap_and_slab_pools` | Compacts live objects inside a caller-owned slab and updates offsets. It is not a process-wide heap or GC. | Native |
| 25 | `pl_accelerate_bigint_modular_ops` | Overflow-safe 64-bit modular multiplication with a portable scalar fallback. It is not a constant-time cryptographic bigint implementation. | Native |
| 26 | `pl_enable_hardware_crypto_offload` | Selects CPU/device/scalar backend from explicitly supplied capability flags; dispatch is external. | Planning only |
| 27 | `pl_synthesize_fpga_bit_stream_overlay` | Invokes a supplied vendor synthesis callback and validates the returned byte count. The core never fabricates a bitstream. | Adapter required |
| 28 | `pl_patch_microcode_simulation_table` | Replaces a bounded opcode-to-micro-op sequence in a simulator table only. | Simulated |
| 29 | `pl_define_custom_isa_extension` | Registers a bounded custom instruction definition and encodes its operand field for a simulator/compiler adapter. | Simulated |
| 30 | `pl_program_pmu_hardware_counters` | Aggregates read-only PMU samples supplied by a collector, calculating IPC and miss rates. It does not program counters. | Planning only |
| 31 | `pl_configure_cache_way_partitioning` | Computes a CAT way mask from available/requested ways. No MSR, resctrl or kernel setting is changed. | Planning only |
| 32 | `pl_inject_fault_and_latency_model` | Produces deterministic virtual latency/fault events from a seed for bus and resilience tests. | Simulated |
| 33 | `pl_verify_and_seal_firmware_overlay` | Checks a digest, version policy and optional signature verifier before reporting a local artifact as sealed. It never writes firmware or signs an image. | Adapter required |

## API contracts

- Public functions return `0` on success and a negative value for invalid input, rejected policy, missing capacity or callback failure. Some APIs use positive values for normal queue states: ring-buffer push/pop return `1` for full/empty.
- Arrays, buffers, callback contexts and state ownership remain with the caller. Capacity and count arguments are validated before bounded writes.
- Cryptographic correctness depends on the caller's hash, proof, signature and ZK adapters. Tests use a deterministic non-cryptographic test hash and do not establish production cryptographic security.
- RPC scheduling is a local token-bucket implementation. It must be configured to the endpoint provider's documented quotas; endpoint rotation is not a license to evade rate restrictions.
- The ring buffer is SPSC only. One producer and one consumer may access it concurrently; multi-producer/multi-consumer use requires external synchronization or separate queues.
- The slab compactor is limited to caller-owned object descriptors and a single caller-owned byte buffer. It cannot relocate pointers held outside those descriptors.
- Vector widths and hardware crypto backends are plans based on caller-provided capability data. The module does not execute AVX/NEON instructions or dynamically load accelerators.
- Cache, scheduler, PMU, CAT, frequency and voltage functions are non-privileged plans. Physical application is deliberately outside the core.
- FPGA synthesis requires an explicitly supplied vendor/toolchain callback. No production RTL synthesis, placement, routing or timing closure is claimed.

## Build and verification

    cmake -S . -B build -DBUILD_TESTING=ON
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure
    ./build/pytyliki --systems-lab

CTest covers all 33 catalogue entries and exercises their success paths plus representative capacity, finality, rollback, dispatch and policy failures. Windows and Linux CI results are the evidence for a commit; the local tests do not establish support for untested microarchitectures, blockchain protocols, ZK systems or FPGA devices.
