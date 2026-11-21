#include "compute_ops.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>

static int incremental_step(double current, size_t iteration, void *context, double *next) {
    const double increment = context != 0 ? *(const double *)context : 1.0;
    *next = current + increment * (double)(iteration + 1u);
    return 0;
}

int main(void) {
    double scalar = 0.0;
    assert(pl_compute_multiply(6.0, 7.0, &scalar) == 0 && scalar == 42.0);
    assert(pl_compute_multiply(DBL_MAX, 2.0, &scalar) == -1);
    assert(pl_compute_calculate(10.0, 5.0, PL_COMPUTE_ADD, &scalar) == 0 && scalar == 15.0);
    assert(pl_compute_calculate(10.0, 5.0, PL_COMPUTE_DIVIDE, &scalar) == 0 && scalar == 2.0);
    assert(pl_compute_calculate(10.0, 0.0, PL_COMPUTE_DIVIDE, &scalar) == -1);
    assert(pl_compute_exponentiate(2.0, 10.0, &scalar) == 0 && scalar == 1024.0);
    assert(pl_compute_exponentiate(0.0, -1.0, &scalar) == -1);

    const double values[] = {1.0, 2.0, 3.0};
    double output[16] = {0.0};
    assert(pl_compute_scale(values, 3u, 2.0, output) == 0);
    assert(output[0] == 2.0 && output[1] == 4.0 && output[2] == 6.0);
    assert(pl_compute_accumulate(values, 3u, output) == 0);
    assert(output[0] == 1.0 && output[1] == 3.0 && output[2] == 6.0);
    assert(pl_compute_combine(values, values, 3u, output) == 0 && output[2] == 6.0);
    assert(pl_compute_compound(values, values, 3u, output) == 0);
    assert(output[0] == 1.0 && output[1] == 4.0 && output[2] == 9.0);
    size_t expanded_count = 0u;
    assert(pl_compute_expand(values, 3u, 2u, output, 16u, &expanded_count) == 0);
    assert(expanded_count == 6u && output[0] == 1.0 && output[3] == 1.0);
    const double increment = 1.0;
    assert(pl_compute_iterate(2.0, 3u, incremental_step, (void *)&increment, output, 16u) == 0);
    assert(output[0] == 2.0 && output[1] == 3.0 && output[2] == 5.0 && output[3] == 8.0);

    const double right[] = {4.0, 5.0};
    assert(pl_compute_outer_product(values, 3u, right, 2u, output, 16u) == 0);
    assert(output[0] == 4.0 && output[1] == 5.0 && output[4] == 12.0);

    double left_pairs[8] = {0.0}, right_pairs[8] = {0.0};
    size_t pair_count = 0u;
    assert(pl_compute_cartesianize(values, 3u, right, 2u,
                                   left_pairs, right_pairs, 8u, &pair_count) == 0);
    assert(pair_count == 6u && left_pairs[5] == 3.0 && right_pairs[5] == 5.0);

    const double signal[] = {1.0, 2.0, 3.0};
    const double kernel[] = {1.0, 1.0};
    size_t output_count = 0u;
    assert(pl_compute_convolve(signal, 3u, kernel, 2u,
                               output, 16u, &output_count) == 0);
    assert(output_count == 4u);
    assert(output[0] == 1.0 && output[1] == 3.0 &&
           output[2] == 5.0 && output[3] == 3.0);

    pl_compute_summary summary;
    assert(pl_compute_summarize(values, 3u, &summary) == 0);
    assert(summary.count == 3u && summary.sum == 6.0 &&
           summary.product == 6.0 && summary.mean == 2.0 &&
           summary.minimum == 1.0 && summary.maximum == 3.0);

    uint64_t factors[40] = {0u};
    size_t factor_count = 0u;
    assert(pl_compute_factorize(360u, factors, 40u, &factor_count) == 0);
    assert(factor_count == 6u);
    assert(factors[0] == 2u && factors[1] == 2u &&
           factors[2] == 2u && factors[3] == 3u &&
           factors[4] == 3u && factors[5] == 5u);
    assert(pl_compute_factorize(360u, factors, 2u, &factor_count) == -1);
    assert(pl_compute_factorize(0u, factors, 40u, &factor_count) == -1);
    pl_compute_statistics_result statistics;
    assert(pl_compute_statistics(values, 3u, &statistics) == 0);
    assert(statistics.count == 3u && fabs(statistics.mean - 2.0) < 1e-12);
    assert(fabs(statistics.population_variance - (2.0 / 3.0)) < 1e-12);
    assert(fabs(statistics.sample_variance - 1.0) < 1e-12);
    assert(fabs(statistics.standard_deviation - sqrt(2.0 / 3.0)) < 1e-12);
    assert(statistics.minimum == 1.0 && statistics.maximum == 3.0);
    assert(statistics.sample_variance_defined == 1u);

    const double singleton[] = {9.0};
    assert(pl_compute_statistics(singleton, 1u, &statistics) == 0);
    assert(statistics.mean == 9.0 && statistics.population_variance == 0.0);
    assert(statistics.sample_variance == 0.0 && statistics.sample_variance_defined == 0u);

    const double invalid_statistics[] = {1.0, NAN};
    statistics.mean = 123.0;
    assert(pl_compute_statistics(invalid_statistics, 2u, &statistics) == -1);
    assert(statistics.mean == 123.0);
    const double overflowing_statistics[] = {-DBL_MAX, DBL_MAX};
    assert(pl_compute_statistics(overflowing_statistics, 2u, &statistics) == -1);
    return 0;
}
