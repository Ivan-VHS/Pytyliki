#include "processor_placement.h"

#include <inttypes.h>
#include <string.h>

typedef struct {
    uint32_t task_id;
    uint32_t logical_id;
    uint32_t core_id;
    uint32_t numa_node_id;
    uint64_t bandwidth;
    uint64_t working_set;
    uint32_t load_basis_points;
} assigned_task;

static uint64_t add_saturating(uint64_t left, uint64_t right) {
    return left > UINT64_MAX - right ? UINT64_MAX : left + right;
}

static int mask_contains(const uint64_t mask[PL_TOPOLOGY_MASK_WORDS],
                         uint32_t logical_id) {
    if (logical_id >= PL_PLACEMENT_MAX_WORKERS) return 0;
    return (mask[logical_id / 64u] &
            (UINT64_C(1) << (logical_id % 64u))) != 0u;
}

static const pl_placement_numa_capacity *find_numa(
    const pl_placement_input *input, uint32_t node_id
) {
    size_t i;
    for (i = 0u; i < input->numa_capacity_count; ++i)
        if (input->numa_capacity[i].node_id == node_id)
            return &input->numa_capacity[i];
    return NULL;
}

static uint64_t assigned_bandwidth_on_node(const assigned_task *assigned,
                                           size_t count, uint32_t node_id) {
    size_t i;
    uint64_t total = 0u;
    for (i = 0u; i < count; ++i)
        if (assigned[i].numa_node_id == node_id)
            total = add_saturating(total, assigned[i].bandwidth);
    return total;
}

static uint64_t assigned_working_set_on_cache(
    const pl_processor_placement_input *input,
    const assigned_task *assigned, size_t assigned_count,
    const pl_topology_cache *cache
) {
    size_t i, j;
    uint64_t total = 0u;
    for (i = 0u; i < assigned_count; ++i) {
        int shares = 0;
        for (j = 0u; j < input->worker_count; ++j) {
            if (input->workers[j].logical_id == assigned[i].logical_id) {
                shares = mask_contains(cache->sharing_mask, input->workers[j].logical_id);
                break;
            }
        }
        if (shares) total = add_saturating(total, assigned[i].working_set);
    }
    return total;
}

static uint64_t ratio_basis_points(uint64_t value, uint64_t capacity) {
    if (capacity == 0u) return value == 0u ? 0u : 10000u;
    if (value >= capacity) return 10000u;
    /* Avoid overflow by scaling the smaller operand in bounded chunks. */
    return (uint64_t)(((long double)value * 10000.0L) /
                      (long double)capacity);
}

static int better(const pl_placement_result *candidate,
                  const pl_placement_result *best,
                  uint32_t candidate_logical, uint32_t best_logical) {
    if (!best->assigned) return 1;
    if (candidate->total_cost != best->total_cost)
        return candidate->total_cost < best->total_cost;
    return candidate_logical < best_logical;
}

pl_placement_status pl_processor_placement_plan(
    const pl_placement_input *input,
    pl_placement_result *results,
    size_t result_capacity,
    size_t *result_count
) {
    assigned_task assigned[PL_PLACEMENT_MAX_TASKS];
    uint8_t task_done[PL_PLACEMENT_MAX_TASKS];
    size_t assigned_count = 0u, output_count = 0u, step;
    if (result_count == NULL) return PL_PLACEMENT_INVALID_ARGUMENT;
    *result_count = 0u;
    if (input == NULL || results == NULL ||
        (input->worker_count != 0u && input->workers == NULL) ||
        (input->cache_count != 0u && input->caches == NULL) ||
        (input->numa_capacity_count != 0u && input->numa_capacity == NULL) ||
        (input->task_count != 0u && input->tasks == NULL))
        return PL_PLACEMENT_INVALID_ARGUMENT;
    if (input->worker_count == 0u || input->task_count == 0u ||
        input->worker_count > PL_PLACEMENT_MAX_WORKERS ||
        input->task_count > PL_PLACEMENT_MAX_TASKS ||
        input->cache_count > PL_TOPOLOGY_MAX_CACHES ||
        input->numa_capacity_count > PL_TOPOLOGY_MAX_NUMA_NODES)
        return PL_PLACEMENT_LIMIT_EXCEEDED;
    if (result_capacity < input->task_count) return PL_PLACEMENT_OUTPUT_TOO_SMALL;
    memset(assigned, 0, sizeof(assigned));
    memset(task_done, 0, sizeof(task_done));

    /* Choose each next task by priority descending, then stable task ID. */
    for (step = 0u; step < input->task_count; ++step) {
        size_t task_index = input->task_count;
        size_t i, worker_index;
        const pl_placement_task *task;
        pl_placement_result best;
        uint32_t best_logical = UINT32_MAX;
        memset(&best, 0, sizeof(best));
        for (i = 0u; i < input->task_count; ++i) {
            if (task_done[i]) continue;
            if (task_index == input->task_count ||
                input->tasks[i].priority > input->tasks[task_index].priority ||
                (input->tasks[i].priority == input->tasks[task_index].priority &&
                 input->tasks[i].task_id < input->tasks[task_index].task_id))
                task_index = i;
        }
        if (task_index == input->task_count) return PL_PLACEMENT_INVALID_ARGUMENT;
        task_done[task_index] = 1u;
        task = &input->tasks[task_index];
        results[output_count].task_id = task->task_id;
        results[output_count].logical_id = UINT32_MAX;
        results[output_count].assigned = 0u;
        results[output_count].reasons = 0u;
        if (task->requested_load_basis_points > 10000u) return PL_PLACEMENT_INVALID_LOAD;
        if (!task->dependencies_ready) {
            ++output_count;
            continue;
        }

        for (worker_index = 0u; worker_index < input->worker_count; ++worker_index) {
            const pl_placement_worker *worker = &input->workers[worker_index];
            pl_placement_result candidate;
            uint64_t load, bandwidth_after, cache_pressure = 0u;
            uint32_t reasons = 0u;
            size_t previous;
            if (!worker->online) {
                results[output_count].reasons |= PL_PLACEMENT_REASON_OFFLINE_FILTERED;
                continue;
            }
            if ((worker->isa_usable_mask & task->required_isa_mask) != task->required_isa_mask) {
                results[output_count].reasons |= PL_PLACEMENT_REASON_ISA_FILTERED;
                continue;
            }
            if (task->allowed_logical_mask[0] != 0u ||
                task->allowed_logical_mask[1] != 0u ||
                task->allowed_logical_mask[2] != 0u ||
                task->allowed_logical_mask[3] != 0u) {
                if (!mask_contains(task->allowed_logical_mask, worker->logical_id)) {
                    results[output_count].reasons |= PL_PLACEMENT_REASON_AFFINITY_FILTERED;
                    continue;
                }
            }
            memset(&candidate, 0, sizeof(candidate));
            candidate.task_id = task->task_id;
            candidate.logical_id = worker->logical_id;
            candidate.assigned = 1u;
            load = (uint64_t)worker->current_load_basis_points +
                   worker->interrupt_load_basis_points;
            for (previous = 0u; previous < assigned_count; ++previous) {
                if (assigned[previous].logical_id == worker->logical_id)
                    load = add_saturating(load, task->requested_load_basis_points);
            }
            candidate.load_cost = add_saturating(load, task->requested_load_basis_points);
            if (worker->capacity_basis_points == 0u ||
                load + task->requested_load_basis_points >
                    worker->capacity_basis_points) {
                reasons |= PL_PLACEMENT_REASON_CAPACITY_PRESSURE;
                candidate.load_cost = add_saturating(candidate.load_cost, 10000u);
            }
            if (task->preferred_numa_node_id != PL_TOPOLOGY_UNKNOWN_ID) {
                if (worker->numa_node_id == PL_TOPOLOGY_UNKNOWN_ID) {
                    reasons |= PL_PLACEMENT_REASON_NUMA_UNKNOWN;
                    if (task->locality_required) continue;
                    candidate.numa_cost = 2500u;
                } else if (worker->numa_node_id != task->preferred_numa_node_id) {
                    reasons |= PL_PLACEMENT_REASON_NUMA_REMOTE;
                    if (task->locality_required) continue;
                    candidate.numa_cost = 3000u;
                }
            }
            for (previous = 0u; previous < assigned_count; ++previous) {
                if (worker->core_id != PL_TOPOLOGY_UNKNOWN_ID &&
                    assigned[previous].core_id == worker->core_id) {
                    reasons |= PL_PLACEMENT_REASON_SHARED_CORE;
                    candidate.smt_cost = add_saturating(candidate.smt_cost, 1800u);
                }
            }
            if (task->working_set_bytes != 0u && input->cache_count == 0u) {
                reasons |= PL_PLACEMENT_REASON_SHARED_CACHE;
                candidate.cache_cost = 700u;
            }
            for (previous = 0u; previous < input->cache_count; ++previous) {
                const pl_topology_cache *cache = &input->caches[previous];
                uint64_t working, pressure;
                if (!mask_contains(cache->sharing_mask, worker->logical_id)) continue;
                working = add_saturating(task->working_set_bytes,
                    assigned_working_set_on_cache(input, assigned, assigned_count, cache));
                pressure = ratio_basis_points(working, cache->size_bytes);
                if (pressure > 10000u) pressure = 10000u;
                candidate.cache_cost = add_saturating(candidate.cache_cost, pressure / 4u);
                if (assigned_count != 0u) reasons |= PL_PLACEMENT_REASON_SHARED_CACHE;
            }
            if (task->bandwidth_demand_bytes_per_sec != 0u) {
                const pl_placement_numa_capacity *capacity =
                    find_numa(input, worker->numa_node_id);
                if (capacity == NULL || capacity->bandwidth_capacity_bytes_per_sec == 0u) {
                    reasons |= PL_PLACEMENT_REASON_BANDWIDTH_PRESSURE;
                    candidate.bandwidth_cost = 1500u;
                } else {
                    bandwidth_after = add_saturating(
                        assigned_bandwidth_on_node(assigned, assigned_count, worker->numa_node_id),
                        task->bandwidth_demand_bytes_per_sec);
                    candidate.bandwidth_cost = ratio_basis_points(
                        bandwidth_after, capacity->bandwidth_capacity_bytes_per_sec);
                    if (bandwidth_after > capacity->bandwidth_capacity_bytes_per_sec)
                        reasons |= PL_PLACEMENT_REASON_BANDWIDTH_PRESSURE;
                }
            }
            candidate.total_cost = add_saturating(candidate.load_cost, candidate.numa_cost);
            candidate.total_cost = add_saturating(candidate.total_cost, candidate.cache_cost);
            candidate.total_cost = add_saturating(candidate.total_cost, candidate.smt_cost);
            candidate.total_cost = add_saturating(candidate.total_cost, candidate.bandwidth_cost);
            candidate.reasons = reasons;
            if (better(&candidate, &best, worker->logical_id, best_logical)) {
                best = candidate;
                best_logical = worker->logical_id;
            }
        }
        if (best.assigned) {
            results[output_count] = best;
            assigned[assigned_count].task_id = task->task_id;
            assigned[assigned_count].logical_id = best.logical_id;
            assigned[assigned_count].core_id = PL_TOPOLOGY_UNKNOWN_ID;
            assigned[assigned_count].numa_node_id = PL_TOPOLOGY_UNKNOWN_ID;
            assigned[assigned_count].bandwidth = task->bandwidth_demand_bytes_per_sec;
            assigned[assigned_count].working_set = task->working_set_bytes;
            assigned[assigned_count].load_basis_points = task->requested_load_basis_points;
            for (worker_index = 0u; worker_index < input->worker_count; ++worker_index)
                if (input->workers[worker_index].logical_id == best.logical_id) {
                    assigned[assigned_count].core_id = input->workers[worker_index].core_id;
                    assigned[assigned_count].numa_node_id = input->workers[worker_index].numa_node_id;
                    break;
                }
            ++assigned_count;
        }
        ++output_count;
    }
    *result_count = output_count;
    return PL_PLACEMENT_OK;
}

pl_placement_status pl_processor_placement_write_json(
    const pl_placement_result *results, size_t result_count, FILE *output
) {
    size_t i;
    if (output == NULL || (result_count != 0u && results == NULL) ||
        result_count > PL_PLACEMENT_MAX_TASKS)
        return PL_PLACEMENT_INVALID_ARGUMENT;
    fputs("{\"schema\":\"pytyliki.processor-placement-report.v2\","
          "\"execution_performed\":false,\"affinity_changed\":false,\"assignments\":[", output);
    for (i = 0u; i < result_count; ++i) {
        const pl_placement_result *r = &results[i];
        if (i != 0u) fputc(',', output);
        fprintf(output,
            "{\"task_id\":%u,\"logical_id\":", r->task_id);
        if (r->assigned) fprintf(output, "%u", r->logical_id);
        else fputs("null", output);
        fprintf(output,
            ",\"assigned\":%s,\"cost\":%" PRIu64
            ",\"components\":{\"load\":%" PRIu64 ",\"numa\":%" PRIu64
            ",\"cache\":%" PRIu64 ",\"smt\":%" PRIu64
            ",\"bandwidth\":%" PRIu64 "},\"reasons\":%u}",
            r->assigned ? "true" : "false", r->total_cost, r->load_cost,
            r->numa_cost, r->cache_cost, r->smt_cost, r->bandwidth_cost, r->reasons);
    }
    fputs("]}\n", output);
    return ferror(output) ? PL_PLACEMENT_INVALID_ARGUMENT : PL_PLACEMENT_OK;
}

const char *pl_placement_status_name(pl_placement_status status) {
    switch (status) {
    case PL_PLACEMENT_OK: return "ok";
    case PL_PLACEMENT_INVALID_ARGUMENT: return "invalid-argument";
    case PL_PLACEMENT_LIMIT_EXCEEDED: return "limit-exceeded";
    case PL_PLACEMENT_OUTPUT_TOO_SMALL: return "output-too-small";
    case PL_PLACEMENT_INVALID_LOAD: return "invalid-load";
    }
    return "unknown";
}
