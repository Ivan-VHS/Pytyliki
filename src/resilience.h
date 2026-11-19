#ifndef PYTYLIKI_RESILIENCE_H
#define PYTYLIKI_RESILIENCE_H
#ifdef __cplusplus
extern "C" {
#endif
#define PL_RESILIENCE_ERROR_THRESHOLD_PERCENT 25.0
#define PL_RESILIENCE_LOW_HEADROOM_PERCENT 15.0
#define PL_RESILIENCE_QUEUE_PRESSURE_PERCENT 85.0
#define PL_RESILIENCE_COOLDOWN_SECONDS 30u
#define PL_RESILIENCE_RECOVERY_SUCCESSES 5u
#define PL_RESILIENCE_PROBE_BUDGET 5u
typedef enum { PL_RESILIENCE_CLOSED=0, PL_RESILIENCE_DEGRADED, PL_RESILIENCE_OPEN, PL_RESILIENCE_HALF_OPEN } pl_resilience_state;
typedef struct {
    double error_rate_percent, cpu_headroom_percent, memory_headroom_percent, queue_utilization_percent;
    pl_resilience_state current_state;
    unsigned cooldown_elapsed_seconds, successful_probes;
} pl_resilience_metrics;
typedef struct {
    pl_resilience_state next_state;
    int allow_primary_work, serve_stale_cache, shed_optional_work;
    unsigned max_concurrency_percent, probe_budget;
    const char *reason;
} pl_resilience_decision;
/* Pure policy evaluation over caller-supplied telemetry; never probes or changes services. */
int pl_resilience_evaluate(const pl_resilience_metrics *metrics, pl_resilience_decision *decision);
const char *pl_resilience_state_name(pl_resilience_state state);
#ifdef __cplusplus
}
#endif
#endif
