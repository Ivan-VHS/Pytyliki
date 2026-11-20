#ifndef PYTYLIKI_COMPUTE_OPS_H
#define PYTYLIKI_COMPUTE_OPS_H

#include <stddef.h>
#include <stdint.h>

#define PL_COMPUTE_MAX_ITEMS 1000000u
#define PL_COMPUTE_MAX_FACTOR_INPUT UINT64_C(1000000000000)

typedef enum {
    PL_COMPUTE_ADD = 0,
    PL_COMPUTE_SUBTRACT = 1,
    PL_COMPUTE_MULTIPLY = 2,
    PL_COMPUTE_DIVIDE = 3,
    PL_COMPUTE_POWER = 4
} pl_compute_binary_op;

typedef int (*pl_compute_step_fn)(double current, size_t iteration,
                                  void *context, double *next);

typedef struct {
    size_t count;
    double sum;
    double product;
    double mean;
    double minimum;
    double maximum;
} pl_compute_summary;

/* All functions return 0 on success and -1 for invalid input, capacity or overflow. */
int pl_compute_multiply(double left, double right, double *out);
int pl_compute_calculate(double left, double right, pl_compute_binary_op operation, double *out);
int pl_compute_iterate(double seed, size_t steps, pl_compute_step_fn step,
                       void *context, double *out, size_t capacity);
int pl_compute_expand(const double *values, size_t count, size_t repetitions,
                      double *out, size_t capacity, size_t *out_count);
int pl_compute_compound(const double *left, const double *right,
                        size_t count, double *out);
int pl_compute_exponentiate(double base, double exponent, double *out);
int pl_compute_scale(const double *values, size_t count, double factor, double *out);
int pl_compute_accumulate(const double *values, size_t count, double *out);
int pl_compute_combine(const double *left, const double *right, size_t count, double *out);
int pl_compute_outer_product(const double *left, size_t left_count,
                             const double *right, size_t right_count,
                             double *out, size_t capacity);
int pl_compute_cartesianize(const double *left, size_t left_count,
                            const double *right, size_t right_count,
                            double *left_out, double *right_out,
                            size_t capacity, size_t *pair_count);
int pl_compute_convolve(const double *signal, size_t signal_count,
                        const double *kernel, size_t kernel_count,
                        double *out, size_t capacity, size_t *out_count);
int pl_compute_summarize(const double *values, size_t count, pl_compute_summary *out);
/* Prime factors are returned with multiplicity, in ascending order. Input is bounded. */
int pl_compute_factorize(uint64_t value, uint64_t *factors,
                         size_t capacity, size_t *factor_count);

#endif
