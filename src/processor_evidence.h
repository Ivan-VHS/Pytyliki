#ifndef PYTYLIKI_PROCESSOR_EVIDENCE_H
#define PYTYLIKI_PROCESSOR_EVIDENCE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_TR_EVIDENCE_CAPACITY 256u
#define PL_TR_SOURCE_ID_MAX 96u
#define PL_TR_SHA256_HEX_SIZE 65u
#define PL_TR_UNKNOWN_CPU UINT32_MAX

typedef enum {
    PL_TR_PROVIDER_WHEA = 1,
    PL_TR_PROVIDER_LINUX_EDAC = 2
} pl_tr_evidence_provider;

typedef enum {
    PL_TR_EVIDENCE_UNKNOWN = 0,
    PL_TR_EVIDENCE_CORRECTED = 1,
    PL_TR_EVIDENCE_UNCORRECTED = 2,
    PL_TR_EVIDENCE_FATAL = 3,
    PL_TR_EVIDENCE_COUNTER_SNAPSHOT = 4
} pl_tr_evidence_kind;

typedef enum {
    PL_TR_SEVERITY_UNKNOWN = 0,
    PL_TR_SEVERITY_INFO = 1,
    PL_TR_SEVERITY_WARNING = 2,
    PL_TR_SEVERITY_ERROR = 3,
    PL_TR_SEVERITY_CRITICAL = 4
} pl_tr_evidence_severity;

typedef enum {
    PL_TR_EVIDENCE_OK = 0,
    PL_TR_EVIDENCE_INVALID = -1,
    PL_TR_EVIDENCE_FULL = -2,
    PL_TR_EVIDENCE_INTEGRITY = -3,
    PL_TR_EVIDENCE_IO = -4
} pl_tr_evidence_status;

typedef struct {
    uint64_t timestamp_epoch_ns;
    uint64_t source_record_id;
    uint64_t counter_total;
    uint32_t provider;
    uint32_t event_code;
    uint32_t cpu_id;
    uint32_t severity;
    uint32_t kind;
    char source_identity[PL_TR_SOURCE_ID_MAX];
    /* Empty means the ledger will hash the normalized event metadata. */
    char payload_sha256[PL_TR_SHA256_HEX_SIZE];
} pl_tr_platform_event;

typedef struct {
    uint64_t sequence;
    pl_tr_platform_event event;
    char event_sha256[PL_TR_SHA256_HEX_SIZE];
    char chain_sha256[PL_TR_SHA256_HEX_SIZE];
} pl_tr_evidence_entry;

typedef struct {
    size_t count;
    char chain_head[PL_TR_SHA256_HEX_SIZE];
    pl_tr_evidence_entry entries[PL_TR_EVIDENCE_CAPACITY];
} pl_tr_evidence_ledger;

void pl_tr_evidence_init(pl_tr_evidence_ledger *ledger);
int pl_tr_evidence_append(pl_tr_evidence_ledger *ledger,
                          const pl_tr_platform_event *event);
int pl_tr_evidence_verify(const pl_tr_evidence_ledger *ledger);
size_t pl_tr_evidence_find_cpu_window(const pl_tr_evidence_ledger *ledger,
                                      uint32_t cpu_id,
                                      uint64_t start_epoch_ns,
                                      uint64_t end_epoch_ns,
                                      uint64_t *sequences,
                                      size_t sequence_capacity);
int pl_tr_evidence_export_jsonl(const pl_tr_evidence_ledger *ledger,
                                FILE *output);

const char *pl_tr_evidence_provider_name(uint32_t provider);
const char *pl_tr_evidence_kind_name(uint32_t kind);
const char *pl_tr_evidence_severity_name(uint32_t severity);

/* Small dependency-free SHA-256 helper used for source and chain digests. */
int pl_tr_sha256_hex(const void *data, size_t size,
                     char output[PL_TR_SHA256_HEX_SIZE]);

#ifdef __cplusplus
}
#endif
#endif
