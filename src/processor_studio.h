#ifndef PYTYLIKI_PROCESSOR_STUDIO_H
#define PYTYLIKI_PROCESSOR_STUDIO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_PROCESSOR_STUDIO_ABI_MAJOR 1u
#define PL_PROCESSOR_STUDIO_DOMAIN_COUNT 20u
#define PL_PROCESSOR_STUDIO_MAX_WORKERS 20u
#define PL_PROCESSOR_STUDIO_MAX_TRACE 64u
#define PL_PROCESSOR_STUDIO_DOMAIN_MASK ((UINT32_C(1) << PL_PROCESSOR_STUDIO_DOMAIN_COUNT) - UINT32_C(1))
#define PL_PROCESSOR_STUDIO_SCHEMA "pytyliki.processor-studio-report.v1"
#define PL_PROCESSOR_STUDIO_CATALOG_SCHEMA "pytyliki.processor-studio-catalog.v1"
#define PL_PROCESSOR_STUDIO_REAPER_CONTRACT "pytyliki.reaper-processor-studio.v1"
#define PL_PROCESSOR_STUDIO_QUEUE_AXI_BUS 1u
#define PL_PROCESSOR_STUDIO_PLANNED_JUMP_TARGET UINT64_C(0x00000000C0000400)

#define PL_PROCESSOR_STUDIO_RIGHT_INSPECT UINT32_C(1)
#define PL_PROCESSOR_STUDIO_RIGHT_EXECUTE UINT32_C(2)
#define PL_PROCESSOR_STUDIO_RIGHT_PLAN UINT32_C(4)
#define PL_PROCESSOR_STUDIO_RIGHT_COMMIT UINT32_C(8)
#define PL_PROCESSOR_STUDIO_RIGHT_TRACE UINT32_C(16)
#define PL_PROCESSOR_STUDIO_RIGHT_ALL (PL_PROCESSOR_STUDIO_RIGHT_INSPECT | PL_PROCESSOR_STUDIO_RIGHT_EXECUTE | PL_PROCESSOR_STUDIO_RIGHT_PLAN | PL_PROCESSOR_STUDIO_RIGHT_COMMIT | PL_PROCESSOR_STUDIO_RIGHT_TRACE)

typedef enum {
    PL_PROCESSOR_STUDIO_OK = 0,
    PL_PROCESSOR_STUDIO_INVALID_ARGUMENT,
    PL_PROCESSOR_STUDIO_INVALID_LAYOUT,
    PL_PROCESSOR_STUDIO_ADDRESS_OVERFLOW,
    PL_PROCESSOR_STUDIO_REGION_OVERLAP,
    PL_PROCESSOR_STUDIO_OUT_OF_RANGE,
    PL_PROCESSOR_STUDIO_PERMISSION_DENIED,
    PL_PROCESSOR_STUDIO_INVALID_DOMAIN_MASK,
    PL_PROCESSOR_STUDIO_TRANSACTION_ALREADY_OPEN,
    PL_PROCESSOR_STUDIO_NO_TRANSACTION,
    PL_PROCESSOR_STUDIO_GENERATION_CONFLICT,
    PL_PROCESSOR_STUDIO_GENERATION_EXHAUSTED,
    PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL,
    PL_PROCESSOR_STUDIO_ASSERTION_FAILED,
    PL_PROCESSOR_STUDIO_INTERNAL_ERROR
} pl_processor_studio_status;

typedef enum {
    PL_PROCESSOR_STUDIO_XOR = 0,
    PL_PROCESSOR_STUDIO_ADD,
    PL_PROCESSOR_STUDIO_ROTL_XOR
} pl_processor_studio_transform;

typedef enum {
    PL_PROCESSOR_STUDIO_DOMAIN_DECLARED = 0,
    PL_PROCESSOR_STUDIO_DOMAIN_READY,
    PL_PROCESSOR_STUDIO_DOMAIN_EXECUTED,
    PL_PROCESSOR_STUDIO_DOMAIN_COMMITTED,
    PL_PROCESSOR_STUDIO_DOMAIN_DISABLED,
    PL_PROCESSOR_STUDIO_DOMAIN_FAILED
} pl_processor_studio_domain_state;

typedef enum {
    PL_PROCESSOR_STUDIO_TRACE_INIT = 0,
    PL_PROCESSOR_STUDIO_TRACE_BEGIN,
    PL_PROCESSOR_STUDIO_TRACE_DOMAIN,
    PL_PROCESSOR_STUDIO_TRACE_PLAN,
    PL_PROCESSOR_STUDIO_TRACE_COMMIT,
    PL_PROCESSOR_STUDIO_TRACE_ROLLBACK,
    PL_PROCESSOR_STUDIO_TRACE_REJECT
} pl_processor_studio_trace_event;

typedef struct {
    uint8_t id;
    uint8_t transform;
    uint8_t rotation;
    uint8_t layer;
    uint64_t seed;
    uint64_t state_slot_offset;
    const char *environment;
    const char *engineering_focus;
    const char *organization;
    const char *layer_name;
} pl_processor_studio_domain_descriptor;

typedef struct {
    uint64_t arena_base;
    uint64_t arena_size;
    uint64_t tray_slot;
    uint64_t tray_size;
    uint64_t overlay_slot;
    uint64_t overlay_size;
    uint64_t domain_table;
    uint64_t domain_table_size;
    uint32_t domain_stride;
} pl_processor_studio_layout;

typedef struct {
    uint32_t core_id;
    uint32_t root_rights;
    uint8_t worker_count;
    uint8_t queue_id;
    uint16_t reserved;
    pl_processor_studio_layout layout;
} pl_processor_studio_config;

typedef struct {
    uint8_t domain_id;
    uint8_t state;
    uint8_t parity;
    uint8_t reserved;
    uint32_t generation;
    uint64_t transformed_value;
    uint64_t accumulated_value;
    uint32_t commit_count;
    uint32_t reserved2;
} pl_processor_studio_domain_state_record;

typedef struct {
    uint64_t sequence;
    uint32_t generation;
    uint32_t source_offset;
    uint8_t domain_id;
    uint8_t event;
    int16_t status;
} pl_processor_studio_trace_record;

typedef struct {
    uint8_t worker_id;
    uint8_t domain_id;
    uint16_t flags;
    uint32_t estimated_cost;
} pl_processor_studio_assignment;

typedef struct {
    uint64_t r[8];
    uint64_t regit;
    uint64_t pc;
    uint64_t planned_jump_target;
    uint64_t flags;
} pl_processor_studio_register_file;

typedef struct {
    uint32_t generation;
    uint32_t requested_domain_mask;
    uint32_t completed_domain_mask;
    uint32_t committed_domain_mask;
    uint32_t worker_count;
    uint32_t queue_id;
    uint64_t domain_fold;
    uint64_t matrix_fold;
    uint64_t sweep_digest;
    uint64_t tournament_value;
    uint8_t low_byte_signature;
    uint8_t bit_parity;
    uint8_t tournament_domain;
    uint8_t predicate_nonzero;
    uint32_t trace_dropped;
    uint32_t planned_assignment_count;
    pl_processor_studio_register_file registers;
    uint8_t physical_memory_accessed;
    uint8_t mmio_accessed;
    uint8_t dma_submitted;
    uint8_t threads_spawned;
} pl_processor_studio_report;

typedef struct {
    pl_processor_studio_status status;
    uint32_t domain_id;
    uint64_t address;
    char message[160];
} pl_processor_studio_error;

typedef struct {
    pl_processor_studio_config config;
    pl_processor_studio_domain_state_record committed[PL_PROCESSOR_STUDIO_DOMAIN_COUNT];
    pl_processor_studio_domain_state_record pending[PL_PROCESSOR_STUDIO_DOMAIN_COUNT];
    pl_processor_studio_trace_record trace[PL_PROCESSOR_STUDIO_MAX_TRACE];
    pl_processor_studio_assignment assignments[PL_PROCESSOR_STUDIO_DOMAIN_COUNT];
    pl_processor_studio_assignment committed_assignments[PL_PROCESSOR_STUDIO_DOMAIN_COUNT];
    pl_processor_studio_report committed_report;
    pl_processor_studio_register_file committed_registers;
    pl_processor_studio_register_file pending_registers;
    uint32_t generation;
    uint32_t transaction_expected_generation;
    uint32_t transaction_mask;
    uint32_t planned_mask;
    uint32_t completed_mask;
    uint32_t trace_dropped;
    uint32_t trace_head;
    uint32_t trace_count;
    uint32_t assignment_count;
    uint32_t committed_assignment_count;
    uint64_t trace_sequence;
    uint64_t pending_fold;
    uint64_t pending_matrix_fold;
    uint64_t pending_sweep_digest;
    uint64_t pending_tournament_value;
    uint8_t pending_tournament_domain;
    uint8_t transaction_open;
    uint8_t pending_executed;
    uint8_t reserved;
} pl_processor_studio;

const pl_processor_studio_domain_descriptor *pl_processor_studio_catalog(size_t *count);
void pl_processor_studio_default_config(pl_processor_studio_config *config);
pl_processor_studio_status pl_processor_studio_validate_layout(
    const pl_processor_studio_layout *layout,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_init(
    pl_processor_studio *studio,
    const pl_processor_studio_config *config,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_begin(
    pl_processor_studio *studio,
    uint32_t expected_generation,
    uint32_t rights,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_execute(
    pl_processor_studio *studio,
    uint32_t domain_mask,
    uint32_t rights,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_plan(
    pl_processor_studio *studio,
    uint32_t domain_mask,
    uint32_t rights,
    size_t *assignment_count,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_commit(
    pl_processor_studio *studio,
    uint32_t expected_generation,
    uint32_t rights,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_rollback(
    pl_processor_studio *studio,
    uint32_t rights,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_get_report(
    const pl_processor_studio *studio,
    uint32_t rights,
    pl_processor_studio_report *report,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_copy_plan(
    const pl_processor_studio *studio,
    uint32_t rights,
    pl_processor_studio_assignment *assignments,
    size_t capacity,
    size_t *assignment_count,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_copy_trace(
    const pl_processor_studio *studio,
    uint32_t rights,
    pl_processor_studio_trace_record *records,
    size_t capacity,
    size_t *record_count,
    pl_processor_studio_error *error);
pl_processor_studio_status pl_processor_studio_report_json(
    const pl_processor_studio *studio,
    uint32_t rights,
    char *buffer,
    size_t capacity,
    size_t *written,
    pl_processor_studio_error *error);
const char *pl_processor_studio_status_name(pl_processor_studio_status status);
const char *pl_processor_studio_transform_name(pl_processor_studio_transform transform);
const char *pl_processor_studio_domain_state_name(pl_processor_studio_domain_state state);

#ifdef __cplusplus
}
#endif
#endif
