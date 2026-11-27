#ifndef PYTYLIKI_PROCESSOR_PLACEMENT_H
#define PYTYLIKI_PROCESSOR_PLACEMENT_H

#include "processor_topology.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PL_PLACEMENT_MAX_TASKS 256u
#define PL_PLACEMENT_REASON_NUMA_UNKNOWN       (UINT32_C(1) << 0)
#define PL_PLACEMENT_REASON_REMOTE_NUMA       (UINT32_C(1) << 1)
#define PL_PLACEMENT_REASON_SHARED_CORE       (UINT32_C(1) << 2)
#define PL_PLACEMENT_REASON_SHARED_CACHE      (UINT32_C(1) << 3)
#define PL_PLACEMENT_REASON_BANDWIDTH_UNKNOWN (UINT32_C(1) << 4)
#define PL_PLACEMENT_REASON_BANDWIDTH_PRESSURE (UINT32_C(1) << 5)
#define PL_PLACEMENT_REASON_CAPACITY_PRESSURE (UINT32_C(1) << 6)
#define PL_PLACEMENT_REASON_CACHE_UNKNOWN     (UINT32_C(1) << 7)

typedef enum {
    PL_PLACEMENT_ASSIGNED = 0,
    PL_PLACEMENT_NO_COMPATIBLE_CPU,
    PL_PLACEMENT_CAPACITY_EXHAUSTED,
    PL_PLACEMENT_REQUIRED_LOCALITY_UNKNOWN
} pl_placement_state;

typedef struct {
    uint32_t task_id;
    uint64_t affinity_mask[PL_TOPOLOGY_MASK_WORDS]; /* all zero means unrestricted */
    uint32_t required_isa_mask;
    uint32_t preferred_numa_node_id; /* PL_TOPOLOGY_UNKNOWN means no preference */
    uint64_t working_set_bytes;
    uint64_t memory_bandwidth_bytes_per_second;
    uint32_t requested_load_basis_points;
    uint8_t priority;
    uint8_t require_local_numa;
} pl_placement_task;

typedef struct {
    uint32_t generation;
    uint32_t cpu_load_basis_points[PL_TOPOLOGY_MAX_LOGICAL_CPUS];
    uint64_t cache_working_set_bytes[PL_TOPOLOGY_MAX_CACHES];
    uint64_t numa_bandwidth_capacity_bytes_per_second[PL_TOPOLOGY_MAX_NUMA_NODES];
    uint64_t numa_bandwidth_used_bytes_per_second[PL_TOPOLOGY_MAX_NUMA_NODES];
} pl_placement_pressure;

typedef struct {
    uint32_t task_id;
    uint32_t logical_cpu_id;
    pl_placement_state state;
    uint32_t total_cost;
    uint32_t cpu_load_cost;
    uint32_t numa_locality_cost;
    uint32_t cache_pressure_cost;
    uint32_t smt_contention_cost;
    uint32_t bandwidth_pressure_cost;
    uint32_t capacity_cost;
    uint32_t reason_mask;
} pl_placement_assignment;

typedef struct {
    uint32_t generation;
    uint32_t assigned;
    uint32_t no_compatible_cpu;
    uint32_t capacity_exhausted;
    uint32_t locality_unknown;
    uint64_t total_estimated_cost;
    uint8_t execution_performed;
    uint8_t affinity_changed;
    uint8_t scheduling_changed;
} pl_placement_report;

typedef struct {
    pl_placement_assignment assignments[PL_PLACEMENT_MAX_TASKS];
    size_t assignment_count;
    pl_placement_report report;
} pl_placement_result;

typedef enum {
    PL_PLACEMENT_OK = 0,
    PL_PLACEMENT_INVALID_ARGUMENT,
    PL_PLACEMENT_LIMIT_EXCEEDED,
    PL_PLACEMENT_STALE_GENERATION,
    PL_PLACEMENT_INVALID_TOPOLOGY
} pl_placement_status;

/* Deterministic advisory placement. It never binds or starts a host thread. */
pl_placement_status pl_processor_placement_plan(
    const pl_processor_topology_snapshot *topology,
    const pl_placement_task *tasks, size_t task_count,
    pl_placement_pressure *pressure, pl_placement_result *out);
const char *pl_placement_state_name(pl_placement_state state);
const char *pl_placement_status_name(pl_placement_status status);

#ifdef __cplusplus
}
#endif
#endif
