#ifndef PYTYLIKI_ROBOTICS_H
#define PYTYLIKI_ROBOTICS_H

#include "embedded.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_ROBOTICS_ABI_VERSION 1u
#define PL_ROBOTICS_MAX_AXES 6u
#define PL_ROBOTICS_FILTER_MAX 16u
#define PL_ROBOTICS_BUFFER_CAPACITY 256u
#define PL_ROBOTICS_PACKET_MAX_PAYLOAD 48u
#define PL_ROBOTICS_PACKET_OVERHEAD 9u

typedef enum {
    PL_ROBOTICS_OK = 0,
    PL_ROBOTICS_INVALID_ARGUMENT = 1,
    PL_ROBOTICS_RANGE = 2,
    PL_ROBOTICS_CAPACITY = 3,
    PL_ROBOTICS_CONFLICT = 4,
    PL_ROBOTICS_NOT_READY = 5,
    PL_ROBOTICS_SAFETY_LIMIT = 6,
    PL_ROBOTICS_PROTOCOL = 7,
    PL_ROBOTICS_INTEGRITY = 8,
    PL_ROBOTICS_TIME_OVERFLOW = 9
} pl_robotics_status;

typedef enum {
    PL_ROBOTICS_DIAG_NONE = 0,
    PL_ROBOTICS_DIAG_SENSOR_RANGE = 1001,
    PL_ROBOTICS_DIAG_CALIBRATION = 1002,
    PL_ROBOTICS_DIAG_FILTER_WINDOW = 1003,
    PL_ROBOTICS_DIAG_KINEMATICS_REACH = 1004,
    PL_ROBOTICS_DIAG_TRAJECTORY = 1005,
    PL_ROBOTICS_DIAG_ACTUATOR_LIMIT = 1006,
    PL_ROBOTICS_DIAG_PID_INPUT = 1007,
    PL_ROBOTICS_DIAG_AXIS_LIMIT = 1008,
    PL_ROBOTICS_DIAG_WATCHDOG = 1009,
    PL_ROBOTICS_DIAG_PACKET = 1010,
    PL_ROBOTICS_DIAG_TRANSACTION = 1011,
    PL_ROBOTICS_DIAG_SIMULATOR = 1012,
    PL_ROBOTICS_DIAG_TEMPERATURE_LIMIT = 1013
} pl_robotics_diagnostic_code;

typedef struct {
    uint32_t abi_version;
    uint32_t code;
    uint32_t severity;
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t detail;
    char message[128];
} pl_robotics_diagnostic;

typedef struct {
    uint8_t bytes[PL_ROBOTICS_BUFFER_CAPACITY];
    size_t head;
    size_t length;
} pl_robotics_buffer;

typedef struct {
    uint64_t now;
    uint64_t deadline;
    int armed;
} pl_robotics_timer;

typedef struct {
    pl_embedded_register_bank bank;
} pl_robotics_registers;

typedef struct {
    double position[PL_ROBOTICS_MAX_AXES];
    double velocity[PL_ROBOTICS_MAX_AXES];
    double command[PL_ROBOTICS_MAX_AXES];
    uint32_t enabled_mask;
} pl_robotics_state;

typedef struct {
    pl_robotics_state committed;
    pl_robotics_state staged;
    uint64_t generation;
    int active;
} pl_robotics_transaction;

typedef struct {
    pl_robotics_state state;
    pl_robotics_registers registers;
    pl_robotics_timer clock;
    uint64_t step_count;
    double max_velocity;
    int initialized;
} pl_robotics_simulator;

/* Bounded, allocation-free byte transport. Each operation is all-or-nothing. */
void pl_robotics_buffer_init(pl_robotics_buffer *buffer);
pl_robotics_status pl_robotics_buffer_write(pl_robotics_buffer *buffer,
                                             const void *data, size_t size);
pl_robotics_status pl_robotics_buffer_read(pl_robotics_buffer *buffer,
                                            void *out, size_t size);
size_t pl_robotics_buffer_available(const pl_robotics_buffer *buffer);

/* Deterministic logical clock. Advancing time never sleeps or reads wall time. */
void pl_robotics_timer_init(pl_robotics_timer *timer, uint64_t initial_tick);
pl_robotics_status pl_robotics_timer_arm(pl_robotics_timer *timer, uint64_t delay_ticks);
pl_robotics_status pl_robotics_timer_advance(pl_robotics_timer *timer, uint64_t ticks);
int pl_robotics_timer_expired(const pl_robotics_timer *timer);

/* The register bank is an in-memory simulator; it never dereferences MMIO. */
void pl_robotics_registers_init(pl_robotics_registers *registers);
pl_robotics_status pl_robotics_register_add(pl_robotics_registers *registers,
                                             const char *name, uint32_t address,
                                             uint32_t reset_value, uint32_t writable_mask,
                                             int readable, int writable);
pl_robotics_status pl_robotics_register_read(const pl_robotics_registers *registers,
                                              const char *name, uint32_t *out_value);
pl_robotics_status pl_robotics_register_write(pl_robotics_registers *registers,
                                               const char *name, uint32_t value);

/* Optimistic state transaction: publish staged state only for the expected generation. */
pl_robotics_status pl_robotics_transaction_begin(pl_robotics_transaction *transaction,
                                                   const pl_robotics_state *current);
pl_robotics_status pl_robotics_transaction_set_axis(pl_robotics_transaction *transaction,
                                                      size_t axis, double position,
                                                      double velocity, double command);
pl_robotics_status pl_robotics_transaction_commit(pl_robotics_transaction *transaction,
                                                    uint64_t expected_generation,
                                                    pl_robotics_state *published);
pl_robotics_status pl_robotics_transaction_rollback(pl_robotics_transaction *transaction);

/* The thirteen robotics operations. Geometry preserves the caller's length unit; trajectory time is milliseconds and PID delta is seconds. */
pl_robotics_status pl_robotics_sensor_sample(double raw, double scale, double offset,
                                               double minimum, double maximum, double *out);
pl_robotics_status pl_robotics_sensor_calibrate(double raw_zero, double raw_reference,
                                                  double reference_value, double *out_scale,
                                                  double *out_offset);
pl_robotics_status pl_robotics_sensor_filter(const double *samples, size_t sample_count,
                                               size_t window, double *out);
pl_robotics_status pl_robotics_transform_2d(double x, double y, double translate_x,
                                              double translate_y, double cosine,
                                              double sine, double *out_x, double *out_y);
pl_robotics_status pl_robotics_forward_kinematics(double link_a, double link_b,
                                                    double cosine_a, double sine_a,
                                                    double cosine_ab, double sine_ab,
                                                    double *out_x, double *out_y);
pl_robotics_status pl_robotics_inverse_kinematics(double x, double y, double link_a,
                                                    double link_b, int elbow_sign,
                                                    double *out_angle_a, double *out_angle_b);
pl_robotics_status pl_robotics_trajectory_interpolate(double start, double finish,
                                                        uint64_t elapsed_ms, uint64_t duration_ms,
                                                        double *out);
pl_robotics_status pl_robotics_actuator_command(double requested, double current,
                                                  double minimum, double maximum,
                                                  double maximum_step, double *out);
pl_robotics_status pl_robotics_pid_step(double setpoint, double measurement, double delta_seconds,
                                          double kp, double ki, double kd, double integral_limit,
                                          double output_minimum, double output_maximum,
                                          double *integral, double *previous_error,
                                          double *out);
pl_robotics_status pl_robotics_axis_synchronize(const double *current, const double *target,
                                                  size_t axis_count, double maximum_step,
                                                  double *out);
pl_robotics_status pl_robotics_safety_supervise(const double *positions, const double *minimum,
                                                  const double *maximum, size_t axis_count,
                                                  double temperature, double temperature_limit,
                                                  uint64_t now_tick, uint64_t heartbeat_tick,
                                                  uint64_t watchdog_limit,
                                                  pl_robotics_diagnostic *diagnostic);
pl_robotics_status pl_robotics_packet_encode(uint16_t sequence, const uint8_t *payload,
                                               size_t payload_size, uint8_t *out,
                                               size_t out_capacity, size_t *out_size);
pl_robotics_status pl_robotics_packet_decode(const uint8_t *packet, size_t packet_size,
                                               uint16_t *out_sequence, uint8_t *out_payload,
                                               size_t payload_capacity, size_t *out_payload_size);
pl_robotics_status pl_robotics_simulator_init(pl_robotics_simulator *simulator);
pl_robotics_status pl_robotics_simulator_step(pl_robotics_simulator *simulator,
                                                uint32_t delta_ms,
                                                pl_robotics_diagnostic *diagnostic);

void pl_robotics_diagnostic_set(pl_robotics_diagnostic *diagnostic,
                                 pl_robotics_diagnostic_code code, uint32_t severity,
                                 uint64_t detail, const char *message);

const char *pl_robotics_status_name(pl_robotics_status status);

#ifdef __cplusplus
}
#endif

#endif
