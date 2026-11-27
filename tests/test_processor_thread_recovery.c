#include "processor_thread_recovery.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

static void record_faults(pl_tr_monitor *monitor, uint64_t worker,
                          uint32_t cpu, uint32_t code,
                          pl_tr_fault_kind kind, const uint64_t *ticks,
                          size_t count) {
    size_t i;
    for (i = 0u; i < count; ++i) {
        pl_tr_observation event = { worker, ticks[i], cpu, code, kind };
        assert(pl_tr_record(monitor, &event) == PL_TR_OK);
    }
}
static void test_periodic_corrected_hardware_fault(void) {
    pl_tr_monitor monitor;
    pl_tr_policy policy;
    pl_tr_analysis analysis;
    pl_tr_decision decision;
    const uint64_t ticks[] = {
        UINT64_C(1000000000), UINT64_C(2000000000),
        UINT64_C(3000000000), UINT64_C(4000000000),
        UINT64_C(5000000000), UINT64_C(6000000000)
    };
    pl_tr_monitor_init(&monitor);
    pl_tr_policy_default(&policy);
    record_faults(&monitor, 7u, 2u, 0x41u,
                  PL_TR_FAULT_CORRECTED_HARDWARE, ticks,
                  sizeof(ticks) / sizeof(ticks[0]));
    assert(pl_tr_analyze(&monitor, 7u, 0x41u,
        PL_TR_FAULT_CORRECTED_HARDWARE, &policy, &analysis) == PL_TR_OK);
    assert(analysis.periodicity == PL_TR_PERIODIC);
    assert(analysis.estimated_period_ns == UINT64_C(1000000000));
    assert(analysis.jitter_ppm == 0u);
    assert(pl_tr_plan_recovery(&analysis, PL_TR_TASK_IDEMPOTENT |
        PL_TR_TASK_MANAGED_POOL, &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_QUARANTINE_APP_WORKER);
    assert(decision.requires_operator_review == 1u);
    assert(decision.host_cpu_state_change == 0u);
    assert(decision.requires_privileged_adapter == 0u);
    assert(decision.applied == 0u);
    assert(pl_tr_plan_recovery(&analysis, PL_TR_TASK_IDEMPOTENT,
        &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_OBSERVE);
    assert(decision.requires_operator_review == 1u);
}
static void test_irregular_faults_do_not_trigger_quarantine(void) {
    pl_tr_monitor monitor;
    pl_tr_policy policy;
    pl_tr_analysis analysis;
    pl_tr_decision decision;
    const uint64_t ticks[] = {100u, 200u, 300u, 900u, 1000u, 1100u};
    pl_tr_monitor_init(&monitor);
    pl_tr_policy_default(&policy);
    record_faults(&monitor, 8u, 1u, 0x42u,
                  PL_TR_FAULT_CORRECTED_HARDWARE, ticks,
                  sizeof(ticks) / sizeof(ticks[0]));
    assert(pl_tr_analyze(&monitor, 8u, 0x42u,
        PL_TR_FAULT_CORRECTED_HARDWARE, &policy, &analysis) == PL_TR_OK);
    assert(analysis.periodicity == PL_TR_PERIOD_IRREGULAR);
    assert(pl_tr_plan_recovery(&analysis, 0u, &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_OBSERVE);
}
static void test_safe_retry_requires_idempotency(void) {
    pl_tr_monitor monitor;
    pl_tr_policy policy;
    pl_tr_analysis analysis;
    pl_tr_decision decision;
    const uint64_t ticks[] = {100u, 130u};
    pl_tr_monitor_init(&monitor);
    pl_tr_policy_default(&policy);
    record_faults(&monitor, 9u, 0u, 0x43u,
                  PL_TR_FAULT_TRANSIENT, ticks,
                  sizeof(ticks) / sizeof(ticks[0]));
    assert(pl_tr_analyze(&monitor, 9u, 0x43u,
        PL_TR_FAULT_TRANSIENT, &policy, &analysis) == PL_TR_OK);
    assert(analysis.periodicity == PL_TR_PERIOD_UNKNOWN);
    assert(pl_tr_plan_recovery(&analysis, PL_TR_TASK_IDEMPOTENT,
        &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_RETRY_IDEMPOTENT);
    assert(decision.retry_budget == 1u);
    assert(pl_tr_plan_recovery(&analysis, 0u, &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_OBSERVE);
}
static void test_periodic_redistribution_only_for_managed_pool(void) {
    pl_tr_monitor monitor;
    pl_tr_policy policy;
    pl_tr_analysis analysis;
    pl_tr_decision decision;
    const uint64_t ticks[] = {100u, 200u, 300u, 400u, 500u, 600u};
    pl_tr_monitor_init(&monitor);
    pl_tr_policy_default(&policy);
    record_faults(&monitor, 10u, 3u, 0x44u, PL_TR_FAULT_TRANSIENT,
                  ticks, sizeof(ticks) / sizeof(ticks[0]));
    assert(pl_tr_analyze(&monitor, 10u, 0x44u,
        PL_TR_FAULT_TRANSIENT, &policy, &analysis) == PL_TR_OK);
    assert(pl_tr_plan_recovery(&analysis, PL_TR_TASK_IDEMPOTENT |
        PL_TR_TASK_MANAGED_POOL, &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_REDISTRIBUTE_TASK);
    assert(decision.host_cpu_state_change == 0u);
    assert(pl_tr_plan_recovery(&analysis, PL_TR_TASK_IDEMPOTENT,
        &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_OBSERVE);
}
static void test_uncorrected_and_watchdog_faults_stop_safely(void) {
    pl_tr_analysis analysis = {0};
    pl_tr_policy policy;
    pl_tr_decision decision;
    pl_tr_policy_default(&policy);
    analysis.worker_id = 11u;
    analysis.fault_code = 0x45u;
    analysis.observations = 1u;
    analysis.kind = PL_TR_FAULT_UNCORRECTED_HARDWARE;
    assert(pl_tr_plan_recovery(&analysis, PL_TR_TASK_IDEMPOTENT,
        &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_SAFE_STOP);
    analysis.kind = PL_TR_FAULT_WATCHDOG;
    assert(pl_tr_plan_recovery(&analysis, PL_TR_TASK_CHECKPOINTED,
        &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_CHECKPOINT_AND_STOP);
    assert(pl_tr_plan_recovery(&analysis, 0u, &policy, &decision) == PL_TR_OK);
    assert(decision.action == PL_TR_ACTION_SAFE_STOP);
}
static void test_order_bounds_and_capacity(void) {
    pl_tr_monitor monitor;
    pl_tr_policy policy;
    pl_tr_analysis analysis;
    pl_tr_observation event = {1u, 100u, 0u, 1u, PL_TR_FAULT_TRANSIENT};
    pl_tr_monitor_init(&monitor);
    pl_tr_policy_default(&policy);
    assert(pl_tr_record(&monitor, &event) == PL_TR_OK);
    assert(pl_tr_record(&monitor, &event) == PL_TR_ERR_TIME_ORDER);
    event.timestamp_ns = 101u;
    assert(pl_tr_record(&monitor, &event) == PL_TR_OK);
    assert(pl_tr_analyze(&monitor, 1u, 1u, PL_TR_FAULT_TRANSIENT,
        &policy, &analysis) == PL_TR_OK);
    assert(pl_tr_analyze(&monitor, 99u, 1u, PL_TR_FAULT_TRANSIENT,
        &policy, &analysis) == PL_TR_ERR_NOT_FOUND);
    for (uint64_t id = 2u; id <= PL_TR_MAX_SERIES; ++id) {
        event.worker_id = id;
        event.timestamp_ns = 100u;
        event.fault_code = (uint32_t)id;
        assert(pl_tr_record(&monitor, &event) == PL_TR_OK);
    }
    event.worker_id = 999u;
    event.fault_code = 999u;
    assert(pl_tr_record(&monitor, &event) == PL_TR_ERR_CAPACITY);
}
static void test_invalid_policy_and_arguments(void) {
    pl_tr_monitor monitor;
    pl_tr_policy policy;
    pl_tr_analysis analysis = {0};
    pl_tr_decision decision;
    pl_tr_observation event = {1u, 1u, 0u, 1u, PL_TR_FAULT_TRANSIENT};
    pl_tr_monitor_init(&monitor);
    pl_tr_policy_default(&policy);
    policy.period_tolerance_ppm = PL_TR_MAX_TOLERANCE_PPM + 1u;
    assert(pl_tr_policy_valid(&policy) == 0);
    assert(pl_tr_record(&monitor, NULL) == PL_TR_ERR_ARGUMENT);
    assert(pl_tr_record(NULL, &event) == PL_TR_ERR_ARGUMENT);
    assert(pl_tr_analyze(&monitor, 1u, 1u, PL_TR_FAULT_TRANSIENT,
        &policy, &analysis) == PL_TR_ERR_POLICY);
    pl_tr_policy_default(&policy);
    analysis.worker_id = 1u;
    analysis.fault_code = 1u;
    analysis.kind = PL_TR_FAULT_TRANSIENT;
    assert(pl_tr_plan_recovery(&analysis, UINT32_MAX,
        &policy, &decision) == PL_TR_ERR_ARGUMENT);
    assert(pl_tr_plan_recovery(&analysis, 0u,
        NULL, &decision) == PL_TR_ERR_POLICY);
    assert(pl_tr_action_name(PL_TR_ACTION_SAFE_STOP)[0] == 's');
}
int main(void) {
    test_periodic_corrected_hardware_fault();
    test_irregular_faults_do_not_trigger_quarantine();
    test_safe_retry_requires_idempotency();
    test_periodic_redistribution_only_for_managed_pool();
    test_uncorrected_and_watchdog_faults_stop_safely();
    test_order_bounds_and_capacity();
    test_invalid_policy_and_arguments();
    puts("Pytyliki processor thread fault recovery tests passed.");
    return 0;
}
