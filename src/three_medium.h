#ifndef PYTYLIKI_THREE_MEDIUM_H
#define PYTYLIKI_THREE_MEDIUM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PL_MEDIUM_LAND = 0,
    PL_MEDIUM_WATER,
    PL_MEDIUM_SKY
} pl_compute_medium;

typedef enum {
    PL_MEDIUM_OK = 0,
    PL_MEDIUM_INVALID_ARGUMENT = -1,
    PL_MEDIUM_UNSUPPORTED_CAPABILITY = -2,
    PL_MEDIUM_BUDGET_TOO_SMALL = -3
} pl_medium_status;

enum {
    PL_MEDIUM_CAP_CPU = 1u << 0,
    PL_MEDIUM_CAP_STREAM_QUEUE = 1u << 1,
    PL_MEDIUM_CAP_REMOTE_TRANSPORT = 1u << 2,
    PL_MEDIUM_CAP_GPU = 1u << 3
};

typedef struct {
    pl_compute_medium medium;
    size_t memory_budget_bytes;
    size_t item_size_bytes;
    size_t max_in_flight;
    uint32_t available_capabilities;
    int deterministic;
} pl_medium_request;

typedef struct {
    pl_compute_medium medium;
    size_t queue_capacity;
    size_t max_in_flight;
    size_t reserved_memory_bytes;
    uint32_t required_capabilities;
    int backpressure_required;
    int deterministic;
    int ready_for_execution;
    int is_reference_plan;
} pl_medium_plan;

/* Build a bounded reference plan. This does not dispatch work, start network
 * activity, or claim GPU support. Capacity is constrained by both the in-flight
 * limit and memory budget, so a slow consumer cannot imply an unbounded queue. */
pl_medium_status pl_medium_plan_build(const pl_medium_request *request,
                                      pl_medium_plan *out);
const char *pl_medium_name(pl_compute_medium medium);
const char *pl_medium_status_name(pl_medium_status status);

#ifdef __cplusplus
}
#endif
#endif
