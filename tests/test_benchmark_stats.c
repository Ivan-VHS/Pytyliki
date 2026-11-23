#include "benchmark_stats.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <float.h>
#include <math.h>

int main(void) {
    const double samples[] = {1.0, 2.0, 3.0, 4.0, 100.0};
    const double reordered[] = {100.0, 4.0, 2.0, 1.0, 3.0};
    const double steady[] = {5.0, 5.0, 5.0, 5.0};
    const double zeros[] = {0.0, 0.0};
    const double negative[] = {1.0, -0.1};
    const double invalid[] = {1.0, NAN};
    const double extreme[] = {DBL_MAX, 0.0};
    pl_benchmark_stats first;
    pl_benchmark_stats second;

    assert(pl_benchmark_stats_analyze(samples, 5u, &first) == 0);
    assert(first.sample_count == 5u);
    assert(first.minimum == 1.0 && first.maximum == 100.0);
    assert(first.median == 3.0);
    assert(first.p90 == 100.0 && first.p95 == 100.0 && first.p99 == 100.0);
    assert(first.median_absolute_deviation == 1.0);
    assert(first.sample_standard_deviation > 0.0);

    assert(pl_benchmark_stats_analyze(reordered, 5u, &second) == 0);
    assert(fabs(first.mean - second.mean) < 1e-12);
    assert(first.median == second.median);
    assert(first.p95 == second.p95);

    assert(pl_benchmark_stats_analyze(steady, 4u, &first) == 0);
    assert(first.median == 5.0);
    assert(first.sample_standard_deviation == 0.0);
    assert(first.coefficient_of_variation == 0.0);
    assert(first.median_absolute_deviation == 0.0);

    assert(pl_benchmark_stats_analyze(zeros, 2u, &first) == 0);
    assert(first.coefficient_of_variation == 0.0);
    assert(pl_benchmark_stats_analyze(NULL, 1u, &first) != 0);
    assert(pl_benchmark_stats_analyze(samples, 0u, &first) != 0);
    assert(pl_benchmark_stats_analyze(samples, 5u, NULL) != 0);
    assert(pl_benchmark_stats_analyze(negative, 2u, &first) != 0);
    assert(pl_benchmark_stats_analyze(invalid, 2u, &first) != 0);

    /* Finite inputs near DBL_MAX must not overflow the variance path. */
    assert(pl_benchmark_stats_analyze(extreme, 2u, &first) == 0);
    assert(isfinite(first.mean) && first.mean == DBL_MAX / 2.0);
    assert(isfinite(first.sample_standard_deviation));
    assert(isfinite(first.coefficient_of_variation));
    assert(isfinite(first.median_absolute_deviation));

    /* Failed analysis must leave the caller's previous result untouched. */
    first.mean = 123.0;
    assert(pl_benchmark_stats_analyze(negative, 2u, &first) != 0);
    assert(first.mean == 123.0);
    assert(pl_benchmark_stats_analyze(samples, PL_BENCHMARK_MAX_SAMPLES + 1u, &first) != 0);
    return 0;
}
