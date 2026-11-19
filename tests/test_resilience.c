#include "resilience.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>
static pl_resilience_metrics baseline(void) {
    pl_resilience_metrics m={0};
    m.error_rate_percent=1.0; m.cpu_headroom_percent=70.0;
    m.memory_headroom_percent=65.0; m.queue_utilization_percent=25.0;
    m.current_state=PL_RESILIENCE_CLOSED; return m;
}
int main(void) {
    pl_resilience_metrics m=baseline(); pl_resilience_decision d;
    assert(pl_resilience_evaluate(&m,&d)==0 && d.next_state==PL_RESILIENCE_CLOSED);
    assert(d.allow_primary_work==1 && d.max_concurrency_percent==100u);
    m.error_rate_percent=40.0;
    assert(pl_resilience_evaluate(&m,&d)==0 && d.next_state==PL_RESILIENCE_OPEN);
    assert(d.allow_primary_work==0 && d.serve_stale_cache==1);
    m=baseline(); m.memory_headroom_percent=10.0;
    assert(pl_resilience_evaluate(&m,&d)==0 && d.next_state==PL_RESILIENCE_DEGRADED);
    assert(d.shed_optional_work==1 && d.max_concurrency_percent==50u);
    m=baseline(); m.current_state=PL_RESILIENCE_OPEN; m.cooldown_elapsed_seconds=29u;
    assert(pl_resilience_evaluate(&m,&d)==0 && d.next_state==PL_RESILIENCE_OPEN);
    m.cooldown_elapsed_seconds=PL_RESILIENCE_COOLDOWN_SECONDS;
    assert(pl_resilience_evaluate(&m,&d)==0 && d.next_state==PL_RESILIENCE_HALF_OPEN);
    assert(d.probe_budget==PL_RESILIENCE_PROBE_BUDGET && d.max_concurrency_percent==5u);
    m.current_state=PL_RESILIENCE_HALF_OPEN; m.successful_probes=PL_RESILIENCE_RECOVERY_SUCCESSES;
    assert(pl_resilience_evaluate(&m,&d)==0 && d.next_state==PL_RESILIENCE_CLOSED);
    m.error_rate_percent=30.0;
    assert(pl_resilience_evaluate(&m,&d)==0 && d.next_state==PL_RESILIENCE_OPEN);
    m=baseline(); m.cpu_headroom_percent=NAN;
    assert(pl_resilience_evaluate(&m,&d)==-1);
    assert(pl_resilience_evaluate(NULL,&d)==-1 && pl_resilience_evaluate(&m,NULL)==-1);
    puts("Pytyliki resilience state machine tests passed."); return 0;
}
