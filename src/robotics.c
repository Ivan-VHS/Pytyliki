#include "robotics.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define PL_ROBOTICS_REGISTER_BASE UINT32_C(0x50000000)
#define PL_ROBOTICS_SIM_POSITION_BIAS 10000001.0

static int finite_value(double value) {
    return isfinite(value) != 0;
}

static double clamp_value(double value, double minimum, double maximum) {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static int buffer_valid(const pl_robotics_buffer *buffer) {
    return buffer && buffer->head < PL_ROBOTICS_BUFFER_CAPACITY &&
           buffer->length <= PL_ROBOTICS_BUFFER_CAPACITY;
}

static int state_valid(const pl_robotics_state *state) {
    size_t i;
    if (!state) return 0;
    for (i = 0u; i < PL_ROBOTICS_MAX_AXES; ++i) {
        if (!finite_value(state->position[i]) || !finite_value(state->velocity[i]) ||
            !finite_value(state->command[i])) return 0;
    }
    return 1;
}

static uint64_t saturating_u64(double value) {
    if (value <= 0.0) return 0u;
    if (value >= (double)UINT64_MAX) return UINT64_MAX;
    return (uint64_t)value;
}

void pl_robotics_diagnostic_set(pl_robotics_diagnostic *diagnostic,
                                 pl_robotics_diagnostic_code code, uint32_t severity,
                                 uint64_t detail, const char *message) {
    if (!diagnostic) return;
    memset(diagnostic, 0, sizeof(*diagnostic));
    diagnostic->abi_version = PL_ROBOTICS_ABI_VERSION;
    diagnostic->code = (uint32_t)code;
    diagnostic->severity = severity;
    diagnostic->source_offset = UINT32_MAX;
    diagnostic->detail = detail;
    if (message) {
        (void)snprintf(diagnostic->message, sizeof(diagnostic->message), "%s", message);
    }
}

const char *pl_robotics_status_name(pl_robotics_status status) {
    switch (status) {
        case PL_ROBOTICS_OK: return "OK";
        case PL_ROBOTICS_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case PL_ROBOTICS_RANGE: return "RANGE";
        case PL_ROBOTICS_CAPACITY: return "CAPACITY";
        case PL_ROBOTICS_CONFLICT: return "CONFLICT";
        case PL_ROBOTICS_NOT_READY: return "NOT_READY";
        case PL_ROBOTICS_SAFETY_LIMIT: return "SAFETY_LIMIT";
        case PL_ROBOTICS_PROTOCOL: return "PROTOCOL";
        case PL_ROBOTICS_INTEGRITY: return "INTEGRITY";
        case PL_ROBOTICS_TIME_OVERFLOW: return "TIME_OVERFLOW";
        default: return "UNKNOWN";
    }
}

void pl_robotics_buffer_init(pl_robotics_buffer *buffer) {
    if (buffer) memset(buffer, 0, sizeof(*buffer));
}

pl_robotics_status pl_robotics_buffer_write(pl_robotics_buffer *buffer,
                                             const void *data, size_t size) {
    size_t i;
    size_t tail;
    if (!buffer_valid(buffer) || (!data && size != 0u)) return PL_ROBOTICS_INVALID_ARGUMENT;
    if (size > PL_ROBOTICS_BUFFER_CAPACITY - buffer->length) return PL_ROBOTICS_CAPACITY;
    tail = (buffer->head + buffer->length) % PL_ROBOTICS_BUFFER_CAPACITY;
    for (i = 0u; i < size; ++i) {
        buffer->bytes[(tail + i) % PL_ROBOTICS_BUFFER_CAPACITY] =
            ((const uint8_t *)data)[i];
    }
    buffer->length += size;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_buffer_read(pl_robotics_buffer *buffer,
                                            void *out, size_t size) {
    size_t i;
    if (!buffer_valid(buffer) || (!out && size != 0u)) return PL_ROBOTICS_INVALID_ARGUMENT;
    if (size > buffer->length) return PL_ROBOTICS_NOT_READY;
    for (i = 0u; i < size; ++i) {
        ((uint8_t *)out)[i] =
            buffer->bytes[(buffer->head + i) % PL_ROBOTICS_BUFFER_CAPACITY];
    }
    buffer->head = (buffer->head + size) % PL_ROBOTICS_BUFFER_CAPACITY;
    buffer->length -= size;
    return PL_ROBOTICS_OK;
}

size_t pl_robotics_buffer_available(const pl_robotics_buffer *buffer) {
    return buffer_valid(buffer) ? buffer->length : 0u;
}

void pl_robotics_timer_init(pl_robotics_timer *timer, uint64_t initial_tick) {
    if (!timer) return;
    timer->now = initial_tick;
    timer->deadline = initial_tick;
    timer->armed = 0;
}

pl_robotics_status pl_robotics_timer_arm(pl_robotics_timer *timer, uint64_t delay_ticks) {
    if (!timer || delay_ticks == 0u) return PL_ROBOTICS_INVALID_ARGUMENT;
    if (UINT64_MAX - timer->now < delay_ticks) return PL_ROBOTICS_TIME_OVERFLOW;
    timer->deadline = timer->now + delay_ticks;
    timer->armed = 1;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_timer_advance(pl_robotics_timer *timer, uint64_t ticks) {
    if (!timer) return PL_ROBOTICS_INVALID_ARGUMENT;
    if (UINT64_MAX - timer->now < ticks) return PL_ROBOTICS_TIME_OVERFLOW;
    timer->now += ticks;
    return PL_ROBOTICS_OK;
}

int pl_robotics_timer_expired(const pl_robotics_timer *timer) {
    return timer && timer->armed && timer->now >= timer->deadline;
}

void pl_robotics_registers_init(pl_robotics_registers *registers) {
    if (registers) pl_embedded_register_bank_init(&registers->bank);
}

pl_robotics_status pl_robotics_register_add(pl_robotics_registers *registers,
                                             const char *name, uint32_t address,
                                             uint32_t reset_value, uint32_t writable_mask,
                                             int readable, int writable) {
    if (!registers) return PL_ROBOTICS_INVALID_ARGUMENT;
    return pl_embedded_register_add(&registers->bank, name, address, reset_value,
                                    writable_mask, readable, writable) == 0
        ? PL_ROBOTICS_OK : PL_ROBOTICS_INVALID_ARGUMENT;
}

pl_robotics_status pl_robotics_register_read(const pl_robotics_registers *registers,
                                              const char *name, uint32_t *out_value) {
    int result;
    if (!registers) return PL_ROBOTICS_INVALID_ARGUMENT;
    result = pl_embedded_register_read(&registers->bank, name, out_value);
    if (result == -2) return PL_ROBOTICS_SAFETY_LIMIT;
    return result == 0 ? PL_ROBOTICS_OK : PL_ROBOTICS_INVALID_ARGUMENT;
}

pl_robotics_status pl_robotics_register_write(pl_robotics_registers *registers,
                                               const char *name, uint32_t value) {
    int result;
    if (!registers) return PL_ROBOTICS_INVALID_ARGUMENT;
    result = pl_embedded_register_write(&registers->bank, name, value);
    if (result == -2 || result == -3) return PL_ROBOTICS_SAFETY_LIMIT;
    return result == 0 ? PL_ROBOTICS_OK : PL_ROBOTICS_INVALID_ARGUMENT;
}

pl_robotics_status pl_robotics_transaction_begin(pl_robotics_transaction *transaction,
                                                   const pl_robotics_state *current) {
    if (!transaction || !state_valid(current)) return PL_ROBOTICS_INVALID_ARGUMENT;
    memset(transaction, 0, sizeof(*transaction));
    transaction->committed = *current;
    transaction->staged = *current;
    transaction->active = 1;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_transaction_set_axis(pl_robotics_transaction *transaction,
                                                      size_t axis, double position,
                                                      double velocity, double command) {
    if (!transaction || !transaction->active) return PL_ROBOTICS_NOT_READY;
    if (axis >= PL_ROBOTICS_MAX_AXES || !finite_value(position) ||
        !finite_value(velocity) || !finite_value(command)) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    transaction->staged.position[axis] = position;
    transaction->staged.velocity[axis] = velocity;
    transaction->staged.command[axis] = command;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_transaction_commit(pl_robotics_transaction *transaction,
                                                    uint64_t expected_generation,
                                                    pl_robotics_state *published) {
    if (!transaction || !published || !transaction->active) return PL_ROBOTICS_NOT_READY;
    if (expected_generation != transaction->generation) return PL_ROBOTICS_CONFLICT;
    if (transaction->generation == UINT64_MAX) return PL_ROBOTICS_RANGE;
    *published = transaction->staged;
    transaction->committed = transaction->staged;
    ++transaction->generation;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_transaction_rollback(pl_robotics_transaction *transaction) {
    if (!transaction || !transaction->active) return PL_ROBOTICS_NOT_READY;
    transaction->staged = transaction->committed;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_sensor_sample(double raw, double scale, double offset,
                                               double minimum, double maximum, double *out) {
    double value;
    if (!out || !finite_value(raw) || !finite_value(scale) || !finite_value(offset) ||
        !finite_value(minimum) || !finite_value(maximum) || minimum > maximum) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    value = raw * scale + offset;
    if (!finite_value(value)) return PL_ROBOTICS_RANGE;
    if (value < minimum || value > maximum) return PL_ROBOTICS_RANGE;
    *out = value;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_sensor_calibrate(double raw_zero, double raw_reference,
                                                  double reference_value, double *out_scale,
                                                  double *out_offset) {
    double span;
    double scale;
    double offset;
    if (!out_scale || !out_offset || !finite_value(raw_zero) ||
        !finite_value(raw_reference) || !finite_value(reference_value)) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    span = raw_reference - raw_zero;
    if (fabs(span) <= 1e-12) return PL_ROBOTICS_RANGE;
    scale = reference_value / span;
    if (!finite_value(scale)) return PL_ROBOTICS_RANGE;
    offset = -raw_zero * scale;
    if (!finite_value(offset)) return PL_ROBOTICS_RANGE;
    *out_scale = scale;
    *out_offset = offset;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_sensor_filter(const double *samples, size_t sample_count,
                                               size_t window, double *out) {
    size_t i;
    double sum = 0.0;
    double compensation = 0.0;
    if (!samples || !out || sample_count == 0u || sample_count > PL_ROBOTICS_FILTER_MAX ||
        window == 0u || window > sample_count) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    for (i = sample_count - window; i < sample_count; ++i) {
        double adjusted;
        double next;
        if (!finite_value(samples[i])) return PL_ROBOTICS_RANGE;
        adjusted = samples[i] - compensation;
        next = sum + adjusted;
        compensation = (next - sum) - adjusted;
        sum = next;
    }
    {
        double result = sum / (double)window;
        if (!finite_value(result)) return PL_ROBOTICS_RANGE;
        *out = result;
    }
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_transform_2d(double x, double y, double translate_x,
                                              double translate_y, double cosine,
                                              double sine, double *out_x, double *out_y) {
    double rx;
    double ry;
    if (!out_x || !out_y || !finite_value(x) || !finite_value(y) ||
        !finite_value(translate_x) || !finite_value(translate_y) ||
        !finite_value(cosine) || !finite_value(sine)) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    if (fabs((cosine * cosine) + (sine * sine) - 1.0) > 1e-6) {
        return PL_ROBOTICS_RANGE;
    }
    rx = translate_x + (x * cosine) - (y * sine);
    ry = translate_y + (x * sine) + (y * cosine);
    if (!finite_value(rx) || !finite_value(ry)) return PL_ROBOTICS_RANGE;
    *out_x = rx;
    *out_y = ry;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_forward_kinematics(double link_a, double link_b,
                                                    double cosine_a, double sine_a,
                                                    double cosine_ab, double sine_ab,
                                                    double *out_x, double *out_y) {
    double x;
    double y;
    if (!out_x || !out_y || !finite_value(link_a) || !finite_value(link_b) ||
        !finite_value(cosine_a) || !finite_value(sine_a) ||
        !finite_value(cosine_ab) || !finite_value(sine_ab) ||
        link_a <= 0.0 || link_b <= 0.0) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    if (fabs(cosine_a * cosine_a + sine_a * sine_a - 1.0) > 1e-6 ||
        fabs(cosine_ab * cosine_ab + sine_ab * sine_ab - 1.0) > 1e-6) {
        return PL_ROBOTICS_RANGE;
    }
    x = link_a * cosine_a + link_b * cosine_ab;
    y = link_a * sine_a + link_b * sine_ab;
    if (!finite_value(x) || !finite_value(y)) return PL_ROBOTICS_RANGE;
    *out_x = x;
    *out_y = y;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_inverse_kinematics(double x, double y, double link_a,
                                                    double link_b, int elbow_sign,
                                                    double *out_angle_a, double *out_angle_b) {
    double radius_squared;
    double cosine_b;
    double sine_b;
    double angle_b;
    double angle_a;
    double radius;
    if (!out_angle_a || !out_angle_b || !finite_value(x) || !finite_value(y) ||
        !finite_value(link_a) || !finite_value(link_b) ||
        link_a <= 0.0 || link_b <= 0.0 || (elbow_sign != -1 && elbow_sign != 1)) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    radius_squared = x * x + y * y;
    if (!finite_value(radius_squared)) return PL_ROBOTICS_RANGE;
    radius = sqrt(radius_squared);
    if (radius > link_a + link_b + 1e-9 ||
        radius < fabs(link_a - link_b) - 1e-9 || radius <= 1e-12) {
        return PL_ROBOTICS_RANGE;
    }
    cosine_b = (radius_squared - link_a * link_a - link_b * link_b) /
               (2.0 * link_a * link_b);
    cosine_b = clamp_value(cosine_b, -1.0, 1.0);
    sine_b = (double)elbow_sign * sqrt(fmax(0.0, 1.0 - cosine_b * cosine_b));
    angle_b = atan2(sine_b, cosine_b);
    angle_a = atan2(y, x) - atan2(link_b * sine_b, link_a + link_b * cosine_b);
    if (!finite_value(angle_a) || !finite_value(angle_b)) return PL_ROBOTICS_RANGE;
    *out_angle_a = angle_a;
    *out_angle_b = angle_b;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_trajectory_interpolate(double start, double finish,
                                                        uint64_t elapsed_ms, uint64_t duration_ms,
                                                        double *out) {
    double ratio;
    double result;
    if (!out || !finite_value(start) || !finite_value(finish) || duration_ms == 0u) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    ratio = elapsed_ms >= duration_ms ? 1.0 : (double)elapsed_ms / (double)duration_ms;
    result = start + (finish - start) * ratio;
    if (!finite_value(result)) return PL_ROBOTICS_RANGE;
    *out = result;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_actuator_command(double requested, double current,
                                                  double minimum, double maximum,
                                                  double maximum_step, double *out) {
    double target;
    double delta;
    double result;
    if (!out || !finite_value(requested) || !finite_value(current) ||
        !finite_value(minimum) || !finite_value(maximum) ||
        !finite_value(maximum_step) || minimum > maximum || maximum_step < 0.0) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    target = clamp_value(requested, minimum, maximum);
    delta = target - current;
    if (!finite_value(delta)) return PL_ROBOTICS_RANGE;
    delta = clamp_value(delta, -maximum_step, maximum_step);
    result = clamp_value(current + delta, minimum, maximum);
    if (!finite_value(result)) return PL_ROBOTICS_RANGE;
    *out = result;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_pid_step(double setpoint, double measurement, double delta_seconds,
                                          double kp, double ki, double kd, double integral_limit,
                                          double output_minimum, double output_maximum,
                                          double *integral, double *previous_error,
                                          double *out) {
    double error;
    double candidate_integral;
    double derivative;
    double output;
    if (!integral || !previous_error || !out || !finite_value(setpoint) ||
        !finite_value(measurement) || !finite_value(delta_seconds) || delta_seconds <= 0.0 ||
        !finite_value(kp) || !finite_value(ki) || !finite_value(kd) ||
        !finite_value(integral_limit) || integral_limit < 0.0 ||
        !finite_value(output_minimum) || !finite_value(output_maximum) ||
        output_minimum > output_maximum || !finite_value(*integral) ||
        !finite_value(*previous_error)) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    error = setpoint - measurement;
    candidate_integral = clamp_value(*integral + error * delta_seconds,
                                     -integral_limit, integral_limit);
    derivative = (error - *previous_error) / delta_seconds;
    output = kp * error + ki * candidate_integral + kd * derivative;
    if (!finite_value(output)) return PL_ROBOTICS_RANGE;
    {
        double bounded_output = clamp_value(output, output_minimum, output_maximum);
        if (!finite_value(candidate_integral) || !finite_value(bounded_output)) {
            return PL_ROBOTICS_RANGE;
        }
        *integral = candidate_integral;
        *previous_error = error;
        *out = bounded_output;
    }
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_axis_synchronize(const double *current, const double *target,
                                                  size_t axis_count, double maximum_step,
                                                  double *out) {
    size_t i;
    if (!current || !target || !out || axis_count == 0u ||
        axis_count > PL_ROBOTICS_MAX_AXES || !finite_value(maximum_step) ||
        maximum_step < 0.0) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    for (i = 0u; i < axis_count; ++i) {
        if (!finite_value(current[i]) || !finite_value(target[i])) return PL_ROBOTICS_RANGE;
    }
    {
        double next[PL_ROBOTICS_MAX_AXES];
        for (i = 0u; i < axis_count; ++i) {
            double delta = target[i] - current[i];
            if (!finite_value(delta)) return PL_ROBOTICS_RANGE;
            next[i] = current[i] + clamp_value(delta, -maximum_step, maximum_step);
            if (!finite_value(next[i])) return PL_ROBOTICS_RANGE;
        }
        memcpy(out, next, axis_count * sizeof(next[0]));
    }
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_safety_supervise(const double *positions, const double *minimum,
                                                  const double *maximum, size_t axis_count,
                                                  double temperature, double temperature_limit,
                                                  uint64_t now_tick, uint64_t heartbeat_tick,
                                                  uint64_t watchdog_limit,
                                                  pl_robotics_diagnostic *diagnostic) {
    size_t i;
    if (!positions || !minimum || !maximum || axis_count == 0u ||
        axis_count > PL_ROBOTICS_MAX_AXES || !finite_value(temperature) ||
        !finite_value(temperature_limit) || temperature_limit <= 0.0 ||
        watchdog_limit == 0u || heartbeat_tick > now_tick) {
        pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_AXIS_LIMIT, 2u, axis_count,
                                   "invalid safety supervisor input");
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    if (now_tick - heartbeat_tick > watchdog_limit) {
        pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_WATCHDOG, 3u,
                                   now_tick - heartbeat_tick, "robot watchdog expired");
        return PL_ROBOTICS_SAFETY_LIMIT;
    }
    if (temperature > temperature_limit) {
        pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_TEMPERATURE_LIMIT, 3u,
                                   saturating_u64(temperature), "temperature limit exceeded");
        return PL_ROBOTICS_SAFETY_LIMIT;
    }
    for (i = 0u; i < axis_count; ++i) {
        if (!finite_value(positions[i]) || !finite_value(minimum[i]) ||
            !finite_value(maximum[i]) || minimum[i] > maximum[i]) {
            pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_AXIS_LIMIT, 2u, i,
                                       "invalid axis limit descriptor");
            return PL_ROBOTICS_INVALID_ARGUMENT;
        }
        if (positions[i] < minimum[i] || positions[i] > maximum[i]) {
            pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_AXIS_LIMIT, 3u, i,
                                       "axis position outside configured limits");
            return PL_ROBOTICS_SAFETY_LIMIT;
        }
    }
    pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_NONE, 1u, axis_count,
                               "robot safety checks passed");
    return PL_ROBOTICS_OK;
}

static uint16_t crc16_ccitt(const uint8_t *data, size_t size) {
    uint16_t crc = UINT16_C(0xFFFF);
    size_t i;
    for (i = 0u; i < size; ++i) {
        unsigned bit;
        crc ^= (uint16_t)((uint16_t)data[i] << 8u);
        for (bit = 0u; bit < 8u; ++bit) {
            crc = (crc & UINT16_C(0x8000)) != 0u
                ? (uint16_t)((uint16_t)(crc << 1u) ^ UINT16_C(0x1021))
                : (uint16_t)(crc << 1u);
        }
    }
    return crc;
}

pl_robotics_status pl_robotics_packet_encode(uint16_t sequence, const uint8_t *payload,
                                               size_t payload_size, uint8_t *out,
                                               size_t out_capacity, size_t *out_size) {
    size_t total;
    uint16_t crc;
    if (!out || !out_size || (!payload && payload_size != 0u) ||
        payload_size > PL_ROBOTICS_PACKET_MAX_PAYLOAD) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    total = PL_ROBOTICS_PACKET_OVERHEAD + payload_size;
    if (out_capacity < total) return PL_ROBOTICS_CAPACITY;
    out[0] = UINT8_C(0x52);
    out[1] = UINT8_C(0x42);
    out[2] = UINT8_C(1);
    out[3] = (uint8_t)(sequence & UINT16_C(0x00FF));
    out[4] = (uint8_t)(sequence >> 8u);
    out[5] = (uint8_t)(payload_size & 0xFFu);
    out[6] = (uint8_t)(payload_size >> 8u);
    if (payload_size) memmove(out + 7u, payload, payload_size);
    crc = crc16_ccitt(out, 7u + payload_size);
    out[7u + payload_size] = (uint8_t)(crc & UINT16_C(0x00FF));
    out[8u + payload_size] = (uint8_t)(crc >> 8u);
    *out_size = total;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_packet_decode(const uint8_t *packet, size_t packet_size,
                                               uint16_t *out_sequence, uint8_t *out_payload,
                                               size_t payload_capacity, size_t *out_payload_size) {
    size_t payload_size;
    size_t expected_size;
    uint16_t expected_crc;
    uint16_t packet_crc;
    if (!packet || !out_sequence || !out_payload_size ||
        (!out_payload && payload_capacity != 0u) || packet_size < PL_ROBOTICS_PACKET_OVERHEAD) {
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    if (packet[0] != UINT8_C(0x52) || packet[1] != UINT8_C(0x42) || packet[2] != UINT8_C(1)) {
        return PL_ROBOTICS_PROTOCOL;
    }
    payload_size = (size_t)packet[5] | ((size_t)packet[6] << 8u);
    if (payload_size > PL_ROBOTICS_PACKET_MAX_PAYLOAD) return PL_ROBOTICS_PROTOCOL;
    expected_size = PL_ROBOTICS_PACKET_OVERHEAD + payload_size;
    if (packet_size != expected_size) return PL_ROBOTICS_PROTOCOL;
    if (payload_capacity < payload_size) return PL_ROBOTICS_CAPACITY;
    packet_crc = (uint16_t)((uint16_t)packet[7u + payload_size] |
                            ((uint16_t)packet[8u + payload_size] << 8u));
    expected_crc = crc16_ccitt(packet, 7u + payload_size);
    if (packet_crc != expected_crc) return PL_ROBOTICS_INTEGRITY;
    if (payload_size) memmove(out_payload, packet + 7u, payload_size);
    *out_sequence = (uint16_t)((uint16_t)packet[3] | ((uint16_t)packet[4] << 8u));
    *out_payload_size = payload_size;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_simulator_init(pl_robotics_simulator *simulator) {
    size_t i;
    if (!simulator) return PL_ROBOTICS_INVALID_ARGUMENT;
    memset(simulator, 0, sizeof(*simulator));
    pl_robotics_registers_init(&simulator->registers);
    if (pl_robotics_register_add(&simulator->registers, "ROBOT.TICK",
                                 PL_ROBOTICS_REGISTER_BASE, 0u, UINT32_MAX, 1, 1) != PL_ROBOTICS_OK ||
        pl_robotics_register_add(&simulator->registers, "ROBOT.STATUS",
                                 PL_ROBOTICS_REGISTER_BASE + 4u, 1u, UINT32_C(0x0000000F), 1, 1) != PL_ROBOTICS_OK) {
        return PL_ROBOTICS_CAPACITY;
    }
    for (i = 0u; i < PL_ROBOTICS_MAX_AXES; ++i) {
        char position_name[PL_EMBEDDED_REGISTER_NAME_MAX];
        char command_name[PL_EMBEDDED_REGISTER_NAME_MAX];
        uint32_t address = PL_ROBOTICS_REGISTER_BASE + 16u + (uint32_t)(i * 8u);
        (void)snprintf(position_name, sizeof(position_name), "AXIS%lu.POS",
                       (unsigned long)i);
        (void)snprintf(command_name, sizeof(command_name), "AXIS%lu.CMD",
                       (unsigned long)i);
        if (pl_robotics_register_add(&simulator->registers, position_name, address,
                                     0u, UINT32_MAX, 1, 1) != PL_ROBOTICS_OK ||
            pl_robotics_register_add(&simulator->registers, command_name, address + 4u,
                                     0u, UINT32_MAX, 1, 1) != PL_ROBOTICS_OK) {
            return PL_ROBOTICS_CAPACITY;
        }
    }
    pl_robotics_timer_init(&simulator->clock, 0u);
    simulator->max_velocity = 360.0;
    simulator->initialized = 1;
    return PL_ROBOTICS_OK;
}

pl_robotics_status pl_robotics_simulator_step(pl_robotics_simulator *simulator,
                                                uint32_t delta_ms,
                                                pl_robotics_diagnostic *diagnostic) {
    size_t axis;
    double delta_seconds;
    pl_robotics_state next_state;
    pl_robotics_registers next_registers;
    pl_robotics_timer next_clock;
    uint32_t position_registers[PL_ROBOTICS_MAX_AXES];
    uint32_t command_registers[PL_ROBOTICS_MAX_AXES];
    char position_names[PL_ROBOTICS_MAX_AXES][PL_EMBEDDED_REGISTER_NAME_MAX];
    char command_names[PL_ROBOTICS_MAX_AXES][PL_EMBEDDED_REGISTER_NAME_MAX];
    uint64_t next_step;

    if (!simulator || !simulator->initialized || delta_ms == 0u || delta_ms > 1000u ||
        !state_valid(&simulator->state) || !finite_value(simulator->max_velocity) ||
        simulator->max_velocity <= 0.0) {
        pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_SIMULATOR, 2u, delta_ms,
                                   "simulator requires a valid state and a 1..1000 ms step");
        return PL_ROBOTICS_INVALID_ARGUMENT;
    }
    if (simulator->step_count >= UINT32_MAX || simulator->step_count == UINT64_MAX) {
        return PL_ROBOTICS_TIME_OVERFLOW;
    }
    next_clock = simulator->clock;
    next_registers = simulator->registers;
    if (pl_robotics_timer_advance(&next_clock, delta_ms) != PL_ROBOTICS_OK) {
        return PL_ROBOTICS_TIME_OVERFLOW;
    }
    next_step = simulator->step_count + 1u;
    next_state = simulator->state;
    delta_seconds = (double)delta_ms / 1000.0;

    for (axis = 0u; axis < PL_ROBOTICS_MAX_AXES; ++axis) {
        double command = clamp_value(next_state.command[axis], -10000.0, 10000.0);
        double acceleration = (command - next_state.position[axis]) * 6.0;
        double velocity = next_state.velocity[axis] + acceleration * delta_seconds;
        double position;
        if (!finite_value(velocity)) return PL_ROBOTICS_RANGE;
        velocity = clamp_value(velocity, -simulator->max_velocity, simulator->max_velocity);
        position = clamp_value(next_state.position[axis] + velocity * delta_seconds,
                               -10000.0, 10000.0);
        if (!finite_value(position)) return PL_ROBOTICS_RANGE;
        next_state.command[axis] = command;
        next_state.velocity[axis] = velocity;
        next_state.position[axis] = position;
        (void)snprintf(position_names[axis], sizeof(position_names[axis]),
                       "AXIS%lu.POS", (unsigned long)axis);
        (void)snprintf(command_names[axis], sizeof(command_names[axis]),
                       "AXIS%lu.CMD", (unsigned long)axis);
        position_registers[axis] =
            (uint32_t)(position * 1000.0 + PL_ROBOTICS_SIM_POSITION_BIAS);
        command_registers[axis] =
            (uint32_t)(command * 1000.0 + PL_ROBOTICS_SIM_POSITION_BIAS);
    }

    for (axis = 0u; axis < PL_ROBOTICS_MAX_AXES; ++axis) {
        if (pl_robotics_register_write(&next_registers, position_names[axis],
                                       position_registers[axis]) != PL_ROBOTICS_OK ||
            pl_robotics_register_write(&next_registers, command_names[axis],
                                       command_registers[axis]) != PL_ROBOTICS_OK) {
            pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_SIMULATOR, 3u, axis,
                                       "simulator register update failed");
            return PL_ROBOTICS_INTEGRITY;
        }
    }
    if (pl_robotics_register_write(&next_registers, "ROBOT.TICK",
                                   (uint32_t)next_step) != PL_ROBOTICS_OK) {
        pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_SIMULATOR, 3u, next_step,
                                   "simulator tick register update failed");
        return PL_ROBOTICS_INTEGRITY;
    }

    simulator->state = next_state;
    simulator->registers = next_registers;
    simulator->clock = next_clock;
    simulator->step_count = next_step;
    pl_robotics_diagnostic_set(diagnostic, PL_ROBOTICS_DIAG_NONE, 1u,
                               simulator->step_count, "simulator step completed");
    return PL_ROBOTICS_OK;
}
