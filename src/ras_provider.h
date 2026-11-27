#ifndef PYTYLIKI_RAS_PROVIDER_H
#define PYTYLIKI_RAS_PROVIDER_H
#include <stddef.h>
#include <stdint.h>
#define PL_RAS_PROVIDER_BATCH_CAPACITY 32u
#define PL_RAS_EDAC_MAX_CONTROLLERS 16u
#define PL_RAS_SOURCE_UNKNOWN 0u
#define PL_RAS_SOURCE_WINDOWS_WHEA 1u
#define PL_RAS_SOURCE_LINUX_EDAC 2u
#define PL_RAS_SEVERITY_UNKNOWN 0u
#define PL_RAS_SEVERITY_CORRECTED 1u
#define PL_RAS_SEVERITY_UNCORRECTED 2u
#define PL_RAS_EVENT_COUNTER_RESET (1u << 0)
#define PL_RAS_EVENT_SYSTEM_WIDE (1u << 1)
#define PL_RAS_EVENT_COUNTER_BASELINE (1u << 2)
typedef enum {
    PL_RAS_PROVIDER_OK = 0,
    PL_RAS_PROVIDER_UNSUPPORTED = -1,
    PL_RAS_PROVIDER_INVALID_ARGUMENT = -2,
    PL_RAS_PROVIDER_READ_ERROR = -3,
    PL_RAS_PROVIDER_CAPACITY = -4
} pl_ras_provider_status;
typedef struct {
    uint64_t record_id;
    uint64_t timestamp_ns;
    uint32_t event_id;
    uint32_t level;
    uint32_t source;
    uint32_t severity;
    uint64_t count_delta;
    uint32_t controller_id;
    uint32_t flags;
} pl_ras_record;
typedef struct {
    pl_ras_record records[PL_RAS_PROVIDER_BATCH_CAPACITY];
    size_t count;
    uint32_t dropped_records;
    uint8_t may_have_more;
} pl_ras_batch;
typedef struct {
    uint64_t last_record_id;
    uint64_t previous_ce[PL_RAS_EDAC_MAX_CONTROLLERS];
    uint64_t previous_ue[PL_RAS_EDAC_MAX_CONTROLLERS];
    uint32_t controller_ids[PL_RAS_EDAC_MAX_CONTROLLERS];
    uint32_t controller_count;
    uint32_t initialized;
    uintptr_t subscription_handle;
    uintptr_t notification_handle;
    uint32_t provider_state;
    uint32_t reserved;
} pl_ras_provider_cursor;
/* Reads OS-published RAS metadata only; it never changes hardware or OS policy. */
const char *pl_ras_provider_name(void);
pl_ras_provider_status pl_ras_provider_poll(pl_ras_provider_cursor *cursor,
                                             pl_ras_batch *batch);
void pl_ras_provider_close(pl_ras_provider_cursor *cursor);
#endif
