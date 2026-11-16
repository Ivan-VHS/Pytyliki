#ifndef PYTYLIKI_MTTE_H
#define PYTYLIKI_MTTE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_MTTE_LAYER_COUNT 20u
#define PL_MTTE_MAX_PERIOD_NS UINT64_C(1000000000)

typedef struct {
    unsigned id;
    const char *concept;
    const char *function;
    const char *mechanical_operation;
    const char *description;
} pl_mtte_layer;

typedef struct {
    uint64_t tick_count;
    uint64_t period_ns;
    uint64_t elapsed_ns;
} pl_mtte_clock;

/* Static catalogue strings remain valid for the process lifetime. */
size_t pl_mtte_catalog(const pl_mtte_layer **out);
int pl_mtte_get(unsigned id, const pl_mtte_layer **out);
int pl_mtte_find(const char *name, const pl_mtte_layer **out);

/* A deterministic software tick counter; it never changes hardware clocks. */
int pl_mtte_clock_init(pl_mtte_clock *clock, uint64_t period_ns);
int pl_mtte_clock_advance(pl_mtte_clock *clock, uint64_t ticks);
int pl_mtte_clock_reset(pl_mtte_clock *clock);

#ifdef __cplusplus
}
#endif
#endif
