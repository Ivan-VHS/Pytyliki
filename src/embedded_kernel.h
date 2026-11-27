#ifndef PYTYLIKI_EMBEDDED_KERNEL_H
#define PYTYLIKI_EMBEDDED_KERNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_EK_MODEL_VERSION UINT32_C(0x00010000)
#define PL_EK_TRACE_CAPACITY 64u

typedef enum {
    PL_EK_BOOT = 0,
    PL_EK_IDLE,
    PL_EK_ACTIVE,
    PL_EK_DIAGNOSTIC,
    PL_EK_DEGRADED,
    PL_EK_HALTED
} pl_ek_state;

enum {
    PL_EK_HEALTH_CPU = UINT32_C(1) << 0,
    PL_EK_HEALTH_MEMORY = UINT32_C(1) << 1,
    PL_EK_HEALTH_CLOCK = UINT32_C(1) << 2,
    PL_EK_HEALTH_INTERRUPTS = UINT32_C(1) << 3,
    PL_EK_HEALTH_REQUIRED = PL_EK_HEALTH_CPU | PL_EK_HEALTH_MEMORY |
                            PL_EK_HEALTH_CLOCK | PL_EK_HEALTH_INTERRUPTS
};

enum {
    PL_EK_FAULT_BUS = UINT32_C(1) << 0,
    PL_EK_FAULT_THERMAL = UINT32_C(1) << 1,
    PL_EK_FAULT_IRQ_DEADLINE = UINT32_C(1) << 2,
    PL_EK_FAULT_QUEUE_OVERFLOW = UINT32_C(1) << 3,
    PL_EK_FAULT_STACK_GUARD = UINT32_C(1) << 4,
    PL_EK_FAULT_CRITICAL_MASK = PL_EK_FAULT_BUS | PL_EK_FAULT_THERMAL |
                                PL_EK_FAULT_STACK_GUARD
};

typedef enum {
    PL_EK_PAIN_IRQ_JITTER = 1,
    PL_EK_PAIN_BUS_RETRY,
    PL_EK_PAIN_QUEUE_PRESSURE,
    PL_EK_PAIN_THERMAL_WARNING
} pl_ek_pain_source;

typedef enum {
    PL_EK_PRESSURE_NOMINAL = 0,
    PL_EK_PRESSURE_OBSERVE,
    PL_EK_PRESSURE_THROTTLE_DIAGNOSTICS,
    PL_EK_PRESSURE_DEGRADED,
    PL_EK_PRESSURE_SAFE_STOP
} pl_ek_pressure_action;

typedef enum {
    PL_EK_QUOTA_INVALID = 0,
    PL_EK_QUOTA_RUN,
    PL_EK_QUOTA_BREATHE,
    PL_EK_QUOTA_YIELD
} pl_ek_quota_action;

typedef struct {
    uint32_t budget;
    uint32_t spent;
    uint32_t breath_ticks;
} pl_ek_quota;

typedef struct {
    uint64_t sequence;
    uint32_t timestamp;
    uint32_t value;
    uint16_t event_code;
    uint8_t role;
    uint8_t state;
} pl_ek_trace_entry;

typedef struct {
    uint32_t version;
    pl_ek_state state;
    uint32_t health_mask;
    uint32_t fault_mask;
    uint64_t tick_count;
    uint32_t last_tick;
    uint32_t last_delta;
    uint32_t peak_jitter;
    uint32_t current_pressure;
    uint32_t peak_pressure;
    pl_ek_pain_source last_pain_source;
    uint64_t trace_sequence;
    uint64_t trace_dropped;
    size_t trace_head;
    size_t trace_count;
    pl_ek_trace_entry trace[PL_EK_TRACE_CAPACITY];
} pl_embedded_kernel_model;

/*
 * The model is a deterministic, single-writer reference component.
 * Its caller serializes access; it does not provide locks or real context
 * switching. Hardware and privileged actions stay behind trusted adapters.
 */
void pl_ek_init(pl_embedded_kernel_model *model, uint32_t initial_tick);
bool pl_ek_state_valid(pl_ek_state state);
int pl_ek_transition(pl_embedded_kernel_model *model, pl_ek_state next,
                     uint32_t health_mask, uint32_t fault_mask);
int pl_ek_report_fault(pl_embedded_kernel_model *model, uint32_t fault_mask,
                       pl_ek_pain_source source, uint32_t weight);
int pl_ek_clear_faults(pl_embedded_kernel_model *model, uint32_t health_mask);

int pl_ek_sample_tick(pl_embedded_kernel_model *model, uint32_t now,
                      uint32_t expected_period);
void pl_ek_add_pressure(pl_embedded_kernel_model *model,
                        pl_ek_pain_source source, uint32_t weight);
void pl_ek_decay_pressure(pl_embedded_kernel_model *model, uint32_t amount);
pl_ek_pressure_action pl_ek_pressure_decide(
    const pl_embedded_kernel_model *model);

int pl_ek_quota_init(pl_ek_quota *quota, uint32_t budget);
pl_ek_quota_action pl_ek_quota_poll(pl_ek_quota *quota);

int pl_ek_trace_push(pl_embedded_kernel_model *model, uint32_t timestamp,
                     uint8_t role, uint16_t event_code, uint32_t value);
int pl_ek_trace_at(const pl_embedded_kernel_model *model, size_t oldest_index,
                   pl_ek_trace_entry *out);

bool pl_ek_memory_span_valid(uint64_t region_base, uint64_t region_size,
                             uint64_t offset, uint64_t byte_count,
                             uint64_t alignment);
const char *pl_ek_state_name(pl_ek_state state);
const char *pl_ek_pressure_action_name(pl_ek_pressure_action action);

#ifdef __cplusplus
}
#endif

#endif
