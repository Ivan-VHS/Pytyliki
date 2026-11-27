#include "division.h"

#include <limits.h>

static uint64_t magnitude_i64(int64_t value) {
    return value < 0 ? (uint64_t)(-(value + 1)) + UINT64_C(1)
                     : (uint64_t)value;
}

static int64_t signed_magnitude(uint64_t magnitude, int negative) {
    if (!negative) return (int64_t)magnitude;
    if (magnitude == (uint64_t)INT64_MAX + UINT64_C(1)) return INT64_MIN;
    return -(int64_t)magnitude;
}

int pl_euclidean_divmod(int64_t dividend, int64_t divisor,
                        pl_division_result *out) {
    int64_t quotient;
    int64_t remainder;
    if (out == NULL || divisor == 0) return -1;
    if (dividend == INT64_MIN && divisor == -1) return -2;

    quotient = dividend / divisor;
    remainder = dividend % divisor;
    if (remainder < 0) {
        if (divisor > 0) {
            --quotient;
            remainder += divisor;
        } else {
            uint64_t divisor_magnitude = magnitude_i64(divisor);
            uint64_t remainder_magnitude = magnitude_i64(remainder);
            uint64_t adjusted = divisor_magnitude - remainder_magnitude;
            ++quotient;
            remainder = (int64_t)adjusted;
        }
    }
    out->dividend = dividend;
    out->divisor = divisor;
    out->quotient = quotient;
    out->remainder = remainder;
    return 0;
}

int pl_division_split_evenly(int64_t total, size_t parts,
                             int64_t *out, size_t capacity) {
    uint64_t magnitude;
    uint64_t base;
    uint64_t remainder;
    size_t i;
    int negative;
    if (out == NULL || parts == 0u) return -1;
    if (parts > capacity) return -2;
    magnitude = magnitude_i64(total);
    base = magnitude / (uint64_t)parts;
    remainder = magnitude % (uint64_t)parts;
    negative = total < 0;
    for (i = 0u; i < parts; ++i) {
        uint64_t value = base + ((uint64_t)i < remainder ? UINT64_C(1) : UINT64_C(0));
        out[i] = signed_magnitude(value, negative);
    }
    return 0;
}
