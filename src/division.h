#ifndef PYTYLIKI_DIVISION_H
#define PYTYLIKI_DIVISION_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int64_t dividend;
    int64_t divisor;
    int64_t quotient;
    int64_t remainder;
} pl_division_result;

/* Euclidean division: dividend = quotient*divisor + remainder,
 * with 0 <= remainder < abs(divisor). Returns 0 on success,
 * -1 for invalid pointers/zero divisor, -2 for unrepresentable quotient. */
int pl_euclidean_divmod(int64_t dividend, int64_t divisor,
                        pl_division_result *out);

/* Deterministically split a signed total into parts buckets. Earlier buckets
 * receive indivisible units. Returns 0 on success, -1 for invalid arguments,
 * -2 when the output capacity is insufficient. */
int pl_division_split_evenly(int64_t total, size_t parts,
                             int64_t *out, size_t capacity);

#endif
