#include "snapshot_store.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void cleanup(const char *path) {
    char pending[PL_SNAPSHOT_PATH_MAX + 16u];
    (void)remove(path);
    (void)snprintf(pending, sizeof pending, "%s.pending", path);
    (void)remove(pending);
}

int main(void) {
    const char *path = "pytyliki-snapshot-store-test.bin";
    const unsigned char first[] = {0x00u, 0x01u, 0x7fu, 0xffu};
    const unsigned char second[] = "recovered-state";
    unsigned char output[64] = {0};
    pl_snapshot_info info;

    cleanup(path);
    assert(pl_snapshot_save_atomic(path, 1u, 1u, first, sizeof first, &info) == PL_SNAPSHOT_OK);
    assert(info.generation == 1u && info.payload_size == sizeof first);
    assert(pl_snapshot_save_atomic(path, 1u, 1u, second, sizeof second, NULL) == PL_SNAPSHOT_CONFLICT);
    assert(pl_snapshot_save_atomic(path, 2u, 2u, second, sizeof second, NULL) == PL_SNAPSHOT_OK);

    memset(output, 0, sizeof output);
    assert(pl_snapshot_recover(path, output, sizeof output, &info) == PL_SNAPSHOT_OK);
    assert(info.schema_version == 2u && info.generation == 2u);
    assert(info.recovered_pending_write == 0);
    assert(info.payload_size == sizeof second);
    assert(memcmp(output, second, sizeof second) == 0);

    memset(output, 0x5a, sizeof output);
    assert(pl_snapshot_recover(path, output, 2u, &info) == PL_SNAPSHOT_LIMIT);
    assert(output[0] == 0x5au && output[1] == 0x5au);

    assert(pl_snapshot_save_atomic(path, 3u, 3u, NULL, 0u, NULL) == PL_SNAPSHOT_OK);
    assert(pl_snapshot_recover(path, output, sizeof output, &info) == PL_SNAPSHOT_OK);
    assert(info.generation == 3u && info.payload_size == 0u);

    assert(pl_snapshot_recover("missing-pytyliki-snapshot.bin", output,
                               sizeof output, NULL) == PL_SNAPSHOT_NOT_FOUND);
    assert(pl_snapshot_save_atomic(path, 0u, 4u, first, sizeof first, NULL) == PL_SNAPSHOT_INVALID);
    assert(strcmp(pl_snapshot_status_name(PL_SNAPSHOT_OK), "ok") == 0);
    cleanup(path);
    puts("Pytyliki durable snapshot tests passed.");
    return 0;
}
