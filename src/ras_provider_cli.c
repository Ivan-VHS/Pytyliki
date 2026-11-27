#include "ras_provider_cli.h"

#include "ras_provider.h"

#include <stdio.h>

int pl_ras_provider_cli(void) {
    pl_ras_provider_cursor cursor = {0u};
    pl_ras_batch batch;
    const pl_ras_provider_status status =
        pl_ras_provider_poll(&cursor, &batch);

    printf("Pytyliki RAS provider: %s\n", pl_ras_provider_name());
    if (status == PL_RAS_PROVIDER_UNSUPPORTED) {
        puts("status=unsupported platform; no OS hardware-error records were read");
        return 0;
    }
    if (status != PL_RAS_PROVIDER_OK) {
        fprintf(stderr, "RAS provider read failed (status=%d)\n", (int)status);
        return 1;
    }
    printf("records=%zu dropped=%u may_have_more=%s cursor=%llu\n",
           batch.count, batch.dropped_records,
           batch.may_have_more ? "yes" : "no",
           (unsigned long long)cursor.last_record_id);
    for (size_t i = 0u; i < batch.count; ++i) {
        const pl_ras_record *record = &batch.records[i];
        printf("record_id=%llu timestamp_ns=%llu event_id=%u level=%u "
               "classification=unknown attribution=system-only\n",
               (unsigned long long)record->record_id,
               (unsigned long long)record->timestamp_ns,
               record->event_id, record->level);
    }
    puts("read_only=yes hardware_writes=no worker_attribution=not_inferred");
    return 0;
}
