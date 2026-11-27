#ifndef PYTYLIKI_PTFR_EVIDENCE_LINK_H
#define PYTYLIKI_PTFR_EVIDENCE_LINK_H

#include "processor_evidence.h"
#include "processor_thread_recovery.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PL_TR_LINK_CAPACITY 256u

typedef struct {
    uint64_t sequence;
    uint64_t evidence_sequence;
    uint64_t worker_id;
    uint64_t worker_fault_monotonic_ns;
    uint64_t mapped_evidence_monotonic_ns;
    uint64_t allowed_delta_ns;
    uint32_t cpu_id;
    uint32_t fault_code;
    uint32_t fault_kind;
    char evidence_sha256[PL_TR_SHA256_HEX_SIZE];
    char link_sha256[PL_TR_SHA256_HEX_SIZE];
    char chain_sha256[PL_TR_SHA256_HEX_SIZE];
} pl_tr_worker_evidence_link;

typedef struct {
    size_t count;
    char chain_head[PL_TR_SHA256_HEX_SIZE];
    pl_tr_worker_evidence_link entries[PL_TR_LINK_CAPACITY];
} pl_tr_worker_link_ledger;

enum {
    PL_TR_LINK_OK = 0,
    PL_TR_LINK_INVALID = -1,
    PL_TR_LINK_UNATTRIBUTED = -2,
    PL_TR_LINK_TIME_MISMATCH = -3,
    PL_TR_LINK_CAPACITY = -4,
    PL_TR_LINK_INTEGRITY = -5,
    PL_TR_LINK_MONITOR_ERROR = -6
};

void pl_tr_worker_links_init(pl_tr_worker_link_ledger *ledger);
int pl_tr_worker_links_verify(const pl_tr_worker_link_ledger *ledger);

/*
 * Ingest a platform hardware event as a PTFR worker fault only when the caller
 * supplies an independently verified CPU-to-worker attribution and a
 * monotonic clock mapping within the configured uncertainty window.
 *
 * Platform adapters in this increment currently emit unknown CPU IDs or
 * counter snapshots, so those records are deliberately not ingestible.
 */
int pl_tr_evidence_ingest_worker_fault(
    const pl_tr_evidence_ledger *evidence,
    pl_tr_worker_link_ledger *links,
    pl_tr_monitor *monitor,
    uint64_t evidence_sequence,
    uint64_t worker_id,
    uint32_t worker_cpu_id,
    uint64_t worker_fault_monotonic_ns,
    uint64_t mapped_evidence_monotonic_ns,
    uint64_t allowed_delta_ns,
    int attribution_verified
);

int pl_tr_worker_links_export_jsonl(
    const pl_tr_worker_link_ledger *ledger,
    FILE *output
);

#ifdef __cplusplus
}
#endif
#endif
