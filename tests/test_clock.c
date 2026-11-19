#include "clock.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    const double before = pl_monotonic_now_ms();
    volatile unsigned long work = 0ul;
    for (unsigned long i = 0ul; i < 100000ul; ++i) work += i;
    const double after = pl_monotonic_now_ms();

    (void)work;
    assert(isfinite(before) && before >= 0.0);
    assert(isfinite(after) && after >= before);
    puts("Monotonic clock contract passed.");
    return 0;
}
