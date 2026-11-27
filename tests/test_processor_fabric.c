#include "processor_fabric.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HOST_DOMAIN PL_PROCESSOR_FABRIC_DOMAIN_HOST
#define SIM_DOMAIN PL_PROCESSOR_FABRIC_DOMAIN_SIMULATION

static pl_processor_fabric_node make_node(
    uint32_t id, uint32_t parent_id, pl_processor_fabric_node_kind kind,
    pl_processor_fabric_node_state state
) {
    pl_processor_fabric_node node;
    memset(&node, 0, sizeof(node));
    node.id = id;
    node.parent_id = parent_id;
    node.generation = 7u;
    node.kind = kind;
    node.state = state;
    return node;
}

static void make_fixture(
    pl_processor_fabric_node nodes[9],
    pl_processor_fabric_edge edges[1],
    pl_processor_fabric_model *model
) {
    nodes[0] = make_node(1u, 0u, PL_PROCESSOR_FABRIC_PACKAGE,
                         PL_PROCESSOR_FABRIC_VERIFIED);
    nodes[1] = make_node(2u, 1u, PL_PROCESSOR_FABRIC_DIE,
                         PL_PROCESSOR_FABRIC_VERIFIED);
    nodes[2] = make_node(3u, 2u, PL_PROCESSOR_FABRIC_CORE,
                         PL_PROCESSOR_FABRIC_VERIFIED);

    nodes[3] = make_node(4u, 3u, PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR,
                         PL_PROCESSOR_FABRIC_AVAILABLE);
    nodes[3].logical_id = 0u;
    nodes[3].domain_mask = HOST_DOMAIN | SIM_DOMAIN;
    nodes[3].feature_mask = UINT64_C(0x0f);
    nodes[3].capacity_basis_points = 7000u;
    nodes[3].reserved_basis_points = 1000u;

    nodes[4] = make_node(5u, 3u, PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR,
                         PL_PROCESSOR_FABRIC_AVAILABLE);
    nodes[4].logical_id = 1u;
    nodes[4].domain_mask = HOST_DOMAIN;
    nodes[4].feature_mask = UINT64_C(0x07);
    nodes[4].capacity_basis_points = 6000u;
    nodes[4].reserved_basis_points = 500u;

    nodes[5] = make_node(6u, 1u, PL_PROCESSOR_FABRIC_PROCESS_POOL,
                         PL_PROCESSOR_FABRIC_VERIFIED);

    nodes[6] = make_node(10u, 6u, PL_PROCESSOR_FABRIC_PROCESS,
                         PL_PROCESSOR_FABRIC_READY);
    nodes[6].domain_mask = HOST_DOMAIN;
    nodes[6].feature_mask = UINT64_C(0x03);
    nodes[6].affinity_mask = UINT64_C(0x03);
    nodes[6].requested_load_basis_points = 3000u;
    nodes[6].priority = 9u;

    nodes[7] = make_node(11u, 6u, PL_PROCESSOR_FABRIC_PROCESS,
                         PL_PROCESSOR_FABRIC_READY);
    nodes[7].domain_mask = HOST_DOMAIN;
    nodes[7].feature_mask = UINT64_C(0x08);
    nodes[7].affinity_mask = UINT64_C(0x01);
    nodes[7].requested_load_basis_points = 2000u;
    nodes[7].priority = 5u;

    nodes[8] = make_node(12u, 6u, PL_PROCESSOR_FABRIC_PROCESS,
                         PL_PROCESSOR_FABRIC_READY);
    nodes[8].domain_mask = SIM_DOMAIN;
    nodes[8].feature_mask = 0u;
    nodes[8].requested_load_basis_points = 500u;
    nodes[8].priority = 1u;

    edges[0].from_id = 10u;
    edges[0].to_id = 12u;
    edges[0].kind = PL_PROCESSOR_FABRIC_DEPENDS_ON;

    model->generation = 7u;
    model->abi_major = PL_PROCESSOR_FABRIC_ABI_MAJOR;
    model->node_count = 9u;
    model->edge_count = 1u;
    model->nodes = nodes;
    model->edges = edges;
}

static void test_inventory_and_catalogue_aggregation(void) {
    pl_processor_fabric_node nodes[9];
    pl_processor_fabric_edge edges[1];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    pl_processor_fabric_catalogues catalogues;

    make_fixture(nodes, edges, &model);
    assert(pl_processor_fabric_validate(&model, &workspace, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(summary.total_nodes == 9u);
    assert(summary.total_edges == 1u);
    assert(summary.worker_count == 2u);
    assert(summary.available_worker_count == 2u);
    assert(summary.process_count == 3u);
    assert(summary.ready_process_count == 3u);
    assert(summary.dependency_edge_count == 1u);
    assert(summary.total_worker_capacity_basis_points == 13000u);
    assert(summary.total_worker_reserved_basis_points == 1500u);

    assert(pl_processor_fabric_collect_catalogues(&catalogues) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(catalogues.cpu_suite_modules == 78u);
    assert(catalogues.processor_rules == 3u);
    assert(catalogues.hardware_constructs == 20u);
    assert(catalogues.system_sigil_operators == 13u);
    assert(catalogues.iron_core_markers == 20u);
}

static void test_deterministic_process_pool_plan(void) {
    pl_processor_fabric_node nodes[9];
    pl_processor_fabric_edge edges[1];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    pl_processor_fabric_assignment assignments[3];
    size_t assignment_count = 0u;

    make_fixture(nodes, edges, &model);
    assert(pl_processor_fabric_plan(&model, &workspace, assignments, 3u,
                                    &assignment_count, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(assignment_count == 3u);

    assert(assignments[0].task_id == 10u);
    assert(assignments[0].worker_id == 4u);
    assert(assignments[0].state == PL_PROCESSOR_FABRIC_ASSIGNED);
    assert(assignments[0].resulting_load_basis_points == 4000u);

    assert(assignments[1].task_id == 11u);
    assert(assignments[1].worker_id == 4u);
    assert(assignments[1].state == PL_PROCESSOR_FABRIC_ASSIGNED);
    assert(assignments[1].resulting_load_basis_points == 6000u);

    assert(assignments[2].task_id == 12u);
    assert(assignments[2].worker_id == 0u);
    assert(assignments[2].state == PL_PROCESSOR_FABRIC_WAITING_DEPENDENCY);

    /* Planning must not mutate the inventory, process states or reserved load. */
    assert(nodes[3].reserved_basis_points == 1000u);
    assert(nodes[4].reserved_basis_points == 500u);
    assert(nodes[6].state == PL_PROCESSOR_FABRIC_READY);
}

static void test_dependency_cycle_is_rejected(void) {
    pl_processor_fabric_node nodes[9];
    pl_processor_fabric_edge edges[2];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;

    make_fixture(nodes, edges, &model);
    edges[1].from_id = 12u;
    edges[1].to_id = 10u;
    edges[1].kind = PL_PROCESSOR_FABRIC_DEPENDS_ON;
    model.edge_count = 2u;
    assert(pl_processor_fabric_validate(&model, &workspace, &summary, &error) ==
           PL_PROCESSOR_FABRIC_DEPENDENCY_CYCLE);
    assert(error.status == PL_PROCESSOR_FABRIC_DEPENDENCY_CYCLE);
}

static void test_invalid_parent_and_duplicate_ids_are_rejected(void) {
    pl_processor_fabric_node nodes[9];
    pl_processor_fabric_edge edges[1];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;

    make_fixture(nodes, edges, &model);
    nodes[2].parent_id = 999u;
    assert(pl_processor_fabric_validate(&model, &workspace, &summary, &error) ==
           PL_PROCESSOR_FABRIC_MISSING_REFERENCE);

    make_fixture(nodes, edges, &model);
    nodes[4].id = nodes[3].id;
    assert(pl_processor_fabric_validate(&model, &workspace, &summary, &error) ==
           PL_PROCESSOR_FABRIC_DUPLICATE_ID);
}

static void test_capacity_and_feature_constraints(void) {
    pl_processor_fabric_node nodes[9];
    pl_processor_fabric_edge edges[1];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    pl_processor_fabric_assignment assignments[3];
    size_t assignment_count = 0u;

    make_fixture(nodes, edges, &model);
    nodes[7].requested_load_basis_points = 6500u;
    assert(pl_processor_fabric_plan(&model, &workspace, assignments, 3u,
                                    &assignment_count, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(assignments[1].task_id == 11u);
    assert(assignments[1].state == PL_PROCESSOR_FABRIC_CAPACITY_EXHAUSTED);

    make_fixture(nodes, edges, &model);
    nodes[3].feature_mask = 0u;
    nodes[4].feature_mask = 0u;
    assert(pl_processor_fabric_plan(&model, &workspace, assignments, 3u,
                                    &assignment_count, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(assignments[0].task_id == 10u);
    assert(assignments[0].state == PL_PROCESSOR_FABRIC_NO_COMPATIBLE_WORKER);
}

static void test_small_output_buffer_is_rejected(void) {
    pl_processor_fabric_node nodes[9];
    pl_processor_fabric_edge edges[1];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    pl_processor_fabric_assignment assignments[2];
    size_t assignment_count = 99u;

    make_fixture(nodes, edges, &model);
    assert(pl_processor_fabric_plan(&model, &workspace, assignments, 2u,
                                    &assignment_count, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL);
    assert(assignment_count == 0u);
}

static void test_report_is_versioned_and_non_executing(void) {
    pl_processor_fabric_node nodes[9];
    pl_processor_fabric_edge edges[1];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    pl_processor_fabric_catalogues catalogues;
    pl_processor_fabric_assignment assignments[3];
    char json[8192];
    size_t assignment_count = 0u;
    size_t written = 0u;

    make_fixture(nodes, edges, &model);
    assert(pl_processor_fabric_plan(&model, &workspace, assignments, 3u,
                                    &assignment_count, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(pl_processor_fabric_collect_catalogues(&catalogues) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(pl_processor_fabric_report_json(&summary, &catalogues,
                                           assignments, assignment_count,
                                           json, sizeof(json), &written) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(written == strlen(json));
    assert(strstr(json, "pytyliki.processor-fabric-report.v1") != NULL);
    assert(strstr(json, "\"cpu_suite_modules\":78") != NULL);
    assert(strstr(json, "\"hardware_access_performed\":false") != NULL);
    assert(strstr(json, "\"processor_control_changed\":false") != NULL);
    assert(strstr(json, "\"execution_performed\":false") != NULL);
}


static void test_host_topology_builds_ppf_hierarchy(void) {
    pl_processor_topology_snapshot topology;
    pl_processor_fabric_node nodes[PL_PROCESSOR_FABRIC_MAX_NODES];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    memset(&topology, 0, sizeof(topology));
    topology.generation = 11u;
    topology.logical_processor_count = 2u;
    topology.online_logical_processor_count = 2u;
    topology.isa_os_usable = UINT64_C(0x25);
    topology.logical_processors[0].logical_id = 0u; cpus[0].os_id = 2u;
    cpus[0].package_id = 1u; cpus[0].die_id = 0u; cpus[0].core_id = 4u;
    cpus[0].thread_id = 0u; cpus[0].numa_node_id = 0u; cpus[0].online = 1u;
    topology.logical_processors[1].logical_id = 1u; cpus[1].os_id = 6u;
    cpus[1].package_id = 1u; cpus[1].die_id = 0u; cpus[1].core_id = 4u;
    cpus[1].thread_id = 1u; cpus[1].numa_node_id = 0u; cpus[1].online = 1u;
    assert(pl_processor_fabric_build_host_model(
        &topology, nodes, PL_PROCESSOR_FABRIC_MAX_NODES,
        &model, &error) == PL_PROCESSOR_FABRIC_OK);
    assert(model.generation == 11u);
    assert(model.node_count == 5u);
    assert(pl_processor_fabric_validate(&model, &workspace, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OK);
    assert(summary.worker_count == 2u);
    assert(summary.available_worker_count == 2u);
    assert(nodes[3].kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR);
    assert(nodes[4].parent_id == nodes[3].parent_id);
    assert(nodes[3].feature_mask == UINT64_C(0x25));
}

static void test_unknown_parent_topology_does_not_fabricate_hierarchy(void) {
    pl_processor_topology_snapshot topology;
    pl_processor_fabric_node nodes[PL_PROCESSOR_FABRIC_MAX_NODES];
    pl_processor_fabric_model model;
    pl_processor_fabric_workspace workspace;
    pl_processor_fabric_summary summary;
    pl_processor_fabric_error error;
    memset(&topology, 0, sizeof(topology));
    topology.generation = 12u;
    topology.logical_processor_count = 1u;
    topology.logical_processor_count = 1u;
    topology.logical_processors[0].logical_id = 0u; topology.logical_processors[0].os_id = 7u; topology.logical_processors[0].online = 1u;
    topology.logical_processors[0].package_id = PL_TOPOLOGY_UNKNOWN_ID;
    topology.logical_processors[0].die_id = PL_TOPOLOGY_UNKNOWN_ID;
    topology.logical_processors[0].core_id = PL_TOPOLOGY_UNKNOWN_ID;
    topology.logical_processors[0].thread_id = PL_TOPOLOGY_UNKNOWN_ID;
    topology.logical_processors[0].numa_node_id = PL_TOPOLOGY_UNKNOWN_ID;
    assert(pl_processor_fabric_build_host_model(
        &topology, nodes, PL_PROCESSOR_FABRIC_MAX_NODES,
        &model, &error) == PL_PROCESSOR_FABRIC_OK);
    assert(model.node_count == 1u);
    assert(nodes[0].kind == PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR);
    assert(nodes[0].parent_id == 0u);
    assert(pl_processor_fabric_validate(&model, &workspace, &summary, &error) ==
           PL_PROCESSOR_FABRIC_OK);
}


int main(void) {
    test_host_topology_builds_ppf_hierarchy();
    test_unknown_parent_topology_does_not_fabricate_hierarchy();
    test_inventory_and_catalogue_aggregation();
    test_deterministic_process_pool_plan();
    test_dependency_cycle_is_rejected();
    test_invalid_parent_and_duplicate_ids_are_rejected();
    test_capacity_and_feature_constraints();
    test_small_output_buffer_is_rejected();
    test_report_is_versioned_and_non_executing();
    puts("Pytyliki Processor Fabric v1 tests passed");
    return 0;
}
