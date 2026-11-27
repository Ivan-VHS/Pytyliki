#include "processor_thread_recovery.h"
#include <math.h>
#include <string.h>

static int fault_kind_valid(pl_tr_fault_kind kind) {
    return kind >= PL_TR_FAULT_TRANSIENT && kind <= PL_TR_FAULT_FATAL;
}
static int policy_values_valid(const pl_tr_policy *policy) {
    return policy != NULL &&
        policy->minimum_intervals >= 2u &&
        policy->minimum_intervals <= PL_TR_INTERVAL_WINDOW &&
        policy->period_tolerance_ppm > 0u &&
        policy->period_tolerance_ppm <= PL_TR_MAX_TOLERANCE_PPM &&
        policy->maximum_retries <= PL_TR_MAX_RETRIES;
}
static pl_tr_series *find_series(pl_tr_monitor *monitor,
                                 const pl_tr_observation *observation,
                                 pl_tr_series **first_free) {
    size_t i;
    *first_free = NULL;
    for (i = 0u; i < PL_TR_MAX_SERIES; ++i) {
        pl_tr_series *series = &monitor->series[i];
        if (!series->used) {
            if (*first_free == NULL) *first_free = series;
            continue;
        }
        if (series->worker_id == observation->worker_id &&
            series->fault_code == observation->fault_code &&
            series->kind == observation->kind) return series;
    }
    return NULL;
}
static const pl_tr_series *find_const_series(const pl_tr_monitor *monitor,
                                              uint64_t worker_id,
                                              uint32_t fault_code,
                                              pl_tr_fault_kind kind) {
    size_t i;
    for (i = 0u; i < PL_TR_MAX_SERIES; ++i) {
        const pl_tr_series *series = &monitor->series[i];
        if (series->used && series->worker_id == worker_id &&
            series->fault_code == fault_code && series->kind == kind) return series;
    }
    return NULL;
}
static void sort_u64(uint64_t *values, size_t count) {
    size_t i;
    for (i = 1u; i < count; ++i) {
        uint64_t value = values[i];
        size_t j = i;
        while (j > 0u && values[j - 1u] > value) {
            values[j] = values[j - 1u];
            --j;
        }
        values[j] = value;
    }
}
static uint64_t median_u64(const uint64_t *sorted, size_t count) {
    const size_t middle = count / 2u;
    if ((count & 1u) != 0u) return sorted[middle];
    return sorted[middle - 1u] +
        (sorted[middle] - sorted[middle - 1u]) / 2u;
}
static uint32_t deviation_ppm(uint64_t deviation, uint64_t baseline) {
    long double scaled;
    if (baseline == 0u) return UINT32_MAX;
    scaled = ((long double)deviation * 1000000.0L) / (long double)baseline;
    if (!isfinite((double)scaled) || scaled >= (long double)UINT32_MAX) return UINT32_MAX;
    return (uint32_t)scaled;
}
void pl_tr_monitor_init(pl_tr_monitor *monitor) {
    if (monitor != NULL) memset(monitor, 0, sizeof(*monitor));
}
void pl_tr_policy_default(pl_tr_policy *policy) {
    if (policy == NULL) return;
    policy->minimum_intervals = PL_TR_DEFAULT_MIN_INTERVALS;
    policy->period_tolerance_ppm = PL_TR_DEFAULT_TOLERANCE_PPM;
    policy->maximum_retries = 1u;
}
int pl_tr_policy_valid(const pl_tr_policy *policy) {
    return policy_values_valid(policy);
}
int pl_tr_record(pl_tr_monitor *monitor, const pl_tr_observation *observation) {
    pl_tr_series *free_series;
    pl_tr_series *series;
    if (monitor == NULL || observation == NULL || observation->worker_id == 0u ||
        observation->fault_code == 0u || !fault_kind_valid(observation->kind))
        return PL_TR_ERR_ARGUMENT;
    series = find_series(monitor, observation, &free_series);
    if (series == NULL) {
        if (free_series == NULL) return PL_TR_ERR_CAPACITY;
        memset(free_series, 0, sizeof(*free_series));
        free_series->used = 1u;
        free_series->worker_id = observation->worker_id;
        free_series->fault_code = observation->fault_code;
        free_series->kind = observation->kind;
        free_series->first_timestamp_ns = observation->timestamp_ns;
        free_series->last_timestamp_ns = observation->timestamp_ns;
        free_series->cpu_id = observation->cpu_id;
        free_series->observations = 1u;
        return PL_TR_OK;
    }
    if (observation->timestamp_ns <= series->last_timestamp_ns) return PL_TR_ERR_TIME_ORDER;
    if (series->observations == UINT32_MAX) return PL_TR_ERR_OVERFLOW;
    series->intervals[series->interval_head] =
        observation->timestamp_ns - series->last_timestamp_ns;
    series->interval_head = (series->interval_head + 1u) % PL_TR_INTERVAL_WINDOW;
    if (series->interval_count < PL_TR_INTERVAL_WINDOW) series->interval_count++;
    series->observations++;
    series->last_timestamp_ns = observation->timestamp_ns;
    series->cpu_id = observation->cpu_id;
    return PL_TR_OK;
}
int pl_tr_analyze(const pl_tr_monitor *monitor, uint64_t worker_id,
                  uint32_t fault_code, pl_tr_fault_kind kind,
                  const pl_tr_policy *policy, pl_tr_analysis *analysis) {
    const pl_tr_series *series;
    uint64_t sorted[PL_TR_INTERVAL_WINDOW];
    uint64_t median;
    uint64_t maximum_deviation = 0u;
    size_t i;
    if (monitor == NULL || analysis == NULL || worker_id == 0u ||
        fault_code == 0u || !fault_kind_valid(kind)) return PL_TR_ERR_ARGUMENT;
    if (!policy_values_valid(policy)) return PL_TR_ERR_POLICY;
    series = find_const_series(monitor, worker_id, fault_code, kind);
    if (series == NULL) return PL_TR_ERR_NOT_FOUND;
    memset(analysis, 0, sizeof(*analysis));
    analysis->worker_id = series->worker_id;
    analysis->first_timestamp_ns = series->first_timestamp_ns;
    analysis->last_timestamp_ns = series->last_timestamp_ns;
    analysis->cpu_id = series->cpu_id;
    analysis->fault_code = series->fault_code;
    analysis->observations = series->observations;
    analysis->intervals = series->interval_count;
    analysis->kind = series->kind;
    analysis->periodicity = PL_TR_PERIOD_UNKNOWN;
    if (series->interval_count == 0u) return PL_TR_OK;
    for (i = 0u; i < series->interval_count; ++i) {
        size_t index = (series->interval_head + PL_TR_INTERVAL_WINDOW -
                        series->interval_count + i) % PL_TR_INTERVAL_WINDOW;
        sorted[i] = series->intervals[index];
    }
    sort_u64(sorted, series->interval_count);
    median = median_u64(sorted, series->interval_count);
    analysis->estimated_period_ns = median;
    for (i = 0u; i < series->interval_count; ++i) {
        uint64_t deviation = sorted[i] >= median ? sorted[i] - median : median - sorted[i];
        if (deviation > maximum_deviation) maximum_deviation = deviation;
    }
    analysis->jitter_ppm = deviation_ppm(maximum_deviation, median);
    if (series->interval_count >= policy->minimum_intervals) {
        analysis->periodicity = analysis->jitter_ppm <= policy->period_tolerance_ppm
            ? PL_TR_PERIODIC : PL_TR_PERIOD_IRREGULAR;
    }
    return PL_TR_OK;
}
int pl_tr_plan_recovery(const pl_tr_analysis *analysis, uint32_t task_flags,
                        const pl_tr_policy *policy, pl_tr_decision *decision) {
    const uint32_t valid_flags = PL_TR_TASK_IDEMPOTENT |
        PL_TR_TASK_CHECKPOINTED | PL_TR_TASK_MANAGED_POOL;
    if (analysis == NULL || decision == NULL) return PL_TR_ERR_ARGUMENT;
    if (!policy_values_valid(policy)) return PL_TR_ERR_POLICY;
    if (analysis->worker_id == 0u || analysis->fault_code == 0u ||
        analysis->observations == 0u || analysis->intervals >= analysis->observations ||
        analysis->first_timestamp_ns > analysis->last_timestamp_ns ||
        !fault_kind_valid(analysis->kind) ||
        analysis->periodicity < PL_TR_PERIOD_UNKNOWN ||
        analysis->periodicity > PL_TR_PERIODIC ||
        analysis->intervals > PL_TR_INTERVAL_WINDOW ||
        (analysis->periodicity == PL_TR_PERIODIC &&
         (analysis->estimated_period_ns == 0u ||
          analysis->intervals < policy->minimum_intervals ||
          analysis->jitter_ppm > policy->period_tolerance_ppm)) ||
        (analysis->periodicity == PL_TR_PERIOD_IRREGULAR &&
         (analysis->intervals < policy->minimum_intervals ||
          analysis->jitter_ppm <= policy->period_tolerance_ppm)) ||
        (analysis->periodicity == PL_TR_PERIOD_UNKNOWN &&
         analysis->intervals >= policy->minimum_intervals) ||
        task_flags != (task_flags & valid_flags)) return PL_TR_ERR_ARGUMENT;
    memset(decision, 0, sizeof(*decision));
    decision->worker_id = analysis->worker_id;
    decision->action = PL_TR_ACTION_OBSERVE;
    decision->reason = "insufficient evidence; continue observation";
    if (analysis->kind == PL_TR_FAULT_FATAL ||
        analysis->kind == PL_TR_FAULT_UNCORRECTED_HARDWARE) {
        decision->action = PL_TR_ACTION_SAFE_STOP;
        decision->reason = "uncorrected or fatal hardware error requires platform containment";
    } else if (analysis->kind == PL_TR_FAULT_WATCHDOG) {
        decision->action = (task_flags & PL_TR_TASK_CHECKPOINTED) != 0u
            ? PL_TR_ACTION_CHECKPOINT_AND_STOP : PL_TR_ACTION_SAFE_STOP;
        decision->reason = (task_flags & PL_TR_TASK_CHECKPOINTED) != 0u
            ? "watchdog recurrence: preserve checkpoint and stop the affected task"
            : "watchdog recurrence without a checkpoint: stop the affected task";
    } else if (analysis->kind == PL_TR_FAULT_CORRECTED_HARDWARE &&
               analysis->periodicity == PL_TR_PERIODIC) {
        decision->requires_operator_review = 1u;
        if ((task_flags & PL_TR_TASK_MANAGED_POOL) != 0u) {
            decision->action = PL_TR_ACTION_QUARANTINE_APP_WORKER;
            decision->reason = "periodic corrected hardware reports correlate with one application worker; quarantine only that managed worker and request platform review";
        } else if ((task_flags & PL_TR_TASK_CHECKPOINTED) != 0u) {
            decision->action = PL_TR_ACTION_CHECKPOINT_AND_STOP;
            decision->reason = "periodic corrected hardware reports correlate with this worker; checkpoint and stop because no managed worker pool is available";
        } else {
            decision->reason = "periodic corrected hardware reports correlate with this worker; request platform review without changing host CPU state";
        }
    } else if (analysis->kind == PL_TR_FAULT_TRANSIENT &&
               analysis->periodicity == PL_TR_PERIODIC) {
        if ((task_flags & (PL_TR_TASK_IDEMPOTENT | PL_TR_TASK_MANAGED_POOL)) ==
            (PL_TR_TASK_IDEMPOTENT | PL_TR_TASK_MANAGED_POOL)) {
            decision->action = PL_TR_ACTION_REDISTRIBUTE_TASK;
            decision->retry_budget = policy->maximum_retries;
            decision->reason = "periodic transient fault: redistribute the idempotent task within the application-managed worker pool";
        } else if ((task_flags & PL_TR_TASK_CHECKPOINTED) != 0u) {
            decision->action = PL_TR_ACTION_CHECKPOINT_AND_STOP;
            decision->reason = "periodic transient fault: save the checkpoint and stop because safe redistribution is unavailable";
        } else {
            decision->reason = "periodic transient fault lacks idempotency and checkpoint guarantees";
        }
    } else if (analysis->kind == PL_TR_FAULT_TRANSIENT &&
               (task_flags & PL_TR_TASK_IDEMPOTENT) != 0u &&
               policy->maximum_retries > 0u) {
        decision->action = PL_TR_ACTION_RETRY_IDEMPOTENT;
        decision->retry_budget = policy->maximum_retries;
        decision->reason = "transient fault: retry only the declared idempotent task";
    } else if ((task_flags & PL_TR_TASK_CHECKPOINTED) != 0u) {
        decision->action = PL_TR_ACTION_CHECKPOINT_AND_STOP;
        decision->reason = "task is checkpointed but no safe automatic retry policy applies";
    }
    decision->requires_privileged_adapter = 0u;
    decision->host_cpu_state_change = 0u;
    decision->applied = 0u;
    return PL_TR_OK;
}
const char *pl_tr_fault_kind_name(pl_tr_fault_kind kind) {
    switch (kind) {
    case PL_TR_FAULT_TRANSIENT: return "transient";
    case PL_TR_FAULT_CORRECTED_HARDWARE: return "corrected_hardware";
    case PL_TR_FAULT_UNCORRECTED_HARDWARE: return "uncorrected_hardware";
    case PL_TR_FAULT_WATCHDOG: return "watchdog";
    case PL_TR_FAULT_FATAL: return "fatal";
    default: return "unknown";
    }
}
const char *pl_tr_periodicity_name(pl_tr_periodicity periodicity) {
    switch (periodicity) {
    case PL_TR_PERIOD_UNKNOWN: return "unknown";
    case PL_TR_PERIOD_IRREGULAR: return "irregular";
    case PL_TR_PERIODIC: return "periodic";
    default: return "invalid";
    }
}
const char *pl_tr_action_name(pl_tr_action action) {
    switch (action) {
    case PL_TR_ACTION_OBSERVE: return "observe";
    case PL_TR_ACTION_RETRY_IDEMPOTENT: return "retry_idempotent";
    case PL_TR_ACTION_REDISTRIBUTE_TASK: return "redistribute_task";
    case PL_TR_ACTION_QUARANTINE_APP_WORKER: return "quarantine_application_worker";
    case PL_TR_ACTION_CHECKPOINT_AND_STOP: return "checkpoint_and_stop";
    case PL_TR_ACTION_SAFE_STOP: return "safe_stop";
    default: return "invalid";
    }
}
