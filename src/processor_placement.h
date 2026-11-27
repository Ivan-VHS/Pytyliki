#ifndef PYTYLIKI_PROCESSOR_PLACEMENT_H
#define PYTYLIKI_PROCESSOR_PLACEMENT_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "processor_topology.h"

#define PL_PLACEMENT_MAX_TASKS 128u
#define PL_PLACEMENT_MAX_LOAD_BASIS_POINTS 10000u
#define PL_PLACEMENT_MAX_WORKERS PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS
#define PL_PLACEMENT_REASON_NUMA_REMOTE       (UINT32_C(1) << 0)
#define PL_PLACEMENT_REASON_NUMA_UNKNOWN      (UINT32_C(1) << 1)
#define PL_PLACEMENT_REASON_SHARED_CORE       (UINT32_C(1) << 2)
#define PL_PLACEMENT_REASON_SHARED_CACHE      (UINT32_C(1) << 3)
#define PL_PLACEMENT_REASON_BANDWIDTH_PRESSURE (UINT32_C(1) << 4)
#define PL_PLACEMENT_REASON_CAPACITY_PRESSURE  (UINT32_C(1) << 5)
#define PL_PLACEMENT_REASON_ISA_FILTERED       (UINT32_C(1) << 6)
#define PL_PLACEMENT_REASON_AFFINITY_FILTERED  (UINT32_C(1) << 7)
#define PL_PLACEMENT_REASON_OFFLINE_FILTERED   (UINT32_C(1) << 8)
#define PL_PLACEMENT_REASON_CACHE_UNKNOWN      (UINT32_C(1) << 9)
#define PL_PLACEMENT_REASON_CORE_UNKNOWN       (UINT32_C(1) << 10)
#define PL_PLACEMENT_REASON_DEPENDENCY_WAIT    (UINT32_C(1) << 11)

typedef struct {
    uint32_t logical_id;
    uint32_t core_id;
    uint32_t numa_node_id;
    uint32_t current_load_basis_points;
    uint32_t interrupt_load_basis_points;
    uint32_t capacity_basis_points;
    uint32_t isa_usable_mask;
    uint8_t online;
    uint32_t package_id;
    uint32_t die_id;
} pl_placement_worker;

typedef struct {
    uint32_t task_id;
    uint32_t priority;
    uint32_t requested_load_basis_points;
    uint32_t required_isa_mask;
    uint32_t preferred_numa_node_id;
    uint64_t working_set_bytes;
    uint64_t bandwidth_demand_bytes_per_sec;
    uint64_t allowed_logical_mask[PL_TOPOLOGY_MASK_WORDS];
    uint8_t locality_required;
    uint8_t dependencies_ready;
} pl_placement_task;

typedef struct {
    uint32_t node_id;
    uint64_t bandwidth_capacity_bytes_per_sec;
} pl_placement_numa_capacity;

typedef struct {
    uint32_t task_id;
    uint32_t logical_id;
    uint64_t total_cost;
    uint64_t load_cost;
    uint64_t numa_cost;
    uint64_t cache_cost;
    uint64_t smt_cost;
    uint64_t bandwidth_cost;
    uint32_t reasons;
    uint8_t assigned;
} pl_placement_result;

typedef struct {
    const pl_placement_worker *workers;
    size_t worker_count;
    const pl_topology_cache *caches;
    size_t cache_count;
    const pl_placement_numa_capacity *numa_capacity;
    size_t numa_capacity_count;
    const pl_placement_task *tasks;
    size_t task_count;
} pl_placement_input;

typedef enum {
    PL_PLACEMENT_OK = 0,
    PL_PLACEMENT_INVALID_ARGUMENT,
    PL_PLACEMENT_LIMIT_EXCEEDED,
    PL_PLACEMENT_OUTPUT_TOO_SMALL,
    PL_PLACEMENT_INVALID_LOAD
} pl_placement_status;

/* Deterministic estimate only: no OS thread creation, affinity or memory pinning. */
pl_placement_status pl_processor_placement_plan(
    const pl_placement_input *input,
    pl_placement_result *results,
    size_t result_capacity,
    size_t *result_count
);
pl_placement_status pl_processor_placement_write_json(
    const pl_placement_result *results,
    size_t result_count,
    FILE *output
);
const char *pl_placement_status_name(pl_placement_status status);

#endif
