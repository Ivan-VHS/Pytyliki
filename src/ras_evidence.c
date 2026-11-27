#include "ras_evidence.h"
#include "snapshot_store.h"
#include <stdio.h>
#include <string.h>

static int record_valid(const pl_ras_record *record) {
    if (record == NULL || record->record_id == 0u ||
        record->timestamp_ns == 0u || record->count_delta == 0u) return 0;
    if (record->source != PL_RAS_SOURCE_WINDOWS_WHEA &&
        record->source != PL_RAS_SOURCE_LINUX_EDAC) return 0;
    if (record->severity > PL_RAS_SEVERITY_UNCORRECTED) return 0;
    if (record->source == PL_RAS_SOURCE_LINUX_EDAC &&
        record->severity != PL_RAS_SEVERITY_CORRECTED &&
        record->severity != PL_RAS_SEVERITY_UNCORRECTED) return 0;
    return 1;
}
void pl_ras_evidence_init(pl_ras_evidence_ledger *ledger) {
    if (ledger == NULL) return;
    memset(ledger, 0, sizeof(*ledger));
    ledger->schema_version = PL_RAS_EVIDENCE_SCHEMA_VERSION;
    ledger->generation = 1u;
}
int pl_ras_evidence_valid(const pl_ras_evidence_ledger *ledger) {
    size_t i;
    uint64_t previous_sequence = 0u;
    if (ledger == NULL || ledger->schema_version != PL_RAS_EVIDENCE_SCHEMA_VERSION ||
        ledger->count > PL_RAS_EVIDENCE_CAPACITY ||
        ledger->head >= PL_RAS_EVIDENCE_CAPACITY ||
        ledger->controller_count > PL_RAS_EDAC_MAX_CONTROLLERS ||
        ledger->generation == 0u) return 0;
    if (ledger->count == 0u) return ledger->last_sequence == 0u;
    if (ledger->last_sequence < ledger->count) return 0;
    for (i = 0u; i < ledger->count; ++i) {
        size_t index = (ledger->head + PL_RAS_EVIDENCE_CAPACITY -
                        ledger->count + i) % PL_RAS_EVIDENCE_CAPACITY;
        const pl_ras_evidence_entry *entry = &ledger->entries[index];
        if (entry->sequence == 0u || entry->provider_record_id == 0u ||
            entry->timestamp_ns == 0u || entry->count_delta == 0u ||
            (entry->source != PL_RAS_SOURCE_WINDOWS_WHEA &&
             entry->source != PL_RAS_SOURCE_LINUX_EDAC) ||
            entry->severity > PL_RAS_SEVERITY_UNCORRECTED ||
            (i != 0u && entry->sequence <= previous_sequence)) return 0;
        previous_sequence = entry->sequence;
    }
    return previous_sequence == ledger->last_sequence;
}
static int record_already_present(const pl_ras_evidence_ledger *ledger,
                                  const pl_ras_record *record) {
    size_t i;
    for (i = 0u; i < ledger->count; ++i) {
        size_t index = (ledger->head + PL_RAS_EVIDENCE_CAPACITY -
                        ledger->count + i) % PL_RAS_EVIDENCE_CAPACITY;
        const pl_ras_evidence_entry *entry = &ledger->entries[index];
        if (entry->source == record->source &&
            entry->provider_record_id == record->record_id &&
            entry->controller_id == record->controller_id) return 1;
    }
    return 0;
}
pl_ras_evidence_status pl_ras_evidence_append(
    pl_ras_evidence_ledger *ledger, const pl_ras_batch *batch) {
    const pl_ras_record *fresh[PL_RAS_PROVIDER_BATCH_CAPACITY];
    size_t fresh_count = 0u;
    size_t i;
    if (!pl_ras_evidence_valid(ledger) || batch == NULL ||
        batch->count > PL_RAS_PROVIDER_BATCH_CAPACITY) return PL_RAS_EVIDENCE_INVALID;

    /* Validate and deduplicate the complete batch before mutating the journal. */
    for (i = 0u; i < batch->count; ++i) {
        const pl_ras_record *record = &batch->records[i];
        size_t previous;
        if (!record_valid(record)) return PL_RAS_EVIDENCE_INVALID;
        if (record_already_present(ledger, record)) continue;
        for (previous = 0u; previous < fresh_count; ++previous) {
            if (fresh[previous]->source == record->source &&
                fresh[previous]->record_id == record->record_id &&
                fresh[previous]->controller_id == record->controller_id) break;
        }
        if (previous == fresh_count) fresh[fresh_count++] = record;
    }
    if (fresh_count == 0u) return PL_RAS_EVIDENCE_DUPLICATE_ONLY;
    if (ledger->last_sequence > UINT64_MAX - (uint64_t)fresh_count ||
        ledger->generation > UINT64_MAX - (uint64_t)fresh_count)
        return PL_RAS_EVIDENCE_CAPACITY_ERROR;

    for (i = 0u; i < fresh_count; ++i) {
        const pl_ras_record *record = fresh[i];
        pl_ras_evidence_entry *entry = &ledger->entries[ledger->head];
        memset(entry, 0, sizeof(*entry));
        entry->sequence = ++ledger->last_sequence;
        entry->provider_record_id = record->record_id;
        entry->timestamp_ns = record->timestamp_ns;
        entry->count_delta = record->count_delta;
        entry->source = record->source;
        entry->event_id = record->event_id;
        entry->severity = record->severity;
        entry->controller_id = record->controller_id;
        entry->flags = record->flags;
        ledger->head = (ledger->head + 1u) % PL_RAS_EVIDENCE_CAPACITY;
        if (ledger->count < PL_RAS_EVIDENCE_CAPACITY) {
            ledger->count++;
        } else if (ledger->dropped != UINT32_MAX) {
            ledger->dropped++;
        }
        if (record->source == PL_RAS_SOURCE_WINDOWS_WHEA &&
            record->record_id > ledger->last_whea_record_id)
            ledger->last_whea_record_id = record->record_id;
        ledger->generation++;
    }
    return PL_RAS_EVIDENCE_OK;
}
int pl_ras_evidence_at(const pl_ras_evidence_ledger *ledger,
                       size_t oldest_index, pl_ras_evidence_entry *out) {
    size_t index;
    if (out == NULL || !pl_ras_evidence_valid(ledger) ||
        oldest_index >= ledger->count) return -1;
    index = (ledger->head + PL_RAS_EVIDENCE_CAPACITY -
             ledger->count + oldest_index) % PL_RAS_EVIDENCE_CAPACITY;
    *out = ledger->entries[index];
    return 0;
}
void pl_ras_evidence_restore_cursor(const pl_ras_evidence_ledger *ledger,
                                    pl_ras_provider_cursor *cursor) {
    if (ledger == NULL || cursor == NULL ||
        !pl_ras_evidence_valid(ledger)) return;
    memset(cursor, 0, sizeof(*cursor));
    cursor->last_record_id = ledger->last_whea_record_id;
    memcpy(cursor->previous_ce, ledger->previous_ce, sizeof(cursor->previous_ce));
    memcpy(cursor->previous_ue, ledger->previous_ue, sizeof(cursor->previous_ue));
    memcpy(cursor->controller_ids, ledger->controller_ids, sizeof(cursor->controller_ids));
    cursor->controller_count = ledger->controller_count;
    cursor->initialized = ledger->provider_initialized;
}
void pl_ras_evidence_capture_cursor(pl_ras_evidence_ledger *ledger,
                                    const pl_ras_provider_cursor *cursor) {
    if (ledger == NULL || cursor == NULL ||
        cursor->controller_count > PL_RAS_EDAC_MAX_CONTROLLERS) return;
    ledger->last_whea_record_id = cursor->last_record_id;
    memcpy(ledger->previous_ce, cursor->previous_ce, sizeof(ledger->previous_ce));
    memcpy(ledger->previous_ue, cursor->previous_ue, sizeof(ledger->previous_ue));
    memcpy(ledger->controller_ids, cursor->controller_ids, sizeof(ledger->controller_ids));
    ledger->controller_count = cursor->controller_count;
    ledger->provider_initialized = cursor->initialized;
    if (ledger->generation != UINT64_MAX) ledger->generation++;
}
pl_ras_evidence_status pl_ras_evidence_save(
    const char *path, pl_ras_evidence_ledger *ledger) {
    pl_snapshot_info info;
    if (path == NULL || path[0] == '\0' || !pl_ras_evidence_valid(ledger))
        return PL_RAS_EVIDENCE_INVALID;
    if (pl_snapshot_save_atomic(path, PL_RAS_EVIDENCE_SCHEMA_VERSION,
            ledger->generation, ledger, sizeof(*ledger), &info) != PL_SNAPSHOT_OK)
        return PL_RAS_EVIDENCE_IO_ERROR;
    return PL_RAS_EVIDENCE_OK;
}
pl_ras_evidence_status pl_ras_evidence_load(
    const char *path, pl_ras_evidence_ledger *ledger) {
    pl_snapshot_info info;
    pl_ras_evidence_ledger recovered;
    pl_snapshot_status status;
    if (path == NULL || path[0] == '\0' || ledger == NULL)
        return PL_RAS_EVIDENCE_INVALID;
    memset(&recovered, 0, sizeof(recovered));
    status = pl_snapshot_recover(path, &recovered, sizeof(recovered), &info);
    if (status == PL_SNAPSHOT_NOT_FOUND) return PL_RAS_EVIDENCE_DUPLICATE_ONLY;
    if (status != PL_SNAPSHOT_OK || info.schema_version != PL_RAS_EVIDENCE_SCHEMA_VERSION ||
        info.payload_size != sizeof(recovered) || info.generation != recovered.generation ||
        !pl_ras_evidence_valid(&recovered))
        return PL_RAS_EVIDENCE_INTEGRITY_ERROR;
    *ledger = recovered;
    return PL_RAS_EVIDENCE_OK;
}
int pl_ras_evidence_cli(const char *path) {
    pl_ras_evidence_ledger ledger;
    pl_ras_provider_cursor cursor = {0};
    pl_ras_batch batch;
    pl_ras_evidence_status status = pl_ras_evidence_load(path, &ledger);
    if (status == PL_RAS_EVIDENCE_DUPLICATE_ONLY) pl_ras_evidence_init(&ledger);
    else if (status != PL_RAS_EVIDENCE_OK) {
        fprintf(stderr, "RAS evidence journal recovery failed (status=%d)\n", (int)status);
        return 2;
    }
    pl_ras_evidence_restore_cursor(&ledger, &cursor);
    {
        const pl_ras_provider_status provider_status =
            pl_ras_provider_poll(&cursor, &batch);
        if (provider_status == PL_RAS_PROVIDER_UNSUPPORTED) {
            pl_ras_provider_close(&cursor);
            puts("RAS evidence: provider unsupported on this platform; journal unchanged");
            return 0;
        }
        if (provider_status != PL_RAS_PROVIDER_OK) {
            pl_ras_provider_close(&cursor);
            fprintf(stderr, "RAS provider read failed (status=%d)\\n",
                    (int)provider_status);
            return 1;
        }
    }
    status = pl_ras_evidence_append(&ledger, &batch);
    pl_ras_evidence_capture_cursor(&ledger, &cursor);
    pl_ras_provider_close(&cursor);
    if (status != PL_RAS_EVIDENCE_OK &&
        status != PL_RAS_EVIDENCE_DUPLICATE_ONLY) {
        fprintf(stderr, "RAS evidence append failed (status=%d)\n", (int)status);
        return 1;
    }
    status = pl_ras_evidence_save(path, &ledger);
    if (status != PL_RAS_EVIDENCE_OK) {
        fprintf(stderr, "RAS evidence save failed (status=%d)\n", (int)status);
        return 1;
    }
    printf("Pytyliki RAS evidence journal: schema=v1 generation=%llu "
           "entries=%u dropped=%u last_sequence=%llu read_only=yes "
           "worker_attribution=none\n",
           (unsigned long long)ledger.generation, ledger.count, ledger.dropped,
           (unsigned long long)ledger.last_sequence);
    return 0;
}
