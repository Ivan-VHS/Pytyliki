#include "embedded_kernel.h"

#include <limits.h>
#include <string.h>

static bool health_ready(uint32_t health_mask) {
    return (health_mask & PL_EK_HEALTH_REQUIRED) == PL_EK_HEALTH_REQUIRED;
}

void pl_ek_init(pl_embedded_kernel_model *model, uint32_t initial_tick) {
    if (model == NULL) {
        return;
    }
    memset(model, 0, sizeof(*model));
    model->version = PL_EK_MODEL_VERSION;
    model->state = PL_EK_BOOT;
    model->last_tick = initial_tick;
}

bool pl_ek_state_valid(pl_ek_state state) {
    return state >= PL_EK_BOOT && state <= PL_EK_HALTED;
}

int pl_ek_transition(pl_embedded_kernel_model *model, pl_ek_state next,
                     uint32_t health_mask, uint32_t fault_mask) {
    bool allowed = false;
    uint32_t effective_fault_mask;
    if (model == NULL || model->version != PL_EK_MODEL_VERSION ||
        !pl_ek_state_valid(model->state) || !pl_ek_state_valid(next)) {
        return -1;
    }
    effective_fault_mask = model->fault_mask | fault_mask;
    if (model->state == PL_EK_HALTED) {
        return -2;
    }
    if (next == PL_EK_HALTED) {
        allowed = true;
    } else if (next == PL_EK_DEGRADED) {
        allowed = effective_fault_mask != 0u;
    } else if (model->state == PL_EK_BOOT && next == PL_EK_IDLE) {
        allowed = health_ready(health_mask) && effective_fault_mask == 0u;
    } else if (model->state == PL_EK_IDLE && next == PL_EK_ACTIVE) {
        allowed = health_ready(health_mask) && effective_fault_mask == 0u;
    } else if (model->state == PL_EK_ACTIVE && next == PL_EK_IDLE) {
        allowed = health_ready(health_mask) && effective_fault_mask == 0u;
    } else if (model->state == PL_EK_DEGRADED && next == PL_EK_IDLE) {
        allowed = health_ready(health_mask) && effective_fault_mask == 0u;
    } else if (model->state == PL_EK_IDLE && next == PL_EK_DIAGNOSTIC) {
        allowed = effective_fault_mask == 0u;
    } else if (model->state == PL_EK_DIAGNOSTIC && next == PL_EK_IDLE) {
        allowed = health_ready(health_mask) && effective_fault_mask == 0u;
    }

    if (!allowed) {
        return -3;
    }
    model->health_mask = health_mask;
    model->fault_mask = effective_fault_mask;
    model->state = next;
    return 0;
}

void pl_ek_add_pressure(pl_embedded_kernel_model *model,
                        pl_ek_pain_source source, uint32_t weight) {
    if (model == NULL || source < PL_EK_PAIN_IRQ_JITTER ||
        source > PL_EK_PAIN_THERMAL_WARNING) {
        return;
    }
    if (UINT32_MAX - model->current_pressure < weight) {
        model->current_pressure = UINT32_MAX;
    } else {
        model->current_pressure += weight;
    }
    if (model->current_pressure > model->peak_pressure) {
        model->peak_pressure = model->current_pressure;
    }
    model->last_pain_source = source;
}

int pl_ek_report_fault(pl_embedded_kernel_model *model, uint32_t fault_mask,
                       pl_ek_pain_source source, uint32_t weight) {
    if (model == NULL || model->version != PL_EK_MODEL_VERSION ||
        !pl_ek_state_valid(model->state) || model->state == PL_EK_HALTED ||
        fault_mask == 0u || source < PL_EK_PAIN_IRQ_JITTER ||
        source > PL_EK_PAIN_THERMAL_WARNING) {
        return -1;
    }
    model->fault_mask |= fault_mask;
    pl_ek_add_pressure(model, source, weight);
    if (model->state != PL_EK_HALTED && model->state != PL_EK_DEGRADED) {
        return pl_ek_transition(model, PL_EK_DEGRADED,
                                model->health_mask, model->fault_mask);
    }
    return 0;
}

int pl_ek_clear_faults(pl_embedded_kernel_model *model, uint32_t health_mask) {
    if (model == NULL || model->version != PL_EK_MODEL_VERSION) {
        return -1;
    }
    if (!health_ready(health_mask)) {
        return -2;
    }
    model->health_mask = health_mask;
    model->fault_mask = 0u;
    return 0;
}

int pl_ek_sample_tick(pl_embedded_kernel_model *model, uint32_t now,
                      uint32_t expected_period) {
    uint32_t delta;
    uint32_t jitter;
    if (model == NULL || model->version != PL_EK_MODEL_VERSION ||
        expected_period == 0u) {
        return -1;
    }

    /* Unsigned subtraction is defined modulo 2^32 and handles counter wrap. */
    delta = now - model->last_tick;
    if (delta > UINT32_C(0x7fffffff)) {
        return -2;
    }

    if (delta >= expected_period) {
        jitter = delta - expected_period;
    } else {
        jitter = expected_period - delta;
    }
    if (jitter > model->peak_jitter) {
        model->peak_jitter = jitter;
    }

    model->last_tick = now;
    model->last_delta = delta;
    if (model->tick_count != UINT64_MAX) {
        model->tick_count++;
    }
    return 0;
}

void pl_ek_decay_pressure(pl_embedded_kernel_model *model, uint32_t amount) {
    if (model == NULL) {
        return;
    }
    model->current_pressure = model->current_pressure > amount
        ? model->current_pressure - amount : 0u;
}

pl_ek_pressure_action pl_ek_pressure_decide(
    const pl_embedded_kernel_model *model) {
    if (model == NULL || model->version != PL_EK_MODEL_VERSION) {
        return PL_EK_PRESSURE_SAFE_STOP;
    }
    if ((model->fault_mask & PL_EK_FAULT_CRITICAL_MASK) != 0u ||
        model->current_pressure >= 320u) {
        return PL_EK_PRESSURE_SAFE_STOP;
    }
    if (model->current_pressure >= 240u) {
        return PL_EK_PRESSURE_DEGRADED;
    }
    if (model->current_pressure >= 120u) {
        return PL_EK_PRESSURE_THROTTLE_DIAGNOSTICS;
    }
    if (model->current_pressure >= 60u) {
        return PL_EK_PRESSURE_OBSERVE;
    }
    return PL_EK_PRESSURE_NOMINAL;
}

int pl_ek_quota_init(pl_ek_quota *quota, uint32_t budget) {
    if (quota == NULL || budget == 0u) {
        return -1;
    }
    quota->budget = budget;
    quota->spent = 0u;
    quota->breath_ticks = 0u;
    return 0;
}

pl_ek_quota_action pl_ek_quota_poll(pl_ek_quota *quota) {
    if (quota == NULL || quota->budget == 0u ||
        quota->spent > quota->budget) {
        return PL_EK_QUOTA_INVALID;
    }
    if (quota->breath_ticks > 0u) {
        quota->breath_ticks--;
        return PL_EK_QUOTA_BREATHE;
    }
    if (quota->spent == quota->budget) {
        quota->spent = 0u;
        quota->breath_ticks = 2u;
        return PL_EK_QUOTA_YIELD;
    }
    quota->spent++;
    return PL_EK_QUOTA_RUN;
}

int pl_ek_trace_push(pl_embedded_kernel_model *model, uint32_t timestamp,
                     uint8_t role, uint16_t event_code, uint32_t value) {
    pl_ek_trace_entry *entry;
    if (model == NULL || model->version != PL_EK_MODEL_VERSION ||
        model->trace_head >= PL_EK_TRACE_CAPACITY ||
        model->trace_count > PL_EK_TRACE_CAPACITY ||
        !pl_ek_state_valid(model->state) ||
        model->trace_sequence == UINT64_MAX) {
        return -1;
    }
    entry = &model->trace[model->trace_head];
    model->trace_sequence++;
    entry->sequence = model->trace_sequence;
    entry->timestamp = timestamp;
    entry->role = role;
    entry->event_code = event_code;
    entry->value = value;
    entry->state = (uint8_t)model->state;

    model->trace_head = (model->trace_head + 1u) % PL_EK_TRACE_CAPACITY;
    if (model->trace_count < PL_EK_TRACE_CAPACITY) {
        model->trace_count++;
    } else if (model->trace_dropped != UINT64_MAX) {
        model->trace_dropped++;
    }
    return 0;
}

int pl_ek_trace_at(const pl_embedded_kernel_model *model, size_t oldest_index,
                   pl_ek_trace_entry *out) {
    size_t oldest;
    size_t index;
    if (model == NULL || out == NULL ||
        model->trace_head >= PL_EK_TRACE_CAPACITY ||
        model->trace_count > PL_EK_TRACE_CAPACITY ||
        oldest_index >= model->trace_count) {
        return -1;
    }
    oldest = (model->trace_head + PL_EK_TRACE_CAPACITY - model->trace_count) %
             PL_EK_TRACE_CAPACITY;
    index = (oldest + oldest_index) % PL_EK_TRACE_CAPACITY;
    *out = model->trace[index];
    return 0;
}

bool pl_ek_memory_span_valid(uint64_t region_base, uint64_t region_size,
                             uint64_t offset, uint64_t byte_count,
                             uint64_t alignment) {
    uint64_t address;
    if (region_size == 0u || byte_count == 0u || alignment == 0u ||
        region_size > UINT64_MAX - region_base) {
        return false;
    }
    if (offset > region_size || byte_count > region_size - offset) {
        return false;
    }
    if (offset > UINT64_MAX - region_base) {
        return false;
    }
    address = region_base + offset;
    return (address % alignment) == 0u;
}

const char *pl_ek_state_name(pl_ek_state state) {
    static const char *const names[] = {
        "boot", "idle", "active", "diagnostic", "degraded", "halted"
    };
    return pl_ek_state_valid(state) ? names[(unsigned)state] : "invalid";
}

const char *pl_ek_pressure_action_name(pl_ek_pressure_action action) {
    static const char *const names[] = {
        "nominal", "observe", "throttle-diagnostics", "degraded", "safe-stop"
    };
    return action >= PL_EK_PRESSURE_NOMINAL && action <= PL_EK_PRESSURE_SAFE_STOP
        ? names[(unsigned)action] : "invalid";
}
