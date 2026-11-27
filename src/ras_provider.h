#ifndef PYTYLIKI_RAS_PROVIDER_H
#define PYTYLIKI_RAS_PROVIDER_H

#include <stddef.h>
#include <stdint.h>

#define PL_RAS_PROVIDER_BATCH_CAPACITY 32u

typedef enum {
    PL_RAS_PROVIDER_OK = 0,
    PL_RAS_PROVIDER_UNSUPPORTED = -1,
    PL_RAS_PROVIDER_INVALID_ARGUMENT = -2,
    PL_RAS_PROVIDER_READ_ERROR = -3
} pl_ras_provider_status;

typedef enum {
    PL_RAS_SOURCE_UNKNOWN = 0,
    PL_RAS_SOURCE_WINDOWS_WHEA = 1
} pl_ras_source;

typedef enum {
    PL_RAS_SEVERITY_UNKNOWN = 0
} pl_ras_severity;

typedef struct {
    uint64_t record_id;
    uint64_t timestamp_ns;
    uint32_t event_id;
    uint32_t level;
    pl_ras_source source;
    pl_ras_severity severity;
} pl_ras_record;

typedef struct {
    pl_ras_record records[PL_RAS_PROVIDER_BATCH_CAPACITY];
    size_t count;
    uint32_t dropped_records;
    uint8_t may_have_more;
} pl_ras_batch;

typedef struct {
    uint64_t last_record_id;
} pl_ras_provider_cursor;

/* Reads OS-published RAS metadata only; it never changes hardware or OS policy. */
const char *pl_ras_provider_name(void);
pl_ras_provider_status pl_ras_provider_poll(
    pl_ras_provider_cursor *cursor,
    pl_ras_batch *batch
);

#endif
