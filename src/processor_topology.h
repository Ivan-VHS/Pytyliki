#ifndef PYTYLIKI_PROCESSOR_TOPOLOGY_H
#define PYTYLIKI_PROCESSOR_TOPOLOGY_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "processor.h"

#define PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS 256u
#define PL_TOPOLOGY_MAX_CACHES 256u
#define PL_TOPOLOGY_MAX_NUMA_NODES 64u
#define PL_TOPOLOGY_MASK_WORDS (PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS / 64u)
#define PL_TOPOLOGY_UNKNOWN_ID UINT32_MAX

#define PL_TOPOLOGY_ISA_SSE2       (UINT32_C(1) << 0)
#define PL_TOPOLOGY_ISA_SSE42      (UINT32_C(1) << 1)
#define PL_TOPOLOGY_ISA_AVX        (UINT32_C(1) << 2)
#define PL_TOPOLOGY_ISA_AVX2       (UINT32_C(1) << 3)
#define PL_TOPOLOGY_ISA_AVX512F    (UINT32_C(1) << 4)
#define PL_TOPOLOGY_ISA_AES        (UINT32_C(1) << 5)
#define PL_TOPOLOGY_ISA_FMA        (UINT32_C(1) << 6)
#define PL_TOPOLOGY_ISA_VIRTUALIZE (UINT32_C(1) << 7)

typedef enum {
    PL_TOPOLOGY_CACHE_UNKNOWN = 0,
    PL_TOPOLOGY_CACHE_DATA,
    PL_TOPOLOGY_CACHE_INSTRUCTION,
    PL_TOPOLOGY_CACHE_UNIFIED,
    PL_TOPOLOGY_CACHE_TRACE
} pl_topology_cache_kind;

typedef enum {
    PL_TOPOLOGY_EVIDENCE_UNKNOWN = 0,
    PL_TOPOLOGY_EVIDENCE_PARTIAL,
    PL_TOPOLOGY_EVIDENCE_OBSERVED
} pl_topology_evidence;

typedef struct {
    uint32_t logical_id;
    uint32_t os_id;
    uint32_t group_id;
    uint32_t thread_id;
    uint32_t package_id;
    uint32_t die_id;
    uint32_t core_id;
    uint32_t numa_node_id;
    uint8_t online;
    pl_topology_evidence evidence;
} pl_topology_logical_processor;

typedef struct {
    uint32_t cache_id;
    uint32_t level;
    pl_topology_cache_kind kind;
    uint64_t size_bytes;
    uint32_t line_bytes;
    uint64_t sharing_mask[PL_TOPOLOGY_MASK_WORDS];
    pl_topology_evidence evidence;
} pl_topology_cache;

typedef struct {
    uint32_t node_id;
    uint64_t logical_mask[PL_TOPOLOGY_MASK_WORDS];
    uint64_t memory_total_bytes;
    uint64_t memory_available_bytes;
    uint8_t has_memory_total;
    uint8_t has_memory_available;
    pl_topology_evidence evidence;
} pl_topology_numa_node;

typedef struct {
    uint32_t logical_processor_count;
    uint32_t online_logical_processor_count;
    uint32_t cache_count;
    uint32_t numa_node_count;
    pl_topology_logical_processor logical_processors[PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS];
    pl_topology_cache caches[PL_TOPOLOGY_MAX_CACHES];
    pl_topology_numa_node numa_nodes[PL_TOPOLOGY_MAX_NUMA_NODES];
    uint32_t isa_hardware_present;
    uint32_t isa_os_usable;
    uint32_t isa_known;
    uint32_t isa_os_known;
    uint32_t generation;
    char architecture[32];
    char vendor[32];
    char model[96];
    char collector[64];
    char captured_at[32];
    uint8_t complete;
} pl_processor_topology_snapshot;

typedef enum {
    PL_TOPOLOGY_OK = 0,
    PL_TOPOLOGY_INVALID_ARGUMENT,
    PL_TOPOLOGY_UNSUPPORTED_PLATFORM,
    PL_TOPOLOGY_LIMIT_EXCEEDED,
    PL_TOPOLOGY_IO_ERROR,
    PL_TOPOLOGY_INTERNAL_ERROR
} pl_topology_status;

/* Read-only OS inventory. Unsupported fields remain PL_TOPOLOGY_UNKNOWN_ID. */
pl_topology_status pl_processor_topology_collect(
    pl_processor_topology_snapshot *out,
    pl_processor_snapshot *cpu_profile
);

/* Writes pytyliki.processor-topology-evidence.v2 JSON to a caller-owned stream. */
pl_topology_status pl_processor_topology_write_json(
    const pl_processor_topology_snapshot *snapshot,
    FILE *output
);

const char *pl_topology_status_name(pl_topology_status status);
const char *pl_topology_cache_kind_name(pl_topology_cache_kind kind);
const char *pl_topology_evidence_name(pl_topology_evidence evidence);

#endif
