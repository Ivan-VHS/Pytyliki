#ifndef PYTYLIKI_CLIFE_H
#define PYTYLIKI_CLIFE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_CLIFE_SCHEMA_VERSION 1u
#define PL_CLIFE_PROJECT_ID_CAPACITY 129u
#define PL_CLIFE_MAX_PAYLOAD_BYTES ((size_t)64u * 1024u * 1024u)

typedef enum {
    PL_CLIFE_FAULT_STORAGE_IO = 0,
    PL_CLIFE_FAULT_ARTIFACT_INTEGRITY = 1,
    PL_CLIFE_FAULT_RESOURCE_PRESSURE = 2,
    PL_CLIFE_FAULT_EXECUTION_INTERRUPTION = 3
} pl_clife_fault_type;

typedef enum {
    PL_CLIFE_NORMAL_RESUME = 0,
    PL_CLIFE_VERIFIED_RESTORE = 1,
    PL_CLIFE_PARTIAL_HOLD = 2,
    PL_CLIFE_EMERGENCY_SAFE_STOP = 3,
    PL_CLIFE_INVALID_STATE = 4
} pl_clife_recovery_branch;

typedef struct {
    char project_id[PL_CLIFE_PROJECT_ID_CAPACITY];
    uint64_t generation;
    uint8_t *payload;
    size_t payload_size;
    uint64_t checksum_fnv1a64;
    uint32_t schema_version;
} pl_clife_checkpoint;

typedef struct {
    char project_id[PL_CLIFE_PROJECT_ID_CAPACITY];
    double artifact_integrity;
    double resource_health;
    double execution_health;
    uint64_t generation;
    int recovery_locked;
} pl_clife_runtime_state;

typedef struct {
    pl_clife_fault_type type;
    double severity;
    int mitigated;
} pl_clife_fault_event;

/* Capture copies caller-owned bytes; it never takes ownership of the source. */
int pl_clife_checkpoint_capture(pl_clife_checkpoint *out, const char *project_id,
                                uint64_t generation, const void *payload,
                                size_t payload_size);
/* Verify metadata, bounds and the non-cryptographic payload checksum. */
int pl_clife_checkpoint_verify(const pl_clife_checkpoint *checkpoint);
/* Copy verified bytes only when project identity matches and capacity is sufficient. */
int pl_clife_checkpoint_restore(const pl_clife_checkpoint *checkpoint,
                                const char *expected_project_id, void *output,
                                size_t output_capacity, size_t *written);
void pl_clife_checkpoint_free(pl_clife_checkpoint *checkpoint);

/* Select a bounded recovery branch. Inputs must be finite values in [0, 1]. */
pl_clife_recovery_branch pl_clife_branch_awakening(
    pl_clife_runtime_state *state, const pl_clife_checkpoint *checkpoint,
    double external_threat);

/* Apply one typed fault and report whether the resulting state is mitigated. */
pl_clife_recovery_branch pl_clife_handle_fault(
    pl_clife_runtime_state *state, pl_clife_fault_event *fault,
    const pl_clife_checkpoint *checkpoint, double external_threat);

#ifdef __cplusplus
}
#endif
#endif
