#include "ras_provider_cli.h"
#include "ras_provider.h"
#include <stdio.h>
static const char *source_name(uint32_t source) {
    switch (source) {
    case PL_RAS_SOURCE_WINDOWS_WHEA: return "windows-whea";
    case PL_RAS_SOURCE_LINUX_EDAC: return "linux-edac";
    default: return "unknown";
    }
}
static const char *severity_name(uint32_t severity) {
    switch (severity) {
    case PL_RAS_SEVERITY_CORRECTED: return "corrected";
    case PL_RAS_SEVERITY_UNCORRECTED: return "uncorrected";
    default: return "unknown";
    }
}
int pl_ras_provider_cli(void) {
    pl_ras_provider_cursor cursor = {0};
    pl_ras_batch batch;
    const pl_ras_provider_status status = pl_ras_provider_poll(&cursor, &batch);
    if (status == PL_RAS_PROVIDER_UNSUPPORTED) {
        printf("{\"schema\":\"pytyliki.ras-provider-report.v1\","
               "\"provider\":\"%s\",\"status\":\"unsupported\","
               "\"read_only\":true,\"events\":[]}\n",
               pl_ras_provider_name());
        pl_ras_provider_close(&cursor);
        return 0;
    }
    if (status != PL_RAS_PROVIDER_OK) {
        pl_ras_provider_close(&cursor);
        fprintf(stderr, "RAS provider read failed (status=%d)\n", (int)status);
        return 1;
    }
    printf("{\"schema\":\"pytyliki.ras-provider-report.v1\","
           "\"provider\":\"%s\",\"status\":\"ok\","
           "\"read_only\":true,\"hardware_writes\":false,"
           "\"worker_attribution\":\"not_inferred\",\"events\":[",
           pl_ras_provider_name());
    for (size_t i = 0u; i < batch.count; ++i) {
        const pl_ras_record *record = &batch.records[i];
        if (i != 0u) putchar(',');
        printf("{\"recordId\":%llu,\"timestampNs\":%llu,"
               "\"clockDomain\":\"%s\",\"source\":\"%s\","
               "\"eventId\":%u,\"level\":%u,\"severity\":\"%s\","
               "\"countDelta\":%llu,\"controllerId\":%u,\"flags\":%u,"
               "\"attribution\":\"system-wide\"}",
               (unsigned long long)record->record_id,
               (unsigned long long)record->timestamp_ns,
               record->source == PL_RAS_SOURCE_WINDOWS_WHEA
                   ? "unix-realtime" : "process-monotonic",
               source_name(record->source), record->event_id, record->level,
               severity_name(record->severity),
               (unsigned long long)record->count_delta,
               record->controller_id, record->flags);
    }
    printf("],\"droppedRecords\":%u,\"mayHaveMore\":%s}\n",
           batch.dropped_records, batch.may_have_more ? "true" : "false");
    pl_ras_provider_close(&cursor);
    return 0;
}
