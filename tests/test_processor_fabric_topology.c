#include "processor_fabric.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void set_cpu(pl_topology_logical_cpu *cpu, uint32_t logical_id,
                    uint32_t os_id, uint8_t online, uint32_t package_id,
                    uint32_t die_id, uint32_t core_id, uint32_t numa_id,
                    uint32_t group_id) {
    memset(cpu, 0, sizeof(*cpu));
    cpu->logical_id = logical_id;
    cpu->os_id = os_id;
    cpu->group_id = group_id;
    cpu->package_id = package_id;
    cpu->die_id = die_id;
    cpu->core_id = core_id;
    cpu->numa_node_id = numa_id;
    cpu->online = online;
    cpu->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
}

int main(void) {
    pl_processor_topology_snapshot topology;
    pl_processor_fabric_node nodes[PL_PROCESSOR_FABRIC_MAX_NODES];
    pl_processor_fabric_edge edges[PL_PROCESSOR_FABRIC_MAX_EDGES];
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    pl_processor_fabric_model model;
    pl_processor_fabric_assignment assignments[1];
    pl_placement_task placement_tasks[3];
    pl_placement_pressure pressure;
    pl_placement_result placement;
    size_t node_count = 0u, edge_count = 0u, assignment_count = 0u;

    memset(&topology, 0, sizeof(topology));
    topology.generation = 42u;
    topology.logical_cpu_count = 4u;
    topology.online_logical_cpu_count = 3u;
    topology.cache_count = 1u;
    topology.numa_node_count = 1u;
    topology.completeness = PL_TOPOLOGY_PARTIAL;
    (void)snprintf(topology.architecture, sizeof(topology.architecture), "x86-64");
    set_cpu(&topology.logical_cpus[0], 2u, 2u, 1u, 0u, 0u, 3u, 0u, 1u);
    set_cpu(&topology.logical_cpus[1], 5u, 5u, 1u, 0u, 0u, 3u, 0u, 1u);
    set_cpu(&topology.logical_cpus[2], 8u, 8u, 0u,
            PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN,
            PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN,
            PL_TOPOLOGY_UNKNOWN);
    set_cpu(&topology.logical_cpus[3], 130u, 130u, 1u,
            PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN,
            PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN,
            PL_TOPOLOGY_UNKNOWN);
    topology.caches[0].cache_id = 0u;
    topology.caches[0].level = 3u;
    topology.caches[0].kind = PL_TOPOLOGY_CACHE_UNIFIED;
    topology.caches[0].size_bytes = UINT64_C(8) * 1024u * 1024u;
    topology.caches[0].line_bytes = 64u;
    topology.caches[0].sharing_mask[0] =
        (UINT64_C(1) << 2u) | (UINT64_C(1) << 5u);
    topology.caches[0].sharing_count = 2u;
    topology.caches[0].evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
    topology.numa_nodes[0].node_id = 0u;
    topology.numa_nodes[0].logical_mask[0] =
        (UINT64_C(1) << 2u) | (UINT64_C(1) << 5u);
    topology.numa_nodes[0].logical_count = 2u;
    topology.numa_nodes[0].memory_total_bytes = UINT64_C(8) * 1024u * 1024u * 1024u;
    topology.numa_nodes[0].memory_available_bytes = UINT64_C(4) * 1024u * 1024u * 1024u;
    topology.numa_nodes[0].memory_total_observed = 1u;
    topology.numa_nodes[0].memory_available_observed = 1u;
    topology.numa_nodes[0].evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
    topology.isa[PL_TOPOLOGY_ISA_SSE2].hardware_present = 1;
    topology.isa[PL_TOPOLOGY_ISA_SSE2].os_usable = 1;
    topology.isa[PL_TOPOLOGY_ISA_SSE2].evidence = 1u;

    assert(pl_processor_topology_validate(&topology) == PL_TOPOLOGY_OK);
    assert(pl_processor_fabric_from_topology(
        &topology, nodes, PL_PROCESSOR_FABRIC_MAX_NODES, &node_count,
        edges, PL_PROCESSOR_FABRIC_MAX_EDGES, &edge_count, &error)
        == PL_PROCESSOR_FABRIC_OK);
    assert(node_count >= 8u);
    assert(edge_count >= 6u);

    memset(&model, 0, sizeof(model));
    model.generation = topology.generation;
    model.abi_major = PL_PROCESSOR_FABRIC_ABI_MAJOR;
    model.node_count = node_count;
    model.edge_count = edge_count;
    model.nodes = nodes;
    model.edges = edges;
    assert(pl_processor_fabric_validate(&model, &workspace, &summary, &error)
           == PL_PROCESSOR_FABRIC_OK);
    assert(summary.worker_count == 4u);
    assert(summary.available_worker_count == 3u);
    assert(summary.node_kind_counts[PL_PROCESSOR_FABRIC_CACHE] == 1u);
    assert(summary.node_kind_counts[PL_PROCESSOR_FABRIC_NUMA_NODE] == 1u);
    assert(summary.resource_edge_count == edge_count);

    {
        int saw_sparse_cpu_2 = 0, saw_sparse_cpu_5 = 0, saw_offline_root = 0;
        int saw_cpu_130 = 0;
        for (size_t i = 0u; i < node_count; ++i) {
            if (nodes[i].kind != PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR) continue;
            if (nodes[i].logical_id == 2u) {
                saw_sparse_cpu_2 = 1;
                assert(nodes[i].state == PL_PROCESSOR_FABRIC_AVAILABLE);
                assert(nodes[i].feature_mask == (UINT64_C(1) << PL_TOPOLOGY_ISA_SSE2));
            } else if (nodes[i].logical_id == 5u) {
                saw_sparse_cpu_5 = 1;
            } else if (nodes[i].logical_id == 8u) {
                saw_offline_root = 1;
                assert(nodes[i].state == PL_PROCESSOR_FABRIC_DISABLED);
                assert(nodes[i].capacity_basis_points == 0u);
                assert(nodes[i].parent_id == 0u);
            } else if (nodes[i].logical_id == 130u) {
                saw_cpu_130 = 1;
                assert(nodes[i].state == PL_PROCESSOR_FABRIC_AVAILABLE);
                assert(nodes[i].affinity_mask[2] == (UINT64_C(1) << 2u));
            }
        }
        assert(saw_sparse_cpu_2 && saw_sparse_cpu_5 && saw_offline_root && saw_cpu_130);
    }
    assert(pl_processor_fabric_plan(&model, &workspace, assignments, 1u,
        &assignment_count, &summary, &error) == PL_PROCESSOR_FABRIC_OK);
    assert(assignment_count == 0u);

    /* Extend the inventory with a process pool and a dependent READY process. */
    {
        pl_processor_fabric_node pool;
        pl_processor_fabric_node ready_local;
        pl_processor_fabric_node ready_waiting;
        pl_processor_fabric_node prerequisite;
        pl_processor_fabric_node ready_wide;
        pl_processor_fabric_edge dependency;
        memset(&pool, 0, sizeof(pool));
        pool.id = 100u; pool.generation = topology.generation;
        pool.kind = PL_PROCESSOR_FABRIC_PROCESS_POOL;
        pool.state = PL_PROCESSOR_FABRIC_DECLARED;
        memset(&ready_local, 0, sizeof(ready_local));
        ready_local.id = 101u; ready_local.parent_id = pool.id;
        ready_local.generation = topology.generation;
        ready_local.kind = PL_PROCESSOR_FABRIC_PROCESS;
        ready_local.state = PL_PROCESSOR_FABRIC_READY;
        ready_local.domain_mask = PL_PROCESSOR_FABRIC_DOMAIN_HOST;
        ready_local.feature_mask = UINT64_C(1) << PL_TOPOLOGY_ISA_SSE2;
        ready_local.affinity_mask[0] = UINT64_C(1) << 2u;
        ready_local.requested_load_basis_points = 1200u;
        ready_local.priority = 5u;
        memset(&ready_waiting, 0, sizeof(ready_waiting));
        ready_waiting.id = 102u; ready_waiting.parent_id = pool.id;
        ready_waiting.generation = topology.generation;
        ready_waiting.kind = PL_PROCESSOR_FABRIC_PROCESS;
        ready_waiting.state = PL_PROCESSOR_FABRIC_READY;
        ready_waiting.domain_mask = PL_PROCESSOR_FABRIC_DOMAIN_HOST;
        ready_waiting.feature_mask = UINT64_C(1) << PL_TOPOLOGY_ISA_SSE2;
        ready_waiting.requested_load_basis_points = 900u;
        ready_waiting.priority = 9u;
        memset(&prerequisite, 0, sizeof(prerequisite));
        prerequisite.id = 103u; prerequisite.parent_id = pool.id;
        prerequisite.generation = topology.generation;
        prerequisite.kind = PL_PROCESSOR_FABRIC_PROCESS;
        prerequisite.state = PL_PROCESSOR_FABRIC_DECLARED;
        prerequisite.domain_mask = PL_PROCESSOR_FABRIC_DOMAIN_HOST;
        memset(&ready_wide, 0, sizeof(ready_wide));
        ready_wide.id = 104u; ready_wide.parent_id = pool.id;
        ready_wide.generation = topology.generation;
        ready_wide.kind = PL_PROCESSOR_FABRIC_PROCESS;
        ready_wide.state = PL_PROCESSOR_FABRIC_READY;
        ready_wide.domain_mask = PL_PROCESSOR_FABRIC_DOMAIN_HOST;
        ready_wide.affinity_mask[2] = UINT64_C(1) << 2u;
        ready_wide.requested_load_basis_points = 500u;
        ready_wide.priority = 4u;
        dependency.from_id = prerequisite.id;
        dependency.to_id = ready_waiting.id;
        dependency.kind = PL_PROCESSOR_FABRIC_DEPENDS_ON;
        nodes[node_count++] = pool;
        nodes[node_count++] = ready_local;
        nodes[node_count++] = ready_waiting;
        nodes[node_count++] = prerequisite;
        nodes[node_count++] = ready_wide;
        edges[edge_count++] = dependency;
        model.node_count = node_count;
        model.edge_count = edge_count;

        memset(placement_tasks, 0, sizeof(placement_tasks));
        placement_tasks[0].task_id = ready_local.id;
        placement_tasks[0].preferred_numa_node_id = 0u;
        placement_tasks[0].working_set_bytes = 1024u * 1024u;
        placement_tasks[0].memory_bandwidth_bytes_per_second = 1000000u;
        placement_tasks[0].require_local_numa = 1u;
        placement_tasks[1].task_id = ready_waiting.id;
        placement_tasks[1].preferred_numa_node_id = 0u;
        placement_tasks[1].working_set_bytes = 2048u;
        placement_tasks[1].require_local_numa = 1u;
        placement_tasks[2].task_id = ready_wide.id;
        placement_tasks[2].preferred_numa_node_id = PL_TOPOLOGY_UNKNOWN;
        memset(&pressure, 0, sizeof(pressure));
        pressure.generation = topology.generation;
        pressure.numa_bandwidth_capacity_bytes_per_second[0] = 100000000u;
        assert(pl_processor_fabric_place_v2(
            &model, &topology, placement_tasks, 3u, &pressure, &placement,
            &workspace, &summary, &error) == PL_PROCESSOR_FABRIC_OK);
        assert(placement.assignment_count == 3u);
        assert(placement.assignments[0].task_id == ready_waiting.id);
        assert(placement.assignments[0].state == PL_PLACEMENT_WAITING_DEPENDENCY);
        assert(placement.assignments[1].task_id == ready_local.id);
        assert(placement.assignments[1].state == PL_PLACEMENT_ASSIGNED);
        assert(placement.assignments[1].logical_cpu_id == 2u);
        assert(placement.assignments[2].task_id == ready_wide.id);
        assert(placement.assignments[2].state == PL_PLACEMENT_ASSIGNED);
        assert(placement.assignments[2].logical_cpu_id == 130u);
        assert(placement.report.execution_performed == 0u);
        assert(placement.report.affinity_changed == 0u);
        assert(placement.report.scheduling_changed == 0u);
    }

    puts("processor fabric topology bridge tests passed");
    return 0;
}
