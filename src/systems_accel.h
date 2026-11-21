#ifndef PYTYLIKI_SYSTEMS_ACCEL_H
#define PYTYLIKI_SYSTEMS_ACCEL_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SYSTEMS_ACCEL_ABI_VERSION 1u
#define PL_SYSTEMS_FUNCTION_COUNT 33u
#define PL_SYSTEMS_MAX_PROOF_DEPTH 64u
#define PL_SYSTEMS_MAX_PROOF_BYTES 1024u
#define PL_SYSTEMS_MAX_SELECTORS 256u
#define PL_SYSTEMS_MAX_STORAGE_SLOTS 128u
#define PL_SYSTEMS_MAX_TRACE_ITEMS 4096u
#define PL_SYSTEMS_MAX_TX_ITEMS 256u
#define PL_SYSTEMS_MAX_VM_MICRO_OPS 32u
#define PL_SYSTEMS_MAX_ISA_EXTENSIONS 32u
#define PL_SYSTEMS_MAX_LAYOUT_FIELDS 256u
#define PL_SYSTEMS_MAX_FUSION_NODES 256u
#define PL_SYSTEMS_MAX_ARENA_ALIGNMENT 4096u

typedef enum {
    PL_SYSTEMS_MODE_NATIVE = 0,
    PL_SYSTEMS_MODE_SIMULATED = 1,
    PL_SYSTEMS_MODE_ADAPTER_REQUIRED = 2,
    PL_SYSTEMS_MODE_PLANNING = 3
} pl_systems_mode;

typedef struct {
    unsigned id;
    const char *name;
    pl_systems_mode mode;
    const char *description;
} pl_systems_function_info;

size_t pl_systems_lab_catalog(const pl_systems_function_info **items);
const char *pl_systems_mode_name(pl_systems_mode mode);

typedef int (*pl_system_hash32_fn)(const uint8_t *data, size_t size,
                                   uint8_t digest[32], void *context);

/* 1. Verify a Merkle inclusion path. leaf_hash and every sibling are 32 bytes. */
int pl_unlock_light_client_state_proofs(const uint8_t leaf_hash[32],
                                        const uint8_t *siblings,
                                        size_t sibling_count,
                                        uint64_t leaf_index,
                                        const uint8_t expected_root[32],
                                        pl_system_hash32_fn hash32,
                                        void *hash_context);

/* 2. Provider-compliant token-bucket scheduling across a fixed RPC endpoint pool. */
#define PL_RPC_MAX_ENDPOINTS 16u
typedef struct {
    size_t endpoint_count;
    uint64_t last_refill_ns[PL_RPC_MAX_ENDPOINTS];
    double tokens[PL_RPC_MAX_ENDPOINTS];
    double tokens_per_second;
    double burst_capacity;
    size_t next_endpoint;
} pl_rpc_governor;
int pl_rpc_governor_init(pl_rpc_governor *governor, size_t endpoint_count,
                         double requests_per_second, double burst_capacity);
int pl_bypass_rpc_rate_throttling(pl_rpc_governor *governor,
                                  uint64_t now_ns, size_t request_count,
                                  size_t *selected_endpoint);

/* 3. Execute one transaction against a caller-owned fork; base_state is never changed. */
typedef int (*pl_mempool_execute_fn)(const uint8_t *transaction, size_t tx_size,
                                     uint8_t *fork_state, size_t state_size,
                                     uint64_t *gas_used, void *context);
int pl_unlock_mempool_simulation_hooks(const uint8_t *base_state,
                                       uint8_t *fork_state, size_t state_size,
                                       const uint8_t *transaction, size_t tx_size,
                                       pl_mempool_execute_fn execute,
                                       void *context, uint64_t *gas_used);

/* 4. Recover EVM PUSH4 selectors, safely skipping PUSH immediate bytes. */
int pl_extract_abi_and_bytecode_signatures(const uint8_t *bytecode,
                                            size_t bytecode_size,
                                            uint32_t *selectors,
                                            size_t selector_capacity,
                                            size_t *selector_count);

/* 5. Verify finality and serialize a relay envelope; this does not submit a network transaction. */
typedef int (*pl_relay_proof_verify_fn)(uint64_t source_chain_id,
                                        uint64_t block_number,
                                        const uint8_t *proof, size_t proof_size,
                                        void *context);
typedef struct {
    uint64_t source_chain_id;
    uint64_t target_chain_id;
    uint64_t source_block;
    uint64_t confirmations;
    uint8_t event_hash[32];
    size_t proof_size;
    uint8_t proof[PL_SYSTEMS_MAX_PROOF_BYTES];
} pl_cross_chain_relay_packet;
int pl_unlock_cross_chain_message_relay(uint64_t source_chain_id,
                                        uint64_t target_chain_id,
                                        uint64_t source_block,
                                        uint64_t finalized_block,
                                        uint64_t minimum_confirmations,
                                        const uint8_t event_hash[32],
                                        const uint8_t *proof, size_t proof_size,
                                        pl_relay_proof_verify_fn verify,
                                        void *verify_context,
                                        pl_cross_chain_relay_packet *out);

/* 6. Reconstruct slot access summaries from an execution trace (heuristic only). */
typedef enum { PL_STORAGE_SLOAD = 0, PL_STORAGE_SSTORE = 1 } pl_storage_opcode;
typedef struct {
    uint64_t slot;
    uint64_t key_fingerprint;
    pl_storage_opcode opcode;
} pl_storage_access;
typedef enum {
    PL_STORAGE_LAYOUT_UNKNOWN = 0,
    PL_STORAGE_LAYOUT_SCALAR = 1,
    PL_STORAGE_LAYOUT_MAPPING_CANDIDATE = 2,
    PL_STORAGE_LAYOUT_ARRAY_CANDIDATE = 3
} pl_storage_layout_guess;
typedef struct {
    uint64_t slot;
    size_t reads;
    size_t writes;
    size_t distinct_key_fingerprints;
    pl_storage_layout_guess guess;
} pl_storage_slot_summary;
typedef struct {
    size_t slot_count;
    pl_storage_slot_summary slots[PL_SYSTEMS_MAX_STORAGE_SLOTS];
} pl_storage_layout_report;
int pl_deobfuscate_storage_layout_slots(const pl_storage_access *accesses,
                                        size_t access_count,
                                        pl_storage_layout_report *out);

/* 7. Stable replay fingerprint over explicit event fields and the supplied initial state. */
typedef struct {
    uint64_t block_number;
    uint64_t timestamp;
    uint64_t oracle_value;
    uint8_t transaction_hash[32];
} pl_replay_event;
int pl_unlock_deterministic_replay_snapshot(uint64_t first_block,
                                            uint64_t last_block,
                                            uint64_t seed,
                                            const uint8_t *initial_state,
                                            size_t state_size,
                                            const pl_replay_event *events,
                                            size_t event_count,
                                            pl_system_hash32_fn hash32,
                                            void *hash_context,
                                            uint8_t fingerprint[32]);

/* 8. ZK aggregation is delegated to a caller-supplied, protocol-specific prover. */
typedef int (*pl_zk_aggregate_fn)(const uint8_t *proofs, size_t proof_count,
                                  size_t proof_size, const uint8_t *verifier_params,
                                  size_t params_size, uint8_t *aggregate,
                                  size_t aggregate_capacity, size_t *aggregate_size,
                                  void *context);
int pl_enable_offchain_zk_proof_aggregation(const uint8_t *proofs,
                                            size_t proof_count, size_t proof_size,
                                            const uint8_t *verifier_params,
                                            size_t params_size,
                                            pl_zk_aggregate_fn aggregate_fn,
                                            void *context,
                                            uint8_t *aggregate,
                                            size_t aggregate_capacity,
                                            size_t *aggregate_size);

/* 9. Transaction bundle callbacks define the caller's atomic transaction boundary. */
typedef struct {
    const uint8_t *data;
    size_t size;
    uint64_t estimated_gas;
} pl_bundle_transaction;
typedef struct {
    int (*begin)(void *context);
    int (*apply)(const pl_bundle_transaction *transaction, size_t index, void *context);
    int (*commit)(void *context);
    void (*rollback)(void *context);
    void *context;
} pl_bundle_executor;
int pl_unlock_batch_transaction_bundling(const pl_bundle_transaction *transactions,
                                         size_t transaction_count,
                                         uint64_t gas_limit,
                                         const pl_bundle_executor *executor,
                                         uint64_t *estimated_gas);

/* 10. Summarize an already collected VM call trace, including bounded revert text. */
typedef enum {
    PL_TRACE_CALL = 0, PL_TRACE_DELEGATECALL = 1,
    PL_TRACE_STATICCALL = 2, PL_TRACE_CREATE2 = 3
} pl_trace_call_kind;
typedef struct {
    pl_trace_call_kind kind;
    unsigned depth;
    uint64_t gas_used;
    int reverted;
    const char *revert_reason;
} pl_call_trace_item;
typedef struct {
    size_t call_count;
    size_t revert_count;
    unsigned maximum_depth;
    uint64_t total_gas;
    char first_revert_reason[128];
} pl_call_trace_summary;
int pl_inspect_call_trace_and_revert_reasons(const pl_call_trace_item *trace,
                                             size_t trace_count,
                                             size_t maximum_depth,
                                             pl_call_trace_summary *out);

/* 11. Custom opcode hooks are scoped to an emulator instance, never physical hardware. */
typedef int (*pl_vm_opcode_handler)(uint8_t opcode, uint64_t *registers,
                                    size_t register_count, void *context);
typedef struct {
    pl_vm_opcode_handler handlers[256];
    void *contexts[256];
} pl_vm_hook_table;
void pl_vm_hook_table_init(pl_vm_hook_table *table);
int pl_unlock_custom_vm_opcode_hook(pl_vm_hook_table *table, uint8_t opcode,
                                    pl_vm_opcode_handler handler, void *context);
int pl_vm_dispatch_opcode(pl_vm_hook_table *table, uint8_t opcode,
                          uint64_t *registers, size_t register_count);

/* 12. Virtual clock/quota plan. No host clock, voltage or frequency is changed. */
typedef struct {
    double requested_ghz;
    double effective_virtual_ghz;
    double cpu_quota_percent;
    int thermally_limited;
} pl_virtual_boost_plan;
int pl_virtual_boost_clock_scaling(unsigned vcpu_id, double target_ghz,
                                   double thermal_cap_ghz,
                                   pl_virtual_boost_plan *out);

/* 13. Calculate an affinity mask; applying it is left to an explicitly authorized OS adapter. */
typedef struct {
    uint64_t selected_core_mask;
    unsigned numa_node;
    unsigned vcpu_group;
} pl_turbo_pinning_plan;
int pl_turbo_scheduler_pinning(uint64_t requested_core_mask,
                               uint64_t available_core_mask,
                               unsigned numa_node, unsigned vcpu_group,
                               pl_turbo_pinning_plan *out);

/* 14. Produce deduplicated prefetch hints from a profile, without touching hardware caches. */
typedef struct { uint64_t instruction_address; uint64_t target_address; int taken; } pl_branch_history_item;
int pl_speculative_trace_prewarming(const pl_branch_history_item *history,
                                    size_t history_count, size_t prefetch_window,
                                    uint64_t *hints, size_t hint_capacity,
                                    size_t *hint_count);

/* 15. Profile-guided tier-promotion decision for an external JIT. */
typedef struct { uint64_t invocation_count; uint64_t loop_backedges; unsigned current_tier; } pl_jit_profile;
int pl_jit_hot_loop_tier_promotion(pl_jit_profile *profile,
                                   uint64_t hit_threshold, unsigned maximum_tier,
                                   unsigned *promoted_tier);

/* 16. Single-producer/single-consumer, bounded C11 atomic ring buffer. */
typedef struct {
    uint8_t *storage;
    size_t capacity;
    size_t item_size;
    atomic_size_t head;
    atomic_size_t tail;
} pl_lockless_ring_buffer;
int pl_lockless_ring_buffer_pipeline_init(pl_lockless_ring_buffer *queue,
                                         void *storage, size_t capacity,
                                         size_t item_size);
int pl_lockless_ring_buffer_push(pl_lockless_ring_buffer *queue, const void *item);
int pl_lockless_ring_buffer_pop(pl_lockless_ring_buffer *queue, void *item);

/* 17. Select a vector-width plan; execution backends must retain scalar fallback. */
typedef struct { unsigned lane_count; size_t vector_groups; size_t scalar_tail; } pl_vectorization_plan;
int pl_adaptive_batch_vectorization(size_t item_count, unsigned requested_lanes,
                                    uint32_t supported_lane_mask,
                                    pl_vectorization_plan *out);

/* 18. Caller-owned aligned arena for zero-copy stages. */
typedef struct { uint8_t *buffer; size_t capacity; size_t used; } pl_zero_copy_arena;
int pl_zero_copy_arena_init(pl_zero_copy_arena *arena, void *buffer, size_t capacity);
int pl_zero_copy_memory_arena_boost(pl_zero_copy_arena *arena, size_t bytes,
                                    size_t alignment, void **out, size_t *remaining);

/* 19. Latency/thermal governor returns a recommendation, not a DVFS write. */
typedef struct {
    double observed_latency_us;
    double temperature_c;
    double utilization_percent;
} pl_governor_telemetry;
typedef struct {
    double recommended_quota_percent;
    int latency_target_missed;
    int thermal_limited;
} pl_virtual_governor_plan;
int pl_dynamic_voltage_frequency_governor(const pl_governor_telemetry *telemetry,
                                          double latency_sla_us,
                                          double thermal_cap_c,
                                          double maximum_quota_percent,
                                          pl_virtual_governor_plan *out);

/* 20. Cache-line placement planner. heat_group 0 is hottest; alignment is a power of two. */
typedef struct { unsigned id; size_t size; size_t alignment; unsigned heat_group; } pl_layout_field;
typedef struct { unsigned id; size_t offset; size_t size; } pl_layout_placement;
int pl_optimize_cache_line_layout(const pl_layout_field *fields, size_t field_count,
                                  size_t cache_line_bytes,
                                  pl_layout_placement *placements,
                                  size_t placement_capacity, size_t *total_size);

/* 21. Prove array bounds for a simple monotonic integer loop. */
typedef struct {
    int64_t first_index;
    int64_t last_index;
    int64_t step;
    size_t array_length;
} pl_loop_range_invariant;
int pl_eliminate_redundant_bounds_checks(const pl_loop_range_invariant *invariant,
                                         int *bounds_proven);

/* 22. Build fusion groups from an ordered kernel dependency chain. */
typedef struct {
    unsigned id;
    uint64_t input_resources;
    uint64_t output_resources;
    int fusible;
    int has_side_effect_barrier;
} pl_kernel_node;
typedef struct { size_t group_count; size_t node_count; unsigned group_for_node[PL_SYSTEMS_MAX_FUSION_NODES]; } pl_kernel_fusion_plan;
int pl_fuse_compute_kernels(const pl_kernel_node *nodes, size_t node_count,
                            pl_kernel_fusion_plan *out);

/* 23. Profile-guided branch hint from observed branch counters. */
typedef enum { PL_BRANCH_NEUTRAL = 0, PL_BRANCH_LIKELY = 1, PL_BRANCH_UNLIKELY = 2 } pl_branch_hint;
int pl_tune_branch_predictor_hints(uint64_t taken_count, uint64_t not_taken_count,
                                   double confidence_threshold,
                                   pl_branch_hint *out, double *taken_probability);

/* 24. Compact live objects in a caller-owned slab and update offsets. */
typedef struct { size_t offset; size_t size; size_t alignment; int live; } pl_slab_object;
int pl_defragment_heap_and_slab_pools(void *slab, size_t slab_size,
                                      pl_slab_object *objects, size_t object_count,
                                      size_t *bytes_reclaimed);

/* 25. Bounded modular multiply for 64-bit limbs (portable scalar fallback). */
int pl_accelerate_bigint_modular_ops(uint64_t left, uint64_t right,
                                     uint64_t modulus, unsigned limb_width,
                                     uint64_t *out);

/* 26. Select a crypto backend from declared device/CPU capabilities. */
typedef enum {
    PL_CRYPTO_SHA256 = 0, PL_CRYPTO_SHA3 = 1,
    PL_CRYPTO_AES_GCM = 2, PL_CRYPTO_POLY1305 = 3
} pl_crypto_suite;
typedef enum { PL_CRYPTO_BACKEND_SCALAR = 0, PL_CRYPTO_BACKEND_CPU = 1, PL_CRYPTO_BACKEND_DEVICE = 2 } pl_crypto_backend;
typedef struct { int sha256; int sha3; int aes; int pclmul; int external_device; } pl_crypto_device_caps;
int pl_enable_hardware_crypto_offload(pl_crypto_suite suite,
                                      const pl_crypto_device_caps *caps,
                                      pl_crypto_backend *backend);

/* 27. Invoke a vendor synthesis adapter; no bitstream is fabricated by the core. */
typedef int (*pl_fpga_synthesis_fn)(const char *dsl_spec, const char *target_fabric,
                                    uint8_t *bitstream, size_t bitstream_capacity,
                                    size_t *bitstream_size, void *context);
int pl_synthesize_fpga_bit_stream_overlay(const char *dsl_spec,
                                          const char *target_fabric,
                                          pl_fpga_synthesis_fn synthesize,
                                          void *context,
                                          uint8_t *bitstream,
                                          size_t bitstream_capacity,
                                          size_t *bitstream_size);

/* 28. Patch a micro-op lookup table in a CPU simulator instance. */
typedef struct {
    uint8_t count[256];
    uint16_t micro_ops[256][PL_SYSTEMS_MAX_VM_MICRO_OPS];
} pl_microcode_sim_table;
int pl_patch_microcode_simulation_table(pl_microcode_sim_table *simulator,
                                        uint8_t opcode, const uint16_t *micro_ops,
                                        size_t micro_op_count);

/* 29. Register and encode a simulator/compiler-facing custom ISA extension. */
typedef struct {
    char name[32];
    uint32_t opcode_mask;
    uint32_t opcode_value;
    unsigned instruction_bits;
    unsigned operand_bits;
} pl_isa_extension;
typedef struct { size_t count; pl_isa_extension entries[PL_SYSTEMS_MAX_ISA_EXTENSIONS]; } pl_isa_extension_registry;
int pl_define_custom_isa_extension(pl_isa_extension_registry *registry,
                                   const pl_isa_extension *extension);
int pl_isa_extension_encode(const pl_isa_extension *extension,
                           uint32_t operand_value, uint32_t *instruction_word);

/* 30. Aggregate caller-collected PMU samples; this function never programs counters. */
typedef struct {
    uint64_t cycles;
    uint64_t instructions;
    uint64_t cache_references;
    uint64_t cache_misses;
    uint64_t branches;
    uint64_t branch_misses;
} pl_pmu_sample;
typedef struct {
    size_t sample_count;
    uint64_t total_cycles;
    uint64_t total_instructions;
    double ipc;
    double cache_miss_rate;
    double branch_miss_rate;
} pl_pmu_report;
int pl_program_pmu_hardware_counters(const pl_pmu_sample *samples,
                                     size_t sample_count, pl_pmu_report *out);

/* 31. Plan a CAT way mask; no MSR or kernel control is performed. */
typedef struct { uint64_t way_mask; unsigned selected_ways; unsigned available_ways; } pl_cat_partition_plan;
int pl_configure_cache_way_partitioning(unsigned available_ways,
                                        unsigned requested_ways,
                                        pl_cat_partition_plan *out);

/* 32. Deterministic fault/latency injection model for virtual buses and tests. */
typedef struct {
    uint64_t maximum_latency_ns;
    uint32_t fault_probability_per_million;
    unsigned data_bit_count;
} pl_fault_latency_profile;
typedef struct { uint64_t latency_ns; int fault_injected; unsigned bit_index; uint64_t next_seed; } pl_fault_latency_result;
int pl_inject_fault_and_latency_model(const pl_fault_latency_profile *profile,
                                      uint64_t seed, pl_fault_latency_result *out);

/* 33. Verify an overlay digest and signature before marking a local artifact sealed. */
typedef int (*pl_overlay_signature_verify_fn)(const uint8_t *image, size_t image_size,
                                              const uint8_t *signature, size_t signature_size,
                                              void *context);
typedef struct {
    uint32_t minimum_version;
    uint32_t maximum_version;
    int signature_required;
    int rollback_allowed;
} pl_overlay_policy;
typedef struct {
    uint8_t digest[32];
    int digest_matches;
    int signature_valid;
    int sealed;
    int rollback_allowed;
} pl_overlay_seal_report;
int pl_verify_and_seal_firmware_overlay(const uint8_t *image, size_t image_size,
                                         uint32_t image_version,
                                         const uint8_t expected_digest[32],
                                         const uint8_t *signature, size_t signature_size,
                                         pl_system_hash32_fn hash32,
                                         pl_overlay_signature_verify_fn verify_signature,
                                         void *context, const pl_overlay_policy *policy,
                                         pl_overlay_seal_report *out);

#ifdef __cplusplus
}
#endif
#endif
