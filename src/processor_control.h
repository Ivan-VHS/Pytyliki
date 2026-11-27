#ifndef PYTYLIKI_PROCESSOR_CONTROL_H
#define PYTYLIKI_PROCESSOR_CONTROL_H

#include <stddef.h>
#include <stdint.h>

#define PL_PROCESSOR_CONTROL_MAX_CPUS 64U

typedef enum {
    PL_PROCESSOR_CONTROL_OK = 0,
    PL_PROCESSOR_CONTROL_INVALID_ARGUMENT,
    PL_PROCESSOR_CONTROL_INVALID_TOPOLOGY,
    PL_PROCESSOR_CONTROL_DUPLICATE_CPU,
    PL_PROCESSOR_CONTROL_NO_ELIGIBLE_CPU,
    PL_PROCESSOR_CONTROL_INVALID_EVIDENCE,
    PL_PROCESSOR_CONTROL_INCOMPATIBLE_ABI,
    PL_PROCESSOR_CONTROL_DENIED,
    PL_PROCESSOR_CONTROL_UNSUPPORTED
} pl_processor_control_status;

typedef enum {
    PL_PROCESSOR_METRIC_FIXTURE = 0,
    PL_PROCESSOR_METRIC_LOCAL_READ_ONLY = 1
} pl_processor_metric_mode;

typedef enum {
    PL_PROCESSOR_REVISION_DRAFT = 0,
    PL_PROCESSOR_REVISION_VERIFIED,
    PL_PROCESSOR_REVISION_STAGED,
    PL_PROCESSOR_REVISION_ACTIVE,
    PL_PROCESSOR_REVISION_RETIRED,
    PL_PROCESSOR_REVISION_REJECTED
} pl_processor_revision_state;

typedef enum {
    PL_PROCESSOR_OP_INVENTORY = 0,
    PL_PROCESSOR_OP_MEASURE,
    PL_PROCESSOR_OP_PROFILE,
    PL_PROCESSOR_OP_ANALYZE,
    PL_PROCESSOR_OP_COMPARE,
    PL_PROCESSOR_OP_EXPORT_REPORT,
    PL_PROCESSOR_OP_SET_AFFINITY,
    PL_PROCESSOR_OP_SET_FREQUENCY,
    PL_PROCESSOR_OP_SET_VOLTAGE,
    PL_PROCESSOR_OP_WRITE_FIRMWARE,
    PL_PROCESSOR_OP_WRITE_MSR,
    PL_PROCESSOR_OP_INSTALL_DRIVER
} pl_processor_operation;

typedef struct {
    uint32_t logical_id;
    uint32_t physical_core_id;
    uint32_t package_id;
    uint32_t generation;
    uint8_t online;
} pl_processor_control_cpu;

typedef struct {
    uint32_t logical_cpu_count;
    uint32_t online_cpu_count;
    uint32_t generation;
} pl_processor_control_topology;

typedef struct {
    uint32_t cpu_id;
    uint32_t runnable_load_basis_points;
    uint32_t interrupt_load_basis_points;
    uint8_t online;
} pl_processor_control_worker;

typedef struct {
    const char *provider;
    const char *metric_id;
    const char *unit;
    const char *workload_id;
    const char *captured_at;
    double value;
    double minimum;
    double maximum;
    uint32_t sample_count;
    pl_processor_metric_mode mode;
} pl_processor_control_metric;

typedef struct {
    uint32_t abi_major;
    uint64_t generation;
    pl_processor_revision_state state;
    const uint8_t *source_digest;
    uint32_t tests_total;
    uint32_t tests_passed;
    uint32_t tests_failed;
    uint32_t coverage_basis_points;
    const uint8_t *artifact_digest;
    uint8_t signature_valid;
    uint8_t safe_point;
    uint8_t quiescent;
    uint8_t rollback_ready;
} pl_processor_control_revision;

pl_processor_control_status pl_processor_control_topology_validate(
    const pl_processor_control_cpu *cpus, size_t cpu_count,
    const pl_processor_control_topology *topology
);

pl_processor_control_status pl_processor_control_select_worker(
    const pl_processor_control_worker *workers, size_t worker_count,
    uint64_t affinity_mask, uint32_t *selected_cpu
);

pl_processor_control_status pl_processor_control_partition(
    size_t total_items, size_t worker_count,
    size_t *allocations, size_t allocation_capacity
);

pl_processor_control_status pl_processor_control_metric_validate(
    const pl_processor_control_metric *metric
);

int pl_processor_control_metrics_comparable(
    const pl_processor_control_metric *left,
    const pl_processor_control_metric *right
);

pl_processor_control_status pl_processor_control_revision_admit(
    const pl_processor_control_revision *active,
    const pl_processor_control_revision *candidate,
    uint32_t required_abi_major
);

pl_processor_control_status pl_processor_control_operation_admit(
    pl_processor_operation operation, int bounded, int explicitly_requested
);

const char *pl_processor_control_status_name(pl_processor_control_status status);

#endif
