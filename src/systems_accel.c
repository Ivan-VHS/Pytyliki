#include "systems_accel.h"

#include <math.h>
#include <limits.h>
#include <string.h>

#define PL_SYSTEMS_MAX_BYTECODE (1024u * 1024u)
#define PL_SYSTEMS_MAX_TX_BYTES (1024u * 1024u)
#define PL_SYSTEMS_MAX_REPLAY_EVENTS 4096u
#define PL_SYSTEMS_MAX_RING_ITEM (1024u * 1024u)

static const pl_systems_function_info g_systems_functions[PL_SYSTEMS_FUNCTION_COUNT] = {
    { 1u, "unlockLightClientStateProofs", PL_SYSTEMS_ADAPTER_REQUIRED, "Verify a Merkle branch against a trusted state root using a caller-supplied hash." },
    { 2u, "bypassRpcRateThrottling", PL_SYSTEMS_NATIVE, "Schedule requests through per-endpoint token buckets while respecting configured provider quotas." },
    { 3u, "unlockMempoolSimulationHooks", PL_SYSTEMS_SIMULATED, "Run a transaction against a caller-owned fork buffer without changing its source snapshot." },
    { 4u, "extractAbiAndBytecodeSignatures", PL_SYSTEMS_NATIVE, "Extract unique EVM PUSH4 selectors while correctly skipping PUSH immediates." },
    { 5u, "unlockCrossChainMessageRelay", PL_SYSTEMS_ADAPTER_REQUIRED, "Verify proof and finality and serialize a relay packet; network submission is external." },
    { 6u, "deobfuscateStorageLayoutSlots", PL_SYSTEMS_NATIVE, "Summarize SLOAD/SSTORE traces and produce explicitly heuristic slot classifications." },
    { 7u, "unlockDeterministicReplaySnapshot", PL_SYSTEMS_ADAPTER_REQUIRED, "Build a stable replay fingerprint from explicit event fields and initial state." },
    { 8u, "enableOffchainZkProofAggregation", PL_SYSTEMS_ADAPTER_REQUIRED, "Validate and invoke a protocol-specific ZK aggregation adapter." },
    { 9u, "unlockBatchTransactionBundling", PL_SYSTEMS_NATIVE, "Preflight gas and orchestrate an atomic caller-provided begin/apply/commit/rollback boundary." },
    { 10u, "inspectCallTraceAndRevertReasons", PL_SYSTEMS_NATIVE, "Summarize bounded VM traces, nested depth, gas and the first revert reason." },
    { 11u, "unlockCustomVmOpcodeHook", PL_SYSTEMS_SIMULATED, "Register per-instance opcode handlers for a VM or emulator." },
    { 12u, "virtualBoostClockScaling", PL_SYSTEMS_SIMULATED, "Compute a virtual vCPU quota and capped virtual clock profile." },
    { 13u, "turboSchedulerPinning", PL_SYSTEMS_PLANNING, "Intersect requested core affinity with an available topology mask." },
    { 14u, "speculativeTracePrewarming", PL_SYSTEMS_PLANNING, "Generate deduplicated prefetch hints from a bounded branch profile." },
    { 15u, "jitHotLoopTierPromotion", PL_SYSTEMS_PLANNING, "Promote a hot-loop tier from measured counters for an external JIT." },
    { 16u, "locklessRingBufferPipeline", PL_SYSTEMS_NATIVE, "Provide a bounded single-producer/single-consumer C11 atomic ring buffer." },
    { 17u, "adaptiveBatchVectorization", PL_SYSTEMS_PLANNING, "Choose a supported vector width and scalar tail without unsafe ISA dispatch." },
    { 18u, "zeroCopyMemoryArenaBoost", PL_SYSTEMS_NATIVE, "Allocate aligned slices from caller-owned memory without copying payloads." },
    { 19u, "dynamicVoltageFrequencyGovernor", PL_SYSTEMS_PLANNING, "Recommend a virtual CPU quota from latency and thermal telemetry." },
    { 20u, "optimizeCacheLineLayout", PL_SYSTEMS_NATIVE, "Plan stable field placement by heat group, alignment and cache-line boundaries." },
    { 21u, "eliminateRedundantBoundsChecks", PL_SYSTEMS_NATIVE, "Prove simple monotonic loop index bounds; it does not rewrite compiler IR." },
    { 22u, "fuseComputeKernels", PL_SYSTEMS_NATIVE, "Group adjacent dependency-compatible kernels without crossing side-effect barriers." },
    { 23u, "tuneBranchPredictorHints", PL_SYSTEMS_NATIVE, "Derive likely/unlikely hints from observed branch counters." },
    { 24u, "defragmentHeapAndSlabPools", PL_SYSTEMS_NATIVE, "Compact live objects in a caller-owned slab and update object offsets." },
    { 25u, "accelerateBigIntModularOps", PL_SYSTEMS_NATIVE, "Perform overflow-safe modular multiplication with a portable scalar fallback." },
    { 26u, "enableHardwareCryptoOffload", PL_SYSTEMS_PLANNING, "Select a declared CPU/device backend; execution remains the caller's responsibility." },
    { 27u, "synthesizeFpgaBitStreamOverlay", PL_SYSTEMS_ADAPTER_REQUIRED, "Call an explicit vendor synthesis adapter; the core never fabricates bitstreams." },
    { 28u, "patchMicrocodeSimulationTable", PL_SYSTEMS_SIMULATED, "Patch a micro-op lookup table belonging to a simulator instance only." },
    { 29u, "defineCustomIsaExtension", PL_SYSTEMS_SIMULATED, "Register and encode a simulator/compiler-facing custom instruction definition." },
    { 30u, "programPmuHardwareCounters", PL_SYSTEMS_PLANNING, "Aggregate counters supplied by a read-only PMU collection adapter." },
    { 31u, "configureCacheWayPartitioning", PL_SYSTEMS_PLANNING, "Compute a CAT way mask without writing model-specific registers." },
    { 32u, "injectFaultAndLatencyModel", PL_SYSTEMS_SIMULATED, "Generate deterministic virtual bus latency and fault events from a seed." },
    { 33u, "verifyAndSealFirmwareOverlay", PL_SYSTEMS_ADAPTER_REQUIRED, "Verify digest, version policy and signature before marking an artifact sealed." }
};

static int is_power_of_two(size_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}

static int align_up_size(size_t value, size_t alignment, size_t *out) {
    size_t mask;
    if (out == NULL || !is_power_of_two(alignment)) return -1;
    mask = alignment - 1u;
    if (value > SIZE_MAX - mask) return -1;
    *out = (value + mask) & ~mask;
    return 0;
}

static uint64_t add_mod_u64(uint64_t a, uint64_t b, uint64_t modulus) {
    return a >= modulus - b ? a - (modulus - b) : a + b;
}

static uint64_t multiply_mod_u64(uint64_t a, uint64_t b, uint64_t modulus) {
    uint64_t result = 0u;
    a %= modulus;
    while (b != 0u) {
        if ((b & 1u) != 0u) result = add_mod_u64(result, a, modulus);
        b >>= 1u;
        if (b != 0u) a = add_mod_u64(a, a, modulus);
    }
    return result;
}

static uint64_t xorshift64(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13u;
    x ^= x >> 7u;
    x ^= x << 17u;
    *state = x;
    return x;
}

static void store_le64(uint8_t out[8], uint64_t value) {
    for (unsigned i = 0u; i < 8u; ++i) out[i] = (uint8_t)(value >> (i * 8u));
}

size_t pl_systems_lab_catalog(const pl_systems_function_info **items) {
    if (items != NULL) *items = g_systems_functions;
    return PL_SYSTEMS_FUNCTION_COUNT;
}

const char *pl_systems_mode_name(pl_systems_mode mode) {
    switch (mode) {
        case PL_SYSTEMS_MODE_NATIVE: return "native";
        case PL_SYSTEMS_MODE_SIMULATED: return "simulated";
        case PL_SYSTEMS_MODE_ADAPTER_REQUIRED: return "adapter-required";
        case PL_SYSTEMS_MODE_PLANNING: return "planning-only";
        default: return "unknown";
    }
}

int pl_unlock_light_client_state_proofs(const uint8_t leaf_hash[32],
                                        const uint8_t *siblings,
                                        size_t sibling_count,
                                        uint64_t leaf_index,
                                        const uint8_t expected_root[32],
                                        pl_system_hash32_fn hash32,
                                        void *hash_context) {
    uint8_t current[32], pair[64], next[32];
    if (leaf_hash == NULL || expected_root == NULL || hash32 == NULL ||
        (sibling_count != 0u && siblings == NULL) ||
        sibling_count > PL_SYSTEMS_MAX_PROOF_DEPTH ||
        (sibling_count < 64u && (leaf_index >> sibling_count) != 0u)) return -1;
    memcpy(current, leaf_hash, sizeof(current));
    for (size_t i = 0u; i < sibling_count; ++i) {
        const uint8_t *sibling = siblings + i * 32u;
        if (((leaf_index >> i) & 1u) == 0u) {
            memcpy(pair, current, 32u);
            memcpy(pair + 32u, sibling, 32u);
        } else {
            memcpy(pair, sibling, 32u);
            memcpy(pair + 32u, current, 32u);
        }
        if (hash32(pair, sizeof(pair), next, hash_context) != 0) return -2;
        memcpy(current, next, sizeof(current));
    }
    return memcmp(current, expected_root, sizeof(current)) == 0 ? 0 : -2;
}

int pl_rpc_governor_init(pl_rpc_governor *governor, size_t endpoint_count,
                         double requests_per_second, double burst_capacity) {
    if (governor == NULL || endpoint_count == 0u ||
        endpoint_count > PL_RPC_MAX_ENDPOINTS ||
        !isfinite(requests_per_second) || requests_per_second <= 0.0 ||
        !isfinite(burst_capacity) || burst_capacity < 1.0) return -1;
    memset(governor, 0, sizeof(*governor));
    governor->endpoint_count = endpoint_count;
    governor->tokens_per_second = requests_per_second;
    governor->burst_capacity = burst_capacity;
    for (size_t i = 0u; i < endpoint_count; ++i)
        governor->tokens[i] = burst_capacity;
    return 0;
}

int pl_bypass_rpc_rate_throttling(pl_rpc_governor *governor,
                                  uint64_t now_ns, size_t request_count,
                                  size_t *selected_endpoint) {
    if (governor == NULL || selected_endpoint == NULL ||
        governor->endpoint_count == 0u || request_count == 0u ||
        !isfinite(governor->tokens_per_second) ||
        !isfinite(governor->burst_capacity) ||
        (double)request_count > governor->burst_capacity) return -1;
    for (size_t n = 0u; n < governor->endpoint_count; ++n) {
        size_t index = (governor->next_endpoint + n) % governor->endpoint_count;
        uint64_t previous = governor->last_refill_ns[index];
        if (now_ns > previous) {
            double elapsed = (double)(now_ns - previous) / 1000000000.0;
            double refill = elapsed * governor->tokens_per_second;
            governor->tokens[index] += refill;
            if (governor->tokens[index] > governor->burst_capacity)
                governor->tokens[index] = governor->burst_capacity;
            governor->last_refill_ns[index] = now_ns;
        } else if (previous == 0u) {
            governor->last_refill_ns[index] = now_ns;
        }
        if (governor->tokens[index] >= (double)request_count) {
            governor->tokens[index] -= (double)request_count;
            *selected_endpoint = index;
            governor->next_endpoint = (index + 1u) % governor->endpoint_count;
            return 0;
        }
    }
    return -2;
}

int pl_unlock_mempool_simulation_hooks(const uint8_t *base_state,
                                       uint8_t *fork_state, size_t state_size,
                                       const uint8_t *transaction, size_t tx_size,
                                       pl_mempool_execute_fn execute,
                                       void *context, uint64_t *gas_used) {
    uint64_t local_gas = 0u;
    if (base_state == NULL || fork_state == NULL || state_size == 0u ||
        state_size > 16u * 1024u * 1024u || transaction == NULL ||
        tx_size == 0u || tx_size > PL_SYSTEMS_MAX_TX_BYTES ||
        execute == NULL || gas_used == NULL) return -1;
    memcpy(fork_state, base_state, state_size);
    if (execute(transaction, tx_size, fork_state, state_size,
                &local_gas, context) != 0) {
        memcpy(fork_state, base_state, state_size);
        *gas_used = 0u;
        return -2;
    }
    *gas_used = local_gas;
    return 0;
}

int pl_extract_abi_and_bytecode_signatures(const uint8_t *bytecode,
                                            size_t bytecode_size,
                                            uint32_t *selectors,
                                            size_t selector_capacity,
                                            size_t *selector_count) {
    size_t count = 0u;
    if (bytecode == NULL || bytecode_size == 0u ||
        bytecode_size > PL_SYSTEMS_MAX_BYTECODE ||
        selectors == NULL || selector_count == NULL) return -1;
    for (size_t pc = 0u; pc < bytecode_size;) {
        uint8_t opcode = bytecode[pc++];
        if (opcode >= 0x60u && opcode <= 0x7fu) {
            size_t immediate = (size_t)(opcode - 0x5fu);
            if (immediate > bytecode_size - pc) return -2;
            if (opcode == 0x63u) {
                uint32_t selector = ((uint32_t)bytecode[pc] << 24u) |
                                    ((uint32_t)bytecode[pc + 1u] << 16u) |
                                    ((uint32_t)bytecode[pc + 2u] << 8u) |
                                    (uint32_t)bytecode[pc + 3u];
                size_t i;
                for (i = 0u; i < count; ++i) if (selectors[i] == selector) break;
                if (i == count) {
                    if (count >= selector_capacity) return -3;
                    selectors[count++] = selector;
                }
            }
            pc += immediate;
        }
    }
    *selector_count = count;
    return 0;
}

int pl_unlock_cross_chain_message_relay(uint64_t source_chain_id,
                                        uint64_t target_chain_id,
                                        uint64_t source_block,
                                        uint64_t finalized_block,
                                        uint64_t minimum_confirmations,
                                        const uint8_t event_hash[32],
                                        const uint8_t *proof, size_t proof_size,
                                        pl_relay_proof_verify_fn verify,
                                        void *verify_context,
                                        pl_cross_chain_relay_packet *out) {
    uint64_t confirmations;
    if (source_chain_id == 0u || target_chain_id == 0u ||
        source_chain_id == target_chain_id || event_hash == NULL ||
        proof == NULL || proof_size == 0u ||
        proof_size > PL_SYSTEMS_MAX_PROOF_BYTES || verify == NULL || out == NULL ||
        finalized_block < source_block) return -1;
    confirmations = finalized_block - source_block + 1u;
    if (confirmations < minimum_confirmations) return -2;
    if (verify(source_chain_id, source_block, proof, proof_size, verify_context) != 0)
        return -3;
    memset(out, 0, sizeof(*out));
    out->source_chain_id = source_chain_id;
    out->target_chain_id = target_chain_id;
    out->source_block = source_block;
    out->confirmations = confirmations;
    memcpy(out->event_hash, event_hash, 32u);
    memcpy(out->proof, proof, proof_size);
    out->proof_size = proof_size;
    return 0;
}

int pl_deobfuscate_storage_layout_slots(const pl_storage_access *accesses,
                                        size_t access_count,
                                        pl_storage_layout_report *out) {
    if (accesses == NULL || out == NULL || access_count == 0u ||
        access_count > PL_SYSTEMS_MAX_TRACE_ITEMS) return -1;
    memset(out, 0, sizeof(*out));
    for (size_t i = 0u; i < access_count; ++i) {
        if (accesses[i].opcode != PL_STORAGE_SLOAD &&
            accesses[i].opcode != PL_STORAGE_SSTORE) return -1;
        size_t slot_index;
        for (slot_index = 0u; slot_index < out->slot_count; ++slot_index)
            if (out->slots[slot_index].slot == accesses[i].slot) break;
        if (slot_index == out->slot_count) {
            if (out->slot_count >= PL_SYSTEMS_MAX_STORAGE_SLOTS) return -2;
            out->slots[slot_index].slot = accesses[i].slot;
            out->slots[slot_index].guess = PL_STORAGE_LAYOUT_UNKNOWN;
            ++out->slot_count;
        }
        pl_storage_slot_summary *summary = &out->slots[slot_index];
        if (accesses[i].opcode == PL_STORAGE_SLOAD) ++summary->reads;
        else ++summary->writes;
        if (accesses[i].key_fingerprint != 0u) {
            int seen = 0;
            for (size_t j = 0u; j < i; ++j) {
                if (accesses[j].slot == accesses[i].slot &&
                    accesses[j].key_fingerprint == accesses[i].key_fingerprint) {
                    seen = 1;
                    break;
                }
            }
            if (!seen) ++summary->distinct_key_fingerprints;
        }
    }
    for (size_t i = 0u; i < out->slot_count; ++i) {
        pl_storage_slot_summary *s = &out->slots[i];
        if (s->distinct_key_fingerprints > 1u)
            s->guess = PL_STORAGE_LAYOUT_MAPPING_CANDIDATE;
        else if (s->reads + s->writes > 1u)
            s->guess = PL_STORAGE_LAYOUT_ARRAY_CANDIDATE;
        else
            s->guess = PL_STORAGE_LAYOUT_SCALAR;
    }
    return 0;
}

int pl_unlock_deterministic_replay_snapshot(uint64_t first_block,
                                            uint64_t last_block,
                                            uint64_t seed,
                                            const uint8_t *initial_state,
                                            size_t state_size,
                                            const pl_replay_event *events,
                                            size_t event_count,
                                            pl_system_hash32_fn hash32,
                                            void *hash_context,
                                            uint8_t fingerprint[32]) {
    uint8_t buffer[120], state_digest[32], event_digest[32];
    if (first_block > last_block || hash32 == NULL || fingerprint == NULL ||
        (state_size != 0u && initial_state == NULL) ||
        (event_count != 0u && events == NULL) ||
        event_count > PL_SYSTEMS_MAX_REPLAY_EVENTS ||
        state_size > 16u * 1024u * 1024u) return -1;
    {
        const uint8_t empty_state = 0u;
        const uint8_t *state_data = initial_state != NULL ? initial_state : &empty_state;
        if (hash32(state_data, state_size, state_digest, hash_context) != 0) return -2;
    }
    store_le64(buffer, first_block);
    store_le64(buffer + 8u, last_block);
    store_le64(buffer + 16u, seed);
    memcpy(buffer + 24u, state_digest, 32u);
    if (hash32(buffer, 56u, fingerprint, hash_context) != 0) return -2;
    for (size_t i = 0u; i < event_count; ++i) {
        if (events[i].block_number < first_block ||
            events[i].block_number > last_block) return -1;
        memcpy(buffer, fingerprint, 32u);
        store_le64(buffer + 32u, events[i].block_number);
        store_le64(buffer + 40u, events[i].timestamp);
        store_le64(buffer + 48u, events[i].oracle_value);
        memcpy(buffer + 56u, events[i].transaction_hash, 32u);
        if (hash32(buffer, 88u, event_digest, hash_context) != 0) return -2;
        memcpy(fingerprint, event_digest, 32u);
    }
    return 0;
}

int pl_enable_offchain_zk_proof_aggregation(const uint8_t *proofs,
                                            size_t proof_count, size_t proof_size,
                                            const uint8_t *verifier_params,
                                            size_t params_size,
                                            pl_zk_aggregate_fn aggregate_fn,
                                            void *context,
                                            uint8_t *aggregate,
                                            size_t aggregate_capacity,
                                            size_t *aggregate_size) {
    if (proofs == NULL || proof_count == 0u || proof_count > 4096u ||
        proof_size == 0u || proof_size > PL_SYSTEMS_MAX_PROOF_BYTES ||
        proof_count > SIZE_MAX / proof_size ||
        (params_size != 0u && verifier_params == NULL) ||
        params_size > PL_SYSTEMS_MAX_PROOF_BYTES ||
        aggregate_fn == NULL || aggregate == NULL ||
        aggregate_capacity == 0u || aggregate_size == NULL) return -1;
    *aggregate_size = 0u;
    if (aggregate_fn(proofs, proof_count, proof_size, verifier_params, params_size,
                     aggregate, aggregate_capacity, aggregate_size, context) != 0)
        return -2;
    if (*aggregate_size == 0u || *aggregate_size > aggregate_capacity) {
        *aggregate_size = 0u;
        return -3;
    }
    return 0;
}

int pl_unlock_batch_transaction_bundling(const pl_bundle_transaction *transactions,
                                         size_t transaction_count,
                                         uint64_t gas_limit,
                                         const pl_bundle_executor *executor,
                                         uint64_t *estimated_gas) {
    uint64_t total = 0u;
    if (transactions == NULL || transaction_count == 0u ||
        transaction_count > PL_SYSTEMS_MAX_TX_ITEMS || gas_limit == 0u ||
        executor == NULL || executor->begin == NULL || executor->apply == NULL ||
        executor->commit == NULL || executor->rollback == NULL ||
        estimated_gas == NULL) return -1;
    for (size_t i = 0u; i < transaction_count; ++i) {
        if (transactions[i].data == NULL || transactions[i].size == 0u ||
            transactions[i].size > PL_SYSTEMS_MAX_TX_BYTES ||
            UINT64_MAX - total < transactions[i].estimated_gas) return -1;
        total += transactions[i].estimated_gas;
        if (total > gas_limit) return -2;
    }
    *estimated_gas = total;
    if (executor->begin(executor->context) != 0) return -3;
    for (size_t i = 0u; i < transaction_count; ++i) {
        if (executor->apply(&transactions[i], i, executor->context) != 0) {
            executor->rollback(executor->context);
            return -4;
        }
    }
    if (executor->commit(executor->context) != 0) {
        executor->rollback(executor->context);
        return -5;
    }
    return 0;
}

int pl_inspect_call_trace_and_revert_reasons(const pl_call_trace_item *trace,
                                             size_t trace_count,
                                             size_t maximum_depth,
                                             pl_call_trace_summary *out) {
    if (trace == NULL || out == NULL || trace_count > PL_SYSTEMS_MAX_TRACE_ITEMS ||
        maximum_depth == 0u || maximum_depth > 1024u) return -1;
    memset(out, 0, sizeof(*out));
    for (size_t i = 0u; i < trace_count; ++i) {
        if (trace[i].kind < PL_TRACE_CALL || trace[i].kind > PL_TRACE_CREATE2 ||
            trace[i].depth > maximum_depth ||
            UINT64_MAX - out->total_gas < trace[i].gas_used) return -1;
        ++out->call_count;
        out->total_gas += trace[i].gas_used;
        if (trace[i].depth > out->maximum_depth) out->maximum_depth = trace[i].depth;
        if (trace[i].reverted) {
            ++out->revert_count;
            if (out->first_revert_reason[0] == '\0' && trace[i].revert_reason != NULL) {
                size_t n = strlen(trace[i].revert_reason);
                if (n >= sizeof(out->first_revert_reason)) n = sizeof(out->first_revert_reason) - 1u;
                memcpy(out->first_revert_reason, trace[i].revert_reason, n);
                out->first_revert_reason[n] = '\0';
            }
        }
    }
    return 0;
}

void pl_vm_hook_table_init(pl_vm_hook_table *table) {
    if (table != NULL) memset(table, 0, sizeof(*table));
}

int pl_unlock_custom_vm_opcode_hook(pl_vm_hook_table *table, uint8_t opcode,
                                    pl_vm_opcode_handler handler, void *context) {
    if (table == NULL || handler == NULL) return -1;
    table->handlers[opcode] = handler;
    table->contexts[opcode] = context;
    return 0;
}

int pl_vm_dispatch_opcode(pl_vm_hook_table *table, uint8_t opcode,
                          uint64_t *registers, size_t register_count) {
    if (table == NULL || (register_count != 0u && registers == NULL)) return -1;
    if (table->handlers[opcode] == NULL) return -2;
    return table->handlers[opcode](opcode, registers, register_count,
                                   table->contexts[opcode]);
}

int pl_virtual_boost_clock_scaling(unsigned vcpu_id, double target_ghz,
                                   double thermal_cap_ghz,
                                   pl_virtual_boost_plan *out) {
    if (out == NULL || vcpu_id > 65535u || !isfinite(target_ghz) ||
        !isfinite(thermal_cap_ghz) || target_ghz <= 0.0 ||
        thermal_cap_ghz <= 0.0) return -1;
    out->requested_ghz = target_ghz;
    out->effective_virtual_ghz = target_ghz < thermal_cap_ghz ? target_ghz : thermal_cap_ghz;
    out->cpu_quota_percent = out->effective_virtual_ghz / target_ghz * 100.0;
    out->thermally_limited = target_ghz > thermal_cap_ghz;
    return 0;
}

int pl_turbo_scheduler_pinning(uint64_t requested_core_mask,
                               uint64_t available_core_mask,
                               unsigned numa_node, unsigned vcpu_group,
                               pl_turbo_pinning_plan *out) {
    if (out == NULL || requested_core_mask == 0u || available_core_mask == 0u)
        return -1;
    uint64_t selected = requested_core_mask & available_core_mask;
    if (selected == 0u) return -2;
    out->selected_core_mask = selected;
    out->numa_node = numa_node;
    out->vcpu_group = vcpu_group;
    return 0;
}

int pl_speculative_trace_prewarming(const pl_branch_history_item *history,
                                    size_t history_count, size_t prefetch_window,
                                    uint64_t *hints, size_t hint_capacity,
                                    size_t *hint_count) {
    size_t count = 0u;
    if (history == NULL || hints == NULL || hint_count == NULL ||
        history_count == 0u || history_count > PL_SYSTEMS_MAX_TRACE_ITEMS ||
        prefetch_window == 0u || prefetch_window > history_count) return -1;
    size_t start = history_count - prefetch_window;
    for (size_t i = start; i < history_count; ++i) {
        if (!history[i].taken || history[i].target_address == 0u) continue;
        size_t j;
        for (j = 0u; j < count; ++j) if (hints[j] == history[i].target_address) break;
        if (j == count) {
            if (count >= hint_capacity) return -2;
            hints[count++] = history[i].target_address;
        }
    }
    *hint_count = count;
    return 0;
}

int pl_jit_hot_loop_tier_promotion(pl_jit_profile *profile,
                                   uint64_t hit_threshold, unsigned maximum_tier,
                                   unsigned *promoted_tier) {
    if (profile == NULL || promoted_tier == NULL || hit_threshold == 0u ||
        maximum_tier == 0u || maximum_tier > 16u ||
        profile->current_tier > maximum_tier) return -1;
    *promoted_tier = profile->current_tier;
    uint64_t total = profile->invocation_count;
    if (UINT64_MAX - total < profile->loop_backedges) total = UINT64_MAX;
    else total += profile->loop_backedges;
    if (total >= hit_threshold && profile->current_tier < maximum_tier) {
        ++profile->current_tier;
        *promoted_tier = profile->current_tier;
    }
    return 0;
}

int pl_lockless_ring_buffer_pipeline_init(pl_lockless_ring_buffer *queue,
                                         void *storage, size_t capacity,
                                         size_t item_size) {
    if (queue == NULL || storage == NULL || capacity < 2u ||
        capacity > 65536u || item_size == 0u ||
        item_size > PL_SYSTEMS_MAX_RING_ITEM ||
        capacity > SIZE_MAX / item_size) return -1;
    queue->storage = (uint8_t *)storage;
    queue->capacity = capacity;
    queue->item_size = item_size;
    atomic_init(&queue->head, 0u);
    atomic_init(&queue->tail, 0u);
    return 0;
}

int pl_lockless_ring_buffer_push(pl_lockless_ring_buffer *queue, const void *item) {
    if (queue == NULL || item == NULL || queue->storage == NULL ||
        queue->capacity < 2u || queue->item_size == 0u) return -1;
    size_t head = atomic_load_explicit(&queue->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&queue->tail, memory_order_acquire);
    if (head - tail >= queue->capacity) return 1;
    memcpy(queue->storage + (head % queue->capacity) * queue->item_size,
           item, queue->item_size);
    atomic_store_explicit(&queue->head, head + 1u, memory_order_release);
    return 0;
}

int pl_lockless_ring_buffer_pop(pl_lockless_ring_buffer *queue, void *item) {
    if (queue == NULL || item == NULL || queue->storage == NULL ||
        queue->capacity < 2u || queue->item_size == 0u) return -1;
    size_t tail = atomic_load_explicit(&queue->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&queue->head, memory_order_acquire);
    if (tail == head) return 1;
    memcpy(item, queue->storage + (tail % queue->capacity) * queue->item_size,
           queue->item_size);
    atomic_store_explicit(&queue->tail, tail + 1u, memory_order_release);
    return 0;
}

int pl_adaptive_batch_vectorization(size_t item_count, unsigned requested_lanes,
                                    uint32_t supported_lane_mask,
                                    pl_vectorization_plan *out) {
    static const unsigned lanes[] = {16u, 8u, 4u, 2u};
    unsigned selected = 1u;
    if (out == NULL || item_count == 0u || requested_lanes == 0u ||
        requested_lanes > 16u) return -1;
    for (size_t i = 0u; i < sizeof(lanes) / sizeof(lanes[0]); ++i) {
        unsigned width = lanes[i];
        if (width <= requested_lanes && width <= item_count &&
            (supported_lane_mask & (UINT32_C(1) << width)) != 0u) {
            selected = width;
            break;
        }
    }
    out->lane_count = selected;
    out->vector_groups = item_count / selected;
    out->scalar_tail = item_count % selected;
    return 0;
}

int pl_zero_copy_arena_init(pl_zero_copy_arena *arena, void *buffer, size_t capacity) {
    if (arena == NULL || buffer == NULL || capacity == 0u) return -1;
    arena->buffer = (uint8_t *)buffer;
    arena->capacity = capacity;
    arena->used = 0u;
    return 0;
}

int pl_zero_copy_memory_arena_boost(pl_zero_copy_arena *arena, size_t bytes,
                                    size_t alignment, void **out, size_t *remaining) {
    uintptr_t base, current, aligned;
    if (arena == NULL || arena->buffer == NULL || bytes == 0u ||
        out == NULL || remaining == NULL ||
        !is_power_of_two(alignment) || alignment > PL_SYSTEMS_MAX_ARENA_ALIGNMENT ||
        arena->used > arena->capacity) return -1;
    base = (uintptr_t)arena->buffer;
    if (base > UINTPTR_MAX - arena->used) return -1;
    current = base + arena->used;
    if (current > UINTPTR_MAX - (alignment - 1u)) return -1;
    aligned = (current + (alignment - 1u)) & ~(uintptr_t)(alignment - 1u);
    size_t padding = (size_t)(aligned - current);
    if (padding > arena->capacity - arena->used ||
        bytes > arena->capacity - arena->used - padding) return -2;
    arena->used += padding + bytes;
    *out = (void *)aligned;
    *remaining = arena->capacity - arena->used;
    return 0;
}

int pl_dynamic_voltage_frequency_governor(const pl_governor_telemetry *telemetry,
                                          double latency_sla_us,
                                          double thermal_cap_c,
                                          double maximum_quota_percent,
                                          pl_virtual_governor_plan *out) {
    double quota = 50.0;
    if (telemetry == NULL || out == NULL ||
        !isfinite(telemetry->observed_latency_us) ||
        !isfinite(telemetry->temperature_c) ||
        !isfinite(telemetry->utilization_percent) ||
        !isfinite(latency_sla_us) || !isfinite(thermal_cap_c) ||
        !isfinite(maximum_quota_percent) ||
        telemetry->observed_latency_us < 0.0 ||
        telemetry->utilization_percent < 0.0 || telemetry->utilization_percent > 100.0 ||
        latency_sla_us <= 0.0 || thermal_cap_c <= 0.0 ||
        maximum_quota_percent < 1.0 || maximum_quota_percent > 100.0) return -1;
    if (telemetry->observed_latency_us > latency_sla_us) {
        double ratio = telemetry->observed_latency_us / latency_sla_us;
        quota = 50.0 + (ratio > 1.0 ? (ratio - 1.0) * 25.0 : 0.0);
    } else if (telemetry->observed_latency_us < latency_sla_us * 0.5) {
        quota = 35.0;
    }
    if (quota > maximum_quota_percent) quota = maximum_quota_percent;
    out->latency_target_missed = telemetry->observed_latency_us > latency_sla_us;
    out->thermal_limited = telemetry->temperature_c >= thermal_cap_c;
    if (out->thermal_limited) {
        double thermal_scale = telemetry->temperature_c > thermal_cap_c
            ? thermal_cap_c / telemetry->temperature_c : 0.9;
        if (thermal_scale < 0.25) thermal_scale = 0.25;
        quota *= thermal_scale;
    }
    if (quota < 1.0) quota = 1.0;
    out->recommended_quota_percent = quota;
    return 0;
}

int pl_optimize_cache_line_layout(const pl_layout_field *fields, size_t field_count,
                                  size_t cache_line_bytes,
                                  pl_layout_placement *placements,
                                  size_t placement_capacity, size_t *total_size) {
    size_t order[PL_SYSTEMS_MAX_LAYOUT_FIELDS], offset = 0u;
    if (fields == NULL || placements == NULL || total_size == NULL ||
        field_count == 0u || field_count > PL_SYSTEMS_MAX_LAYOUT_FIELDS ||
        placement_capacity < field_count || cache_line_bytes == 0u) return -1;
    for (size_t i = 0u; i < field_count; ++i) {
        if (fields[i].size == 0u || !is_power_of_two(fields[i].alignment) ||
            fields[i].alignment > PL_SYSTEMS_MAX_ARENA_ALIGNMENT) return -1;
        order[i] = i;
    }
    for (size_t i = 1u; i < field_count; ++i) {
        size_t item = order[i], j = i;
        while (j > 0u && fields[order[j - 1u]].heat_group > fields[item].heat_group) {
            order[j] = order[j - 1u];
            --j;
        }
        order[j] = item;
    }
    for (size_t i = 0u; i < field_count; ++i) {
        const pl_layout_field *field = &fields[order[i]];
        if (align_up_size(offset, field->alignment, &offset) != 0) return -2;
        if (field->size <= cache_line_bytes && (offset % cache_line_bytes) + field->size > cache_line_bytes) {
            size_t line = offset / cache_line_bytes;
            if (line == SIZE_MAX / cache_line_bytes) return -2;
            offset = (line + 1u) * cache_line_bytes;
            if (align_up_size(offset, field->alignment, &offset) != 0) return -2;
        }
        placements[i].id = field->id;
        placements[i].offset = offset;
        placements[i].size = field->size;
        if (field->size > SIZE_MAX - offset) return -2;
        offset += field->size;
    }
    *total_size = offset;
    return 0;
}

int pl_eliminate_redundant_bounds_checks(const pl_loop_range_invariant *invariant,
                                         int *bounds_proven) {
    if (invariant == NULL || bounds_proven == NULL || invariant->step <= 0 ||
        invariant->array_length == 0u ||
        invariant->first_index > invariant->last_index) return -1;
    *bounds_proven = invariant->first_index >= 0 &&
                     (uint64_t)invariant->last_index < (uint64_t)invariant->array_length;
    return 0;
}

int pl_fuse_compute_kernels(const pl_kernel_node *nodes, size_t node_count,
                            pl_kernel_fusion_plan *out) {
    if (nodes == NULL || out == NULL || node_count == 0u ||
        node_count > PL_SYSTEMS_MAX_FUSION_NODES) return -1;
    memset(out, 0, sizeof(*out));
    out->node_count = node_count;
    out->group_count = 1u;
    out->group_for_node[0] = 0u;
    for (size_t i = 0u; i < node_count; ++i) {
        if (i == 0u) continue;
        int connected = (nodes[i - 1u].output_resources & nodes[i].input_resources) != 0u;
        int merge = connected && nodes[i - 1u].fusible && nodes[i].fusible &&
                    !nodes[i - 1u].has_side_effect_barrier && !nodes[i].has_side_effect_barrier;
        if (!merge) ++out->group_count;
        out->group_for_node[i] = (unsigned)(out->group_count - 1u);
    }
    return 0;
}

int pl_tune_branch_predictor_hints(uint64_t taken_count, uint64_t not_taken_count,
                                   double confidence_threshold,
                                   pl_branch_hint *out, double *taken_probability) {
    uint64_t total;
    double probability;
    if (out == NULL || taken_probability == NULL ||
        UINT64_MAX - taken_count < not_taken_count ||
        !isfinite(confidence_threshold) || confidence_threshold < 0.5 ||
        confidence_threshold > 1.0) return -1;
    total = taken_count + not_taken_count;
    if (total == 0u) return -1;
    probability = (double)taken_count / (double)total;
    *taken_probability = probability;
    if (probability >= confidence_threshold) *out = PL_BRANCH_LIKELY;
    else if (probability <= 1.0 - confidence_threshold) *out = PL_BRANCH_UNLIKELY;
    else *out = PL_BRANCH_NEUTRAL;
    return 0;
}

int pl_defragment_heap_and_slab_pools(void *slab, size_t slab_size,
                                      pl_slab_object *objects, size_t object_count,
                                      size_t *bytes_reclaimed) {
    size_t order[PL_SYSTEMS_MAX_LAYOUT_FIELDS], cursor = 0u, live_bytes = 0u;
    if (slab == NULL || slab_size == 0u || objects == NULL ||
        bytes_reclaimed == NULL || object_count > PL_SYSTEMS_MAX_LAYOUT_FIELDS) return -1;
    for (size_t i = 0u; i < object_count; ++i) {
        if (!is_power_of_two(objects[i].alignment) ||
            objects[i].alignment > PL_SYSTEMS_MAX_ARENA_ALIGNMENT ||
            objects[i].offset > slab_size || objects[i].size > slab_size - objects[i].offset)
            return -1;
        order[i] = i;
        if (objects[i].live) {
            if (objects[i].size == 0u || live_bytes > SIZE_MAX - objects[i].size) return -1;
            live_bytes += objects[i].size;
        }
    }
    for (size_t i = 1u; i < object_count; ++i) {
        size_t item = order[i], j = i;
        while (j > 0u && objects[order[j - 1u]].offset > objects[item].offset) {
            order[j] = order[j - 1u];
            --j;
        }
        order[j] = item;
    }
    size_t previous_end = 0u;
    int have_previous = 0;
    for (size_t k = 0u; k < object_count; ++k) {
        pl_slab_object *o = &objects[order[k]];
        if (!o->live) continue;
        if (have_previous && o->offset < previous_end) return -2;
        previous_end = o->offset + o->size;
        have_previous = 1;
    }
    for (size_t k = 0u; k < object_count; ++k) {
        pl_slab_object *o = &objects[order[k]];
        if (!o->live) continue;
        size_t destination;
        if (align_up_size(cursor, o->alignment, &destination) != 0 ||
            destination > o->offset || o->size > slab_size - destination) return -2;
        if (destination != o->offset)
            memmove((uint8_t *)slab + destination, (uint8_t *)slab + o->offset, o->size);
        o->offset = destination;
        cursor = destination + o->size;
    }
    *bytes_reclaimed = slab_size >= cursor ? slab_size - cursor : 0u;
    (void)live_bytes;
    return 0;
}

int pl_accelerate_bigint_modular_ops(uint64_t left, uint64_t right,
                                     uint64_t modulus, unsigned limb_width,
                                     uint64_t *out) {
    if (out == NULL || modulus < 2u ||
        (limb_width != 8u && limb_width != 16u &&
         limb_width != 32u && limb_width != 64u)) return -1;
    *out = multiply_mod_u64(left % modulus, right % modulus, modulus);
    return 0;
}

int pl_enable_hardware_crypto_offload(pl_crypto_suite suite,
                                      const pl_crypto_device_caps *caps,
                                      pl_crypto_backend *backend) {
    if (caps == NULL || backend == NULL ||
        suite < PL_CRYPTO_SHA256 || suite > PL_CRYPTO_POLY1305 ||
        (caps->sha256 != 0 && caps->sha256 != 1) ||
        (caps->sha3 != 0 && caps->sha3 != 1) ||
        (caps->aes != 0 && caps->aes != 1) ||
        (caps->pclmul != 0 && caps->pclmul != 1) ||
        (caps->external_device != 0 && caps->external_device != 1)) return -1;
    if (caps->external_device) {
        *backend = PL_CRYPTO_BACKEND_DEVICE;
        return 0;
    }
    switch (suite) {
        case PL_CRYPTO_SHA256: *backend = caps->sha256 ? PL_CRYPTO_BACKEND_CPU : PL_CRYPTO_BACKEND_SCALAR; break;
        case PL_CRYPTO_SHA3: *backend = caps->sha3 ? PL_CRYPTO_BACKEND_CPU : PL_CRYPTO_BACKEND_SCALAR; break;
        case PL_CRYPTO_AES_GCM: *backend = (caps->aes && caps->pclmul) ? PL_CRYPTO_BACKEND_CPU : PL_CRYPTO_BACKEND_SCALAR; break;
        case PL_CRYPTO_POLY1305: *backend = caps->pclmul ? PL_CRYPTO_BACKEND_CPU : PL_CRYPTO_BACKEND_SCALAR; break;
        default: return -1;
    }
    return 0;
}

int pl_synthesize_fpga_bit_stream_overlay(const char *dsl_spec,
                                          const char *target_fabric,
                                          pl_fpga_synthesis_fn synthesize,
                                          void *context,
                                          uint8_t *bitstream,
                                          size_t bitstream_capacity,
                                          size_t *bitstream_size) {
    if (dsl_spec == NULL || dsl_spec[0] == '\0' ||
        target_fabric == NULL || target_fabric[0] == '\0' ||
        synthesize == NULL || bitstream == NULL ||
        bitstream_capacity == 0u || bitstream_size == NULL ||
        strlen(dsl_spec) > 65536u || strlen(target_fabric) > 128u) return -1;
    *bitstream_size = 0u;
    if (synthesize(dsl_spec, target_fabric, bitstream, bitstream_capacity,
                   bitstream_size, context) != 0) return -2;
    if (*bitstream_size == 0u || *bitstream_size > bitstream_capacity) {
        *bitstream_size = 0u;
        return -3;
    }
    return 0;
}

int pl_patch_microcode_simulation_table(pl_microcode_sim_table *simulator,
                                        uint8_t opcode, const uint16_t *micro_ops,
                                        size_t micro_op_count) {
    if (simulator == NULL || micro_ops == NULL || micro_op_count == 0u ||
        micro_op_count > PL_SYSTEMS_MAX_VM_MICRO_OPS) return -1;
    memcpy(simulator->micro_ops[opcode], micro_ops, micro_op_count * sizeof(uint16_t));
    simulator->count[opcode] = (uint8_t)micro_op_count;
    return 0;
}

int pl_define_custom_isa_extension(pl_isa_extension_registry *registry,
                                   const pl_isa_extension *extension) {
    uint32_t width_mask, operand_mask;
    if (registry == NULL || extension == NULL ||
        extension->name[0] == '\0' ||
        memchr(extension->name, '\0', sizeof(extension->name)) == NULL ||
        extension->instruction_bits < 8u || extension->instruction_bits > 32u ||
        extension->operand_bits == 0u ||
        extension->operand_bits >= extension->instruction_bits ||
        registry->count >= PL_SYSTEMS_MAX_ISA_EXTENSIONS) return -1;
    width_mask = extension->instruction_bits == 32u ? UINT32_MAX :
                 (UINT32_C(1) << extension->instruction_bits) - 1u;
    operand_mask = extension->operand_bits == 32u ? UINT32_MAX :
                   (UINT32_C(1) << extension->operand_bits) - 1u;
    if ((extension->opcode_mask & ~width_mask) != 0u ||
        (extension->opcode_value & ~extension->opcode_mask) != 0u ||
        (extension->opcode_mask & operand_mask) != 0u) return -1;
    for (size_t i = 0u; i < registry->count; ++i) {
        if (strcmp(registry->entries[i].name, extension->name) == 0 ||
            (((registry->entries[i].opcode_value ^ extension->opcode_value) &
              (registry->entries[i].opcode_mask & extension->opcode_mask)) == 0u))
            return -2;
    }
    registry->entries[registry->count++] = *extension;
    return 0;
}

int pl_isa_extension_encode(const pl_isa_extension *extension,
                           uint32_t operand_value, uint32_t *instruction_word) {
    uint32_t operand_mask, width_mask;
    if (extension == NULL || instruction_word == NULL ||
        extension->instruction_bits < 8u || extension->instruction_bits > 32u ||
        extension->operand_bits == 0u ||
        extension->operand_bits >= extension->instruction_bits) return -1;
    width_mask = extension->instruction_bits == 32u ? UINT32_MAX :
                 (UINT32_C(1) << extension->instruction_bits) - 1u;
    operand_mask = (UINT32_C(1) << extension->operand_bits) - 1u;
    if ((operand_value & ~operand_mask) != 0u ||
        (extension->opcode_value & ~width_mask) != 0u ||
        (extension->opcode_value & ~extension->opcode_mask) != 0u ||
        (extension->opcode_mask & operand_mask) != 0u) return -1;
    *instruction_word = (extension->opcode_value | operand_value) & width_mask;
    return 0;
}

int pl_program_pmu_hardware_counters(const pl_pmu_sample *samples,
                                     size_t sample_count, pl_pmu_report *out) {
    if (samples == NULL || out == NULL || sample_count == 0u ||
        sample_count > PL_SYSTEMS_MAX_TRACE_ITEMS) return -1;
    memset(out, 0, sizeof(*out));
    for (size_t i = 0u; i < sample_count; ++i) {
        if (UINT64_MAX - out->total_cycles < samples[i].cycles ||
            UINT64_MAX - out->total_instructions < samples[i].instructions)
            return -2;
        out->total_cycles += samples[i].cycles;
        out->total_instructions += samples[i].instructions;
        if (samples[i].cache_misses > samples[i].cache_references ||
            samples[i].branch_misses > samples[i].branches) return -1;
    }
    uint64_t cache_refs = 0u, cache_misses = 0u, branches = 0u, branch_misses = 0u;
    for (size_t i = 0u; i < sample_count; ++i) {
        if (UINT64_MAX - cache_refs < samples[i].cache_references ||
            UINT64_MAX - cache_misses < samples[i].cache_misses ||
            UINT64_MAX - branches < samples[i].branches ||
            UINT64_MAX - branch_misses < samples[i].branch_misses) return -2;
        cache_refs += samples[i].cache_references;
        cache_misses += samples[i].cache_misses;
        branches += samples[i].branches;
        branch_misses += samples[i].branch_misses;
    }
    out->sample_count = sample_count;
    out->ipc = out->total_cycles == 0u ? 0.0 :
               (double)out->total_instructions / (double)out->total_cycles;
    out->cache_miss_rate = cache_refs == 0u ? 0.0 :
                           (double)cache_misses / (double)cache_refs;
    out->branch_miss_rate = branches == 0u ? 0.0 :
                            (double)branch_misses / (double)branches;
    return 0;
}

int pl_configure_cache_way_partitioning(unsigned available_ways,
                                        unsigned requested_ways,
                                        pl_cat_partition_plan *out) {
    if (out == NULL || available_ways == 0u || available_ways > 64u ||
        requested_ways == 0u || requested_ways > available_ways) return -1;
    out->available_ways = available_ways;
    out->selected_ways = requested_ways;
    out->way_mask = requested_ways == 64u ? UINT64_MAX :
                    (UINT64_C(1) << requested_ways) - 1u;
    return 0;
}

int pl_inject_fault_and_latency_model(const pl_fault_latency_profile *profile,
                                      uint64_t seed, pl_fault_latency_result *out) {
    if (profile == NULL || out == NULL ||
        profile->fault_probability_per_million > 1000000u ||
        profile->data_bit_count > 64u ||
        (profile->fault_probability_per_million != 0u &&
         profile->data_bit_count == 0u)) return -1;
    if (seed == 0u) seed = UINT64_C(0x9e3779b97f4a7c15);
    uint64_t random = xorshift64(&seed);
    out->latency_ns = profile->maximum_latency_ns == UINT64_MAX
        ? random : random % (profile->maximum_latency_ns + 1u);
    uint64_t decision = xorshift64(&seed) % UINT64_C(1000000);
    out->fault_injected = decision < profile->fault_probability_per_million;
    out->bit_index = out->fault_injected
        ? (unsigned)(xorshift64(&seed) % profile->data_bit_count) : 0u;
    out->next_seed = seed;
    return 0;
}

int pl_verify_and_seal_firmware_overlay(const uint8_t *image, size_t image_size,
                                         uint32_t image_version,
                                         const uint8_t expected_digest[32],
                                         const uint8_t *signature, size_t signature_size,
                                         pl_system_hash32_fn hash32,
                                         pl_overlay_signature_verify_fn verify_signature,
                                         void *context, const pl_overlay_policy *policy,
                                         pl_overlay_seal_report *out) {
    if (image == NULL || image_size == 0u || expected_digest == NULL ||
        hash32 == NULL || policy == NULL || out == NULL ||
        policy->minimum_version > policy->maximum_version ||
        (policy->signature_required != 0 && policy->signature_required != 1) ||
        (policy->rollback_allowed != 0 && policy->rollback_allowed != 1) ||
        (signature_size != 0u && signature == NULL)) return -1;
    memset(out, 0, sizeof(*out));
    if (hash32(image, image_size, out->digest, context) != 0) return -2;
    out->digest_matches = memcmp(out->digest, expected_digest, 32u) == 0;
    out->rollback_allowed = policy->rollback_allowed;
    if (signature_size != 0u && verify_signature != NULL)
        out->signature_valid = verify_signature(image, image_size, signature,
                                                 signature_size, context) == 0;
    else if (!policy->signature_required && signature_size == 0u)
        out->signature_valid = 1;
    out->sealed = out->digest_matches && out->signature_valid &&
                  image_version >= policy->minimum_version &&
                  image_version <= policy->maximum_version;
    return out->sealed ? 0 : -3;
}
