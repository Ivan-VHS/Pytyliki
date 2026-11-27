#include "ras_evidence.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    pl_ras_evidence_ledger ledger;
    pl_ras_evidence_ledger recovered;
    pl_ras_batch batch = {0};
    pl_ras_evidence_entry entry;
    pl_ras_provider_cursor cursor = {0};
    const char *path = "pytyliki-ras-evidence-test.snapshot";
    const char *pending_path = "pytyliki-ras-evidence-test.snapshot.pending";

    remove(path);
    remove(pending_path);
    pl_ras_evidence_init(&ledger);
    assert(pl_ras_evidence_valid(&ledger));

    batch.count = 2u;
    batch.records[0] = (pl_ras_record){
        1u, 100u, 17u, 2u, PL_RAS_SOURCE_WINDOWS_WHEA,
        PL_RAS_SEVERITY_UNKNOWN, 1u, 0u, PL_RAS_EVENT_SYSTEM_WIDE
    };
    batch.records[1] = (pl_ras_record){
        1u, 200u, 1u, 0u, PL_RAS_SOURCE_LINUX_EDAC,
        PL_RAS_SEVERITY_CORRECTED, 5u, 0u, PL_RAS_EVENT_SYSTEM_WIDE
    };
    assert(pl_ras_evidence_append(&ledger, &batch) == PL_RAS_EVIDENCE_OK);
    assert(ledger.count == 2u && ledger.last_sequence == 2u);
    assert(pl_ras_evidence_append(&ledger, &batch) == PL_RAS_EVIDENCE_DUPLICATE_ONLY);
    assert(ledger.count == 2u);

    {
        pl_ras_batch malformed = {0};
        malformed.count = 2u;
        malformed.records[0] = (pl_ras_record){
            9u, 300u, 1u, 0u, PL_RAS_SOURCE_LINUX_EDAC,
            PL_RAS_SEVERITY_CORRECTED, 2u, 0u, PL_RAS_EVENT_SYSTEM_WIDE
        };
        malformed.records[1] = malformed.records[0];
        malformed.records[1].timestamp_ns = 0u;
        assert(pl_ras_evidence_append(&ledger, &malformed) == PL_RAS_EVIDENCE_INVALID);
        assert(ledger.count == 2u && ledger.last_sequence == 2u);
    }

    assert(pl_ras_evidence_at(&ledger, 0u, &entry) == 0);
    assert(entry.source == PL_RAS_SOURCE_WINDOWS_WHEA);
    assert(pl_ras_evidence_at(&ledger, 1u, &entry) == 0);
    assert(entry.source == PL_RAS_SOURCE_LINUX_EDAC && entry.count_delta == 5u);

    cursor.last_record_id = 12u;
    cursor.initialized = 1u;
    cursor.controller_count = 1u;
    cursor.controller_ids[0] = 3u;
    cursor.previous_ce[0] = 9u;
    cursor.previous_ue[0] = 2u;
    pl_ras_evidence_capture_cursor(&ledger, &cursor);
    memset(&cursor, 0, sizeof(cursor));
    pl_ras_evidence_restore_cursor(&ledger, &cursor);
    assert(cursor.last_record_id == 12u && cursor.previous_ce[0] == 9u);
    assert(cursor.initialized == 1u && cursor.controller_ids[0] == 3u);

    for (uint64_t i = 2u; i <= PL_RAS_EVIDENCE_CAPACITY + 2u; ++i) {
        pl_ras_batch next = {0};
        next.count = 1u;
        next.records[0] = (pl_ras_record){
            i, 300u + i, 17u, 2u, PL_RAS_SOURCE_WINDOWS_WHEA,
            PL_RAS_SEVERITY_UNKNOWN, 1u, 0u, PL_RAS_EVENT_SYSTEM_WIDE
        };
        assert(pl_ras_evidence_append(&ledger, &next) == PL_RAS_EVIDENCE_OK);
    }
    assert(ledger.count == PL_RAS_EVIDENCE_CAPACITY);
    assert(ledger.dropped > 0u);
    assert(pl_ras_evidence_valid(&ledger));

    assert(pl_ras_evidence_save(path, &ledger) == PL_RAS_EVIDENCE_OK);
    memset(&recovered, 0, sizeof(recovered));
    assert(pl_ras_evidence_load(path, &recovered) == PL_RAS_EVIDENCE_OK);
    assert(recovered.generation == ledger.generation);
    assert(recovered.last_sequence == ledger.last_sequence);
    assert(recovered.count == ledger.count && recovered.dropped == ledger.dropped);
    assert(pl_ras_evidence_valid(&recovered));

    remove(path);
    remove(pending_path);
    puts("Pytyliki RAS evidence journal tests passed.");
    return 0;
}
