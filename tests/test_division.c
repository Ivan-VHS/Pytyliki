#include "division.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <stdio.h>

static void test_euclidean_cases(void) {
    const int64_t inputs[][4] = {
        {17, 5, 3, 2}, {-17, 5, -4, 3},
        {17, -5, -3, 2}, {-17, -5, 4, 3},
        {-1, INT64_MIN, 1, INT64_MAX},
        {INT64_MIN, 1, INT64_MIN, 0}
    };
    size_t i;
    for (i = 0u; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        pl_division_result result;
        assert(pl_euclidean_divmod(inputs[i][0], inputs[i][1], &result) == 0);
        assert(result.quotient == inputs[i][2]);
        assert(result.remainder == inputs[i][3]);
        assert(result.remainder >= 0);
        assert((uint64_t)result.remainder < (inputs[i][1] < 0
            ? (uint64_t)(-(inputs[i][1] + 1)) + 1u : (uint64_t)inputs[i][1]));
    }
}

static void test_errors(void) {
    pl_division_result result;
    assert(pl_euclidean_divmod(1, 0, &result) == -1);
    assert(pl_euclidean_divmod(1, 2, NULL) == -1);
    assert(pl_euclidean_divmod(INT64_MIN, -1, &result) == -2);
}

static void test_distribution(void) {
    int64_t values[4] = {0, 0, 0, 0};
    assert(pl_division_split_evenly(11, 4u, values, 4u) == 0);
    assert(values[0] == 3 && values[1] == 3 && values[2] == 3 && values[3] == 2);
    assert(pl_division_split_evenly(-11, 4u, values, 4u) == 0);
    assert(values[0] == -3 && values[1] == -3 && values[2] == -3 && values[3] == -2);
    assert(pl_division_split_evenly(INT64_MIN, 1u, values, 4u) == 0);
    assert(values[0] == INT64_MIN);
    assert(pl_division_split_evenly(1, 0u, values, 4u) == -1);
    assert(pl_division_split_evenly(1, 4u, values, 3u) == -2);
    assert(pl_division_split_evenly(1, 1u, NULL, 1u) == -1);
}

int main(void) {
    test_euclidean_cases();
    test_errors();
    test_distribution();
    puts("All division and decomposition tests passed.");
    return 0;
}
