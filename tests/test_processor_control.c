#include "processor_control.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_topology(void) {
    pl_processor_control_cpu cpus[] = {
        {0U, 0U, 0U, 3U, 1U},
        {1U, 0U, 0U, 3U, 1U},
        {2U, 1U, 0U, 3U, 0U},
        {3U, 1U, 0U, 3U, 1U}
    };
    pl_processor_control_topology topology = {4U, 3U, 3U};
    assert(pl_processor_control_topology_validate(cpus, 4U, &topology) == PL_PROCESSOR_CONTROL_OK);
    cpus[3].logical_id = 2U;
    assert(pl_processor_control_topology_validate(cpus, 4U, &topology) == PL_PROCESSOR_CONTROL_DUPLICATE_CPU);
    cpus[3].logical_id = 3U;
    topology.online_cpu_count = 4U;
    assert(pl_processor_control_topology_validate(cpus, 4U, &topology) == PL_PROCESSOR_CONTROL_INVALID_TOPOLOGY);
}

static void test_worker_selection_and_partition(void) {
    pl_processor_control_worker workers[] = {
        {0U, 4200U, 100U, 1U},
        {1U, 1800U, 50U, 1U},
        {2U, 10U, 0U, 0U}
    };
    uint32_t selected = UINT32_MAX;
    assert(pl_processor_control_select_worker(workers, 3U, UINT64_C(0x7), &selected) == PL_PROCESSOR_CONTROL_OK);
    assert(selected == 1U);
    assert(pl_processor_control_select_worker(workers, 3U, UINT64_C(0x4), &selected) == PL_PROCESSOR_CONTROL_NO_ELIGIBLE_CPU);
    workers[0].runnable_load_basis_points = 10000U;
    workers[0].interrupt_load_basis_points = 1U;
    assert(pl_processor_control_select_worker(workers, 3U, UINT64_C(0x1), &selected) == PL_PROCESSOR_CONTROL_INVALID_ARGUMENT);

    size_t allocations[6] = {0U};
    assert(pl_processor_control_partition(100U, 6U, allocations, 6U) == PL_PROCESSOR_CONTROL_OK);
    assert(allocations[0] == 17U && allocations[3] == 17U &&
           allocations[4] == 16U && allocations[5] == 16U);
    assert(pl_processor_control_partition(1U, 0U, allocations, 6U) == PL_PROCESSOR_CONTROL_INVALID_ARGUMENT);
}

static void test_metric_provenance(void) {
    pl_processor_control_metric a = {
        "pytyliki.c11", "cpu.utilization", "percent", "workload-a", "2026-11-27T10:00:00Z",
        37.0, 0.0, 100.0, 64U, PL_PROCESSOR_METRIC_LOCAL_READ_ONLY
    };
    pl_processor_control_metric b = a;
    assert(pl_processor_control_metric_validate(&a) == PL_PROCESSOR_CONTROL_OK);
    assert(pl_processor_control_metrics_comparable(&a, &b));
    b.workload_id = "workload-b";
    assert(!pl_processor_control_metrics_comparable(&a, &b));
    b = a;
    b.mode = PL_PROCESSOR_METRIC_FIXTURE;
    assert(!pl_processor_control_metrics_comparable(&a, &b));
    b = a;
    b.value = 101.0;
    assert(pl_processor_control_metric_validate(&b) == PL_PROCESSOR_CONTROL_INVALID_EVIDENCE);
}

static void test_revision_and_operation_policy(void) {
    static const uint8_t active_digest[32] = {1U};
    static const uint8_t candidate_digest[32] = {2U};
    pl_processor_control_revision active = {
        1U, 4U, PL_PROCESSOR_REVISION_ACTIVE, active_digest,
        0U, 0U, 0U, 0U, NULL, 0U, 0U, 0U, 0U
    };
    pl_processor_control_revision candidate = {
        1U, 5U, PL_PROCESSOR_REVISION_STAGED, candidate_digest,
        100U, 100U, 0U, 9500U, candidate_digest, 1U, 1U, 1U, 1U
    };
    assert(pl_processor_control_revision_admit(&active, &candidate, 1U) == PL_PROCESSOR_CONTROL_OK);
    candidate.quiescent = 0U;
    assert(pl_processor_control_revision_admit(&active, &candidate, 1U) == PL_PROCESSOR_CONTROL_DENIED);
    candidate.quiescent = 1U;
    candidate.abi_major = 2U;
    assert(pl_processor_control_revision_admit(&active, &candidate, 1U) == PL_PROCESSOR_CONTROL_INCOMPATIBLE_ABI);

    assert(pl_processor_control_operation_admit(PL_PROCESSOR_OP_INVENTORY, 1, 1) == PL_PROCESSOR_CONTROL_OK);
    assert(pl_processor_control_operation_admit(PL_PROCESSOR_OP_SET_FREQUENCY, 1, 1) == PL_PROCESSOR_CONTROL_DENIED);
    assert(pl_processor_control_operation_admit(PL_PROCESSOR_OP_WRITE_MSR, 1, 1) == PL_PROCESSOR_CONTROL_DENIED);
    assert(pl_processor_control_operation_admit(PL_PROCESSOR_OP_MEASURE, 0, 1) == PL_PROCESSOR_CONTROL_DENIED);
}

int main(void) {
    test_topology();
    test_worker_selection_and_partition();
    test_metric_provenance();
    test_revision_and_operation_policy();
    puts("Pytyliki native processor-control tests passed");
    return 0;
}
