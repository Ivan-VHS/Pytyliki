#include "robotics_core.h"
#include "robotics_time.h"
#include "robotics_motion.h"
#include "robotics_io.h"
#include "robotics_sim.h"

#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static pl_rb_status publish_ok(const pl_rb_actuator_frame *frame, void *context) {
    pl_rb_actuator_frame *published = (pl_rb_actuator_frame *)context;
    *published = *frame;
    return PL_RB_OK;
}

static pl_rb_status publish_fail(const pl_rb_actuator_frame *frame, void *context) {
    (void)frame;
    (void)context;
    return PL_RB_IO;
}

static void test_timing_and_quotas(void) {
    const pl_rb_timing_budget budget = { 50u, 40u, 3u };
    pl_rb_timing_result timing;
    pl_rb_quota quota = {0};

    assert(pl_rb_timing_evaluate(&budget, 100u, 102u, 105u, 140u, &timing) == PL_RB_OK);
    assert(timing.deadline_tick == 152u);
    assert(timing.execution_ticks == 35u);
    assert(timing.jitter_ticks == 2u);
    assert(pl_rb_timing_evaluate(&budget, 100u, 102u, 105u, 153u, &timing) == PL_RB_DEADLINE);
    assert(pl_rb_timing_evaluate(&budget, 100u, 104u, 105u, 140u, &timing) == PL_RB_QUOTA);
    assert(pl_rb_timing_evaluate(&budget, 100u, 102u, 105u, 150u, &timing) == PL_RB_QUOTA);
    assert(pl_rb_timing_evaluate(&budget, 100u, 102u, 99u, 140u, &timing) == PL_RB_INVALID);

    assert(pl_rb_quota_init(&quota, 100u, 40u, 10u, 100u) == PL_RB_OK);
    assert(pl_rb_quota_consume(&quota, 110u, 20u, 5u) == PL_RB_OK);
    assert(pl_rb_quota_consume(&quota, 120u, 25u, 1u) == PL_RB_QUOTA);
    assert(pl_rb_quota_consume(&quota, 205u, 10u, 2u) == PL_RB_OK);
    assert(quota.cpu_used_ticks == 10u && quota.bus_used_units == 2u);
    assert(pl_rb_quota_consume(&quota, 90u, 1u, 1u) == PL_RB_INVALID);
}

static void test_sensor_time_alignment(void) {
    const pl_rb_sensor_sample sample = { 1000u, 1000u, 100, 10 };
    pl_rb_sensor_prediction prediction;

    assert(pl_rb_sensor_predict(&sample, 1500u, 600u, &prediction) == PL_RB_OK);
    assert(prediction.age_ticks == 500u);
    assert(prediction.predicted_position_q == 105);
    assert(pl_rb_sensor_predict(&sample, 900u, 600u, &prediction) == PL_RB_INVALID);
    assert(pl_rb_sensor_predict(&sample, 2000u, 500u, &prediction) == PL_RB_STALE);
}

static void test_physical_dimensions(void) {
    pl_rb_quantity input = { 1000, PL_RB_UNIT_MM };
    pl_rb_quantity converted;
    pl_rb_dimension torque;
    pl_rb_dimension angular_speed;
    pl_rb_dimension power;
    pl_rb_dimension expected_power;

    assert(pl_rb_quantity_convert(&input, PL_RB_UNIT_M, &converted) == PL_RB_OK);
    assert(converted.value == 1 && converted.unit == PL_RB_UNIT_M);
    input.value = 2500;
    input.unit = PL_RB_UNIT_MA;
    assert(pl_rb_quantity_convert(&input, PL_RB_UNIT_A, &converted) == PL_RB_OK);
    assert(converted.value == 2);
    assert(pl_rb_unit_dimension(PL_RB_UNIT_NM, &torque) == PL_RB_OK);
    assert(pl_rb_unit_dimension(PL_RB_UNIT_RADPS, &angular_speed) == PL_RB_OK);
    assert(pl_rb_dimension_multiply(&torque, &angular_speed, &power) == PL_RB_OK);
    assert(pl_rb_unit_dimension(PL_RB_UNIT_W, &expected_power) == PL_RB_OK);
    assert(pl_rb_dimension_equal(&power, &expected_power));
    input.value = 1;
    input.unit = PL_RB_UNIT_M;
    converted.value = 1;
    converted.unit = PL_RB_UNIT_RAD;
    assert(pl_rb_quantity_add(&input, &converted, &input) == PL_RB_INVALID);
}

static void test_actuator_transactions_and_failsafe(void) {
    pl_rb_actuator_transaction transaction;
    pl_rb_actuator_frame published = {0};
    pl_rb_failsafe_policy policy = {0};
    pl_rb_failsafe_action action = PL_RB_ACTION_NONE;

    pl_rb_actuator_init(&transaction);
    assert(pl_rb_actuator_begin(&transaction, 2u) == PL_RB_OK);
    assert(pl_rb_actuator_set(&transaction, 10u, 500) == PL_RB_OK);
    assert(pl_rb_actuator_set(&transaction, 11u, -500) == PL_RB_OK);
    assert(pl_rb_actuator_commit(&transaction, publish_ok, &published) == PL_RB_OK);
    assert(published.count == 2u && published.generation == 1u);
    assert(pl_rb_actuator_begin(&transaction, 1u) == PL_RB_OK);
    assert(pl_rb_actuator_set(&transaction, 10u, 100) == PL_RB_OK);
    assert(pl_rb_actuator_commit(&transaction, publish_fail, NULL) == PL_RB_IO);
    assert(transaction.active.generation == 1u);
    assert(transaction.active.targets[0].setpoint == 500);
    assert(pl_rb_actuator_begin(&transaction, 2u) == PL_RB_OK);
    assert(pl_rb_actuator_set(&transaction, 10u, 1) == PL_RB_OK);
    assert(pl_rb_actuator_set(&transaction, 10u, 2) == PL_RB_CONFLICT);
    assert(pl_rb_actuator_rollback(&transaction) == PL_RB_OK);

    for (size_t i = 0u; i < PL_RB_FAULT_COUNT; ++i)
        policy.action_for_fault[i] = PL_RB_ACTION_BRAKE;
    policy.action_for_fault[PL_RB_FAULT_OVERTEMP] = PL_RB_ACTION_SAFE_STOP;
    policy.response_ticks = 5u;
    assert(pl_rb_failsafe_validate(&policy) == PL_RB_OK);
    assert(pl_rb_failsafe_resolve(&policy, PL_RB_FAULT_OVERTEMP, &action) == PL_RB_OK);
    assert(action == PL_RB_ACTION_SAFE_STOP);
    policy.action_for_fault[PL_RB_FAULT_SENSOR] = PL_RB_ACTION_NONE;
    assert(pl_rb_failsafe_validate(&policy) == PL_RB_INVALID);
}

static void test_shadow_thermal_calibration_and_limp(void) {
    pl_rb_shadow_result shadow;
    pl_rb_thermal_state thermal = {0};
    const pl_rb_thermal_config thermal_config = {
        UINT64_C(2000000000), 60000u, 80000u
    };
    uint16_t derate = 0u;
    const pl_rb_calibration calibration = {
        0, 4095, 100, 2u, 1u, 100u, 3u
    };
    pl_rb_calibration_state calibration_state = {0};
    int64_t calibrated = 0;
    const pl_rb_limp_policy limp = {
        (UINT32_C(1) << 0) | (UINT32_C(1) << 1),
        UINT32_C(1) << 0,
        300u
    };
    pl_rb_limp_decision decision;

    assert(pl_rb_shadow_compare(100, 110, 10u, &shadow) == PL_RB_OK);
    assert(!shadow.outside_tolerance);
    assert(pl_rb_shadow_compare(100, 111, 10u, &shadow) == PL_RB_OK);
    assert(shadow.outside_tolerance && shadow.residual_q == 11);

    assert(pl_rb_thermal_update(&thermal, &thermal_config, 1000u, 1000u,
                                65000u, &derate) == PL_RB_OK);
    assert(derate < 1000u && derate > 0u);
    assert(pl_rb_thermal_update(&thermal, &thermal_config, 1000u, 1000u,
                                85000u, &derate) == PL_RB_OK);
    assert(derate == 0u && thermal.tripped);

    assert(pl_rb_calibrate_sample(&calibration, &calibration_state, 100, &calibrated) == PL_RB_OK);
    assert(calibrated == 0);
    assert(pl_rb_calibrate_sample(&calibration, &calibration_state, 105, &calibrated) == PL_RB_OK);
    assert(calibrated == 10);
    assert(pl_rb_calibrate_sample(&calibration, &calibration_state, 5000, &calibrated) == PL_RB_RANGE);
    assert(pl_rb_limp_select(&limp, 0u, &decision) == PL_RB_OK);
    assert(decision.mode == PL_RB_LIMP_NOMINAL && decision.speed_limit_permille == 1000u);
    assert(pl_rb_limp_select(&limp, UINT32_C(1) << 0, &decision) == PL_RB_OK);
    assert(decision.mode == PL_RB_LIMP_LIMITED && decision.speed_limit_permille == 300u);
    assert(pl_rb_limp_select(&limp, UINT32_C(1) << 1, &decision) == PL_RB_OK);
    assert(decision.mode == PL_RB_LIMP_SAFE_STOP && decision.speed_limit_permille == 0u);
    assert(pl_rb_limp_select(&limp, UINT32_C(1) << 3, &decision) == PL_RB_UNSAFE);
}

static void test_dma_lifecycle(void) {
    alignas(16) unsigned char bytes[64];
    pl_rb_dma_buffer buffer;

    assert(pl_rb_dma_init(&buffer, bytes, sizeof bytes, 16u,
                          PL_RB_DMA_TO_DEVICE) == PL_RB_OK);
    assert(pl_rb_dma_submit(&buffer, buffer.generation) == PL_RB_OK);
    assert(buffer.state == PL_RB_DMA_DEVICE_OWNED);
    assert(pl_rb_dma_submit(&buffer, buffer.generation) == PL_RB_STATE);
    assert(pl_rb_dma_reclaim(&buffer, buffer.generation) == PL_RB_STATE);
    assert(pl_rb_dma_complete(&buffer, buffer.generation) == PL_RB_OK);
    assert(pl_rb_dma_reclaim(&buffer, buffer.generation) == PL_RB_OK);
    assert(buffer.state == PL_RB_DMA_CPU_OWNED);
}

static void test_flight_recorder(void) {
    pl_rb_recorder recorder;
    pl_rb_event event;
    pl_rb_event received;

    pl_rb_recorder_init(&recorder);
    for (uint32_t i = 0u; i < PL_RB_RECORDER_CAPACITY; ++i) {
        event.tick = i;
        event.code = i + 1u;
        event.value = (int32_t)i;
        assert(pl_rb_recorder_write(&recorder, &event) == PL_RB_OK);
    }
    assert(pl_rb_recorder_write(&recorder, &event) == PL_RB_FULL);
    assert(pl_rb_recorder_dropped(&recorder) == 1u);
    assert(pl_rb_recorder_read(&recorder, &received) == PL_RB_OK);
    assert(received.tick == 0u && received.code == 1u);
    assert(pl_rb_recorder_write(&recorder, &event) == PL_RB_OK);
    for (uint32_t i = 1u; i < PL_RB_RECORDER_CAPACITY; ++i) {
        assert(pl_rb_recorder_read(&recorder, &received) == PL_RB_OK);
        assert(received.tick == i);
    }
    assert(pl_rb_recorder_read(&recorder, &received) == PL_RB_OK);
    assert(pl_rb_recorder_read(&recorder, &received) == PL_RB_EMPTY);
}

static void test_live_tuning(void) {
    pl_rb_tuning_store store;
    pl_rb_tuning_snapshot snapshot;
    const int64_t first[] = { 10, 20, 30 };
    const int64_t second[] = { 11, 21, 31 };
    const int64_t third[] = { 12, 22, 32 };

    pl_rb_tuning_init(&store);
    assert(pl_rb_tuning_publish(&store, first, 3u, 1u,
                                pl_rb_tuning_checksum(first, 3u)) == PL_RB_OK);
    assert(pl_rb_tuning_acquire(&store, &snapshot) == PL_RB_OK);
    assert(snapshot.generation == 1u && snapshot.count == 3u);
    assert(snapshot.values[1] == 20);
    assert(pl_rb_tuning_publish(&store, second, 3u, 2u,
                                pl_rb_tuning_checksum(second, 3u)) == PL_RB_OK);
    assert(pl_rb_tuning_publish(&store, third, 3u, 3u,
                                pl_rb_tuning_checksum(third, 3u)) == PL_RB_BUSY);
    assert(pl_rb_tuning_release(&store, &snapshot) == PL_RB_OK);
    assert(pl_rb_tuning_publish(&store, third, 3u, 3u,
                                pl_rb_tuning_checksum(third, 3u)) == PL_RB_OK);
    assert(pl_rb_tuning_publish(&store, third, 3u, 3u, 0u) == PL_RB_CONFLICT);
}

static void test_deterministic_simulator(void) {
    pl_rb_simulator simulator;
    pl_rb_actuator_transaction transaction;

    pl_rb_sim_init(&simulator, 1000u);
    assert(pl_rb_sim_advance(&simulator, 250u) == PL_RB_OK);
    assert(pl_rb_sim_now(&simulator) == 1250u);
    pl_rb_actuator_init(&transaction);
    assert(pl_rb_actuator_begin(&transaction, 1u) == PL_RB_OK);
    assert(pl_rb_actuator_set(&transaction, 1u, 42) == PL_RB_OK);
    assert(pl_rb_actuator_commit(&transaction, pl_rb_sim_publish, &simulator) == PL_RB_OK);
    assert(simulator.last_frame.count == 1u);
    assert(simulator.last_frame.targets[0].setpoint == 42);
}

int main(void) {
    test_timing_and_quotas();
    test_sensor_time_alignment();
    test_physical_dimensions();
    test_actuator_transactions_and_failsafe();
    test_shadow_thermal_calibration_and_limp();
    test_dma_lifecycle();
    test_flight_recorder();
    test_live_tuning();
    test_deterministic_simulator();
    puts("Pytyliki Robotics Core tests passed.");
    return 0;
}
