#include "systems_accel.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static int test_hash32(const uint8_t *data, size_t size, uint8_t digest[32], void *context) {
    (void)context;
    uint32_t h[8] = {
        UINT32_C(2166136261), UINT32_C(0x9e3779b9), UINT32_C(0x85ebca6b),
        UINT32_C(0xc2b2ae35), UINT32_C(0x27d4eb2f), UINT32_C(0x165667b1),
        UINT32_C(0xd3a2646c), UINT32_C(0xfd7046c5)
    };
    for (size_t i = 0u; i < size; ++i) {
        for (size_t j = 0u; j < 8u; ++j) {
            h[j] ^= (uint32_t)data[i] + (uint32_t)j;
            h[j] *= UINT32_C(16777619) + (uint32_t)(j * 2u);
            h[j] ^= h[j] >> 13u;
        }
    }
    for (size_t j = 0u; j < 8u; ++j) {
        digest[j * 4u] = (uint8_t)h[j];
        digest[j * 4u + 1u] = (uint8_t)(h[j] >> 8u);
        digest[j * 4u + 2u] = (uint8_t)(h[j] >> 16u);
        digest[j * 4u + 3u] = (uint8_t)(h[j] >> 24u);
    }
    return 0;
}

static int sim_execute(const uint8_t *tx, size_t tx_size, uint8_t *state,
                       size_t state_size, uint64_t *gas, void *context) {
    int fail = context != NULL ? *(int *)context : 0;
    if (fail) return -1;
    if (tx_size == 0u || state_size == 0u) return -1;
    state[0] = (uint8_t)(state[0] + tx[0]);
    *gas = 21000u;
    return 0;
}

static int relay_verify(uint64_t chain, uint64_t block, const uint8_t *proof,
                        size_t size, void *context) {
    (void)context;
    return chain == 1u && block == 100u && proof != NULL && size == 2u ? 0 : -1;
}

static int zk_copy(const uint8_t *proofs, size_t count, size_t proof_size,
                   const uint8_t *params, size_t params_size, uint8_t *out,
                   size_t capacity, size_t *out_size, void *context) {
    (void)params; (void)params_size; (void)context;
    if (count == 0u || proof_size > capacity) return -1;
    memcpy(out, proofs, proof_size);
    *out_size = proof_size;
    return 0;
}

typedef struct { int active; int applied; } bundle_state;
static int bundle_begin(void *context) {
    bundle_state *s = (bundle_state *)context;
    s->active = 1; s->applied = 0; return 0;
}
static int bundle_apply(const pl_bundle_transaction *tx, size_t index, void *context) {
    bundle_state *s = (bundle_state *)context;
    (void)index;
    if (!s->active || tx->size == 0u) return -1;
    ++s->applied; return 0;
}
static int bundle_commit(void *context) {
    bundle_state *s = (bundle_state *)context;
    if (!s->active) return -1;
    s->active = 0; return 0;
}
static void bundle_rollback(void *context) {
    bundle_state *s = (bundle_state *)context;
    s->active = 0; s->applied = 0;
}

static int opcode_handler(uint8_t opcode, uint64_t *registers,
                          size_t count, void *context) {
    (void)context;
    if (count == 0u) return -1;
    registers[0] += opcode;
    return 0;
}

static int fpga_adapter(const char *dsl, const char *target, uint8_t *out,
                        size_t capacity, size_t *size, void *context) {
    (void)context;
    if (strcmp(dsl, "add a,b") != 0 || strcmp(target, "test-fabric") != 0 || capacity == 0u)
        return -1;
    out[0] = 0x42u; *size = 1u; return 0;
}

static int signature_valid(const uint8_t *image, size_t image_size,
                           const uint8_t *signature, size_t signature_size,
                           void *context) {
    (void)image; (void)image_size; (void)context;
    return signature != NULL && signature_size == 1u && signature[0] == 0x5au ? 0 : -1;
}

int main(void) {
    const pl_systems_function_info *catalog = NULL;
    assert(pl_systems_lab_catalog(&catalog) == 33u);
    assert(catalog != NULL && catalog[0].id == 1u && catalog[32].id == 33u);
    assert(strcmp(pl_systems_mode_name(PL_SYSTEMS_MODE_SIMULATED), "simulated") == 0);

    /* 1. Merkle proof verification. */
    uint8_t leaf[32] = {1u}, sibling[32] = {2u}, root[32], pair[64];
    memcpy(pair, leaf, 32u); memcpy(pair + 32u, sibling, 32u);
    assert(test_hash32(pair, sizeof(pair), root, NULL) == 0);
    assert(pl_unlock_light_client_state_proofs(leaf, sibling, 1u, 0u, root,
                                                test_hash32, NULL) == 0);
    root[0] ^= 1u;
    assert(pl_unlock_light_client_state_proofs(leaf, sibling, 1u, 0u, root,
                                                test_hash32, NULL) == -2);

    /* 2. Provider-compliant endpoint token buckets. */
    pl_rpc_governor rpc;
    size_t endpoint = 99u;
    assert(pl_rpc_governor_init(&rpc, 2u, 1.0, 2.0) == 0);
    assert(pl_bypass_rpc_rate_throttling(&rpc, 100u, 2u, &endpoint) == 0);
    assert(endpoint == 0u);
    assert(pl_bypass_rpc_rate_throttling(&rpc, 100u, 1u, &endpoint) == 0);
    assert(endpoint == 1u);
    assert(pl_bypass_rpc_rate_throttling(&rpc, 100u, 1u, &endpoint) == -2);

    /* 3. Isolated mempool fork. */
    uint8_t base_state[4] = {10u, 0u, 0u, 0u}, fork_state[4] = {0u}, tx[] = {5u};
    uint64_t gas = 0u;
    int fail = 0;
    assert(pl_unlock_mempool_simulation_hooks(base_state, fork_state, sizeof(base_state),
                                               tx, sizeof(tx), sim_execute, &fail, &gas) == 0);
    assert(base_state[0] == 10u && fork_state[0] == 15u && gas == 21000u);
    fail = 1;
    assert(pl_unlock_mempool_simulation_hooks(base_state, fork_state, sizeof(base_state),
                                               tx, sizeof(tx), sim_execute, &fail, &gas) == -2);
    assert(fork_state[0] == base_state[0]);

    /* 4. PUSH4 selector extraction ignores bytes inside PUSH payloads. */
    const uint8_t bytecode[] = {0x63u, 0x12u, 0x34u, 0x56u, 0x78u, 0x64u,
                                0x63u, 0x00u, 0x00u, 0x00u, 0x00u, 0x63u,
                                0x12u, 0x34u, 0x56u, 0x78u};
    uint32_t selectors[4] = {0u}; size_t selector_count = 0u;
    assert(pl_extract_abi_and_bytecode_signatures(bytecode, sizeof(bytecode),
                                                   selectors, 4u, &selector_count) == 0);
    assert(selector_count == 1u && selectors[0] == UINT32_C(0x12345678));

    /* 5. Finality-gated cross-chain packet. */
    uint8_t proof[2] = {7u, 8u}, event_hash[32] = {9u};
    pl_cross_chain_relay_packet packet;
    assert(pl_unlock_cross_chain_message_relay(1u, 2u, 100u, 110u, 5u,
                                                event_hash, proof, sizeof(proof),
                                                relay_verify, NULL, &packet) == 0);
    assert(packet.confirmations == 11u && packet.proof_size == 2u);
    assert(pl_unlock_cross_chain_message_relay(1u, 2u, 100u, 101u, 5u,
                                                event_hash, proof, sizeof(proof),
                                                relay_verify, NULL, &packet) == -2);

    /* 6. Storage trace summaries. */
    const pl_storage_access accesses[] = {
        {1u, 11u, PL_STORAGE_SLOAD}, {1u, 22u, PL_STORAGE_SSTORE},
        {1u, 11u, PL_STORAGE_SLOAD}, {2u, 0u, PL_STORAGE_SSTORE}
    };
    pl_storage_layout_report storage_report;
    assert(pl_deobfuscate_storage_layout_slots(accesses, 4u, &storage_report) == 0);
    assert(storage_report.slot_count == 2u);
    assert(storage_report.slots[0].guess == PL_STORAGE_LAYOUT_MAPPING_CANDIDATE);

    /* 7. Deterministic replay fingerprint. */
    pl_replay_event replay = {0};
    replay.block_number = 101u; replay.timestamp = 12345u;
    replay.oracle_value = 77u; replay.transaction_hash[0] = 0xabu;
    uint8_t initial[] = {1u, 2u, 3u}, fingerprint_a[32], fingerprint_b[32];
    assert(pl_unlock_deterministic_replay_snapshot(100u, 105u, 9u, initial,
             sizeof(initial), &replay, 1u, test_hash32, NULL, fingerprint_a) == 0);
    assert(pl_unlock_deterministic_replay_snapshot(100u, 105u, 9u, initial,
             sizeof(initial), &replay, 1u, test_hash32, NULL, fingerprint_b) == 0);
    assert(memcmp(fingerprint_a, fingerprint_b, 32u) == 0);

    /* 8. Delegated ZK aggregation adapter. */
    uint8_t proofs[] = {1u, 2u, 3u, 4u}, aggregate[8] = {0u};
    size_t aggregate_size = 0u;
    assert(pl_enable_offchain_zk_proof_aggregation(proofs, 2u, 2u, NULL, 0u,
             zk_copy, NULL, aggregate, sizeof(aggregate), &aggregate_size) == 0);
    assert(aggregate_size == 2u && aggregate[0] == 1u && aggregate[1] == 2u);

    /* 9. Atomic bundle orchestration. */
    uint8_t tx_a[] = {1u}, tx_b[] = {2u};
    pl_bundle_transaction bundle[] = {{tx_a, 1u, 21000u}, {tx_b, 1u, 30000u}};
    bundle_state bs = {0};
    pl_bundle_executor executor = {bundle_begin, bundle_apply, bundle_commit, bundle_rollback, &bs};
    uint64_t estimated_gas = 0u;
    assert(pl_unlock_batch_transaction_bundling(bundle, 2u, 60000u,
             &executor, &estimated_gas) == 0);
    assert(estimated_gas == 51000u && bs.active == 0 && bs.applied == 2);
    assert(pl_unlock_batch_transaction_bundling(bundle, 2u, 40000u,
             &executor, &estimated_gas) == -2);

    /* 10. Call trace and revert reason summary. */
    const pl_call_trace_item trace[] = {
        {PL_TRACE_CALL, 0u, 100u, 0, NULL},
        {PL_TRACE_DELEGATECALL, 2u, 200u, 1, "execution reverted"}
    };
    pl_call_trace_summary trace_report;
    assert(pl_inspect_call_trace_and_revert_reasons(trace, 2u, 4u, &trace_report) == 0);
    assert(trace_report.call_count == 2u && trace_report.revert_count == 1u);
    assert(trace_report.total_gas == 300u && strcmp(trace_report.first_revert_reason, "execution reverted") == 0);

    /* 11. Instance-scoped VM opcode hook. */
    pl_vm_hook_table hooks; uint64_t registers[2] = {10u, 0u};
    pl_vm_hook_table_init(&hooks);
    assert(pl_unlock_custom_vm_opcode_hook(&hooks, 0x42u, opcode_handler, NULL) == 0);
    assert(pl_vm_dispatch_opcode(&hooks, 0x42u, registers, 2u) == 0 && registers[0] == 76u);

    /* 12-13. Virtual boost and affinity planning. */
    pl_virtual_boost_plan boost;
    assert(pl_virtual_boost_clock_scaling(0u, 5.0, 4.0, &boost) == 0);
    assert(boost.thermally_limited && boost.effective_virtual_ghz == 4.0);
    pl_turbo_pinning_plan pinning;
    assert(pl_turbo_scheduler_pinning(0x0fu, 0x33u, 1u, 0u, &pinning) == 0);
    assert(pinning.selected_core_mask == 0x03u);

    /* 14. Profile-driven prefetch hints. */
    const pl_branch_history_item history[] = {{1u, 0x100u, 1}, {2u, 0x100u, 1}, {3u, 0x200u, 0}};
    uint64_t hints[4]; size_t hint_count = 0u;
    assert(pl_speculative_trace_prewarming(history, 3u, 3u, hints, 4u, &hint_count) == 0);
    assert(hint_count == 1u && hints[0] == 0x100u);

    /* 15. JIT tier promotion decision. */
    pl_jit_profile jit = {90u, 20u, 1u}; unsigned tier = 0u;
    assert(pl_jit_hot_loop_tier_promotion(&jit, 100u, 3u, &tier) == 0 && tier == 2u);

    /* 16. Bounded SPSC ring buffer. */
    pl_lockless_ring_buffer queue; uint32_t queue_storage[2], value = 42u, read_value = 0u;
    assert(pl_lockless_ring_buffer_pipeline_init(&queue, queue_storage, 2u, sizeof(uint32_t)) == 0);
    assert(pl_lockless_ring_buffer_push(&queue, &value) == 0);
    assert(pl_lockless_ring_buffer_push(&queue, &value) == 0);
    assert(pl_lockless_ring_buffer_push(&queue, &value) == 1);
    assert(pl_lockless_ring_buffer_pop(&queue, &read_value) == 0 && read_value == 42u);

    /* 17-19. Vector, zero-copy arena and virtual governor. */
    pl_vectorization_plan vector_plan;
    assert(pl_adaptive_batch_vectorization(19u, 16u, (1u << 8u) | (1u << 4u), &vector_plan) == 0);
    assert(vector_plan.lane_count == 8u && vector_plan.scalar_tail == 3u);
    uint8_t arena_bytes[64]; pl_zero_copy_arena arena; void *allocation = NULL; size_t remaining = 0u;
    assert(pl_zero_copy_arena_init(&arena, arena_bytes, sizeof(arena_bytes)) == 0);
    assert(pl_zero_copy_memory_arena_boost(&arena, 8u, 16u, &allocation, &remaining) == 0);
    assert(((uintptr_t)allocation % 16u) == 0u && remaining < sizeof(arena_bytes));
    pl_governor_telemetry telemetry = {2000.0, 60.0, 90.0};
    pl_virtual_governor_plan governor_plan;
    assert(pl_dynamic_voltage_frequency_governor(&telemetry, 1000.0, 90.0, 100.0, &governor_plan) == 0);
    assert(governor_plan.latency_target_missed && governor_plan.recommended_quota_percent > 50.0);

    /* 20-23. Layout, range proof, kernel fusion and branch hint. */
    const pl_layout_field fields[] = {{1u, 8u, 8u, 1u}, {2u, 8u, 8u, 0u}};
    pl_layout_placement placements[2]; size_t total_size = 0u;
    assert(pl_optimize_cache_line_layout(fields, 2u, 64u, placements, 2u, &total_size) == 0);
    assert(placements[0].id == 2u && placements[0].offset == 0u);
    pl_loop_range_invariant invariant = {0, 99, 1, 100u}; int proven = 0;
    assert(pl_eliminate_redundant_bounds_checks(&invariant, &proven) == 0 && proven);
    const pl_kernel_node kernels[] = {
        {1u, 0u, 1u, 1, 0}, {2u, 1u, 2u, 1, 0}, {3u, 2u, 4u, 0, 1}
    };
    pl_kernel_fusion_plan fusion;
    assert(pl_fuse_compute_kernels(kernels, 3u, &fusion) == 0);
    assert(fusion.group_count == 2u && fusion.group_for_node[0] == fusion.group_for_node[1]);
    pl_branch_hint hint; double probability = 0.0;
    assert(pl_tune_branch_predictor_hints(90u, 10u, 0.8, &hint, &probability) == 0);
    assert(hint == PL_BRANCH_LIKELY && probability == 0.9);

    /* 24-26. Slab compaction, modular arithmetic and crypto backend selection. */
    uint8_t slab[64] = {0}; slab[8] = 1u; slab[24] = 2u;
    pl_slab_object slab_objects[] = {{8u, 4u, 4u, 1}, {12u, 4u, 4u, 0}, {24u, 4u, 8u, 1}};
    size_t reclaimed = 0u;
    assert(pl_defragment_heap_and_slab_pools(slab, sizeof(slab), slab_objects, 3u, &reclaimed) == 0);
    assert(slab_objects[0].offset == 0u && slab_objects[2].offset == 8u && reclaimed == 52u);
    uint64_t modular = 0u;
    assert(pl_accelerate_bigint_modular_ops(123u, 456u, 97u, 64u, &modular) == 0 && modular == 22u);
    pl_crypto_device_caps caps = {1, 0, 1, 1, 0}; pl_crypto_backend backend;
    assert(pl_enable_hardware_crypto_offload(PL_CRYPTO_AES_GCM, &caps, &backend) == 0);
    assert(backend == PL_CRYPTO_BACKEND_CPU);

    /* 27-29. Explicit FPGA adapter, simulator micro-ops and custom ISA registry. */
    uint8_t bitstream[8]; size_t bitstream_size = 0u;
    assert(pl_synthesize_fpga_bit_stream_overlay("add a,b", "test-fabric", fpga_adapter,
             NULL, bitstream, sizeof(bitstream), &bitstream_size) == 0);
    assert(bitstream_size == 1u && bitstream[0] == 0x42u);
    pl_microcode_sim_table microcode = {0}; const uint16_t micro_ops[] = {1u, 3u, 5u};
    assert(pl_patch_microcode_simulation_table(&microcode, 0x10u, micro_ops, 3u) == 0);
    assert(microcode.count[0x10u] == 3u && microcode.micro_ops[0x10u][2] == 5u);
    pl_isa_extension isa = {"P_ADD", 0xf0000000u, 0xa0000000u, 32u, 20u};
    pl_isa_extension_registry isa_registry = {0}; uint32_t word = 0u;
    assert(pl_define_custom_isa_extension(&isa_registry, &isa) == 0);
    assert(pl_isa_extension_encode(&isa, 0x12345u, &word) == 0 && word == 0xa0012345u);

    /* 30-33. PMU aggregation, CAT plan, deterministic faults and overlay sealing. */
    const pl_pmu_sample samples[] = {{100u, 200u, 100u, 10u, 20u, 5u}};
    pl_pmu_report pmu;
    assert(pl_program_pmu_hardware_counters(samples, 1u, &pmu) == 0);
    assert(pmu.ipc == 2.0 && pmu.cache_miss_rate == 0.1 && pmu.branch_miss_rate == 0.25);
    pl_cat_partition_plan cat;
    assert(pl_configure_cache_way_partitioning(8u, 3u, &cat) == 0 && cat.way_mask == 7u);
    pl_fault_latency_profile fault = {100u, 1000000u, 8u};
    pl_fault_latency_result fault_result;
    assert(pl_inject_fault_and_latency_model(&fault, 123u, &fault_result) == 0);
    assert(fault_result.fault_injected && fault_result.bit_index < 8u && fault_result.latency_ns <= 100u);
    uint8_t image[] = {1u, 2u, 3u}, digest[32], signature[] = {0x5au};
    assert(test_hash32(image, sizeof(image), digest, NULL) == 0);
    pl_overlay_policy overlay_policy = {1u, 3u, 1, 0};
    pl_overlay_seal_report seal_report;
    assert(pl_verify_and_seal_firmware_overlay(image, sizeof(image), 2u, digest,
             signature, sizeof(signature), test_hash32, signature_valid, NULL,
             &overlay_policy, &seal_report) == 0);
    assert(seal_report.sealed && seal_report.digest_matches && seal_report.signature_valid);

    return 0;
}
