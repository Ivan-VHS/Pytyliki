#define _POSIX_C_SOURCE 200809L
#include "clock.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

double pl_monotonic_now_ms(void) {
#if defined(_WIN32)
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    if (QueryPerformanceFrequency(&frequency) &&
        QueryPerformanceCounter(&counter) && frequency.QuadPart > 0) {
        return 1000.0 * (double)counter.QuadPart / (double)frequency.QuadPart;
    }
    return (double)GetTickCount64();
#else
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) == 0) {
        return 1000.0 * (double)value.tv_sec +
               (double)value.tv_nsec / 1000000.0;
    }
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
#endif
}
