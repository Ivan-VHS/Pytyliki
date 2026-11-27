#include "robotics.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int close_to(double left, double right, double tolerance) {
    return fabs(left - right) <= tolerance;
}

static void test_fixed_buffer(void) {
    pl_robotics_buffer buffer;
    char output[8] = {0};
    const char first[] = "abcd";
    const char second[] = "efghi";

    pl_robotics_buffer_init(&buffer);
    assert(pl_robotics_buffer_available(&buffer) == 0u);
    assert(pl_robotics_buffer_write(&buffer, first, 4u) == PL_ROBOTICS_OK);
    assert(pl_robotics_buffer_read(&buffer, output, 3u) == PL_ROBOTICS_OK);
    assert(memcmp(output, "abc", 3u) == 0);
    assert(pl_robotics_buffer_write(&buffer, second, 5u) == PL_ROBOTICS_OK);
    memset(output, 0, sizeof(output));
    assert(pl_robotics_buffer_read(&buffer, output, 6u) == PL_ROBOTICS_OK);
    assert(memcmp(output, "defghi", 6u) == 0);
    assert(pl_robotics_buffer_available(&buffer) == 0u);
    assert(pl_robotics_buffer_read(&buffer, output, 1u) == PL_ROBOTICS_NOT_READY);

    {
        uint8_t full[PL_ROBOTICS_BUFFER_CAPACITY];
        memset(full, 0xA5, sizeof(full));
        assert(pl_robotics_buffer_write(&buffer, full, sizeof(full)) == PL_ROBOTICS_OK);
        assert(pl_robotics_buffer_write(&buffer, "x", 1u) == PL_ROBOTICS_CAPACITY);
        assert(pl_robotics_buffer_available(&buffer) == sizeof(full));
    }
}

static void test_logical_timer(void) {
    pl_robotics_timer timer;
    pl_robotics_timer_init(&timer, 100u);
    assert(pl_robotics_timer_arm(&timer, 20u) == PL_ROBOTICS_OK);
    assert(pl_robotics_timer_advance(&timer, 19u) == PL_ROBOTICS_OK);
    assert(!pl_robotics_timer_expired(&timer));
    assert(pl_robotics_timer_advance(&timer, 1u) == PL_ROBOTICS_OK);
    assert(pl_robotics_timer_expired(&timer));
    assert(pl_robotics_timer_arm(&timer, 0u) == PL_ROBOTICS_INVALID_ARGUMENT);

    pl_robotics_timer_init(&timer, UINT64_MAX - 1u);
    assert(pl_robotics_timer_arm(&timer, 1u) == PL_ROBOTICS_OK);
    assert(pl_robotics_timer_advance(&timer, 1u) == PL_ROBOTICS_OK);
    assert(pl_robotics_timer_expired(&timer));
    assert(pl_robotics_timer_advance(&timer, 1u) == PL_ROBOTICS_TIME_OVERFLOW);
}

static void test_registers_and_state_transactions(void) {
    pl_robotics_registers registers;
    uint32_t value = 0u;
    pl_robotics_state initial = {0};
    pl_robotics_state published = {0};
    pl_robotics_transaction transaction;

    pl_robotics_registers_init(&registers);
    assert(pl_robotics_register_add(&registers, "MOTOR.CMD", 0x50000000u,
                                    0u, 0x000000FFu, 1, 1) == PL_ROBOTICS_OK);
    assert(pl_robotics_register_add(&registers, "MOTOR.STATUS", 0x50000004u,
                                    3u, 0u, 1, 0) == PL_ROBOTICS_OK);
    assert(pl_robotics_register_write(&registers, "MOTOR.CMD", 0x25u) == PL_ROBOTICS_OK);
    assert(pl_robotics_register_read(&registers, "MOTOR.CMD", &value) == PL_ROBOTICS_OK);
    assert(value == 0x25u);
    assert(pl_robotics_register_read(&registers, "MOTOR.STATUS", &value) == PL_ROBOTICS_OK);
    assert(value == 3u);
    assert(pl_robotics_register_write(&registers, "MOTOR.STATUS", 1u) == PL_ROBOTICS_SAFETY_LIMIT);
    assert(pl_robotics_register_write(&registers, "MOTOR.CMD", 0x100u) == PL_ROBOTICS_SAFETY_LIMIT);
    assert(pl_robotics_register_read(&registers, "MISSING", &value) == PL_ROBOTICS_INVALID_ARGUMENT);

    initial.position[0] = 10.0;
    published.position[0] = -99.0;
    assert(pl_robotics_transaction_begin(&transaction, &initial) == PL_ROBOTICS_OK);
    assert(pl_robotics_transaction_set_axis(&transaction, 0u, 12.0, 1.5, 15.0) == PL_ROBOTICS_OK);
    assert(pl_robotics_transaction_commit(&transaction, 1u, &published) == PL_ROBOTICS_CONFLICT);
    assert(published.position[0] == -99.0);
    assert(pl_robotics_transaction_commit(&transaction, 0u, &published) == PL_ROBOTICS_OK);
    assert(published.position[0] == 12.0 && transaction.generation == 1u);
    assert(pl_robotics_transaction_set_axis(&transaction, 0u, 99.0, 0.0, 0.0) == PL_ROBOTICS_OK);
    assert(pl_robotics_transaction_rollback(&transaction) == PL_ROBOTICS_OK);
    assert(transaction.staged.position[0] == 12.0);
    assert(pl_robotics_transaction_set_axis(&transaction, PL_ROBOTICS_MAX_AXES, 0.0, 0.0, 0.0)
           == PL_ROBOTICS_INVALID_ARGUMENT);
}

static void test_sensor_operations(void) {
    const double samples[] = {1.0, 2.0, 3.0, 4.0};
    double value = -1.0;
    double scale = 0.0;
    double offset = 0.0;

    assert(pl_robotics_sensor_sample(5.0, 2.0, -1.0, 0.0, 10.0, &value) == PL_ROBOTICS_OK);
    assert(value == 9.0);
    assert(pl_robotics_sensor_sample(6.0, 2.0, 0.0, 0.0, 10.0, &value) == PL_ROBOTICS_RANGE);

    assert(pl_robotics_sensor_calibrate(100.0, 200.0, 2.0, &scale, &offset) == PL_ROBOTICS_OK);
    assert(close_to(scale, 0.02, 1e-12));
    assert(close_to(offset, -2.0, 1e-12));
    assert(pl_robotics_sensor_calibrate(1.0, 1.0, 2.0, &scale, &offset) == PL_ROBOTICS_RANGE);

    assert(pl_robotics_sensor_filter(samples, 4u, 3u, &value) == PL_ROBOTICS_OK);
    assert(close_to(value, 3.0, 1e-12));
    value = 123.0;
    assert(pl_robotics_sensor_filter(samples, 4u, 0u, &value) == PL_ROBOTICS_INVALID_ARGUMENT);
    assert(value == 123.0);
}

static void test_geometry_and_kinematics(void) {
    double x = 0.0;
    double y = 0.0;
    double angle_a = 0.0;
    double angle_b = 0.0;

    assert(pl_robotics_transform_2d(1.0, 0.0, 3.0, 4.0,
                                    0.0, 1.0, &x, &y) == PL_ROBOTICS_OK);
    assert(close_to(x, 3.0, 1e-12) && close_to(y, 5.0, 1e-12));
    assert(pl_robotics_transform_2d(1.0, 0.0, 0.0, 0.0,
                                    0.5, 0.5, &x, &y) == PL_ROBOTICS_RANGE);

    assert(pl_robotics_forward_kinematics(100.0, 50.0,
                                          0.0, 1.0, -1.0, 0.0,
                                          &x, &y) == PL_ROBOTICS_OK);
    assert(close_to(x, -50.0, 1e-10) && close_to(y, 100.0, 1e-10));

    assert(pl_robotics_inverse_kinematics(100.0, 0.0, 100.0, 100.0, 1,
                                          &angle_a, &angle_b) == PL_ROBOTICS_OK);
    assert(close_to(angle_a, -1.0471975511965976, 1e-9));
    assert(close_to(angle_b, 2.0943951023931953, 1e-9));
    assert(pl_robotics_inverse_kinematics(300.0, 0.0, 100.0, 100.0, 1,
                                          &angle_a, &angle_b) == PL_ROBOTICS_RANGE);
}

static void test_motion_and_control(void) {
    double value = 0.0;
    double integral = 0.0;
    double previous_error = 0.0;
    const double current[] = {0.0, 10.0, -5.0};
    const double target[] = {10.0, 0.0, 5.0};
    double synchronized[3] = {0.0};

    assert(pl_robotics_trajectory_interpolate(0.0, 100.0, 500u, 1000u, &value)
           == PL_ROBOTICS_OK);
    assert(value == 50.0);
    assert(pl_robotics_trajectory_interpolate(0.0, 100.0, 2000u, 1000u, &value)
           == PL_ROBOTICS_OK && value == 100.0);
    assert(pl_robotics_trajectory_interpolate(0.0, 1.0, 0u, 0u, &value)
           == PL_ROBOTICS_INVALID_ARGUMENT);

    assert(pl_robotics_actuator_command(100.0, 0.0, -10.0, 10.0, 2.0, &value)
           == PL_ROBOTICS_OK && value == 2.0);
    assert(pl_robotics_actuator_command(-100.0, 0.0, -10.0, 10.0, 5.0, &value)
           == PL_ROBOTICS_OK && value == -5.0);

    assert(pl_robotics_pid_step(5.0, 0.0, 1.0, 2.0, 0.0, 0.0, 100.0,
                                0.0, 8.0, &integral, &previous_error, &value)
           == PL_ROBOTICS_OK);
    assert(value == 8.0 && integral == 5.0 && previous_error == 5.0);
    assert(pl_robotics_pid_step(5.0, 0.0, 0.0, 2.0, 0.0, 0.0, 100.0,
                                0.0, 8.0, &integral, &previous_error, &value)
           == PL_ROBOTICS_INVALID_ARGUMENT);

    assert(pl_robotics_axis_synchronize(current, target, 3u, 2.0, synchronized)
           == PL_ROBOTICS_OK);
    assert(synchronized[0] == 2.0 && synchronized[1] == 8.0 && synchronized[2] == -3.0);
    assert(pl_robotics_axis_synchronize(current, target, 0u, 2.0, synchronized)
           == PL_ROBOTICS_INVALID_ARGUMENT);
    {
        const double huge_current[] = {0.0, DBL_MAX};
        const double huge_target[] = {1.0, -DBL_MAX};
        double untouched[] = {11.0, 22.0};
        assert(pl_robotics_axis_synchronize(huge_current, huge_target, 2u, 1.0, untouched)
               == PL_ROBOTICS_RANGE);
        assert(untouched[0] == 11.0 && untouched[1] == 22.0);
    }
}

static void test_safety_and_wire_protocol(void) {
    const double positions[] = {0.0, 5.0};
    const double minimum[] = {-10.0, 0.0};
    const double maximum[] = {10.0, 10.0};
    double unsafe_positions[] = {0.0, 11.0};
    pl_robotics_diagnostic diagnostic;
    uint8_t packet[PL_ROBOTICS_PACKET_OVERHEAD + PL_ROBOTICS_PACKET_MAX_PAYLOAD];
    uint8_t payload_out[PL_ROBOTICS_PACKET_MAX_PAYLOAD];
    const uint8_t payload[] = {0x10u, 0x20u, 0x30u};
    uint16_t sequence = 0u;
    size_t packet_size = 0u;
    size_t payload_size = 0u;

    assert(pl_robotics_safety_supervise(positions, minimum, maximum, 2u,
                                        35.0, 80.0, 100u, 99u, 2u,
                                        &diagnostic) == PL_ROBOTICS_OK);
    assert(diagnostic.code == PL_ROBOTICS_DIAG_NONE);
    assert(pl_robotics_safety_supervise(unsafe_positions, minimum, maximum, 2u,
                                        35.0, 80.0, 100u, 99u, 2u,
                                        &diagnostic) == PL_ROBOTICS_SAFETY_LIMIT);
    assert(diagnostic.code == PL_ROBOTICS_DIAG_AXIS_LIMIT);
    assert(pl_robotics_safety_supervise(positions, minimum, maximum, 2u,
                                        35.0, 80.0, 100u, 90u, 2u,
                                        &diagnostic) == PL_ROBOTICS_SAFETY_LIMIT);
    assert(diagnostic.code == PL_ROBOTICS_DIAG_WATCHDOG);
    assert(pl_robotics_safety_supervise(positions, minimum, maximum, 2u,
                                        90.0, 80.0, 100u, 99u, 2u,
                                        &diagnostic) == PL_ROBOTICS_SAFETY_LIMIT);
    assert(diagnostic.code == PL_ROBOTICS_DIAG_TEMPERATURE_LIMIT);

    assert(pl_robotics_packet_encode(0x1234u, payload, sizeof(payload),
                                     packet, sizeof(packet), &packet_size) == PL_ROBOTICS_OK);
    assert(packet_size == PL_ROBOTICS_PACKET_OVERHEAD + sizeof(payload));
    assert(pl_robotics_packet_decode(packet, packet_size, &sequence, payload_out,
                                     sizeof(payload_out), &payload_size) == PL_ROBOTICS_OK);
    assert(sequence == 0x1234u && payload_size == sizeof(payload));
    assert(memcmp(payload, payload_out, sizeof(payload)) == 0);
    packet[7] ^= 0x01u;
    assert(pl_robotics_packet_decode(packet, packet_size, &sequence, payload_out,
                                     sizeof(payload_out), &payload_size) == PL_ROBOTICS_INTEGRITY);
    assert(payload_out[0] == payload[0]);
}

static void test_deterministic_simulator(void) {
    pl_robotics_simulator simulator;
    pl_robotics_diagnostic diagnostic;
    uint32_t value = 0u;

    assert(pl_robotics_simulator_init(&simulator) == PL_ROBOTICS_OK);
    simulator.state.command[0] = 90.0;
    assert(pl_robotics_simulator_step(&simulator, 10u, &diagnostic) == PL_ROBOTICS_OK);
    assert(simulator.step_count == 1u && simulator.clock.now == 10u);
    assert(simulator.state.position[0] > 0.0);
    assert(diagnostic.code == PL_ROBOTICS_DIAG_NONE);
    assert(pl_robotics_register_read(&simulator.registers, "ROBOT.TICK", &value)
           == PL_ROBOTICS_OK && value == 1u);
    assert(pl_robotics_register_read(&simulator.registers, "AXIS0.POS", &value)
           == PL_ROBOTICS_OK && value > 1000000u);
    {
        pl_robotics_state before;
        simulator.state.command[1] = NAN;
        before = simulator.state;
        assert(pl_robotics_simulator_step(&simulator, 10u, &diagnostic)
               == PL_ROBOTICS_INVALID_ARGUMENT);
        assert(simulator.clock.now == 10u && simulator.step_count == 1u);
        assert(memcmp(simulator.state.position, before.position, sizeof(before.position)) == 0);
        assert(memcmp(simulator.state.velocity, before.velocity, sizeof(before.velocity)) == 0);
        assert(memcmp(simulator.state.command, before.command, sizeof(before.command)) == 0);
        simulator.state.command[1] = 0.0;
    }
    assert(pl_robotics_simulator_step(&simulator, 0u, &diagnostic)
           == PL_ROBOTICS_INVALID_ARGUMENT);
    assert(diagnostic.code == PL_ROBOTICS_DIAG_SIMULATOR);
}

int main(void) {
    test_fixed_buffer();
    test_logical_timer();
    test_registers_and_state_transactions();
    test_sensor_operations();
    test_geometry_and_kinematics();
    test_motion_and_control();
    test_safety_and_wire_protocol();
    test_deterministic_simulator();
    puts("Pytyliki Reaper Robotics Core tests passed.");
    return 0;
}
