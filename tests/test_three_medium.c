#include "three_medium.h"
#include <assert.h>
#include <stddef.h>

int main(void) {
    pl_medium_request request = {
        PL_MEDIUM_LAND, 4096u, 256u, 64u, PL_MEDIUM_CAP_CPU, 1
    };
    pl_medium_plan plan;
    assert(pl_medium_plan_build(&request, &plan) == PL_MEDIUM_OK);
    assert(plan.queue_capacity == 16u && plan.reserved_memory_bytes == 4096u);
    assert(plan.ready_for_execution && plan.deterministic && plan.is_reference_plan);
    assert(!plan.backpressure_required);

    request.medium = PL_MEDIUM_WATER;
    assert(pl_medium_plan_build(&request, &plan) == PL_MEDIUM_UNSUPPORTED_CAPABILITY);
    assert(!plan.ready_for_execution && plan.backpressure_required);
    request.available_capabilities |= PL_MEDIUM_CAP_STREAM_QUEUE;
    request.max_in_flight = 6u;
    assert(pl_medium_plan_build(&request, &plan) == PL_MEDIUM_OK);
    assert(plan.queue_capacity == 6u && plan.reserved_memory_bytes == 1536u);
    assert(plan.backpressure_required);

    request.medium = PL_MEDIUM_SKY;
    assert(pl_medium_plan_build(&request, &plan) == PL_MEDIUM_UNSUPPORTED_CAPABILITY);
    request.available_capabilities |= PL_MEDIUM_CAP_REMOTE_TRANSPORT;
    assert(pl_medium_plan_build(&request, &plan) == PL_MEDIUM_OK);
    assert(plan.ready_for_execution && plan.backpressure_required);
    assert((plan.required_capabilities & PL_MEDIUM_CAP_REMOTE_TRANSPORT) != 0u);

    request.memory_budget_bytes = 255u;
    assert(pl_medium_plan_build(&request, &plan) == PL_MEDIUM_BUDGET_TOO_SMALL);
    request.memory_budget_bytes = 1024u;
    request.item_size_bytes = 0u;
    assert(pl_medium_plan_build(&request, &plan) == PL_MEDIUM_INVALID_ARGUMENT);
    assert(pl_medium_name(PL_MEDIUM_WATER)[0] == 'w');
    return 0;
}
