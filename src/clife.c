#include "clife.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define PL_CLIFE_FNV_OFFSET UINT64_C(14695981039346656037)
#define PL_CLIFE_FNV_PRIME UINT64_C(1099511628211)

static int valid_unit(double value) {
    return isfinite(value) && value >= 0.0 && value <= 1.0;
}

static size_t bounded_length(const char *text, size_t limit) {
    size_t length = 0u;
    if (text == NULL) return limit;
    while (length < limit && text[length] != '\0') ++length;
    return length;
}

static uint64_t checksum_bytes(const uint8_t *data, size_t size) {
    uint64_t hash = PL_CLIFE_FNV_OFFSET;
    size_t index;
    for (index = 0u; index < size; ++index) {
        hash ^= (uint64_t)data[index];
        hash *= PL_CLIFE_FNV_PRIME;
    }
    return hash;
}

int pl_clife_checkpoint_capture(pl_clife_checkpoint *out, const char *project_id,
                                uint64_t generation, const void *payload,
                                size_t payload_size) {
    size_t id_length;
    uint8_t *copy = NULL;
    if (out == NULL || project_id == NULL ||
        (payload == NULL && payload_size != 0u) ||
        payload_size > PL_CLIFE_MAX_PAYLOAD_BYTES) return -1;
    id_length = bounded_length(project_id, PL_CLIFE_PROJECT_ID_CAPACITY);
    if (id_length == 0u || id_length >= PL_CLIFE_PROJECT_ID_CAPACITY) return -1;
    if (payload_size != 0u) {
        copy = (uint8_t *)malloc(payload_size);
        if (copy == NULL) return -1;
        memcpy(copy, payload, payload_size);
    }
    memset(out, 0, sizeof(*out));
    memcpy(out->project_id, project_id, id_length);
    out->project_id[id_length] = '\0';
    out->generation = generation;
    out->payload = copy;
    out->payload_size = payload_size;
    out->checksum_fnv1a64 = checksum_bytes(copy, payload_size);
    out->schema_version = PL_CLIFE_SCHEMA_VERSION;
    return 0;
}

int pl_clife_checkpoint_verify(const pl_clife_checkpoint *checkpoint) {
    size_t id_length;
    if (checkpoint == NULL || checkpoint->schema_version != PL_CLIFE_SCHEMA_VERSION ||
        checkpoint->payload_size > PL_CLIFE_MAX_PAYLOAD_BYTES ||
        (checkpoint->payload == NULL && checkpoint->payload_size != 0u)) return 0;
    id_length = bounded_length(checkpoint->project_id, PL_CLIFE_PROJECT_ID_CAPACITY);
    if (id_length == 0u || id_length >= PL_CLIFE_PROJECT_ID_CAPACITY) return 0;
    return checksum_bytes(checkpoint->payload, checkpoint->payload_size) ==
           checkpoint->checksum_fnv1a64;
}

int pl_clife_checkpoint_restore(const pl_clife_checkpoint *checkpoint,
                                const char *expected_project_id, void *output,
                                size_t output_capacity, size_t *written) {
    size_t id_length;
    if (written == NULL || expected_project_id == NULL ||
        !pl_clife_checkpoint_verify(checkpoint)) return -1;
    *written = 0u;
    id_length = bounded_length(expected_project_id, PL_CLIFE_PROJECT_ID_CAPACITY);
    if (id_length == 0u || id_length >= PL_CLIFE_PROJECT_ID_CAPACITY ||
        strcmp(checkpoint->project_id, expected_project_id) != 0) return -1;
    if (output_capacity < checkpoint->payload_size ||
        (output == NULL && checkpoint->payload_size != 0u)) return -1;
    if (checkpoint->payload_size != 0u)
        memcpy(output, checkpoint->payload, checkpoint->payload_size);
    *written = checkpoint->payload_size;
    return 0;
}

void pl_clife_checkpoint_free(pl_clife_checkpoint *checkpoint) {
    if (checkpoint == NULL) return;
    free(checkpoint->payload);
    memset(checkpoint, 0, sizeof(*checkpoint));
}

pl_clife_recovery_branch pl_clife_branch_awakening(
    pl_clife_runtime_state *state, const pl_clife_checkpoint *checkpoint,
    double external_threat) {
    double viability;
    int snapshot_valid = 0;
    if (state == NULL || !valid_unit(state->artifact_integrity) ||
        !valid_unit(state->resource_health) || !valid_unit(state->execution_health) ||
        !valid_unit(external_threat)) return PL_CLIFE_INVALID_STATE;
    viability = 0.50 * state->artifact_integrity +
                0.25 * state->resource_health +
                0.25 * state->execution_health;
    if (checkpoint != NULL && pl_clife_checkpoint_verify(checkpoint) &&
        strcmp(checkpoint->project_id, "") != 0 &&
        checkpoint->generation <= state->generation) snapshot_valid = 1;
    if (external_threat > 0.85 && viability < 0.70) {
        state->recovery_locked = 1;
        return PL_CLIFE_EMERGENCY_SAFE_STOP;
    }
    if (viability >= 0.95 && state->execution_health >= 0.95) {
        state->recovery_locked = 0;
        return PL_CLIFE_NORMAL_RESUME;
    }
    if (snapshot_valid && viability >= 0.55) {
        state->recovery_locked = 0;
        return PL_CLIFE_VERIFIED_RESTORE;
    }
    state->recovery_locked = 1;
    return PL_CLIFE_PARTIAL_HOLD;
}

pl_clife_recovery_branch pl_clife_handle_fault(
    pl_clife_runtime_state *state, pl_clife_fault_event *fault,
    const pl_clife_checkpoint *checkpoint, double external_threat) {
    double *metric = NULL;
    double scale = 0.0;
    pl_clife_recovery_branch branch;
    if (state == NULL || fault == NULL || !valid_unit(fault->severity))
        return PL_CLIFE_INVALID_STATE;
    switch (fault->type) {
        case PL_CLIFE_FAULT_STORAGE_IO: metric = &state->resource_health; scale = 0.30; break;
        case PL_CLIFE_FAULT_ARTIFACT_INTEGRITY: metric = &state->artifact_integrity; scale = 0.45; break;
        case PL_CLIFE_FAULT_RESOURCE_PRESSURE: metric = &state->resource_health; scale = 0.45; break;
        case PL_CLIFE_FAULT_EXECUTION_INTERRUPTION: metric = &state->execution_health; scale = 0.50; break;
        default: return PL_CLIFE_INVALID_STATE;
    }
    if (!valid_unit(*metric)) return PL_CLIFE_INVALID_STATE;
    *metric -= fault->severity * scale;
    if (*metric < 0.0) *metric = 0.0;
    fault->mitigated = 0;
    branch = pl_clife_branch_awakening(state, checkpoint, external_threat);
    if (branch == PL_CLIFE_NORMAL_RESUME || branch == PL_CLIFE_VERIFIED_RESTORE)
        fault->mitigated = 1;
    return branch;
}
