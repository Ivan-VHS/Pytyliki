#include "processor_placement.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_numa_and_smt_costs_are_deterministic(void) {
    pl_placement_worker workers[3] = {
        {0u, 0u, 0u, 1000u, 0u, 10000u, 0x03u, 1u},
        {1u, 0u, 0u, 1000u, 0u, 10000u, 0x03u, 1u},
        {2u, 1u, 1u, 500u, 0u, 10000u, 0x03u, 1u}
    };
    pl_topology_cache caches[1];
    pl_placement_numa_capacity numa[2] = {{0u, 100000u}, {1u, 100000u}};
    pl_placement_task tasks[2];
    pl_placement_input input;
    pl_placement_result results[2];
    size_t count = 0u;
    memset(caches, 0, sizeof(caches));
    caches[0].cache_id = 1u;
    caches[0].level = 3u;
    caches[0].size_bytes = 1024u * 1024u;
    caches[0].sharing_mask[0] = 0x03u;
    memset(tasks, 0, sizeof(tasks));
    tasks[0].task_id = 10u; tasks[0].priority = 9u;
    tasks[0].requested_load_basis_points = 2000u;
    tasks[0].preferred_numa_node_id = 1u;
    tasks[0].working_set_bytes = 2u * 1024u * 1024u;
    tasks[0].bandwidth_demand_bytes_per_sec = 40000u;
    tasks[0].dependencies_ready = 1u;
    tasks[1] = tasks[0]; tasks[1].task_id = 11u; tasks[1].priority = 1u;
    input.workers = workers; input.worker_count = 3u;
    input.caches = caches; input.cache_count = 1u;
    input.numa_capacity = numa; input.numa_capacity_count = 2u;
    input.tasks = tasks; input.task_count = 2u;
    assert(pl_processor_placement_plan(&input, results, 2u, &count) == PL_PLACEMENT_OK);
    assert(count == 2u);
    assert(results[0].task_id == 10u);
    assert(results[0].logical_id == 2u);
    assert(results[1].task_id == 11u);
    assert(results[1].assigned == 1u);
    assert(pl_processor_placement_plan(&input, results, 2u, &count) == PL_PLACEMENT_OK);
    assert(results[0].logical_id == 2u);
}

static void test_unknown_locality_fails_closed_when_required(void) {
    pl_placement_worker worker = {0u, 0u, PL_TOPOLOGY_UNKNOWN_ID, 0u, 0u, 10000u, 0u, 1u};
    pl_placement_task task;
    pl_placement_input input;
    pl_placement_result result;
    size_t count = 0u;
    memset(&task, 0, sizeof(task));
    task.task_id = 1u; task.preferred_numa_node_id = 2u;
    task.locality_required = 1u; task.dependencies_ready = 1u;
    input.workers = &worker; input.worker_count = 1u;
    input.caches = NULL; input.cache_count = 0u;
    input.numa_capacity = NULL; input.numa_capacity_count = 0u;
    input.tasks = &task; input.task_count = 1u;
    assert(pl_processor_placement_plan(&input, &result, 1u, &count) == PL_PLACEMENT_OK);
    assert(result.assigned == 0u);
    assert((result.reasons & PL_PLACEMENT_REASON_NUMA_UNKNOWN) != 0u);
}

int main(void) {
    test_numa_and_smt_costs_are_deterministic();
    test_unknown_locality_fails_closed_when_required();
    puts("Pytyliki processor placement v2 tests passed");
    return 0;
}
