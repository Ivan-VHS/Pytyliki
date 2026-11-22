#ifndef PYTYLIKI_RUNTIME_TRANSACTION_H
#define PYTYLIKI_RUNTIME_TRANSACTION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_RT_MAX_BYTES (16u * 1024u * 1024u)
#define PL_RT_ERROR_MAX 192u

typedef enum {
    PL_RT_OK = 0,
    PL_RT_INVALID = 1,
    PL_RT_CONFLICT = 2,
    PL_RT_LIMIT = 3,
    PL_RT_INTEGRITY = 4,
    PL_RT_THROWN = 5,
    PL_RT_STATE = 6
} pl_rt_status;

typedef enum {
    PL_RT_EVENT_EMIT = 1,
    PL_RT_EVENT_YIELD = 2,
    PL_RT_EVENT_FLASH = 3,
    PL_RT_EVENT_REFLASH = 4,
    PL_RT_EVENT_ROLLBACK = 5,
    PL_RT_EVENT_HOT_SWAP = 6,
    PL_RT_EVENT_MIGRATE = 7
} pl_rt_event_kind;

typedef void (*pl_rt_event_fn)(pl_rt_event_kind kind, const char *name,
                               uint64_t generation, void *context);

typedef int (*pl_rt_migration_fn)(const unsigned char *input, size_t input_size,
                                  uint32_t from_schema, uint32_t to_schema,
                                  const unsigned char **output, size_t *output_size,
                                  void *context);

typedef struct {
    uint64_t expected_generation;
    int pending;
} pl_rt_task;

typedef struct {
    unsigned char *committed;
    unsigned char *staged;
    size_t committed_size;
    size_t staged_size;
    uint64_t generation;
    uint64_t binding;
    uint32_t schema_version;
    uint32_t committed_crc32;
    uint32_t staged_crc32;
    int active;
    int failed;
    char last_error[PL_RT_ERROR_MAX];
    pl_rt_event_fn event_callback;
    void *event_context;
} pl_runtime_transaction;

uint32_t pl_rt_crc32(const unsigned char *data, size_t size);
pl_rt_status pl_rt_init(pl_runtime_transaction *tx, const void *data, size_t size,
                        uint32_t schema_version, pl_rt_event_fn callback, void *context);
void pl_rt_destroy(pl_runtime_transaction *tx);
pl_rt_status pl_rt_patch(pl_runtime_transaction *tx, size_t offset, size_t remove_size,
                         const void *replacement, size_t replacement_size);
pl_rt_status pl_rt_rebind(pl_runtime_transaction *tx, uint64_t binding);
pl_rt_status pl_rt_remap(pl_runtime_transaction *tx, const size_t *target_to_source,
                         size_t target_size);
pl_rt_status pl_rt_hot_swap(pl_runtime_transaction *tx, const void *data, size_t size);
pl_rt_status pl_rt_migrate(pl_runtime_transaction *tx, uint32_t target_schema,
                           pl_rt_migration_fn migration, void *context);
pl_rt_status pl_rt_flash(pl_runtime_transaction *tx, uint64_t expected_generation);
pl_rt_status pl_rt_reflash(pl_runtime_transaction *tx, uint64_t expected_generation);
pl_rt_status pl_rt_sync(pl_runtime_transaction *tx);
pl_rt_status pl_rt_rollback(pl_runtime_transaction *tx);
pl_rt_status pl_rt_emit(pl_runtime_transaction *tx, const char *event_name);
pl_rt_status pl_rt_yield(pl_runtime_transaction *tx, const char *checkpoint_name);
pl_rt_status pl_rt_throw(pl_runtime_transaction *tx, const char *message);
pl_rt_status pl_rt_panic(pl_runtime_transaction *tx, const char *message);
pl_rt_status pl_rt_assert(pl_runtime_transaction *tx, int condition, const char *message);
pl_rt_status pl_rt_catch(pl_runtime_transaction *tx, char *message, size_t capacity);
pl_rt_status pl_rt_return(pl_rt_status value);
pl_rt_task pl_rt_async(pl_runtime_transaction *tx);
pl_rt_status pl_rt_await(pl_runtime_transaction *tx, pl_rt_task *task);

#ifdef __cplusplus
}
#endif
#endif
