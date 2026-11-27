#include "ras_provider.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winevt.h>
#include <stdint.h>
#include <wchar.h>

#pragma comment(lib, "wevtapi.lib")

static uint64_t filetime_to_unix_ns(uint64_t ticks) {
    const uint64_t epoch = UINT64_C(116444736000000000);
    if (ticks < epoch) return 0u;
    ticks -= epoch;
    if (ticks > UINT64_MAX / UINT64_C(100)) return 0u;
    return ticks * UINT64_C(100);
}

static int read_event_values(EVT_HANDLE event, pl_ras_record *record) {
    EVT_VARIANT values[EvtSystemPropertyIdEND];
    DWORD buffer_used = 0u;
    DWORD property_count = 0u;
    if (!EvtRender(NULL, event, EvtRenderEventValues, (DWORD)sizeof(values),
                   values, &buffer_used, &property_count)) return 0;
    if (property_count <= (DWORD)EvtSystemTimeCreated ||
        property_count <= (DWORD)EvtSystemEventRecordId) return 0;
    if (values[EvtSystemEventRecordId].Type != EvtVarTypeUInt64) return 0;
    record->record_id = values[EvtSystemEventRecordId].UInt64Val;
    if (values[EvtSystemEventID].Type == EvtVarTypeUInt16)
        record->event_id = values[EvtSystemEventID].UInt16Val;
    else if (values[EvtSystemEventID].Type == EvtVarTypeUInt32)
        record->event_id = values[EvtSystemEventID].UInt32Val;
    else return 0;
    record->level = values[EvtSystemLevel].Type == EvtVarTypeByte
        ? values[EvtSystemLevel].ByteVal : 0u;
    record->timestamp_ns = values[EvtSystemTimeCreated].Type == EvtVarTypeFileTime
        ? filetime_to_unix_ns(values[EvtSystemTimeCreated].FileTimeVal) : 0u;
    record->source = PL_RAS_SOURCE_WINDOWS_WHEA;
    record->severity = PL_RAS_SEVERITY_UNKNOWN;
    record->count_delta = 1u;
    record->controller_id = 0u;
    record->flags = PL_RAS_EVENT_SYSTEM_WIDE;
    /* Preserve raw event metadata; do not infer CPER severity from event ID. */
    return record->record_id != 0u;
}

static int ensure_subscription(pl_ras_provider_cursor *cursor) {
    HANDLE notification;
    EVT_HANDLE subscription;
    static const wchar_t query[] =
        L"*[System[Provider[@Name='Microsoft-Windows-WHEA-Logger']]]";
    if (cursor->provider_state == 1u) return 1;
    notification = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (notification == NULL) return 0;
    subscription = EvtSubscribe(
        NULL, notification, L"System", query, NULL, NULL, NULL,
        EvtSubscribeToFutureEvents);
    if (subscription == NULL) {
        CloseHandle(notification);
        return 0;
    }
    cursor->notification_handle = (uintptr_t)notification;
    cursor->subscription_handle = (uintptr_t)subscription;
    cursor->provider_state = 1u;
    return 1;
}

static void append_event(pl_ras_provider_cursor *cursor, pl_ras_batch *batch,
                         EVT_HANDLE event) {
    pl_ras_record record = {0};
    if (!read_event_values(event, &record)) {
        if (batch->dropped_records != UINT32_MAX) ++batch->dropped_records;
        return;
    }
    if (record.record_id <= cursor->last_record_id) return;
    if (batch->count >= PL_RAS_PROVIDER_BATCH_CAPACITY) {
        batch->may_have_more = 1u;
        if (batch->dropped_records != UINT32_MAX) ++batch->dropped_records;
        return;
    }
    batch->records[batch->count++] = record;
    cursor->last_record_id = record.record_id;
}

static int drain_result(EVT_HANDLE result, pl_ras_provider_cursor *cursor,
                        pl_ras_batch *batch, int *has_more) {
    EVT_HANDLE events[PL_RAS_PROVIDER_BATCH_CAPACITY];
    DWORD returned = 0u;
    const DWORD remaining = (DWORD)(PL_RAS_PROVIDER_BATCH_CAPACITY - batch->count);
    if (remaining == 0u) {
        *has_more = 1;
        return 1;
    }
    if (!EvtNext(result, remaining, events, 0u, 0u, &returned)) {
        const DWORD error = GetLastError();
        if (error == ERROR_NO_MORE_ITEMS || error == ERROR_TIMEOUT) return 1;
        return 0;
    }
    for (DWORD i = 0u; i < returned; ++i) {
        append_event(cursor, batch, events[i]);
        EvtClose(events[i]);
    }
    if (returned == remaining) *has_more = 1;
    return 1;
}

const char *pl_ras_provider_name(void) {
    return "Windows WHEA-Logger / Event Log ETW bridge (read-only)";
}

pl_ras_provider_status pl_ras_provider_poll(
    pl_ras_provider_cursor *cursor, pl_ras_batch *batch) {
    EVT_HANDLE query;
    wchar_t query_text[256];
    int query_length;
    int has_more = 0;
    static const wchar_t history_query[] =
        L"*[System[Provider[@Name='Microsoft-Windows-WHEA-Logger'] and EventRecordID > %llu]]";

    if (cursor == NULL || batch == NULL) return PL_RAS_PROVIDER_INVALID_ARGUMENT;
    ZeroMemory(batch, sizeof(*batch));
    if (!ensure_subscription(cursor)) return PL_RAS_PROVIDER_READ_ERROR;

    query_length = swprintf(query_text,
        sizeof(query_text) / sizeof(query_text[0]), history_query,
        (unsigned long long)cursor->last_record_id);
    if (query_length < 0 ||
        (size_t)query_length >= sizeof(query_text) / sizeof(query_text[0]))
        return PL_RAS_PROVIDER_READ_ERROR;

    query = EvtQuery(NULL, L"System", query_text,
        EvtQueryChannelPath | EvtQueryForwardDirection);
    if (query == NULL) return PL_RAS_PROVIDER_READ_ERROR;
    if (!drain_result(query, cursor, batch, &has_more)) {
        EvtClose(query);
        return PL_RAS_PROVIDER_READ_ERROR;
    }
    EvtClose(query);

    /* EvtSubscribe's pull model signals this event for new WHEA ETW records.
       EvtNext drains them without blocking; the historical query above catches
       records that arrived between process start and subscription setup. */
    if (!has_more && batch->count < PL_RAS_PROVIDER_BATCH_CAPACITY) {
        EVT_HANDLE subscription = (EVT_HANDLE)cursor->subscription_handle;
        if (WaitForSingleObject((HANDLE)cursor->notification_handle, 0u) == WAIT_OBJECT_0) {
            if (!drain_result(subscription, cursor, batch, &has_more))
                return PL_RAS_PROVIDER_READ_ERROR;
        }
    }
    batch->may_have_more = has_more ? 1u : batch->may_have_more;
    return PL_RAS_PROVIDER_OK;
}

void pl_ras_provider_close(pl_ras_provider_cursor *cursor) {
    if (cursor == NULL) return;
    if (cursor->subscription_handle != 0u)
        EvtClose((EVT_HANDLE)cursor->subscription_handle);
    if (cursor->notification_handle != 0u)
        CloseHandle((HANDLE)cursor->notification_handle);
    cursor->subscription_handle = 0u;
    cursor->notification_handle = 0u;
    cursor->provider_state = 0u;
}
