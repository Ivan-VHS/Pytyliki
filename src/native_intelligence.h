#ifndef PYTYLIKI_NATIVE_INTELLIGENCE_H
#define PYTYLIKI_NATIVE_INTELLIGENCE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_NI_ABI_VERSION UINT32_C(0x00010000)
#define PL_NI_MAX_CPUS 256u
#define PL_NI_MAX_CACHES 1024u
#define PL_NI_MAX_NUMA_NODES 16u
#define PL_NI_CPU_WORDS (PL_NI_MAX_CPUS / 64u)

typedef enum {
    PL_NI_OK = 0, PL_NI_INVALID_ARGUMENT, PL_NI_UNSUPPORTED,
    PL_NI_IO_ERROR, PL_NI_CAPACITY_EXCEEDED, PL_NI_INCONSISTENT,
    PL_NI_STALE_GENERATION, PL_NI_OUT_OF_RANGE, PL_NI_ACCESS_DENIED,
    PL_NI_INVALID_STATE
} pl_ni_status;

typedef enum {
    PL_NI_FACT_KNOWN = 1, PL_NI_FACT_DERIVED, PL_NI_FACT_UNAVAILABLE,
    PL_NI_FACT_UNSUPPORTED, PL_NI_FACT_CONFLICTING
} pl_ni_fact_state;

typedef enum {
    PL_NI_ISA_SSE2 = UINT64_C(1) << 0, PL_NI_ISA_SSE42 = UINT64_C(1) << 1,
    PL_NI_ISA_AES = UINT64_C(1) << 2, PL_NI_ISA_AVX = UINT64_C(1) << 3,
    PL_NI_ISA_AVX2 = UINT64_C(1) << 4, PL_NI_ISA_FMA = UINT64_C(1) << 5,
    PL_NI_ISA_AVX512F = UINT64_C(1) << 6,
    PL_NI_ISA_NEON = UINT64_C(1) << 7,
    PL_NI_ISA_SVE = UINT64_C(1) << 8,
    PL_NI_ISA_SVE2 = UINT64_C(1) << 9,
    PL_NI_ISA_ARM_AES = UINT64_C(1) << 10,
    PL_NI_ISA_ARM_SHA1 = UINT64_C(1) << 11,
    PL_NI_ISA_ARM_SHA2 = UINT64_C(1) << 12,
    PL_NI_ISA_ARM_CRC32 = UINT64_C(1) << 13,
    PL_NI_ISA_ARM_ATOMICS = UINT64_C(1) << 14
} pl_ni_isa;

enum {
    PL_NI_ISA_SCOPE_UNKNOWN = 0,
    PL_NI_ISA_SCOPE_EXECUTING_CPU = 1,
    PL_NI_ISA_SCOPE_SYSTEM_COMMON = 2,
    PL_NI_ISA_SCOPE_PER_LOGICAL_CPU = 3
};

typedef struct {
    char platform[16], topology_source[48], allowed_source[48];
    char isa_source[48], memory_source[48];
    uint32_t schema_version, topology_state, allowed_state;
    uint32_t isa_state, isa_scope, memory_state;
    uint64_t snapshot_fingerprint, generation;
} pl_ni_provenance;

typedef struct {
    uint16_t group_id, os_cpu_id, package_id, die_id, core_id, numa_node;
    uint16_t thread_index, reserved;
    uint32_t flags;
    uint64_t isa_mask;
} pl_ni_cpu;

enum {
    PL_NI_CPU_ONLINE = UINT32_C(1) << 0,
    PL_NI_CPU_ALLOWED = UINT32_C(1) << 1,
    PL_NI_CPU_ALLOWED_KNOWN = UINT32_C(1) << 2
};

typedef struct {
    uint8_t level, kind;
    uint16_t group_id;
    uint32_t size_bytes, line_bytes, associativity, sharing_count;
    uint64_t sharing_mask[PL_NI_CPU_WORDS];
} pl_ni_cache;

typedef struct {
    uint16_t id, reserved;
    uint32_t cpu_count;
    uint64_t memory_bytes;
    uint64_t cpu_mask[PL_NI_CPU_WORDS];
} pl_ni_numa_node;

typedef struct {
    uint32_t abi_version, cpu_count, cache_count, numa_count;
    uint64_t total_memory_bytes, available_memory_bytes, isa_mask;
    pl_ni_provenance provenance;
    pl_ni_cpu cpus[PL_NI_MAX_CPUS];
    pl_ni_cache caches[PL_NI_MAX_CACHES];
    pl_ni_numa_node numa[PL_NI_MAX_NUMA_NODES];
    uint16_t numa_distance[PL_NI_MAX_NUMA_NODES][PL_NI_MAX_NUMA_NODES];
} pl_ni_snapshot;

typedef struct {
    uint16_t preferred_numa_node, current_cpu_index; /* UINT16_MAX means unknown */
    uint64_t required_isa;
    uint64_t occupied_cpu_mask[PL_NI_CPU_WORDS];
    uint32_t require_online, require_allowed, require_known_allowed, exclusive_core;
    uint32_t weights[5];
} pl_ppf_request;

typedef struct {
    uint32_t cache_pressure, bandwidth_pressure, migration_cost, valid_mask;
} pl_ppf_telemetry;

enum {
    PL_PPF_TELEMETRY_CACHE = UINT32_C(1) << 0,
    PL_PPF_TELEMETRY_BANDWIDTH = UINT32_C(1) << 1,
    PL_PPF_TELEMETRY_MIGRATION = UINT32_C(1) << 2
};

typedef struct {
    uint32_t cpu_index, accepted, numa_cost, cache_cost, smt_cost;
    uint32_t bandwidth_cost, migration_cost;
    uint64_t total_cost;
    uint32_t evidence_flags;
    pl_ni_status rejection;
} pl_ppf_candidate;

typedef struct {
    uint32_t schema_version, candidate_count, selected_cpu_index, has_selection;
    uint64_t topology_generation, topology_fingerprint, policy_fingerprint;
    uint64_t telemetry_fingerprint;
    uint32_t weights[5];
    pl_ppf_candidate candidates[PL_NI_MAX_CPUS];
} pl_ppf_report;

enum {
    PL_PPF_EVIDENCE_TOPOLOGY = UINT32_C(1) << 0,
    PL_PPF_EVIDENCE_TELEMETRY = UINT32_C(1) << 1,
    PL_PPF_EVIDENCE_ESTIMATED = UINT32_C(1) << 2,
    PL_PPF_EVIDENCE_MISSING_TELEMETRY = UINT32_C(1) << 3,
    PL_PPF_EVIDENCE_MISSING_TOPOLOGY = UINT32_C(1) << 4
};

typedef enum {
    PL_NATIVE_SPACE_VIRTUAL = 1, PL_NATIVE_SPACE_PHYSICAL = 2,
    PL_NATIVE_SPACE_BUS = 3, PL_NATIVE_SPACE_MMIO = 4
} pl_native_address_space;

typedef enum {
    PL_NATIVE_ACCESS_READ = UINT32_C(1) << 0,
    PL_NATIVE_ACCESS_WRITE = UINT32_C(1) << 1,
    PL_NATIVE_ACCESS_W1C = UINT32_C(1) << 2,
    PL_NATIVE_ACCESS_W1S = UINT32_C(1) << 3
} pl_native_access;

typedef struct {
    uint64_t base, size, allowed_value_mask, generation;
    uint32_t address_bits, width_bits, alignment, access, address_space;
} pl_native_register;

typedef struct {
    uint64_t base, length;
    uint32_t address_bits, alignment;
} pl_native_range;

typedef enum {
    PL_DMA_CPU_OWNED = 1, PL_DMA_DEVICE_OWNED = 2, PL_DMA_COMPLETED = 3,
    PL_DMA_UNMAPPED = 4, PL_DMA_FAULTED = 5
} pl_dma_state;

typedef struct {
    uint64_t resource_id, generation, base, length;
    uint32_t state, direction;
} pl_dma_resource;

const char *pl_ni_status_string(pl_ni_status status);
pl_ni_status pl_ni_collect(pl_ni_snapshot *out);
pl_ni_status pl_ni_snapshot_validate(const pl_ni_snapshot *snapshot);
uint64_t pl_ni_snapshot_fingerprint(const pl_ni_snapshot *snapshot);
pl_ni_status pl_ppf_place(const pl_ni_snapshot *snapshot,
                          const pl_ppf_request *request,
                          const pl_ppf_telemetry *telemetry,
                          size_t telemetry_count, pl_ppf_report *out);
pl_ni_status pl_native_range_validate(const pl_native_range *range);
pl_ni_status pl_native_register_validate(const pl_native_register *reg,
                                          uint64_t offset, uint32_t is_write,
                                          uint64_t value, uint64_t expected_generation,
                                          uint64_t *absolute_address);
pl_ni_status pl_dma_transition(pl_dma_resource *resource,
                               uint64_t expected_generation,
                               pl_dma_state next_state);
pl_ni_status pl_dma_cpu_access_validate(const pl_dma_resource *resource,
                                        uint64_t expected_generation);

#ifdef __cplusplus
}
#endif
#endif
