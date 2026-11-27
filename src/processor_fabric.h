#ifndef PYTYLIKI_PROCESSOR_FABRIC_H
#define PYTYLIKI_PROCESSOR_FABRIC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_PROCESSOR_FABRIC_ABI_MAJOR 1u
#define PL_PROCESSOR_FABRIC_MAX_NODES 256u
#define PL_PROCESSOR_FABRIC_MAX_EDGES 512u
#define PL_PROCESSOR_FABRIC_SCHEMA "pytyliki.processor-fabric-report.v1"
#define PL_PROCESSOR_FABRIC_CATALOG_COUNT 5u
#define PL_PROCESSOR_FABRIC_BASIS_POINTS 10000u

#define PL_PROCESSOR_FABRIC_DOMAIN_HOST              (UINT32_C(1) << 0)
#define PL_PROCESSOR_FABRIC_DOMAIN_FIRMWARE          (UINT32_C(1) << 1)
#define PL_PROCESSOR_FABRIC_DOMAIN_ARCHITECTURAL     (UINT32_C(1) << 2)
#define PL_PROCESSOR_FABRIC_DOMAIN_MICROARCHITECTURE  (UINT32_C(1) << 3)
#define PL_PROCESSOR_FABRIC_DOMAIN_LOGIC             (UINT32_C(1) << 4)
#define PL_PROCESSOR_FABRIC_DOMAIN_SIMULATION        (UINT32_C(1) << 5)
#define PL_PROCESSOR_FABRIC_DOMAIN_VERIFICATION      (UINT32_C(1) << 6)
#define PL_PROCESSOR_FABRIC_DOMAIN_ALL               ((UINT32_C(1) << 7) - 1u)

typedef enum {
    PL_PROCESSOR_FABRIC_OK = 0,
    PL_PROCESSOR_FABRIC_INVALID_ARGUMENT,
    PL_PROCESSOR_FABRIC_LIMIT_EXCEEDED,
    PL_PROCESSOR_FABRIC_DUPLICATE_ID,
    PL_PROCESSOR_FABRIC_MISSING_REFERENCE,
    PL_PROCESSOR_FABRIC_INVALID_PARENT,
    PL_PROCESSOR_FABRIC_PARENT_CYCLE,
    PL_PROCESSOR_FABRIC_INVALID_EDGE,
    PL_PROCESSOR_FABRIC_DUPLICATE_EDGE,
    PL_PROCESSOR_FABRIC_DEPENDENCY_CYCLE,
    PL_PROCESSOR_FABRIC_INVALID_GENERATION,
    PL_PROCESSOR_FABRIC_INVALID_CAPACITY,
    PL_PROCESSOR_FABRIC_INVALID_DOMAIN,
    PL_PROCESSOR_FABRIC_OUTPUT_TOO_SMALL,
    PL_PROCESSOR_FABRIC_INTERNAL_ERROR
} pl_processor_fabric_status;

typedef enum {
    PL_PROCESSOR_FABRIC_PACKAGE = 0,
    PL_PROCESSOR_FABRIC_DIE,
    PL_PROCESSOR_FABRIC_CORE,
    PL_PROCESSOR_FABRIC_LOGICAL_PROCESSOR,
    PL_PROCESSOR_FABRIC_REGISTER_BANK,
    PL_PROCESSOR_FABRIC_ISA_PROFILE,
    PL_PROCESSOR_FABRIC_ADDRESS_SPACE,
    PL_PROCESSOR_FABRIC_MEMORY_REGION,
    PL_PROCESSOR_FABRIC_DEVICE,
    PL_PROCESSOR_FABRIC_EXECUTION_UNIT,
    PL_PROCESSOR_FABRIC_PROCESS_POOL,
    PL_PROCESSOR_FABRIC_PROCESS,
    PL_PROCESSOR_FABRIC_ARTIFACT,
    PL_PROCESSOR_FABRIC_NODE_KIND_COUNT
} pl_processor_fabric_node_kind;

typedef enum {
    PL_PROCESSOR_FABRIC_DECLARED = 0,
    PL_PROCESSOR_FABRIC_VERIFIED,
    PL_PROCESSOR_FABRIC_AVAILABLE,
    PL_PROCESSOR_FABRIC_BLOCKED,
    PL_PROCESSOR_FABRIC_COMPLETED,
    PL_PROCESSOR_FABRIC_FAILED,
    PL_PROCESSOR_FABRIC_DISABLED,
    PL_PROCESSOR_FABRIC_RETIRED,
    PL_PROCESSOR_FABRIC_NODE_STATE_COUNT
} pl_processor_fabric_node_state;

typedef enum {
    PL_PROCESSOR_FABRIC_DEPENDS_ON = 0,
    PL_PROCESSOR_FABRIC_MAPS_TO,
    PL_PROCESSOR_FABRIC_USES,
    PL_PROCESSOR_FABRIC_PRODUCES,
    PL_PROCESSOR_FABRIC_EDGE_KIND_COUNT
} pl_processor_fabric_edge_kind;

typedef enum {
    PL_PROCESSOR_FABRIC_ASSIGNED = 0,
    PL_PROCESSOR_FABRIC_WAITING_DEPENDENCY,
    PL_PROCESSOR_FABRIC_NO_COMPATIBLE_WORKER,
    PL_PROCESSOR_FABRIC_CAPACITY_EXHAUSTED
} pl_processor_fabric_assignment_state;

typedef struct {
    uint32_t id;
    uint32_t parent_id;
    uint32_t generation;
    pl_processor_fabric_node_kind kind;
    pl_processor_fabric_node_state state;
    uint32_t domain_mask;
    uint64_t feature_mask;
    uint64_t affinity_mask;
    uint32_t logical_id;
    uint32_t capacity_basis_points;
    uint32_t reserved_basis_points;
    uint32_t requested_load_basis_points;
    uint8_t priority;
} pl_processor_fabric_node;

typedef struct {
    uint32_t from_id;
    uint32_t to_id;
    pl_processor_fabric_edge_kind kind;
} pl_processor_fabric_edge;

typedef struct {
    uint32_t generation;
    uint32_t abi_major;
    size_t node_count;
    size_t edge_count;
    const pl_processor_fabric_node *nodes;
    const pl_processor_fabric_edge *edges;
} pl_processor_fabric_model;

typedef struct {
    size_t indegree[PL_PROCESSOR_FABRIC_MAX_NODES];
    size_t queue[PL_PROCESSOR_FABRIC_MAX_NODES];
    size_t topological_order[PL_PROCESSOR_FABRIC_MAX_NODES];
    uint32_t worker_load[PL_PROCESSOR_FABRIC_MAX_NODES];
    uint8_t selected[PL_PROCESSOR_FABRIC_MAX_NODES];
} pl_processor_fabric_workspace;

typedef struct {
    uint32_t generation;
    uint32_t total_nodes;
    uint32_t total_edges;
    uint32_t node_kind_counts[PL_PROCESSOR_FABRIC_NODE_KIND_COUNT];
    uint32_t node_state_counts[PL_PROCESSOR_FABRIC_NODE_STATE_COUNT];
    uint32_t worker_count;
    uint32_t available_worker_count;
    uint32_t process_count;
    uint32_t ready_process_count;
    uint32_t completed_process_count;
    uint32_t failed_process_count;
    uint32_t dependency_edge_count;
    uint32_t resource_edge_count;
    uint32_t produced_edge_count;
    uint64_t total_worker_capacity_basis_points;
    uint64_t total_worker_reserved_basis_points;
} pl_processor_fabric_summary;

typedef struct {
    uint32_t cpu_suite_modules;
    uint32_t processor_rules;
    uint32_t hardware_constructs;
    uint32_t system_sigil_operators;
    uint32_t iron_core_markers;
} pl_processor_fabric_catalogues;

typedef struct {
    uint32_t task_id;
    uint32_t worker_id;
    uint32_t resulting_load_basis_points;
    pl_processor_fabric_assignment_state state;
} pl_processor_fabric_assignment;

typedef struct {
    pl_processor_fabric_status status;
    uint32_t node_id;
    uint32_t edge_from_id;
    uint32_t edge_to_id;
    char message[160];
} pl_processor_fabric_error;

/*
 * The model is a read-only view over caller-owned arrays. No pointer is
 * retained, no heap allocation is performed, and no processor state changes.
 */
pl_processor_fabric_status pl_processor_fabric_validate(
    const pl_processor_fabric_model *model,
    pl_processor_fabric_workspace *workspace,
    pl_processor_fabric_summary *summary,
    pl_processor_fabric_error *error);

/*
 * Builds a deterministic advisory plan for READY process nodes. Prerequisites
 * must already be COMPLETED. This does not start threads, set affinity, alter
 * scheduling policy, or write hardware state. Caller owns the output array.
 */
pl_processor_fabric_status pl_processor_fabric_plan(
    const pl_processor_fabric_model *model,
    pl_processor_fabric_workspace *workspace,
    pl_processor_fabric_assignment *assignments,
    size_t assignment_capacity,
    size_t *assignment_count,
    pl_processor_fabric_summary *summary,
    pl_processor_fabric_error *error);

/* Counts the existing static catalogues without copying or executing them. */
pl_processor_fabric_status pl_processor_fabric_collect_catalogues(
    pl_processor_fabric_catalogues *out);

/* Serialize a validated summary and plan to a caller-owned JSON buffer. */
pl_processor_fabric_status pl_processor_fabric_report_json(
    const pl_processor_fabric_summary *summary,
    const pl_processor_fabric_catalogues *catalogues,
    const pl_processor_fabric_assignment *assignments,
    size_t assignment_count,
    char *buffer,
    size_t buffer_capacity,
    size_t *written);

const char *pl_processor_fabric_status_name(pl_processor_fabric_status status);
const char *pl_processor_fabric_node_kind_name(pl_processor_fabric_node_kind kind);
const char *pl_processor_fabric_node_state_name(pl_processor_fabric_node_state state);
const char *pl_processor_fabric_edge_kind_name(pl_processor_fabric_edge_kind kind);
const char *pl_processor_fabric_assignment_state_name(pl_processor_fabric_assignment_state state);

#ifdef __cplusplus
}
#endif
#endif
