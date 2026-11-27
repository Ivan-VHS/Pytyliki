#ifndef PYTYLIKI_PROCESSOR_THREAD_RECOVERY_H
#define PYTYLIKI_PROCESSOR_THREAD_RECOVERY_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PL_TR_MAX_SERIES 16u
#define PL_TR_INTERVAL_WINDOW 8u
#define PL_TR_DEFAULT_MIN_INTERVALS 4u
#define PL_TR_DEFAULT_TOLERANCE_PPM 100000u
#define PL_TR_MAX_TOLERANCE_PPM 500000u
#define PL_TR_MAX_RETRIES 3u
typedef enum {
    PL_TR_FAULT_TRANSIENT = 1,
    PL_TR_FAULT_CORRECTED_HARDWARE,
    PL_TR_FAULT_UNCORRECTED_HARDWARE,
    PL_TR_FAULT_WATCHDOG,
    PL_TR_FAULT_FATAL
} pl_tr_fault_kind;
typedef enum {
    PL_TR_PERIOD_UNKNOWN = 0,
    PL_TR_PERIOD_IRREGULAR,
    PL_TR_PERIODIC
} pl_tr_periodicity;
typedef enum {
    PL_TR_ACTION_OBSERVE = 0,
    PL_TR_ACTION_RETRY_IDEMPOTENT,
    PL_TR_ACTION_REDISTRIBUTE_TASK,
    PL_TR_ACTION_QUARANTINE_APP_WORKER,
    PL_TR_ACTION_CHECKPOINT_AND_STOP,
    PL_TR_ACTION_SAFE_STOP
} pl_tr_action;
enum {
    PL_TR_TASK_IDEMPOTENT = 1u << 0,
    PL_TR_TASK_CHECKPOINTED = 1u << 1,
    PL_TR_TASK_MANAGED_POOL = 1u << 2
};
typedef struct {
    uint64_t worker_id;
    uint64_t timestamp_ns;
    uint32_t cpu_id;
    uint32_t fault_code;
    pl_tr_fault_kind kind;
} pl_tr_observation;
typedef struct {
    uint32_t minimum_intervals;
    uint32_t period_tolerance_ppm;
    uint32_t maximum_retries;
} pl_tr_policy;
typedef struct {
    uint64_t worker_id;
    uint64_t first_timestamp_ns;
    uint64_t last_timestamp_ns;
    uint64_t estimated_period_ns;
    uint32_t cpu_id;
    uint32_t fault_code;
    uint32_t observations;
    uint32_t intervals;
    uint32_t jitter_ppm;
    pl_tr_fault_kind kind;
    pl_tr_periodicity periodicity;
} pl_tr_analysis;
typedef struct {
    pl_tr_action action;
    uint64_t worker_id;
    uint32_t retry_budget;
    uint32_t requires_operator_review;
    uint32_t requires_privileged_adapter;
    uint32_t host_cpu_state_change;
    uint32_t applied;
    const char *reason;
} pl_tr_decision;
typedef struct {
    uint64_t intervals[PL_TR_INTERVAL_WINDOW];
    uint64_t worker_id;
    uint64_t first_timestamp_ns;
    uint64_t last_timestamp_ns;
    uint32_t cpu_id;
    uint32_t fault_code;
    uint32_t observations;
    uint32_t interval_count;
    uint32_t interval_head;
    pl_tr_fault_kind kind;
    uint32_t used;
} pl_tr_series;
typedef struct {
    /* Caller-owned, fixed-capacity state. Serialize writers and readers. */
    pl_tr_series series[PL_TR_MAX_SERIES];
} pl_tr_monitor;
typedef enum {
    PL_TR_OK = 0,
    PL_TR_ERR_ARGUMENT = -1,
    PL_TR_ERR_POLICY = -2,
    PL_TR_ERR_TIME_ORDER = -3,
    PL_TR_ERR_CAPACITY = -4,
    PL_TR_ERR_NOT_FOUND = -5,
    PL_TR_ERR_OVERFLOW = -6
} pl_tr_status;
/* No allocation, I/O, OS calls or hardware access occur in these functions. */
void pl_tr_monitor_init(pl_tr_monitor *monitor);
void pl_tr_policy_default(pl_tr_policy *policy);
int pl_tr_policy_valid(const pl_tr_policy *policy);
int pl_tr_record(pl_tr_monitor *monitor, const pl_tr_observation *observation);
int pl_tr_analyze(const pl_tr_monitor *monitor, uint64_t worker_id,
                  uint32_t fault_code, pl_tr_fault_kind kind,
                  const pl_tr_policy *policy, pl_tr_analysis *analysis);
int pl_tr_plan_recovery(const pl_tr_analysis *analysis, uint32_t task_flags,
                        const pl_tr_policy *policy, pl_tr_decision *decision);
const char *pl_tr_fault_kind_name(pl_tr_fault_kind kind);
const char *pl_tr_periodicity_name(pl_tr_periodicity periodicity);
const char *pl_tr_action_name(pl_tr_action action);
#ifdef __cplusplus
}
#endif
#endif
