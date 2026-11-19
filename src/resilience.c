#include "resilience.h"
#include <math.h>
const char *pl_resilience_state_name(pl_resilience_state state) {
    static const char *const names[]={"CLOSED","DEGRADED","OPEN","HALF_OPEN"};
    if ((int)state<0 || state>PL_RESILIENCE_HALF_OPEN) return "UNKNOWN";
    return names[(unsigned)state];
}
static int valid_percent(double v) { return isfinite(v) && v>=0.0 && v<=100.0; }
int pl_resilience_evaluate(const pl_resilience_metrics *m, pl_resilience_decision *d) {
    pl_resilience_state next;
    const char *reason;
    int pressure;
    if (!m || !d || !valid_percent(m->error_rate_percent) ||
        !valid_percent(m->cpu_headroom_percent) || !valid_percent(m->memory_headroom_percent) ||
        !valid_percent(m->queue_utilization_percent) || (int)m->current_state<0 ||
        m->current_state>PL_RESILIENCE_HALF_OPEN) return -1;
    pressure=m->cpu_headroom_percent<=PL_RESILIENCE_LOW_HEADROOM_PERCENT ||
        m->memory_headroom_percent<=PL_RESILIENCE_LOW_HEADROOM_PERCENT ||
        m->queue_utilization_percent>=PL_RESILIENCE_QUEUE_PRESSURE_PERCENT;
    switch(m->current_state) {
    case PL_RESILIENCE_OPEN:
        if(m->error_rate_percent>=PL_RESILIENCE_ERROR_THRESHOLD_PERCENT) {
            next=PL_RESILIENCE_OPEN; reason="error rate remains above the trip threshold";
        } else if(m->cooldown_elapsed_seconds<PL_RESILIENCE_COOLDOWN_SECONDS) {
            next=PL_RESILIENCE_OPEN; reason="cooldown has not elapsed";
        } else { next=PL_RESILIENCE_HALF_OPEN; reason="cooldown elapsed; admit a limited recovery probe"; }
        break;
    case PL_RESILIENCE_HALF_OPEN:
        if(m->error_rate_percent>=PL_RESILIENCE_ERROR_THRESHOLD_PERCENT) {
            next=PL_RESILIENCE_OPEN; reason="recovery probe failed; reopen the circuit";
        } else if(m->successful_probes>=PL_RESILIENCE_RECOVERY_SUCCESSES && !pressure &&
                  m->error_rate_percent<10.0) {
            next=PL_RESILIENCE_CLOSED; reason="recovery evidence is sufficient; close the circuit";
        } else { next=PL_RESILIENCE_HALF_OPEN; reason="continue limited recovery probes"; }
        break;
    case PL_RESILIENCE_CLOSED:
    case PL_RESILIENCE_DEGRADED:
        if(m->error_rate_percent>=PL_RESILIENCE_ERROR_THRESHOLD_PERCENT) {
            next=PL_RESILIENCE_OPEN; reason="error rate crossed the circuit-breaker threshold";
        } else if(pressure) {
            next=PL_RESILIENCE_DEGRADED; reason="resource headroom or queue capacity is under pressure";
        } else { next=PL_RESILIENCE_CLOSED; reason="telemetry is within the normal operating envelope"; }
        break;
    default: return -1;
    }
    d->next_state=next;
    d->allow_primary_work=next!=PL_RESILIENCE_OPEN;
    d->serve_stale_cache=next==PL_RESILIENCE_OPEN || next==PL_RESILIENCE_HALF_OPEN;
    d->shed_optional_work=next==PL_RESILIENCE_DEGRADED || next==PL_RESILIENCE_OPEN || next==PL_RESILIENCE_HALF_OPEN;
    d->max_concurrency_percent=100u; d->probe_budget=0u;
    if(next==PL_RESILIENCE_DEGRADED) d->max_concurrency_percent=50u;
    else if(next==PL_RESILIENCE_OPEN) d->max_concurrency_percent=0u;
    else if(next==PL_RESILIENCE_HALF_OPEN) { d->max_concurrency_percent=5u; d->probe_budget=PL_RESILIENCE_PROBE_BUDGET; }
    d->reason=reason;
    return 0;
}
