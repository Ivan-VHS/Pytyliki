#include "compute_ops.h"

#include <float.h>
#include <math.h>
#include <stdint.h>

static int valid_values(const double *values, size_t count) {
    if (values == NULL || count == 0u || count > PL_COMPUTE_MAX_ITEMS) return 0;
    for (size_t i = 0u; i < count; ++i) {
        if (!isfinite(values[i])) return 0;
    }
    return 1;
}

static int store_finite(long double value, double *out) {
    if (out == NULL || !isfinite(value) || fabsl(value) > (long double)DBL_MAX) return -1;
    *out = (double)value;
    return isfinite(*out) ? 0 : -1;
}

int pl_compute_multiply(double left, double right, double *out) {
    if (!isfinite(left) || !isfinite(right) || out == NULL) return -1;
    return store_finite((long double)left * (long double)right, out);
}

int pl_compute_exponentiate(double base, double exponent, double *out) {
    if (!isfinite(base) || !isfinite(exponent) || out == NULL) return -1;
    if (base == 0.0 && exponent < 0.0) return -1;
    return store_finite(powl((long double)base, (long double)exponent), out);
}

int pl_compute_scale(const double *values, size_t count, double factor, double *out) {
    if (!valid_values(values, count) || !isfinite(factor) || out == NULL) return -1;
    for (size_t i = 0u; i < count; ++i) {
        if (store_finite((long double)values[i] * (long double)factor, &out[i]) != 0) return -1;
    }
    return 0;
}

int pl_compute_accumulate(const double *values, size_t count, double *out) {
    if (!valid_values(values, count) || out == NULL) return -1;
    long double total = 0.0L;
    for (size_t i = 0u; i < count; ++i) {
        total += (long double)values[i];
        if (store_finite(total, &out[i]) != 0) return -1;
    }
    return 0;
}

int pl_compute_combine(const double *left, const double *right, size_t count, double *out) {
    if (!valid_values(left, count) || !valid_values(right, count) || out == NULL) return -1;
    for (size_t i = 0u; i < count; ++i) {
        if (store_finite((long double)left[i] + (long double)right[i], &out[i]) != 0) return -1;
    }
    return 0;
}

int pl_compute_outer_product(const double *left, size_t left_count,
                             const double *right, size_t right_count,
                             double *out, size_t capacity) {
    if (!valid_values(left, left_count) || !valid_values(right, right_count) ||
        out == NULL || left_count > SIZE_MAX / right_count) return -1;
    size_t required = left_count * right_count;
    if (required > capacity || required > PL_COMPUTE_MAX_ITEMS) return -1;
    for (size_t i = 0u; i < left_count; ++i) {
        for (size_t j = 0u; j < right_count; ++j) {
            if (store_finite((long double)left[i] * (long double)right[j],
                             &out[i * right_count + j]) != 0) return -1;
        }
    }
    return 0;
}

int pl_compute_cartesianize(const double *left, size_t left_count,
                            const double *right, size_t right_count,
                            double *left_out, double *right_out,
                            size_t capacity, size_t *pair_count) {
    if (!valid_values(left, left_count) || !valid_values(right, right_count) ||
        left_out == NULL || right_out == NULL || pair_count == NULL ||
        left_count > SIZE_MAX / right_count) return -1;
    size_t required = left_count * right_count;
    if (required > capacity || required > PL_COMPUTE_MAX_ITEMS) return -1;
    size_t index = 0u;
    for (size_t i = 0u; i < left_count; ++i) {
        for (size_t j = 0u; j < right_count; ++j) {
            left_out[index] = left[i];
            right_out[index] = right[j];
            ++index;
        }
    }
    *pair_count = required;
    return 0;
}

int pl_compute_convolve(const double *signal, size_t signal_count,
                        const double *kernel, size_t kernel_count,
                        double *out, size_t capacity, size_t *out_count) {
    if (!valid_values(signal, signal_count) || !valid_values(kernel, kernel_count) ||
        out == NULL || out_count == NULL ||
        signal_count > SIZE_MAX - kernel_count + 1u) return -1;
    size_t required = signal_count + kernel_count - 1u;
    if (required > capacity || required > PL_COMPUTE_MAX_ITEMS) return -1;
    for (size_t index = 0u; index < required; ++index) {
        long double total = 0.0L;
        size_t first = index >= kernel_count - 1u ? index - (kernel_count - 1u) : 0u;
        size_t last = index < signal_count - 1u ? index : signal_count - 1u;
        for (size_t i = first; i <= last; ++i) {
            size_t j = index - i;
            total += (long double)signal[i] * (long double)kernel[j];
        }
        if (store_finite(total, &out[index]) != 0) return -1;
    }
    *out_count = required;
    return 0;
}

int pl_compute_summarize(const double *values, size_t count, pl_compute_summary *out) {
    if (!valid_values(values, count) || out == NULL) return -1;
    long double sum = 0.0L, product = 1.0L;
    double minimum = values[0], maximum = values[0];
    for (size_t i = 0u; i < count; ++i) {
        sum += (long double)values[i];
        product *= (long double)values[i];
        if (values[i] < minimum) minimum = values[i];
        if (values[i] > maximum) maximum = values[i];
    }
    if (!isfinite(sum) || !isfinite(product) ||
        fabsl(sum) > (long double)DBL_MAX || fabsl(product) > (long double)DBL_MAX) return -1;
    out->count = count;
    out->sum = (double)sum;
    out->product = (double)product;
    out->mean = (double)(sum / (long double)count);
    out->minimum = minimum;
    out->maximum = maximum;
    return 0;
}

int pl_compute_factorize(uint64_t value, uint64_t *factors,
                         size_t capacity, size_t *factor_count) {
    if (value == 0u || value > PL_COMPUTE_MAX_FACTOR_INPUT ||
        factors == NULL || factor_count == NULL) return -1;
    uint64_t remaining = value;
    uint64_t local[40];
    size_t count = 0u;
    for (uint64_t divisor = 2u; divisor <= remaining / divisor; ++divisor) {
        while (remaining % divisor == 0u) {
            if (count >= sizeof(local) / sizeof(local[0])) return -1;
            local[count++] = divisor;
            remaining /= divisor;
        }
    }
    if (remaining > 1u) {
        if (count >= sizeof(local) / sizeof(local[0])) return -1;
        local[count++] = remaining;
    }
    *factor_count = count;
    if (count > capacity) return -1;
    for (size_t i = 0u; i < count; ++i) factors[i] = local[i];
    return 0;
}
