#ifndef PYTYLIKI_RAS_EVIDENCE_H
#define PYTYLIKI_RAS_EVIDENCE_H
#include "ras_provider.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PL_RAS_EVIDENCE_SCHEMA_VERSION 1u
#define PL_RAS_EVIDENCE_CAPACITY 128u
typedef enum {
    PL_RAS_EVIDENCE_OK = 0,
    PL_RAS_EVIDENCE_INVALID = -1,
    PL_RAS_EVIDENCE_CAPACITY_ERROR = -2,
    PL_RAS_EVIDENCE_IO_ERROR = -3,
    PL_RAS_EVIDENCE_INTEGRITY_ERROR = -4,
    PL_RAS_EVIDENCE_DUPLICATE_ONLY = 1
} pl_ras_evidence_status;
typedef struct {
    uint64_t sequence;
    uint64_t provider_record_id;
    uint64_t timestamp_ns;
    uint64_t count_delta;
    uint32_t source;
    uint32_t event_id;
    uint32_t severity;
    uint32_t controller_id;
    uint32_t flags;
    uint32_t reserved;
} pl_ras_evidence_entry;
typedef struct {
    uint32_t schema_version;
    uint32_t count;
    uint32_t head;
    uint32_t dropped;
    uint64_t generation;
    uint64_t last_sequence;
    uint64_t last_whea_record_id;
    uint64_t previous_ce[PL_RAS_EDAC_MAX_CONTROLLERS];
    uint64_t previous_ue[PL_RAS_EDAC_MAX_CONTROLLERS];
    uint32_t controller_ids[PL_RAS_EDAC_MAX_CONTROLLERS];
    uint32_t controller_count;
    uint32_t provider_initialized;
    pl_ras_evidence_entry entries[PL_RAS_EVIDENCE_CAPACITY];
} pl_ras_evidence_ledger;
void pl_ras_evidence_init(pl_ras_evidence_ledger *ledger);
int pl_ras_evidence_valid(const pl_ras_evidence_ledger *ledger);
pl_ras_evidence_status pl_ras_evidence_append(
    pl_ras_evidence_ledger *ledger, const pl_ras_batch *batch);
int pl_ras_evidence_at(const pl_ras_evidence_ledger *ledger,
                       size_t oldest_index, pl_ras_evidence_entry *out);
void pl_ras_evidence_restore_cursor(const pl_ras_evidence_ledger *ledger,
                                    pl_ras_provider_cursor *cursor);
void pl_ras_evidence_capture_cursor(pl_ras_evidence_ledger *ledger,
                                    const pl_ras_provider_cursor *cursor);
pl_ras_evidence_status pl_ras_evidence_save(
    const char *path, pl_ras_evidence_ledger *ledger);
pl_ras_evidence_status pl_ras_evidence_load(
    const char *path, pl_ras_evidence_ledger *ledger);
int pl_ras_evidence_cli(const char *path);
#ifdef __cplusplus
}
#endif
#endif
