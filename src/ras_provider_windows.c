#include "ras_provider.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winevt.h>

#include <stdint.h>
#include <wchar.h>

#pragma comment(lib, "wevtapi.lib")

static uint64_t filetime_to_unix_ns(uint64_t filetime_ticks) {
    const uint64_t unix_epoch_ticks = UINT64_C(116444736000000000);
    if (filetime_ticks < unix_epoch_ticks) return 0u;
    const uint64_t unix_ticks = filetime_ticks - unix_epoch_ticks;
    if (unix_ticks > UINT64_MAX / UINT64_C(100)) return 0u;
    return unix_ticks * UINT64_C(100);
}

static int read_event_values(EVT_HANDLE event,
                             pl_ras_record *record) {
    EVT_VARIANT values[EvtSystemPropertyIdEND];
    DWORD buffer_used = 0u;
    DWORD property_count = 0u;
    if (!EvtRender(NULL, event, EvtRenderEventValues,
                   (DWORD)sizeof(values), values,
                   &buffer_used, &property_count)) {
        return 0;
    }
    if (property_count <= (DWORD)EvtSystemEventRecordId) return 0;

    if (values[EvtSystemEventRecordId].Type == EvtVarTypeUInt64) {
        record->record_id = values[EvtSystemEventRecordId].UInt64Val;
    } else {
        return 0;
    }
    if (values[EvtSystemEventID].Type == EvtVarTypeUInt16) {
        record->event_id = values[EvtSystemEventID].UInt16Val;
    } else if (values[EvtSystemEventID].Type == EvtVarTypeUInt32) {
        record->event_id = values[EvtSystemEventID].UInt32Val;
    } else {
        return 0;
    }
    if (values[EvtSystemLevel].Type == EvtVarTypeByte) {
        record->level = values[EvtSystemLevel].ByteVal;
    } else {
        record->level = 0u;
    }
    if (values[EvtSystemTimeCreated].Type == EvtVarTypeFileTime) {
        record->timestamp_ns =
            filetime_to_unix_ns(values[EvtSystemTimeCreated].FileTimeVal);
    } else {
        record->timestamp_ns = 0u;
    }
    record->source = PL_RAS_SOURCE_WINDOWS_WHEA;
    /*
     * Event level and ID are preserved as source metadata. They are not a
     * portable corrected/uncorrected classification, so severity stays unknown.
     */
    record->severity = PL_RAS_SEVERITY_UNKNOWN;
    return record->record_id != 0u;
}

const char *pl_ras_provider_name(void) {
    return "Windows Event Log / Microsoft-Windows-WHEA-Logger (read-only)";
}

pl_ras_provider_status pl_ras_provider_poll(
    pl_ras_provider_cursor *cursor,
    pl_ras_batch *batch
) {
    EVT_HANDLE query;
    EVT_HANDLE events[PL_RAS_PROVIDER_BATCH_CAPACITY];
    wchar_t query_text[256];
    int query_length;
    DWORD returned = 0u;

    if (cursor == NULL || batch == NULL) return PL_RAS_PROVIDER_INVALID_ARGUMENT;
    for (size_t i = 0u; i < sizeof(*batch); ++i)
        ((unsigned char *)batch)[i] = 0u;

    query_length = swprintf(query_text, sizeof(query_text) / sizeof(query_text[0]),
        L"*[System[Provider[@Name='Microsoft-Windows-WHEA-Logger'] and EventRecordID > %llu]]",
        (unsigned long long)cursor->last_record_id);
    if (query_length < 0 ||
        (size_t)query_length >= sizeof(query_text) / sizeof(query_text[0])) {
        return PL_RAS_PROVIDER_READ_ERROR;
    }

    query = EvtQuery(NULL, L"System", query_text,
                     EvtQueryChannelPath | EvtQueryForwardDirection);
    if (query == NULL) return PL_RAS_PROVIDER_READ_ERROR;

    if (!EvtNext(query, PL_RAS_PROVIDER_BATCH_CAPACITY,
                 events, 0u, 0u, &returned)) {
        const DWORD error = GetLastError();
        EvtClose(query);
        return error == ERROR_NO_MORE_ITEMS
            ? PL_RAS_PROVIDER_OK : PL_RAS_PROVIDER_READ_ERROR;
    }

    for (DWORD i = 0u; i < returned; ++i) {
        pl_ras_record record = {0};
        if (read_event_values(events[i], &record)) {
            if (record.record_id > cursor->last_record_id) {
                batch->records[batch->count++] = record;
                cursor->last_record_id = record.record_id;
            }
        } else if (batch->dropped_records != UINT32_MAX) {
            ++batch->dropped_records;
        }
        EvtClose(events[i]);
    }
    batch->may_have_more =
        returned == PL_RAS_PROVIDER_BATCH_CAPACITY ? 1u : 0u;
    EvtClose(query);
    return PL_RAS_PROVIDER_OK;
}
