#include "runtime_transaction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *copy_bytes(const void *data, size_t size) {
    unsigned char *copy;
    if (size == 0u) {
        return (unsigned char *)malloc(1u);
    }
    if (!data) return NULL;
    copy = (unsigned char *)malloc(size);
    if (!copy) return NULL;
    memcpy(copy, data, size);
    return copy;
}

static void set_error(pl_runtime_transaction *tx, const char *message) {
    if (!tx) return;
    tx->failed = 1;
    if (!message) message = "runtime transaction failed";
    (void)snprintf(tx->last_error, sizeof(tx->last_error), "%s", message);
}

static int usable(const pl_runtime_transaction *tx) {
    return tx && tx->active && !tx->failed && tx->committed && tx->staged;
}

static void notify(pl_runtime_transaction *tx, pl_rt_event_kind kind, const char *name) {
    if (tx && tx->event_callback) {
        tx->event_callback(kind, name ? name : "", tx->generation, tx->event_context);
    }
}

uint32_t pl_rt_crc32(const unsigned char *data, size_t size) {
    uint32_t crc = UINT32_C(0xFFFFFFFF);
    size_t i;
    if (!data && size != 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        unsigned bit;
        crc ^= (uint32_t)data[i];
        for (bit = 0u; bit < 8u; ++bit) {
            const uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
            crc = (crc >> 1u) ^ (UINT32_C(0xEDB88320) & mask);
        }
    }
    return ~crc;
}

pl_rt_status pl_rt_init(pl_runtime_transaction *tx, const void *data, size_t size,
                        uint32_t schema_version, pl_rt_event_fn callback, void *context) {
    if (!tx || (!data && size != 0u) || size > PL_RT_MAX_BYTES || schema_version == 0u) {
        return size > PL_RT_MAX_BYTES ? PL_RT_LIMIT : PL_RT_INVALID;
    }
    memset(tx, 0, sizeof(*tx));
    tx->committed = copy_bytes(data, size);
    tx->staged = copy_bytes(data, size);
    if (!tx->committed || !tx->staged) {
        free(tx->committed);
        free(tx->staged);
        memset(tx, 0, sizeof(*tx));
        return PL_RT_LIMIT;
    }
    tx->committed_size = size;
    tx->staged_size = size;
    tx->schema_version = schema_version;
    tx->committed_schema_version = schema_version;
    tx->committed_crc32 = pl_rt_crc32(tx->committed, size);
    tx->staged_crc32 = tx->committed_crc32;
    tx->event_callback = callback;
    tx->event_context = context;
    tx->active = 1;
    return PL_RT_OK;
}

void pl_rt_destroy(pl_runtime_transaction *tx) {
    if (!tx) return;
    free(tx->committed);
    free(tx->staged);
    memset(tx, 0, sizeof(*tx));
}

pl_rt_status pl_rt_patch(pl_runtime_transaction *tx, size_t offset, size_t remove_size,
                         const void *replacement, size_t replacement_size) {
    size_t new_size;
    unsigned char *next;
    if (!usable(tx)) return PL_RT_STATE;
    if ((!replacement && replacement_size != 0u) || offset > tx->staged_size
        || remove_size > tx->staged_size - offset) return PL_RT_INVALID;
    if (replacement_size > PL_RT_MAX_BYTES - (tx->staged_size - remove_size)) return PL_RT_LIMIT;
    new_size = tx->staged_size - remove_size + replacement_size;
    next = (unsigned char *)malloc(new_size ? new_size : 1u);
    if (!next) return PL_RT_LIMIT;
    if (offset) memcpy(next, tx->staged, offset);
    if (replacement_size) memcpy(next + offset, replacement, replacement_size);
    if (tx->staged_size > offset + remove_size) {
        memcpy(next + offset + replacement_size, tx->staged + offset + remove_size,
               tx->staged_size - offset - remove_size);
    }
    free(tx->staged);
    tx->staged = next;
    tx->staged_size = new_size;
    tx->staged_crc32 = pl_rt_crc32(next, new_size);
    return PL_RT_OK;
}

pl_rt_status pl_rt_rebind(pl_runtime_transaction *tx, uint64_t binding) {
    if (!usable(tx)) return PL_RT_STATE;
    tx->binding = binding;
    return PL_RT_OK;
}

pl_rt_status pl_rt_remap(pl_runtime_transaction *tx, const size_t *target_to_source,
                         size_t target_size) {
    size_t i;
    unsigned char *next;
    if (!usable(tx) || (!target_to_source && target_size != 0u)) return PL_RT_INVALID;
    if (target_size > PL_RT_MAX_BYTES) return PL_RT_LIMIT;
    for (i = 0u; i < target_size; ++i) {
        if (target_to_source[i] >= tx->staged_size) return PL_RT_INVALID;
    }
    next = (unsigned char *)malloc(target_size ? target_size : 1u);
    if (!next) return PL_RT_LIMIT;
    for (i = 0u; i < target_size; ++i) next[i] = tx->staged[target_to_source[i]];
    free(tx->staged);
    tx->staged = next;
    tx->staged_size = target_size;
    tx->staged_crc32 = pl_rt_crc32(next, target_size);
    return PL_RT_OK;
}

pl_rt_status pl_rt_hot_swap(pl_runtime_transaction *tx, const void *data, size_t size) {
    unsigned char *next;
    if (!usable(tx) || (!data && size != 0u)) return PL_RT_INVALID;
    if (size > PL_RT_MAX_BYTES) return PL_RT_LIMIT;
    next = copy_bytes(data, size);
    if (!next) return PL_RT_LIMIT;
    free(tx->staged);
    tx->staged = next;
    tx->staged_size = size;
    tx->staged_crc32 = pl_rt_crc32(next, size);
    notify(tx, PL_RT_EVENT_HOT_SWAP, "hotSwap");
    return PL_RT_OK;
}

pl_rt_status pl_rt_migrate(pl_runtime_transaction *tx, uint32_t target_schema,
                           pl_rt_migration_fn migration, void *context) {
    const unsigned char *output = NULL;
    size_t output_size = 0u;
    unsigned char *next;
    if (!usable(tx) || !migration || target_schema <= tx->schema_version) return PL_RT_INVALID;
    if (migration(tx->staged, tx->staged_size, tx->schema_version, target_schema,
                  &output, &output_size, context) != 0) return PL_RT_THROWN;
    if ((!output && output_size != 0u) || output_size > PL_RT_MAX_BYTES) return PL_RT_LIMIT;
    next = copy_bytes(output, output_size);
    if (!next) return PL_RT_LIMIT;
    free(tx->staged);
    tx->staged = next;
    tx->staged_size = output_size;
    tx->staged_crc32 = pl_rt_crc32(next, output_size);
    tx->schema_version = target_schema;
    notify(tx, PL_RT_EVENT_MIGRATE, "migrate");
    return PL_RT_OK;
}

static pl_rt_status commit(pl_runtime_transaction *tx, uint64_t expected_generation,
                           pl_rt_event_kind event_kind, const char *event_name) {
    unsigned char *next;
    if (!usable(tx)) return PL_RT_STATE;
    if (expected_generation != tx->generation) return PL_RT_CONFLICT;
    if (tx->generation == UINT64_MAX) return PL_RT_LIMIT;
    if (tx->staged_crc32 != pl_rt_crc32(tx->staged, tx->staged_size)) {
        set_error(tx, "staged payload checksum mismatch");
        return PL_RT_INTEGRITY;
    }
    next = copy_bytes(tx->staged, tx->staged_size);
    if (!next) return PL_RT_LIMIT;
    free(tx->committed);
    tx->committed = next;
    tx->committed_size = tx->staged_size;
    tx->committed_crc32 = tx->staged_crc32;
    tx->committed_schema_version = tx->schema_version;
    ++tx->generation;
    notify(tx, event_kind, event_name);
    return PL_RT_OK;
}

pl_rt_status pl_rt_flash(pl_runtime_transaction *tx, uint64_t expected_generation) {
    return commit(tx, expected_generation, PL_RT_EVENT_FLASH, "flash");
}

pl_rt_status pl_rt_reflash(pl_runtime_transaction *tx, uint64_t expected_generation) {
    return commit(tx, expected_generation, PL_RT_EVENT_REFLASH, "reflash");
}

pl_rt_status pl_rt_sync(pl_runtime_transaction *tx) {
    if (!tx) return PL_RT_INVALID;
    return commit(tx, tx->generation, PL_RT_EVENT_FLASH, "sync");
}

pl_rt_status pl_rt_rollback(pl_runtime_transaction *tx) {
    unsigned char *next;
    if (!tx || !tx->active || !tx->committed) return PL_RT_STATE;
    next = copy_bytes(tx->committed, tx->committed_size);
    if (!next) return PL_RT_LIMIT;
    free(tx->staged);
    tx->staged = next;
    tx->staged_size = tx->committed_size;
    tx->staged_crc32 = tx->committed_crc32;
    tx->schema_version = tx->committed_schema_version;
    tx->failed = 0;
    tx->last_error[0] = '\0';
    notify(tx, PL_RT_EVENT_ROLLBACK, "rollback");
    return PL_RT_OK;
}

pl_rt_status pl_rt_emit(pl_runtime_transaction *tx, const char *event_name) {
    if (!usable(tx) || !event_name || !event_name[0]) return PL_RT_INVALID;
    notify(tx, PL_RT_EVENT_EMIT, event_name);
    return PL_RT_OK;
}

pl_rt_status pl_rt_yield(pl_runtime_transaction *tx, const char *checkpoint_name) {
    if (!usable(tx) || !checkpoint_name || !checkpoint_name[0]) return PL_RT_INVALID;
    notify(tx, PL_RT_EVENT_YIELD, checkpoint_name);
    return PL_RT_OK;
}

pl_rt_status pl_rt_throw(pl_runtime_transaction *tx, const char *message) {
    if (!tx || !tx->active) return PL_RT_STATE;
    set_error(tx, message);
    return PL_RT_THROWN;
}

pl_rt_status pl_rt_panic(pl_runtime_transaction *tx, const char *message) {
    if (!tx || !tx->active) return PL_RT_STATE;
    set_error(tx, message ? message : "panic");
    return PL_RT_STATE;
}

pl_rt_status pl_rt_assert(pl_runtime_transaction *tx, int condition, const char *message) {
    if (condition) return PL_RT_OK;
    return pl_rt_throw(tx, message ? message : "assertion failed");
}

pl_rt_status pl_rt_catch(pl_runtime_transaction *tx, char *message, size_t capacity) {
    if (!tx || !message || capacity == 0u) return PL_RT_INVALID;
    if (!tx->failed) return PL_RT_STATE;
    (void)snprintf(message, capacity, "%s", tx->last_error);
    tx->failed = 0;
    tx->last_error[0] = '\0';
    return PL_RT_OK;
}

pl_rt_status pl_rt_return(pl_rt_status value) {
    return value;
}

pl_rt_task pl_rt_async(pl_runtime_transaction *tx) {
    pl_rt_task task = {0u, 0};
    if (usable(tx)) {
        task.expected_generation = tx->generation;
        task.pending = 1;
    }
    return task;
}

pl_rt_status pl_rt_await(pl_runtime_transaction *tx, pl_rt_task *task) {
    pl_rt_status status;
    if (!task || !task->pending) return PL_RT_STATE;
    status = pl_rt_flash(tx, task->expected_generation);
    task->pending = 0;
    return status;
}
