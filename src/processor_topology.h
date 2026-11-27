#ifndef PYTYLIKI_PROCESSOR_TOPOLOGY_H
#define PYTYLIKI_PROCESSOR_TOPOLOGY_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_TOPOLOGY_MAX_LOGICAL_CPUS 256u
#define PL_TOPOLOGY_MAX_CACHES 128u
#define PL_TOPOLOGY_MAX_NUMA_NODES 32u
#define PL_TOPOLOGY_MASK_WORDS (PL_TOPOLOGY_MAX_LOGICAL_CPUS / 64u)
#define PL_TOPOLOGY_UNKNOWN UINT32_MAX
#define PL_TOPOLOGY_SCHEMA "pytyliki.processor-topology-evidence.v2"

typedef enum {
    PL_TOPOLOGY_EVIDENCE_UNKNOWN = 0,
    PL_TOPOLOGY_EVIDENCE_PARTIAL,
    PL_TOPOLOGY_EVIDENCE_OBSERVED
} pl_topology_evidence;

typedef enum {
    PL_TOPOLOGY_COMPLETE = 0,
    PL_TOPOLOGY_PARTIAL,
    PL_TOPOLOGY_UNKNOWN_STATE
} pl_topology_completeness;

typedef enum {
    PL_TOPOLOGY_CACHE_DATA = 0,
    PL_TOPOLOGY_CACHE_INSTRUCTION,
    PL_TOPOLOGY_CACHE_UNIFIED,
    PL_TOPOLOGY_CACHE_UNKNOWN
} pl_topology_cache_kind;

typedef enum {
    PL_TOPOLOGY_ISA_SSE2 = 0,
    PL_TOPOLOGY_ISA_SSE42,
    PL_TOPOLOGY_ISA_AES,
    PL_TOPOLOGY_ISA_AVX,
    PL_TOPOLOGY_ISA_AVX2,
    PL_TOPOLOGY_ISA_AVX512F,
    PL_TOPOLOGY_ISA_FMA,
    PL_TOPOLOGY_ISA_VIRTUALIZATION,
    PL_TOPOLOGY_ISA_FEATURE_COUNT
} pl_topology_isa_feature;

typedef struct {
    uint32_t logical_id;
    uint32_t os_id;
    uint32_t group_id;
    uint32_t package_id;
    uint32_t die_id;
    uint32_t core_id;
    uint32_t numa_node_id;
    uint8_t online;
    pl_topology_evidence evidence;
} pl_topology_logical_cpu;

typedef struct {
    uint32_t cache_id;
    uint32_t level;
    pl_topology_cache_kind kind;
    uint64_t size_bytes;
    uint32_t line_bytes;
    uint64_t sharing_mask[PL_TOPOLOGY_MASK_WORDS];
    uint32_t sharing_count;
    pl_topology_evidence evidence;
} pl_topology_cache;

typedef struct {
    uint32_t node_id;
    uint64_t logical_mask[PL_TOPOLOGY_MASK_WORDS];
    uint32_t logical_count;
    uint64_t memory_total_bytes;
    uint64_t memory_available_bytes;
    pl_topology_evidence evidence;
} pl_topology_numa_node;

/* -1 means unknown; 0 means absent/unusable; 1 means present/usable. */
typedef struct {
    int8_t hardware_present;
    int8_t os_usable;
    uint8_t evidence;
} pl_topology_isa_state;

typedef struct {
    uint32_t generation;
    uint32_t logical_cpu_count;
    uint32_t online_logical_cpu_count;
    uint32_t cache_count;
    uint32_t numa_node_count;
    pl_topology_completeness completeness;
    char provider[32];
    char interface_name[40];
    char platform_release[96];
    char captured_at[32];
    char architecture[32];
    char vendor[32];
    char model[96];
    pl_topology_logical_cpu logical_cpus[PL_TOPOLOGY_MAX_LOGICAL_CPUS];
    pl_topology_cache caches[PL_TOPOLOGY_MAX_CACHES];
    pl_topology_numa_node numa_nodes[PL_TOPOLOGY_MAX_NUMA_NODES];
    pl_topology_isa_state isa[PL_TOPOLOGY_ISA_FEATURE_COUNT];
} pl_processor_topology_snapshot;

typedef enum {
    PL_TOPOLOGY_OK = 0,
    PL_TOPOLOGY_INVALID_ARGUMENT,
    PL_TOPOLOGY_LIMIT_EXCEEDED,
    PL_TOPOLOGY_DUPLICATE_CPU,
    PL_TOPOLOGY_INVALID_REFERENCE,
    PL_TOPOLOGY_INVALID_MASK,
    PL_TOPOLOGY_IO_ERROR,
    PL_TOPOLOGY_UNSUPPORTED
} pl_topology_status;

/* Read-only collection from Windows processor topology APIs or Linux sysfs. */
pl_topology_status pl_processor_topology_scan(pl_processor_topology_snapshot *out);
pl_topology_status pl_processor_topology_validate(
    const pl_processor_topology_snapshot *snapshot);
pl_topology_status pl_processor_topology_write_json(
    const pl_processor_topology_snapshot *snapshot, FILE *output);
const char *pl_topology_status_name(pl_topology_status status);

#ifdef __cplusplus
}
#endif
#endif
