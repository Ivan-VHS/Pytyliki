#include "processor_fabric.h"

#include "cpu_suite.h"
#include "hardware_syntax.h"
#include "iron_core.h"
#include "processor_catalog.h"
#include "system_sigil.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

static const char *const node_kind_names[PL_PROCESSOR_FABRIC_NODE_KIND_COUNT] = {
    "package", "die", "core", "logical-processor", "register-bank",
    "isa-profile", "address-space", "memory-region", "device",
    "execution-unit", "process-pool", "process", "artifact",
    "processor-group", "numa-node", "cache"
};

static const char *const node_state_names[PL_PROCESSOR_FABRIC_NODE_STATE_COUNT] = {
    "declared", "verified", "ready", "available", "blocked", "completed",
    "failed", "disabled", "retired"
};

static const char *const edge_kind_names[PL_PROCESSOR_FABRIC_EDGE_KIND_COUNT] = {
    "depends-on", "maps-to", "uses", "produces", "shares-cache"
};

static const char *const assignment_state_names[] = {
    "assigned", "waiting-dependency", "no-compatible-worker", "capacity-exhausted"
};

static void clear_error(pl_processor_fabric_error *error) {
    if (error != NULL) {
        memset(error, 0, sizeof(*error));
        error->status = PL_PROCESSOR_FABRIC_OK;
    }
}

static pl_processor_fabric_status fail(
    pl_processor_fabric_error *error,
    pl_processor_fabric_status status,
    uint32_t node_id,
    uint32_t from_id,
    uint32_t to_id,
    const char *message
) {
    if (error != NULL) {
        error->status = status;
        error->node_id = node_id;
        error->edge_from_id = from_id;
        error->edge_to_id = to_id;
        (void)snprintf(error->message, sizeof(error->message), "%s", message);
    }
    return status;
}

static size_t find_node_index(const pl_processor_fabric_model *model,
                              uint32_t id) {
    size_t index;
    for (index = 0u; index < model->node_count; ++index) {
        if (model->nodes[index].id == id) return index;
    }
    return model->node_count;
}

static int is_worker_kind(pl_processor_fabric_node_kind kind) {
    return kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR ||
           kind == PL_PROCESSOR_FABRIC_EXECUTION_UNIT;
}

static int domain_mask_valid(uint32_t mask) {
    return (mask & ~PL_PROCESSOR_FABRIC_DOMAIN_ALL) == 0u;
}

static int parent_kind_allowed(pl_processor_fabric_node_kind child,
                               pl_processor_fabric_node_kind parent) {
    switch (child) {
    case PL_PROCESSOR_FABRIC_DIE:
        return parent == PL_PROCESSOR_FABRIC_PACKAGE;
    case PL_PROCESSOR_FABRIC_CORE:
        return parent == PL_PROCESSOR_FABRIC_PACKAGE ||
               parent == PL_PROCESSOR_FABRIC_DIE;
    case PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR:
        return parent == PL_PROCESSOR_FABRIC_CORE;
    case PL_PROCESSOR_FABRIC_EXECUTION_UNIT:
        return parent == PL_PROCESSOR_FABRIC_PACKAGE ||
               parent == PL_PROCESSOR_FABRIC_DIE ||
               parent == PL_PROCESSOR_FABRIC_CORE;
    case PL_PROCESSOR_FABRIC_REGISTER_BANK:
    case PL_PROCESSOR_FABRIC_ISA_PROFILE:
        return parent == PL_PROCESSOR_FABRIC_PACKAGE ||
               parent == PL_PROCESSOR_FABRIC_DIE ||
               parent == PL_PROCESSOR_FABRIC_CORE ||
               parent == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR ||
               parent == PL_PROCESSOR_FABRIC_EXECUTION_UNIT;
    case PL_PROCESSOR_FABRIC_ADDRESS_SPACE:
        return parent == PL_PROCESSOR_FABRIC_PACKAGE ||
               parent == PL_PROCESSOR_FABRIC_DIE ||
               parent == PL_PROCESSOR_FABRIC_CORE;
    case PL_PROCESSOR_FABRIC_MEMORY_REGION:
        return parent == PL_PROCESSOR_FABRIC_ADDRESS_SPACE;
    case PL_PROCESSOR_FABRIC_DEVICE:
        return parent == PL_PROCESSOR_FABRIC_PACKAGE ||
               parent == PL_PROCESSOR_FABRIC_DIE ||
               parent == PL_PROCESSOR_FABRIC_CORE;
    case PL_PROCESSOR_FABRIC_PROCESS_POOL:
        return parent == PL_PROCESSOR_FABRIC_PACKAGE;
    case PL_PROCESSOR_FABRIC_PROCESS:
        return parent == PL_PROCESSOR_FABRIC_PROCESS_POOL;
    case PL_PROCESSOR_FABRIC_ARTIFACT:
        return parent == PL_PROCESSOR_FABRIC_PROCESS;
    case PL_PROCESSOR_FABRIC_PROCESSOR_GROUP:
    case PL_PROCESSOR_FABRIC_NUMA_NODE:
    case PL_PROCESSOR_FABRIC_CACHE:
        return 0;
    case PL_PROCESSOR_FABRIC_PACKAGE:
    case PL_PROCESSOR_FABRIC_NODE_KIND_COUNT:
    default:
        return 0;
    }
}

static int root_kind_allowed(pl_processor_fabric_node_kind kind) {
    return kind == PL_PROCESSOR_FABRIC_PACKAGE ||
           kind == PL_PROCESSOR_FABRIC_DIE ||
           kind == PL_PROCESSOR_FABRIC_CORE ||
           kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR ||
           kind == PL_PROCESSOR_FABRIC_PROCESSOR_GROUP ||
           kind == PL_PROCESSOR_FABRIC_NUMA_NODE ||
           kind == PL_PROCESSOR_FABRIC_CACHE ||
           kind == PL_PROCESSOR_FABRIC_PROCESS_POOL ||
           kind == PL_PROCESSOR_FABRIC_ISA_PROFILE ||
           kind == PL_PROCESSOR_FABRIC_ADDRESS_SPACE;
}

static int edge_kind_valid_for_nodes(
    pl_processor_fabric_edge_kind kind,
    pl_processor_fabric_node_kind from,
    pl_processor_fabric_node_kind to
) {
    switch (kind) {
    case PL_PROCESSOR_FABRIC_DEPENDS_ON:
        return from == PL_PROCESSOR_FABRIC_PROCESS &&
               to == PL_PROCESSOR_FABRIC_PROCESS;
    case PL_PROCESSOR_FABRIC_MAPS_TO:
        return (from == PL_PROCESSOR_FABRIC_MEMORY_REGION &&
                to == PL_PROCESSOR_FABRIC_ADDRESS_SPACE) ||
               (from == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR &&
                (to == PL_PROCESSOR_FABRIC_PROCESSOR_GROUP ||
                 to == PL_PROCESSOR_FABRIC_NUMA_NODE));
    case PL_PROCESSOR_FABRIC_USES:
        return from == PL_PROCESSOR_FABRIC_PROCESS &&
               (to == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR ||
                to == PL_PROCESSOR_FABRIC_EXECUTION_UNIT ||
                to == PL_PROCESSOR_FABRIC_REGISTER_BANK ||
                to == PL_PROCESSOR_FABRIC_ISA_PROFILE ||
                to == PL_PROCESSOR_FABRIC_ADDRESS_SPACE ||
                to == PL_PROCESSOR_FABRIC_MEMORY_REGION ||
                to == PL_PROCESSOR_FABRIC_DEVICE);
    case PL_PROCESSOR_FABRIC_PRODUCES:
        return from == PL_PROCESSOR_FABRIC_PROCESS &&
               to == PL_PROCESSOR_FABRIC_ARTIFACT;
    case PL_PROCESSOR_FABRIC_SHARES_CACHE:
        return from == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR &&
               to == PL_PROCESSOR_FABRIC_CACHE;
    case PL_PROCESSOR_FABRIC_EDGE_KIND_COUNT:
    default:
        return 0;
    }
}

static pl_processor_fabric_status validate_dependencies(
    const pl_processor_fabric_model *model,
    pl_processor_fabric_workspace *workspace,
    pl_processor_fabric_error *error
) {
    size_t index;
    size_t head = 0u;
    size_t tail = 0u;
    size_t visited = 0u;
    size_t process_count = 0u;

    memset(workspace->indegree, 0, sizeof(workspace->indegree));
    for (index = 0u; index < model->node_count; ++index) {
        if (model->nodes[index].kind == PL_PROCESSOR_FABRIC_PROCESS)
            ++process_count;
    }

    for (index = 0u; index < model->edge_count; ++index) {
        const pl_processor_fabric_edge *edge = &model->edges[index];
        if (edge->kind == PL_PROCESSOR_FABRIC_DEPENDS_ON) {
            size_t to_index = find_node_index(model, edge->to_id);
            if (to_index >= model->node_count)
                return fail(error, PL_PROCESSOR_FABRIC_MISSING_REFERENCE,
                            0u, edge->from_id, edge->to_id,
                            "dependency target is not present");
            ++workspace->indegree[to_index];
        }
    }

    /* Filter the initial queue to dependency-free process nodes. */
    tail = 0u;
    for (index = 0u; index < model->node_count; ++index) {
        if (model->nodes[index].kind == PL_PROCESSOR_FABRIC_PROCESS &&
            workspace->indegree[index] == 0u)
            workspace->queue[tail++] = index;
    }

    while (head < tail) {
        size_t current = workspace->queue[head++];
        uint32_t current_id = model->nodes[current].id;
        workspace->topological_order[visited++] = current;

        for (index = 0u; index < model->edge_count; ++index) {
            const pl_processor_fabric_edge *edge = &model->edges[index];
            if (edge->kind == PL_PROCESSOR_FABRIC_DEPENDS_ON &&
                edge->from_id == current_id) {
                size_t next = find_node_index(model, edge->to_id);
                if (next >= model->node_count || workspace->indegree[next] == 0u)
                    return fail(error, PL_PROCESSOR_FABRIC_INTERNAL_ERROR,
                                current_id, edge->from_id, edge->to_id,
                                "dependency accounting is inconsistent");
                --workspace->indegree[next];
                if (workspace->indegree[next] == 0u)
                    workspace->queue[tail++] = next;
            }
        }
    }

    if (visited != process_count)
        return fail(error, PL_PROCESSOR_FABRIC_DEPENDENCY_CYCLE,
                    0u, 0u, 0u, "process dependency graph contains a cycle");
    return PL_PROCESSOR_FABRIC_OK;
}

pl_processor_fabric_status pl_processor_fabric_validate(
    const pl_processor_fabric_model *model,
    pl_processor_fabric_workspace *workspace,
    pl_processor_fabric_summary *summary,
    pl_processor_fabric_error *error
) {
    size_t index;
    clear_error(error);

    if (model == NULL || workspace == NULL || summary == NULL ||
        model->nodes == NULL || model->node_count == 0u ||
        (model->edge_count != 0u && model->edges == NULL))
        return fail(error, PL_PROCESSOR_FABRIC_INVALID_ARGUMENT,
                    0u, 0u, 0u, "model, workspace, summary and node array are required");
    if (model->node_count > PL_PROCESSOR_FABRIC_MAX_NODES ||
        model->edge_count > PL_PROCESSOR_FABRIC_MAX_EDGES)
        return fail(error, PL_PROCESSOR_FABRIC_LIMIT_EXCEEDED,
                    0u, 0u, 0u, "model exceeds the bounded node or edge limit");
    if (model->generation == 0u || model->abi_major != PL_PROCESSOR_FABRIC_ABI_MAJOR)
        return fail(error, PL_PROCESSOR_FABRIC_INVALID_GENERATION,
                    0u, 0u, 0u, "snapshot generation or ABI major is unsupported");

    memset(summary, 0, sizeof(*summary));
    summary->generation = model->generation;
    summary->total_nodes = (uint32_t)model->node_count;
    summary->total_edges = (uint32_t)model->edge_count;

    for (index = 0u; index < model->node_count; ++index) {
        const pl_processor_fabric_node *node = &model->nodes[index];
        size_t other;
        size_t parent_index;

        if (node->id == 0u || node->generation != model->generation)
            return fail(error, PL_PROCESSOR_FABRIC_INVALID_GENERATION,
                        node->id, 0u, 0u, "node identity or generation is invalid");
        if ((unsigned)node->kind >= PL_PROCESSOR_FABRIC_NODE_KIND_COUNT ||
            (unsigned)node->state >= PL_PROCESSOR_FABRIC_NODE_STATE_COUNT)
            return fail(error, PL_PROCESSOR_FABRIC_INVALID_ARGUMENT,
                        node->id, 0u, 0u, "node kind or state is outside the v1 catalogue");
        if (!domain_mask_valid(node->domain_mask))
            return fail(error, PL_PROCESSOR_FABRIC_INVALID_DOMAIN,
                        node->id, 0u, 0u, "node contains an unknown execution-domain bit");

        for (other = index + 1u; other < model->node_count; ++other) {
            if (model->nodes[other].id == node->id)
                return fail(error, PL_PROCESSOR_FABRIC_DUPLICATE_ID,
                            node->id, 0u, 0u, "node IDs must be unique within a snapshot");
            if (node->kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR &&
                model->nodes[other].kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR &&
                model->nodes[other].logical_id == node->logical_id)
                return fail(error, PL_PROCESSOR_FABRIC_DUPLICATE_ID,
                            node->id, 0u, 0u, "logical processor IDs must be unique");
        }

        if (node->parent_id == 0u) {
            if (!root_kind_allowed(node->kind))
                return fail(error, PL_PROCESSOR_FABRIC_INVALID_PARENT,
                            node->id, 0u, 0u, "node kind requires a parent");
        } else {
            parent_index = find_node_index(model, node->parent_id);
            if (parent_index >= model->node_count)
                return fail(error, PL_PROCESSOR_FABRIC_MISSING_REFERENCE,
                            node->id, node->parent_id, node->id,
                            "parent node does not exist in this snapshot");
            if (!parent_kind_allowed(node->kind, model->nodes[parent_index].kind))
                return fail(error, PL_PROCESSOR_FABRIC_INVALID_PARENT,
                            node->id, node->parent_id, node->id,
                            "parent kind is not valid for the child node");
        }

        if (is_worker_kind(node->kind)) {
            const int offline_logical_cpu =
                node->kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR &&
                node->state == PL_PROCESSOR_FABRIC_DISABLED &&
                node->capacity_basis_points == 0u;
            if (node->logical_id >= 64u ||
                (!offline_logical_cpu &&
                 (node->capacity_basis_points == 0u ||
                  node->capacity_basis_points > PL_PROCESSOR_FABRIC_BASIS_POINTS)) ||
                node->reserved_basis_points > node->capacity_basis_points ||
                node->requested_load_basis_points != 0u ||
                node->domain_mask == 0u)
                return fail(error, PL_PROCESSOR_FABRIC_INVALID_CAPACITY,
                            node->id, 0u, 0u, "worker logical ID, domain or capacity is invalid");
        } else if (node->capacity_basis_points != 0u ||
                   node->reserved_basis_points != 0u) {
            return fail(error, PL_PROCESSOR_FABRIC_INVALID_CAPACITY,
                        node->id, 0u, 0u, "only worker nodes may declare compute capacity");
        }

        if (node->kind == PL_PROCESSOR_FABRIC_PROCESS) {
            if (node->domain_mask == 0u ||
                (node->state == PL_PROCESSOR_FABRIC_READY &&
                 (node->requested_load_basis_points == 0u ||
                  node->requested_load_basis_points > PL_PROCESSOR_FABRIC_BASIS_POINTS)))
                return fail(error, PL_PROCESSOR_FABRIC_INVALID_DOMAIN,
                            node->id, 0u, 0u, "process domain or requested load is invalid");
            ++summary->process_count;
            if (node->state == PL_PROCESSOR_FABRIC_READY) ++summary->ready_process_count;
            if (node->state == PL_PROCESSOR_FABRIC_COMPLETED) ++summary->completed_process_count;
            if (node->state == PL_PROCESSOR_FABRIC_FAILED) ++summary->failed_process_count;
        }

        ++summary->node_kind_counts[(unsigned)node->kind];
        ++summary->node_state_counts[(unsigned)node->state];

        if (is_worker_kind(node->kind)) {
            ++summary->worker_count;
            if (node->state == PL_PROCESSOR_FABRIC_AVAILABLE)
                ++summary->available_worker_count;
            summary->total_worker_capacity_basis_points += node->capacity_basis_points;
            summary->total_worker_reserved_basis_points += node->reserved_basis_points;
        }
    }

    /* Validate parent chains independently from the process dependency graph. */
    for (index = 0u; index < model->node_count; ++index) {
        uint32_t parent_id = model->nodes[index].parent_id;
        size_t depth = 0u;
        while (parent_id != 0u) {
            size_t parent_index = find_node_index(model, parent_id);
            if (parent_index >= model->node_count)
                return fail(error, PL_PROCESSOR_FABRIC_MISSING_REFERENCE,
                            model->nodes[index].id, parent_id, 0u,
                            "parent chain references a missing node");
            if (parent_id == model->nodes[index].id || ++depth > model->node_count)
                return fail(error, PL_PROCESSOR_FABRIC_PARENT_CYCLE,
                            model->nodes[index].id, parent_id, 0u,
                            "hardware/process hierarchy contains a parent cycle");
            parent_id = model->nodes[parent_index].parent_id;
        }
    }

    for (index = 0u; index < model->edge_count; ++index) {
        const pl_processor_fabric_edge *edge = &model->edges[index];
        size_t from_index = find_node_index(model, edge->from_id);
        size_t to_index = find_node_index(model, edge->to_id);
        size_t other;

        if (from_index >= model->node_count || to_index >= model->node_count)
            return fail(error, PL_PROCESSOR_FABRIC_MISSING_REFERENCE,
                        0u, edge->from_id, edge->to_id,
                        "edge endpoint does not exist in this snapshot");
        if ((unsigned)edge->kind >= PL_PROCESSOR_FABRIC_EDGE_KIND_COUNT ||
            !edge_kind_valid_for_nodes(edge->kind,
                model->nodes[from_index].kind, model->nodes[to_index].kind))
            return fail(error, PL_PROCESSOR_FABRIC_INVALID_EDGE,
                        0u, edge->from_id, edge->to_id,
                        "edge kind is not valid for its endpoint types");
        for (other = index + 1u; other < model->edge_count; ++other) {
            if (model->edges[other].from_id == edge->from_id &&
                model->edges[other].to_id == edge->to_id &&
                model->edges[other].kind == edge->kind)
                return fail(error, PL_PROCESSOR_FABRIC_DUPLICATE_EDGE,
                            0u, edge->from_id, edge->to_id,
                            "duplicate relationship edge");
        }

        if (edge->kind == PL_PROCESSOR_FABRIC_DEPENDS_ON)
            ++summary->dependency_edge_count;
        else if (edge->kind == PL_PROCESSOR_FABRIC_USES)
            ++summary->resource_edge_count;
        else if (edge->kind == PL_PROCESSOR_FABRIC_PRODUCES)
            ++summary->produced_edge_count;
    }

    return validate_dependencies(model, workspace, error);
}

static int has_pending_dependency(const pl_processor_fabric_model *model,
                                  uint32_t task_id) {
    size_t index;
    for (index = 0u; index < model->edge_count; ++index) {
        const pl_processor_fabric_edge *edge = &model->edges[index];
        if (edge->kind == PL_PROCESSOR_FABRIC_DEPENDS_ON &&
            edge->to_id == task_id) {
            size_t prerequisite = find_node_index(model, edge->from_id);
            if (prerequisite >= model->node_count ||
                model->nodes[prerequisite].state != PL_PROCESSOR_FABRIC_COMPLETED)
                return 1;
        }
    }
    return 0;
}

static int worker_supports_task(const pl_processor_fabric_node *worker,
                                const pl_processor_fabric_node *task) {
    if (!is_worker_kind(worker->kind) ||
        worker->state != PL_PROCESSOR_FABRIC_AVAILABLE)
        return 0;
    if ((worker->domain_mask & task->domain_mask) != task->domain_mask)
        return 0;
    if ((worker->feature_mask & task->feature_mask) != task->feature_mask)
        return 0;
    if (task->affinity_mask != 0u &&
        (task->affinity_mask & (UINT64_C(1) << worker->logical_id)) == 0u)
        return 0;
    return 1;
}

static int worker_is_better(uint32_t candidate_load, uint32_t candidate_capacity,
                            uint32_t candidate_logical_id, uint32_t candidate_id,
                            uint32_t best_load, uint32_t best_capacity,
                            uint32_t best_logical_id, uint32_t best_id) {
    uint64_t candidate_ratio = (uint64_t)candidate_load * best_capacity;
    uint64_t best_ratio = (uint64_t)best_load * candidate_capacity;
    if (candidate_ratio != best_ratio) return candidate_ratio < best_ratio;
    if (candidate_logical_id != best_logical_id)
        return candidate_logical_id < best_logical_id;
    return candidate_id < best_id;
}

pl_processor_fabric_status pl_processor_fabric_plan(
    const pl_processor_fabric_model *model,
    pl_processor_fabric_workspace *workspace,
    pl_processor_fabric_assignment *assignments,
    size_t assignment_capacity,
    size_t *assignment_count,
    pl_processor_fabric_summary *summary,
    pl_processor_fabric_error *error
) {
    size_t index;
    size_t output_count = 0u;
    pl_processor_fabric_status status;

    if (assignment_count == NULL)
        return fail(error, PL_PROCESSOR_FABRIC_INVALID_ARGUMENT,
                    0u, 0u, 0u, "assignment count output is required");
    *assignment_count = 0u;

    status = pl_processor_fabric_validate(model, workspace, summary, error);
    if (status != PL_PROCESSOR_FABRIC_OK) return status;
    if (summary->ready_process_count > assignment_capacity ||
        (summary->ready_process_count != 0u && assignments == NULL))
        return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                    0u, 0u, 0u, "assignment buffer is smaller than the READY process count");

    memset(workspace->worker_load, 0, sizeof(workspace->worker_load));
    memset(workspace->selected, 0, sizeof(workspace->selected));
    for (index = 0u; index < model->node_count; ++index) {
        const pl_processor_fabric_node *node = &model->nodes[index];
        if (is_worker_kind(node->kind))
            workspace->worker_load[index] = node->reserved_basis_points;
    }

    while (output_count < summary->ready_process_count) {
        size_t task_index = model->node_count;
        size_t worker_index = model->node_count;
        size_t candidate_index;
        int has_capable_worker = 0;

        /* Highest priority first; stable node ID breaks priority ties. */
        for (index = 0u; index < model->node_count; ++index) {
            const pl_processor_fabric_node *candidate = &model->nodes[index];
            const pl_processor_fabric_node *current;
            if (candidate->kind != PL_PROCESSOR_FABRIC_PROCESS ||
                candidate->state != PL_PROCESSOR_FABRIC_READY ||
                workspace->selected[index] != 0u)
                continue;
            if (task_index == model->node_count) {
                task_index = index;
                continue;
            }
            current = &model->nodes[task_index];
            if (candidate->priority > current->priority ||
                (candidate->priority == current->priority && candidate->id < current->id))
                task_index = index;
        }

        if (task_index == model->node_count)
            return fail(error, PL_PROCESSOR_FABRIC_INTERNAL_ERROR,
                        0u, 0u, 0u, "READY process count changed during planning");
        workspace->selected[task_index] = 1u;

        {
            const pl_processor_fabric_node *task = &model->nodes[task_index];
            pl_processor_fabric_assignment *assignment = &assignments[output_count++];
            assignment->task_id = task->id;
            assignment->worker_id = 0u;
            assignment->resulting_load_basis_points = 0u;

            if (has_pending_dependency(model, task->id)) {
                assignment->state = PL_PROCESSOR_FABRIC_WAITING_DEPENDENCY;
                continue;
            }

            for (candidate_index = 0u; candidate_index < model->node_count; ++candidate_index) {
                const pl_processor_fabric_node *worker = &model->nodes[candidate_index];
                uint32_t projected_load;
                if (!worker_supports_task(worker, task)) continue;
                has_capable_worker = 1;
                projected_load = workspace->worker_load[candidate_index] +
                                 task->requested_load_basis_points;
                if (projected_load > worker->capacity_basis_points) continue;

                if (worker_index == model->node_count) {
                    worker_index = candidate_index;
                } else {
                    const pl_processor_fabric_node *best = &model->nodes[worker_index];
                    uint32_t best_projected =
                        workspace->worker_load[worker_index] + task->requested_load_basis_points;
                    if (worker_is_better(projected_load, worker->capacity_basis_points,
                                         worker->logical_id, worker->id,
                                         best_projected, best->capacity_basis_points,
                                         best->logical_id, best->id))
                        worker_index = candidate_index;
                }
            }

            if (worker_index == model->node_count) {
                assignment->state = has_capable_worker
                    ? PL_PROCESSOR_FABRIC_CAPACITY_EXHAUSTED
                    : PL_PROCESSOR_FABRIC_NO_COMPATIBLE_WORKER;
                continue;
            }

            {
                const pl_processor_fabric_node *worker = &model->nodes[worker_index];
                uint32_t projected_load = workspace->worker_load[worker_index] +
                                          task->requested_load_basis_points;
                workspace->worker_load[worker_index] = projected_load;
                assignment->worker_id = worker->id;
                assignment->resulting_load_basis_points = projected_load;
                assignment->state = PL_PROCESSOR_FABRIC_ASSIGNED;
            }
        }
    }

    *assignment_count = output_count;
    return PL_PROCESSOR_FABRIC_OK;
}

pl_processor_fabric_status pl_processor_fabric_collect_catalogues(
    pl_processor_fabric_catalogues *out
) {
    const pl_cpu_suite_module *suite = NULL;
    const pl_hw_construct_info *constructs = NULL;
    const pl_system_sigil_info *sigils = NULL;

    if (out == NULL)
        return PL_PROCESSOR_FABRIC_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));

    out->cpu_suite_modules = (uint32_t)pl_cpu_suite_catalog(&suite);
    out->processor_rules = (uint32_t)pl_processor_catalog_count();
    out->hardware_constructs = (uint32_t)pl_hw_syntax_catalog(&constructs);
    out->system_sigil_operators = (uint32_t)pl_system_sigil_catalog(&sigils);
    out->iron_core_markers = (uint32_t)PL_IRON_MARKER_COUNT;

    if (suite == NULL || constructs == NULL || sigils == NULL)
        return PL_PROCESSOR_FABRIC_INTERNAL_ERROR;
    return PL_PROCESSOR_FABRIC_OK;
}

static int append_json(char *buffer, size_t capacity, size_t *written,
                       const char *format, ...) {
    int result;
    va_list arguments;
    if (*written >= capacity) return 0;
    va_start(arguments, format);
    result = vsnprintf(buffer + *written, capacity - *written, format, arguments);
    va_end(arguments);
    if (result < 0 || (size_t)result >= capacity - *written) return 0;
    *written += (size_t)result;
    return 1;
}

pl_processor_fabric_status pl_processor_fabric_report_json(
    const pl_processor_fabric_summary *summary,
    const pl_processor_fabric_catalogues *catalogues,
    const pl_processor_fabric_assignment *assignments,
    size_t assignment_count,
    char *buffer,
    size_t buffer_capacity,
    size_t *written
) {
    size_t index;
    size_t used = 0u;

    if (written != NULL) *written = 0u;
    if (summary == NULL || catalogues == NULL || buffer == NULL ||
        buffer_capacity == 0u || written == NULL ||
        (assignment_count != 0u && assignments == NULL) ||
        assignment_count > summary->ready_process_count ||
        summary->total_nodes > PL_PROCESSOR_FABRIC_MAX_NODES ||
        summary->total_edges > PL_PROCESSOR_FABRIC_MAX_EDGES)
        return PL_PROCESSOR_FABRIC_INVALID_ARGUMENT;

    buffer[0] = '\0';
    if (!append_json(buffer, buffer_capacity, &used,
        "{\"schema\":\"%s\",\"abi_major\":%u,\"generation\":%u,"
        "\"inventory\":{\"node_count\":%u,\"edge_count\":%u,"
        "\"worker_count\":%u,\"available_worker_count\":%u,"
        "\"process_count\":%u,\"ready_process_count\":%u,"
        "\"completed_process_count\":%u,\"failed_process_count\":%u,"
        "\"dependency_edge_count\":%u,\"resource_edge_count\":%u,"
        "\"produced_edge_count\":%u,"
        "\"worker_capacity_basis_points\":%llu,"
        "\"worker_reserved_basis_points\":%llu},"
        "\"catalogues\":{\"cpu_suite_modules\":%u,\"processor_rules\":%u,"
        "\"hardware_constructs\":%u,\"system_sigil_operators\":%u,"
        "\"iron_core_markers\":%u},\"node_kinds\":{",
        PL_PROCESSOR_FABRIC_SCHEMA, PL_PROCESSOR_FABRIC_ABI_MAJOR,
        summary->generation, summary->total_nodes, summary->total_edges,
        summary->worker_count, summary->available_worker_count,
        summary->process_count, summary->ready_process_count,
        summary->completed_process_count, summary->failed_process_count,
        summary->dependency_edge_count, summary->resource_edge_count,
        summary->produced_edge_count,
        (unsigned long long)summary->total_worker_capacity_basis_points,
        (unsigned long long)summary->total_worker_reserved_basis_points,
        catalogues->cpu_suite_modules, catalogues->processor_rules,
        catalogues->hardware_constructs, catalogues->system_sigil_operators,
        catalogues->iron_core_markers))
        return PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL;

    for (index = 0u; index < PL_PROCESSOR_FABRIC_NODE_KIND_COUNT; ++index) {
        if (!append_json(buffer, buffer_capacity, &used, "%s\"%s\":%u",
            index == 0u ? "" : ",", node_kind_names[index],
            summary->node_kind_counts[index]))
            return PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL;
    }

    if (!append_json(buffer, buffer_capacity, &used, "},\"node_states\":{"))
        return PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL;
    for (index = 0u; index < PL_PROCESSOR_FABRIC_NODE_STATE_COUNT; ++index) {
        if (!append_json(buffer, buffer_capacity, &used, "%s\"%s\":%u",
            index == 0u ? "" : ",", node_state_names[index],
            summary->node_state_counts[index]))
            return PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL;
    }

    if (!append_json(buffer, buffer_capacity, &used, "},\"assignments\":["))
        return PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL;
    for (index = 0u; index < assignment_count; ++index) {
        const pl_processor_fabric_assignment *assignment = &assignments[index];
        if ((unsigned)assignment->state >= ARRAY_COUNT(assignment_state_names))
            return PL_PROCESSOR_FABRIC_INVALID_ARGUMENT;
        if (!append_json(buffer, buffer_capacity, &used,
            "%s{\"task_id\":%u,\"worker_id\":%u,"
            "\"resulting_load_basis_points\":%u,\"state\":\"%s\"}",
            index == 0u ? "" : ",", assignment->task_id, assignment->worker_id,
            assignment->resulting_load_basis_points,
            assignment_state_names[(unsigned)assignment->state]))
            return PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL;
    }

    if (!append_json(buffer, buffer_capacity, &used,
        "],\"execution_performed\":false,"
        "\"processor_control_changed\":false,"
        "\"hardware_access_performed\":false}"))
        return PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL;

    *written = used;
    return PL_PROCESSOR_FABRIC_OK;
}


typedef struct {
    uint32_t key;
    uint32_t node_id;
} topology_node_map_entry;

static uint32_t find_topology_node(const topology_node_map_entry *entries,
                                   size_t count, uint32_t key) {
    for (size_t i = 0u; i < count; ++i)
        if (entries[i].key == key) return entries[i].node_id;
    return 0u;
}

static uint32_t add_topology_node(
    pl_processor_fabric_node *nodes, size_t capacity, size_t *count,
    uint32_t *next_id, uint32_t generation,
    pl_processor_fabric_node_kind kind,
    pl_processor_fabric_node_state state,
    uint32_t parent_id, uint32_t logical_id,
    uint64_t feature_mask, uint32_t capacity_basis_points
) {
    pl_processor_fabric_node *node;
    if (*count >= capacity || *next_id == 0u) return 0u;
    node = &nodes[*count];
    memset(node, 0, sizeof(*node));
    node->id = (*next_id)++;
    node->parent_id = parent_id;
    node->generation = generation;
    node->kind = kind;
    node->state = state;
    node->domain_mask = kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR
        ? PL_PROCESSOR_FABRIC_DOMAIN_HOST : 0u;
    node->feature_mask = feature_mask;
    node->logical_id = logical_id;
    node->capacity_basis_points = capacity_basis_points;
    ++(*count);
    return node->id;
}

static int add_topology_edge(pl_processor_fabric_edge *edges,
                             size_t capacity, size_t *count,
                             uint32_t from_id, uint32_t to_id,
                             pl_processor_fabric_edge_kind kind) {
    if (*count >= capacity || from_id == 0u || to_id == 0u) return 0;
    edges[*count].from_id = from_id;
    edges[*count].to_id = to_id;
    edges[*count].kind = kind;
    ++(*count);
    return 1;
}

pl_processor_fabric_status pl_processor_fabric_from_topology(
    const pl_processor_topology_snapshot *topology,
    pl_processor_fabric_node *nodes, size_t node_capacity, size_t *node_count,
    pl_processor_fabric_edge *edges, size_t edge_capacity, size_t *edge_count,
    pl_processor_fabric_error *error
) {
    topology_node_map_entry packages[PL_TOPOLOGY_MAX_LOGICAL_CPUS];
    topology_node_map_entry dies[PL_TOPOLOGY_MAX_LOGICAL_CPUS];
    topology_node_map_entry cores[PL_TOPOLOGY_MAX_LOGICAL_CPUS];
    topology_node_map_entry groups[PL_TOPOLOGY_MAX_LOGICAL_CPUS];
    topology_node_map_entry numa[PL_TOPOLOGY_MAX_NUMA_NODES];
    topology_node_map_entry caches[PL_TOPOLOGY_MAX_CACHES];
    size_t package_count = 0u, die_count = 0u, core_count = 0u;
    size_t group_count = 0u, numa_count = 0u, cache_count = 0u;
    size_t used_nodes = 0u, used_edges = 0u;
    uint32_t next_id = 1u;

    clear_error(error);
    if (node_count == NULL || edge_count == NULL || topology == NULL ||
        nodes == NULL || (edge_capacity != 0u && edges == NULL))
        return fail(error, PL_PROCESSOR_FABRIC_INVALID_ARGUMENT,
                    0u, 0u, 0u, "topology and caller-owned graph buffers are required");
    *node_count = 0u;
    *edge_count = 0u;
    if (pl_processor_topology_validate(topology) != PL_TOPOLOGY_OK)
        return fail(error, PL_PROCESSOR_FABRIC_INVALID_GENERATION,
                    0u, 0u, 0u, "topology evidence is invalid");
    if (topology->logical_cpu_count > 64u)
        return fail(error, PL_PROCESSOR_FABRIC_LIMIT_EXCEEDED,
                    0u, 0u, 0u, "PPF v1 worker affinity is limited to 64 logical CPUs");
    if (node_capacity > PL_PROCESSOR_FABRIC_MAX_NODES)
        node_capacity = PL_PROCESSOR_FABRIC_MAX_NODES;
    if (edge_capacity > PL_PROCESSOR_FABRIC_MAX_EDGES)
        edge_capacity = PL_PROCESSOR_FABRIC_MAX_EDGES;
    if (node_capacity == 0u)
        return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                    0u, 0u, 0u, "node output buffer is empty");

    memset(packages, 0, sizeof(packages));
    memset(dies, 0, sizeof(dies));
    memset(cores, 0, sizeof(cores));
    memset(groups, 0, sizeof(groups));
    memset(numa, 0, sizeof(numa));
    memset(caches, 0, sizeof(caches));

    for (size_t i = 0u; i < topology->logical_cpu_count; ++i) {
        const pl_topology_logical_cpu *cpu = &topology->logical_cpus[i];
        uint32_t package_id = 0u, die_id = 0u, core_id = 0u;
        uint32_t group_id = 0u, numa_id = 0u;
        uint64_t feature_mask = 0u;
        uint32_t cpu_node_id;
        pl_processor_fabric_node_state cpu_state =
            cpu->online != 0u ? PL_PROCESSOR_FABRIC_AVAILABLE
                              : PL_PROCESSOR_FABRIC_DISABLED;

        if (cpu->logical_id >= 64u)
            return fail(error, PL_PROCESSOR_FABRIC_LIMIT_EXCEEDED,
                        cpu->logical_id, 0u, 0u,
                        "PPF v1 cannot encode a logical processor ID >= 64");
        for (uint32_t feature = 0u; feature < PL_TOPOLOGY_ISA_FEATURE_COUNT; ++feature)
            if (topology->isa[feature].os_usable == 1)
                feature_mask |= UINT64_C(1) << feature;

        if (cpu->package_id != PL_TOPOLOGY_UNKNOWN) {
            package_id = find_topology_node(packages, package_count, cpu->package_id);
            if (package_id == 0u) {
                package_id = add_topology_node(nodes, node_capacity, &used_nodes,
                    &next_id, topology->generation, PL_PROCESSOR_FABRIC_PACKAGE,
                    PL_PROCESSOR_FABRIC_VERIFIED, 0u, PL_TOPOLOGY_UNKNOWN, 0u, 0u);
                if (package_id == 0u)
                    return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                                cpu->logical_id, 0u, 0u, "node buffer exhausted at package");
                packages[package_count++] = (topology_node_map_entry){cpu->package_id, package_id};
            }
        }
        if (cpu->die_id != PL_TOPOLOGY_UNKNOWN) {
            die_id = find_topology_node(dies, die_count, cpu->die_id);
            if (die_id == 0u) {
                die_id = add_topology_node(nodes, node_capacity, &used_nodes,
                    &next_id, topology->generation, PL_PROCESSOR_FABRIC_DIE,
                    PL_PROCESSOR_FABRIC_VERIFIED, package_id, PL_TOPOLOGY_UNKNOWN, 0u, 0u);
                if (die_id == 0u)
                    return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                                cpu->logical_id, 0u, 0u, "node buffer exhausted at die");
                dies[die_count++] = (topology_node_map_entry){cpu->die_id, die_id};
            }
        }
        if (cpu->core_id != PL_TOPOLOGY_UNKNOWN) {
            /* Linux core IDs are scoped to package/die: key the map by source tuple. */
            uint32_t key = cpu->core_id ^ (cpu->package_id * UINT32_C(2654435761)) ^
                           (cpu->die_id * UINT32_C(2246822519));
            core_id = find_topology_node(cores, core_count, key);
            if (core_id == 0u) {
                uint32_t parent = die_id != 0u ? die_id : package_id;
                core_id = add_topology_node(nodes, node_capacity, &used_nodes,
                    &next_id, topology->generation, PL_PROCESSOR_FABRIC_CORE,
                    PL_PROCESSOR_FABRIC_VERIFIED, parent, PL_TOPOLOGY_UNKNOWN, 0u, 0u);
                if (core_id == 0u)
                    return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                                cpu->logical_id, 0u, 0u, "node buffer exhausted at core");
                cores[core_count++] = (topology_node_map_entry){key, core_id};
            }
        }

        cpu_node_id = add_topology_node(nodes, node_capacity, &used_nodes,
            &next_id, topology->generation, PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR,
            cpu_state, core_id, cpu->logical_id, feature_mask,
            cpu->online != 0u ? PL_PROCESSOR_FABRIC_BASIS_POINTS : 0u);
        if (cpu_node_id == 0u)
            return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                        cpu->logical_id, 0u, 0u, "node buffer exhausted at logical CPU");
        nodes[used_nodes - 1u].affinity_mask = UINT64_C(1) << cpu->logical_id;

        if (cpu->group_id != PL_TOPOLOGY_UNKNOWN) {
            group_id = find_topology_node(groups, group_count, cpu->group_id);
            if (group_id == 0u) {
                group_id = add_topology_node(nodes, node_capacity, &used_nodes,
                    &next_id, topology->generation, PL_PROCESSOR_FABRIC_PROCESSOR_GROUP,
                    PL_PROCESSOR_FABRIC_VERIFIED, 0u, PL_TOPOLOGY_UNKNOWN, 0u, 0u);
                if (group_id == 0u)
                    return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                                cpu->logical_id, 0u, 0u, "node buffer exhausted at processor group");
                groups[group_count++] = (topology_node_map_entry){cpu->group_id, group_id};
            }
            if (!add_topology_edge(edges, edge_capacity, &used_edges,
                                   cpu_node_id, group_id, PL_PROCESSOR_FABRIC_MAPS_TO))
                return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                            cpu_node_id, cpu_node_id, group_id, "edge buffer exhausted at processor group");
        }
        if (cpu->numa_node_id != PL_TOPOLOGY_UNKNOWN) {
            numa_id = find_topology_node(numa, numa_count, cpu->numa_node_id);
            if (numa_id == 0u) {
                numa_id = add_topology_node(nodes, node_capacity, &used_nodes,
                    &next_id, topology->generation, PL_PROCESSOR_FABRIC_NUMA_NODE,
                    PL_PROCESSOR_FABRIC_VERIFIED, 0u, PL_TOPOLOGY_UNKNOWN, 0u, 0u);
                if (numa_id == 0u)
                    return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                                cpu->logical_id, 0u, 0u, "node buffer exhausted at NUMA node");
                numa[numa_count++] = (topology_node_map_entry){cpu->numa_node_id, numa_id};
            }
            if (!add_topology_edge(edges, edge_capacity, &used_edges,
                                   cpu_node_id, numa_id, PL_PROCESSOR_FABRIC_MAPS_TO))
                return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                            cpu_node_id, cpu_node_id, numa_id, "edge buffer exhausted at NUMA node");
        }
    }

    for (size_t i = 0u; i < topology->cache_count; ++i) {
        const pl_topology_cache *cache = &topology->caches[i];
        uint32_t cache_node = add_topology_node(nodes, node_capacity, &used_nodes,
            &next_id, topology->generation, PL_PROCESSOR_FABRIC_CACHE,
            PL_PROCESSOR_FABRIC_VERIFIED, 0u, PL_TOPOLOGY_UNKNOWN, 0u, 0u);
        if (cache_node == 0u)
            return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                        cache->cache_id, 0u, 0u, "node buffer exhausted at cache");
        caches[cache_count++] = (topology_node_map_entry){cache->cache_id, cache_node};
        for (uint32_t cpu = 0u; cpu < PL_TOPOLOGY_MAX_LOGICAL_CPUS; ++cpu) {
            if ((cache->sharing_mask[cpu / 64u] & (UINT64_C(1) << (cpu % 64u))) != 0u) {
                for (size_t n = 0u; n < used_nodes; ++n) {
                    if (nodes[n].kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR &&
                        nodes[n].logical_id == cpu) {
                        if (!add_topology_edge(edges, edge_capacity, &used_edges,
                                nodes[n].id, cache_node, PL_PROCESSOR_FABRIC_SHARES_CACHE))
                            return fail(error, PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
                                        nodes[n].id, nodes[n].id, cache_node, "edge buffer exhausted at cache");
                    }
                }
            }
        }
    }

    *node_count = used_nodes;
    *edge_count = used_edges;
    return PL_PROCESSOR_FABRIC_OK;
}

const char *pl_processor_fabric_status_name(pl_processor_fabric_status status) {
    switch (status) {
    case PL_PROCESSOR_FABRIC_OK: return "ok";
    case PL_PROCESSOR_FABRIC_INVALID_ARGUMENT: return "invalid-argument";
    case PL_PROCESSOR_FABRIC_LIMIT_EXCEEDED: return "limit-exceeded";
    case PL_PROCESSOR_FABRIC_DUPLICATE_ID: return "duplicate-id";
    case PL_PROCESSOR_FABRIC_MISSING_REFERENCE: return "missing-reference";
    case PL_PROCESSOR_FABRIC_INVALID_PARENT: return "invalid-parent";
    case PL_PROCESSOR_FABRIC_PARENT_CYCLE: return "parent-cycle";
    case PL_PROCESSOR_FABRIC_INVALID_EDGE: return "invalid-edge";
    case PL_PROCESSOR_FABRIC_DUPLICATE_EDGE: return "duplicate-edge";
    case PL_PROCESSOR_FABRIC_DEPENDENCY_CYCLE: return "dependency-cycle";
    case PL_PROCESSOR_FABRIC_INVALID_GENERATION: return "invalid-generation";
    case PL_PROCESSOR_FABRIC_INVALID_CAPACITY: return "invalid-capacity";
    case PL_PROCESSOR_FABRIC_INVALID_DOMAIN: return "invalid-domain";
    case PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL: return "output-too-small";
    case PL_PROCESSOR_FABRIC_INTERNAL_ERROR: return "internal-error";
    default: return "unknown";
    }
}

const char *pl_processor_fabric_node_kind_name(pl_processor_fabric_node_kind kind) {
    if ((unsigned)kind >= ARRAY_COUNT(node_kind_names)) return "unknown";
    return node_kind_names[(unsigned)kind];
}

const char *pl_processor_fabric_node_state_name(pl_processor_fabric_node_state state) {
    if ((unsigned)state >= ARRAY_COUNT(node_state_names)) return "unknown";
    return node_state_names[(unsigned)state];
}

const char *pl_processor_fabric_edge_kind_name(pl_processor_fabric_edge_kind kind) {
    if ((unsigned)kind >= ARRAY_COUNT(edge_kind_names)) return "unknown";
    return edge_kind_names[(unsigned)kind];
}

const char *pl_processor_fabric_assignment_state_name(
    pl_processor_fabric_assignment_state state
) {
    if ((unsigned)state >= ARRAY_COUNT(assignment_state_names)) return "unknown";
    return assignment_state_names[(unsigned)state];
}
