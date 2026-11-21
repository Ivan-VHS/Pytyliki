#include "runtime_transaction.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { unsigned events[8]; } event_log;
static void on_event(pl_rt_event_kind kind, const char *name, uint64_t generation, void *context) {
    event_log *log = (event_log *)context;
    (void)name; (void)generation;
    if ((unsigned)kind < 8u) ++log->events[(unsigned)kind];
}

static int migrate_v1_to_v2(const unsigned char *input, size_t input_size,
                            uint32_t from_schema, uint32_t to_schema,
                            const unsigned char **output, size_t *output_size,
                            void *context) {
    (void)context;
    assert(from_schema == 1u && to_schema == 2u);
    assert(input_size == 3u && memcmp(input, "abc", 3u) == 0);
    *output = (const unsigned char *)"ABC!";
    *output_size = 4u;
    return 0;
}

int main(void) {
    pl_runtime_transaction tx;
    event_log log = {{0}};
    char message[PL_RT_ERROR_MAX];
    const unsigned char *data;
    pl_rt_task task;

    assert(pl_rt_init(&tx, "abc", 3u, 1u, on_event, &log) == PL_RT_OK);
    assert(pl_rt_patch(&tx, 1u, 1u, "Z", 1u) == PL_RT_OK);
    assert(tx.staged_size == 3u && memcmp(tx.staged, "aZc", 3u) == 0);
    assert(pl_rt_rebind(&tx, 42u) == PL_RT_OK && tx.binding == 42u);

    {
        const size_t remap[] = {2u, 1u, 0u};
        assert(pl_rt_remap(&tx, remap, 3u) == PL_RT_OK);
        assert(memcmp(tx.staged, "cZa", 3u) == 0);
    }
    assert(pl_rt_flash(&tx, 9u) == PL_RT_CONFLICT);
    assert(pl_rt_hot_swap(&tx, "abc", 3u) == PL_RT_OK);
    assert(pl_rt_migrate(&tx, 2u, migrate_v1_to_v2, NULL) == PL_RT_OK);
    assert(tx.schema_version == 2u && tx.staged_size == 4u);
    task = pl_rt_async(&tx);
    assert(task.pending);
    assert(pl_rt_await(&tx, &task) == PL_RT_OK);
    assert(!task.pending && tx.generation == 1u);
    assert(pl_rt_reflash(&tx, 1u) == PL_RT_OK);
    assert(tx.generation == 2u);

    assert(pl_rt_patch(&tx, 0u, 0u, "x", 1u) == PL_RT_OK);
    assert(pl_rt_rollback(&tx) == PL_RT_OK);
    assert(tx.staged_size == 4u && memcmp(tx.staged, "ABC!", 4u) == 0);
    assert(pl_rt_emit(&tx, "STATE_STABLE") == PL_RT_OK);
    assert(pl_rt_yield(&tx, "checkpoint-1") == PL_RT_OK);
    assert(pl_rt_assert(&tx, 0, "BackpressureOverflowException") == PL_RT_THROWN);
    assert(pl_rt_catch(&tx, message, sizeof(message)) == PL_RT_OK);
    assert(strcmp(message, "BackpressureOverflowException") == 0);
    assert(pl_rt_panic(&tx, "guarded panic") == PL_RT_STATE);
    assert(pl_rt_catch(&tx, message, sizeof(message)) == PL_RT_OK);
    assert(strcmp(message, "guarded panic") == 0);
    assert(pl_rt_return(PL_RT_OK) == PL_RT_OK);
    assert(log.events[PL_RT_EVENT_FLASH] == 1u);
    assert(log.events[PL_RT_EVENT_REFLASH] == 1u);
    data = tx.committed;
    assert(memcmp(data, "ABC!", 4u) == 0);
    pl_rt_destroy(&tx);
    puts("Pytyliki runtime transaction tests passed.");
    return 0;
}
