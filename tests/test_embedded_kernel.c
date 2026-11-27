#include "embedded_kernel.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

int main(void) {
    pl_embedded_kernel_model model;
    pl_ek_quota quota;
    pl_ek_trace_entry entry;
    uint32_t healthy = PL_EK_HEALTH_REQUIRED;

    pl_ek_init(&model, UINT32_C(0xfffffffe));
    assert(model.version == PL_EK_MODEL_VERSION);
    assert(model.state == PL_EK_BOOT);
    assert(pl_ek_transition(&model, PL_EK_ACTIVE, healthy, 0u) != 0);
    assert(pl_ek_transition(&model, PL_EK_IDLE, healthy, 0u) == 0);
    assert(pl_ek_transition(&model, PL_EK_ACTIVE, healthy, 0u) == 0);
    assert(pl_ek_transition(&model, PL_EK_DIAGNOSTIC, healthy, 0u) != 0);
    assert(pl_ek_report_fault(&model, PL_EK_FAULT_QUEUE_OVERFLOW,
                              PL_EK_PAIN_QUEUE_PRESSURE, 80u) == 0);
    assert(model.state == PL_EK_DEGRADED);
    assert(model.fault_mask == PL_EK_FAULT_QUEUE_OVERFLOW);
    assert(pl_ek_transition(&model, PL_EK_IDLE, healthy, model.fault_mask) != 0);
    assert(pl_ek_transition(&model, PL_EK_IDLE, healthy, 0u) != 0);
    assert(pl_ek_clear_faults(&model, healthy) == 0);
    assert(pl_ek_transition(&model, PL_EK_IDLE, healthy, 0u) == 0);
    assert(pl_ek_transition(&model, PL_EK_HALTED, healthy, 0u) == 0);
    assert(pl_ek_transition(&model, PL_EK_IDLE, healthy, 0u) == -2);

    assert(pl_ek_sample_tick(&model, 3u, 0u) == -1);
    assert(pl_ek_sample_tick(&model, 3u, 5u) == 0);
    assert(model.last_delta == 5u);
    assert(model.peak_jitter == 0u);
    assert(pl_ek_sample_tick(&model, UINT32_C(0x80000004), 1u) == -2);

    pl_ek_add_pressure(&model, PL_EK_PAIN_IRQ_JITTER, 60u);
    assert(pl_ek_pressure_decide(&model) == PL_EK_PRESSURE_OBSERVE);
    pl_ek_add_pressure(&model, PL_EK_PAIN_BUS_RETRY, 80u);
    assert(pl_ek_pressure_decide(&model) == PL_EK_PRESSURE_THROTTLE_DIAGNOSTICS);
    pl_ek_add_pressure(&model, PL_EK_PAIN_THERMAL_WARNING, 100u);
    assert(pl_ek_pressure_decide(&model) == PL_EK_PRESSURE_DEGRADED);
    pl_ek_decay_pressure(&model, UINT32_MAX);
    assert(model.current_pressure == 0u);
    pl_ek_add_pressure(&model, PL_EK_PAIN_BUS_RETRY, UINT32_MAX);
    assert(model.current_pressure == UINT32_MAX);
    assert(pl_ek_pressure_decide(&model) == PL_EK_PRESSURE_SAFE_STOP);

    assert(pl_ek_quota_init(&quota, 2u) == 0);
    assert(pl_ek_quota_poll(&quota) == PL_EK_QUOTA_RUN);
    assert(pl_ek_quota_poll(&quota) == PL_EK_QUOTA_RUN);
    assert(pl_ek_quota_poll(&quota) == PL_EK_QUOTA_YIELD);
    assert(pl_ek_quota_poll(&quota) == PL_EK_QUOTA_BREATHE);
    assert(pl_ek_quota_poll(&quota) == PL_EK_QUOTA_BREATHE);
    assert(pl_ek_quota_poll(&quota) == PL_EK_QUOTA_RUN);
    assert(pl_ek_quota_init(&quota, 0u) != 0);

    pl_ek_init(&model, 0u);
    for (uint32_t i = 0u; i < PL_EK_TRACE_CAPACITY + 1u; ++i) {
        assert(pl_ek_trace_push(&model, i, 1u, (uint16_t)i, i * 3u) == 0);
    }
    assert(model.trace_count == PL_EK_TRACE_CAPACITY);
    assert(model.trace_dropped == 1u);
    assert(pl_ek_trace_at(&model, 0u, &entry) == 0);
    assert(entry.timestamp == 1u);
    assert(entry.event_code == 1u);
    assert(pl_ek_trace_at(&model, PL_EK_TRACE_CAPACITY, &entry) != 0);

    assert(pl_ek_memory_span_valid(UINT64_C(0x20000000), 4096u, 64u, 128u, 64u));
    assert(!pl_ek_memory_span_valid(UINT64_C(0x20000000), 4096u, 65u, 128u, 64u));
    assert(!pl_ek_memory_span_valid(UINT64_C(0x20000000), 4096u, 4090u, 16u, 1u));
    assert(!pl_ek_memory_span_valid(UINT64_MAX - 4u, 16u, 0u, 1u, 1u));

    assert(pl_ek_state_name(PL_EK_DEGRADED)[0] == 'd');
    assert(pl_ek_state_name((pl_ek_state)99)[0] == 'i');
    assert(pl_ek_pressure_action_name(PL_EK_PRESSURE_SAFE_STOP)[0] == 's');

    puts("Pytyliki Embedded Kernel Model tests passed.");
    return 0;
}
