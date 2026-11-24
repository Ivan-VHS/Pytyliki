#ifndef PYTYLIKI_SNAPSHOT_STORE_H
#define PYTYLIKI_SNAPSHOT_STORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SNAPSHOT_MAX_BYTES (16u * 1024u * 1024u)
#define PL_SNAPSHOT_PATH_MAX 768u

typedef enum {
    PL_SNAPSHOT_OK = 0,
    PL_SNAPSHOT_INVALID = 1,
    PL_SNAPSHOT_IO = 2,
    PL_SNAPSHOT_INTEGRITY = 3,
    PL_SNAPSHOT_LIMIT = 4,
    PL_SNAPSHOT_NOT_FOUND = 5,
    PL_SNAPSHOT_CONFLICT = 6
} pl_snapshot_status;

typedef struct {
    uint32_t schema_version;
    uint64_t generation;
    size_t payload_size;
    uint32_t crc32;
    int recovered_pending_write;
} pl_snapshot_info;

/* Save an opaque, bounded application snapshot. A flushed .pending file is
 * atomically promoted where the host filesystem supports replacement rename. */
pl_snapshot_status pl_snapshot_save_atomic(
    const char *path, uint32_t schema_version, uint64_t generation,
    const void *payload, size_t payload_size, pl_snapshot_info *info
);

/* Recover the newest valid primary/pending snapshot, then load it. The output
 * buffer is untouched when validation fails or capacity is insufficient. */
pl_snapshot_status pl_snapshot_recover(
    const char *path, void *output, size_t output_capacity,
    pl_snapshot_info *info
);

const char *pl_snapshot_status_name(pl_snapshot_status status);

#ifdef __cplusplus
}
#endif
#endif
