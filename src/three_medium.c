#include "three_medium.h"

#include <string.h>

const char *pl_medium_name(pl_compute_medium medium) {
    switch (medium) {
        case PL_MEDIUM_LAND: return "land/local";
        case PL_MEDIUM_WATER: return "water/stream";
        case PL_MEDIUM_SKY: return "sky/distributed";
        default: return "unknown";
    }
}

const char *pl_medium_status_name(pl_medium_status status) {
    switch (status) {
        case PL_MEDIUM_OK: return "success";
        case PL_MEDIUM_INVALID_ARGUMENT: return "invalid argument";
        case PL_MEDIUM_UNSUPPORTED_CAPABILITY: return "required capability unavailable";
        case PL_MEDIUM_BUDGET_TOO_SMALL: return "memory budget cannot hold one item";
        default: return "unknown status";
    }
}

pl_medium_status pl_medium_plan_build(const pl_medium_request *request,
                                      pl_medium_plan *out) {
    size_t budget_capacity;
    uint32_t required = PL_MEDIUM_CAP_CPU;

    if (request == NULL || out == NULL ||
        request->medium < PL_MEDIUM_LAND || request->medium > PL_MEDIUM_SKY ||
        request->memory_budget_bytes == 0u || request->item_size_bytes == 0u ||
        request->max_in_flight == 0u ||
        (request->available_capabilities & PL_MEDIUM_CAP_CPU) == 0u ||
        (request->deterministic != 0 && request->deterministic != 1)) {
        return PL_MEDIUM_INVALID_ARGUMENT;
    }

    if (request->medium == PL_MEDIUM_WATER) required |= PL_MEDIUM_CAP_STREAM_QUEUE;
    if (request->medium == PL_MEDIUM_SKY) required |= PL_MEDIUM_CAP_REMOTE_TRANSPORT;
    if (request->memory_budget_bytes < request->item_size_bytes)
        return PL_MEDIUM_BUDGET_TOO_SMALL;

    budget_capacity = request->memory_budget_bytes / request->item_size_bytes;
    memset(out, 0, sizeof(*out));
    out->medium = request->medium;
    out->queue_capacity = budget_capacity < request->max_in_flight
        ? budget_capacity : request->max_in_flight;
    out->max_in_flight = out->queue_capacity;
    out->reserved_memory_bytes = out->queue_capacity * request->item_size_bytes;
    out->required_capabilities = required;
    out->backpressure_required = request->medium != PL_MEDIUM_LAND;
    out->deterministic = request->deterministic;
    out->ready_for_execution =
        (request->available_capabilities & required) == required;
    out->is_reference_plan = 1;

    return out->ready_for_execution
        ? PL_MEDIUM_OK : PL_MEDIUM_UNSUPPORTED_CAPABILITY;
}
