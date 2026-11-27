#include "processor_control.h"

#include <math.h>
#include <string.h>

static int digest_present(const uint8_t *digest) {
    if (digest == NULL) return 0;
    uint8_t value = 0U;
    for (size_t i = 0U; i < 32U; ++i) value = (uint8_t)(value | digest[i]);
    return value != 0U;
}

static int text_present(const char *value, size_t maximum) {
    if (value == NULL || value[0] == '\0') return 0;
    return strnlen(value, maximum + 1U) <= maximum;
}

pl_processor_control_status pl_processor_control_topology_validate(
    const pl_processor_control_cpu *cpus, size_t cpu_count,
    const pl_processor_control_topology *topology
) {
    if (topology == NULL || (cpu_count != 0U && cpus == NULL))
        return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;
    if (topology->logical_cpu_count == 0U ||
        topology->logical_cpu_count > PL_PROCESSOR_CONTROL_MAX_CPUS ||
        cpu_count != (size_t)topology->logical_cpu_count ||
        topology->generation == 0U)
        return PL_PROCESSOR_CONTROL_INVALID_TOPOLOGY;

    uint64_t seen = 0U;
    uint32_t online = 0U;
    for (size_t i = 0U; i < cpu_count; ++i) {
        const pl_processor_control_cpu *cpu = &cpus[i];
        if (cpu->logical_id >= topology->logical_cpu_count ||
            cpu->generation != topology->generation || cpu->online > 1U)
            return PL_PROCESSOR_CONTROL_INVALID_TOPOLOGY;
        const uint64_t bit = UINT64_C(1) << cpu->logical_id;
        if ((seen & bit) != 0U) return PL_PROCESSOR_CONTROL_DUPLICATE_CPU;
        seen |= bit;
        if (cpu->online != 0U) ++online;
    }
    const uint64_t expected = topology->logical_cpu_count == 64U
        ? UINT64_MAX
        : ((UINT64_C(1) << topology->logical_cpu_count) - UINT64_C(1));
    if (seen != expected || online != topology->online_cpu_count)
        return PL_PROCESSOR_CONTROL_INVALID_TOPOLOGY;
    return PL_PROCESSOR_CONTROL_OK;
}

pl_processor_control_status pl_processor_control_select_worker(
    const pl_processor_control_worker *workers, size_t worker_count,
    uint64_t affinity_mask, uint32_t *selected_cpu
) {
    if (selected_cpu == NULL || (worker_count != 0U && workers == NULL))
        return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;
    *selected_cpu = UINT32_MAX;
    if (affinity_mask == 0U) return PL_PROCESSOR_CONTROL_NO_ELIGIBLE_CPU;

    uint32_t best_load = UINT32_MAX;
    uint32_t best_cpu = UINT32_MAX;
    for (size_t i = 0U; i < worker_count; ++i) {
        const pl_processor_control_worker *worker = &workers[i];
        if (worker->online > 1U || worker->runnable_load_basis_points > 10000U ||
            worker->interrupt_load_basis_points > 10000U ||
            worker->runnable_load_basis_points >
                10000U - worker->interrupt_load_basis_points)
            return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;
        if (worker->online == 0U || worker->cpu_id >= PL_PROCESSOR_CONTROL_MAX_CPUS ||
            (affinity_mask & (UINT64_C(1) << worker->cpu_id)) == 0U)
            continue;
        const uint32_t load = worker->runnable_load_basis_points +
                              worker->interrupt_load_basis_points;
        if (load < best_load || (load == best_load && worker->cpu_id < best_cpu)) {
            best_load = load;
            best_cpu = worker->cpu_id;
        }
    }
    if (best_cpu == UINT32_MAX) return PL_PROCESSOR_CONTROL_NO_ELIGIBLE_CPU;
    *selected_cpu = best_cpu;
    return PL_PROCESSOR_CONTROL_OK;
}

pl_processor_control_status pl_processor_control_partition(
    size_t total_items, size_t worker_count,
    size_t *allocations, size_t allocation_capacity
) {
    if (worker_count == 0U || allocations == NULL)
        return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;
    if (worker_count > allocation_capacity)
        return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;

    const size_t base = total_items / worker_count;
    const size_t remainder = total_items % worker_count;
    for (size_t i = 0U; i < worker_count; ++i)
        allocations[i] = base + (i < remainder ? 1U : 0U);
    return PL_PROCESSOR_CONTROL_OK;
}

pl_processor_control_status pl_processor_control_metric_validate(
    const pl_processor_control_metric *metric
) {
    if (metric == NULL) return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;
    if (!text_present(metric->provider, 64U) ||
        !text_present(metric->metric_id, 128U) ||
        !text_present(metric->unit, 64U) ||
        !text_present(metric->workload_id, 128U) ||
        !text_present(metric->captured_at, 64U) ||
        !isfinite(metric->value) || !isfinite(metric->minimum) ||
        !isfinite(metric->maximum) || metric->minimum > metric->maximum ||
        metric->value < metric->minimum || metric->value > metric->maximum ||
        metric->sample_count == 0U || metric->sample_count > 1000000U ||
        (metric->mode != PL_PROCESSOR_METRIC_FIXTURE &&
         metric->mode != PL_PROCESSOR_METRIC_LOCAL_READ_ONLY))
        return PL_PROCESSOR_CONTROL_INVALID_EVIDENCE;
    return PL_PROCESSOR_CONTROL_OK;
}

int pl_processor_control_metrics_comparable(
    const pl_processor_control_metric *left,
    const pl_processor_control_metric *right
) {
    if (pl_processor_control_metric_validate(left) != PL_PROCESSOR_CONTROL_OK ||
        pl_processor_control_metric_validate(right) != PL_PROCESSOR_CONTROL_OK)
        return 0;
    return left->mode == right->mode &&
        strcmp(left->provider, right->provider) == 0 &&
        strcmp(left->metric_id, right->metric_id) == 0 &&
        strcmp(left->unit, right->unit) == 0 &&
        strcmp(left->workload_id, right->workload_id) == 0;
}

pl_processor_control_status pl_processor_control_revision_admit(
    const pl_processor_control_revision *active,
    const pl_processor_control_revision *candidate,
    uint32_t required_abi_major
) {
    if (active == NULL || candidate == NULL || required_abi_major == 0U)
        return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;
    if (active->state != PL_PROCESSOR_REVISION_ACTIVE ||
        candidate->state != PL_PROCESSOR_REVISION_STAGED ||
        active->generation == UINT64_MAX ||
        candidate->generation != active->generation + UINT64_C(1) ||
        !digest_present(active->source_digest) ||
        !digest_present(candidate->source_digest))
        return PL_PROCESSOR_CONTROL_INVALID_ARGUMENT;
    if (active->abi_major != required_abi_major ||
        candidate->abi_major != required_abi_major ||
        candidate->abi_major != active->abi_major)
        return PL_PROCESSOR_CONTROL_INCOMPATIBLE_ABI;
    if (candidate->tests_total == 0U ||
        candidate->tests_passed != candidate->tests_total ||
        candidate->tests_failed != 0U ||
        candidate->coverage_basis_points > 10000U ||
        !digest_present(candidate->artifact_digest) ||
        memcmp(candidate->source_digest, candidate->artifact_digest, 32U) != 0)
        return PL_PROCESSOR_CONTROL_INVALID_EVIDENCE;
    if (candidate->signature_valid == 0U || candidate->safe_point == 0U ||
        candidate->quiescent == 0U || candidate->rollback_ready == 0U)
        return PL_PROCESSOR_CONTROL_DENIED;
    return PL_PROCESSOR_CONTROL_OK;
}

pl_processor_control_status pl_processor_control_operation_admit(
    pl_processor_operation operation, int bounded, int explicitly_requested
) {
    if (bounded == 0 || explicitly_requested == 0)
        return PL_PROCESSOR_CONTROL_DENIED;
    switch (operation) {
    case PL_PROCESSOR_OP_INVENTORY:
    case PL_PROCESSOR_OP_MEASURE:
    case PL_PROCESSOR_OP_PROFILE:
    case PL_PROCESSOR_OP_ANALYZE:
    case PL_PROCESSOR_OP_COMPARE:
    case PL_PROCESSOR_OP_EXPORT_REPORT:
        return PL_PROCESSOR_CONTROL_OK;
    case PL_PROCESSOR_OP_SET_AFFINITY:
    case PL_PROCESSOR_OP_SET_FREQUENCY:
    case PL_PROCESSOR_OP_SET_VOLTAGE:
    case PL_PROCESSOR_OP_WRITE_FIRMWARE:
    case PL_PROCESSOR_OP_WRITE_MSR:
    case PL_PROCESSOR_OP_INSTALL_DRIVER:
        return PL_PROCESSOR_CONTROL_DENIED;
    }
    return PL_PROCESSOR_CONTROL_UNSUPPORTED;
}

const char *pl_processor_control_status_name(pl_processor_control_status status) {
    switch (status) {
    case PL_PROCESSOR_CONTROL_OK: return "ok";
    case PL_PROCESSOR_CONTROL_INVALID_ARGUMENT: return "invalid-argument";
    case PL_PROCESSOR_CONTROL_INVALID_TOPOLOGY: return "invalid-topology";
    case PL_PROCESSOR_CONTROL_DUPLICATE_CPU: return "duplicate-cpu";
    case PL_PROCESSOR_CONTROL_NO_ELIGIBLE_CPU: return "no-eligible-cpu";
    case PL_PROCESSOR_CONTROL_INVALID_EVIDENCE: return "invalid-evidence";
    case PL_PROCESSOR_CONTROL_INCOMPATIBLE_ABI: return "incompatible-abi";
    case PL_PROCESSOR_CONTROL_DENIED: return "denied";
    case PL_PROCESSOR_CONTROL_UNSUPPORTED: return "unsupported";
    }
    return "unknown";
}
