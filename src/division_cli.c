#include "division.h"

#include "processor.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_i64(const char *text, int64_t *out) {
    char *end = NULL;
    intmax_t value;
    if (text == NULL || text[0] == '\0' || out == NULL) return -1;
    errno = 0;
    value = strtoimax(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE ||
        value < INT64_MIN || value > INT64_MAX) return -1;
    *out = (int64_t)value;
    return 0;
}

static int parse_u64(const char *text, uint64_t *out) {
    const unsigned char *cursor = (const unsigned char *)text;
    uint64_t value = 0u;
    if (cursor == NULL || out == NULL || *cursor == '\0' || *cursor == '-') {
        return -1;
    }
    if (*cursor == '+') ++cursor;
    if (*cursor == '\0') return -1;
    for (; *cursor != '\0'; ++cursor) {
        uint64_t digit;
        if (*cursor < (unsigned char)'0' || *cursor > (unsigned char)'9')
            return -1;
        digit = (uint64_t)(*cursor - (unsigned char)'0');
        if (value > (UINT64_MAX - digit) / 10u) return -1;
        value = value * 10u + digit;
    }
    *out = value;
    return 0;
}

static int print_division(int64_t dividend, int64_t divisor) {
    pl_division_result result;
    if (pl_division_euclidean(dividend, divisor, &result) != 0) {
        fputs("Division rejected: zero divisor or signed 64-bit overflow.\n", stderr);
        return 2;
    }
    printf("Euclidean division: quotient=%" PRId64 " remainder=%" PRId64
           " exact=%s\n",
           result.quotient, result.remainder,
           result.remainder == 0 ? "yes" : "no");
    return 0;
}

static int print_partition(uint64_t items, uint64_t available_workers,
                           uint64_t minimum_grain) {
    uint64_t workers = 0u;
    pl_partition_plan plan;
    if (pl_division_recommend_workers(items, available_workers,
                                      minimum_grain, &workers) != 0) {
        fputs("Partition request rejected: invalid worker count or minimum grain.\n",
              stderr);
        return 2;
    }
    if (workers == 0u) {
        puts("No work items: no partitions required.");
        return 0;
    }
    if (pl_partition_plan_create(items, workers, &plan) != 0) {
        fputs("Partition plan exceeds the configured safety limit.\n", stderr);
        return 2;
    }
    printf("Balanced partition plan: items=%" PRIu64
           " available-workers=%" PRIu64 " minimum-grain=%" PRIu64
           " recommended-workers=%" PRIu64 "\n",
           items, available_workers, minimum_grain, workers);
    for (uint64_t i = 0u; i < workers; ++i) {
        uint64_t offset = 0u;
        uint64_t count = 0u;
        if (pl_partition_plan_range(&plan, i, &offset, &count) != 0) {
            fputs("Partition plan could not be read.\n", stderr);
            return 1;
        }
        printf("  worker-%" PRIu64 ": offset=%" PRIu64 " count=%" PRIu64 "\n",
               i, offset, count);
    }
    puts("Planning only: no threads are spawned and CPU affinity is unchanged.");
    return 0;
}

/* Returns -1 for commands owned by the main CLI; otherwise returns an exit code. */
int pl_division_cli(int argc, char **argv) {
    const char *command;
    if (argc < 2 || argv == NULL || argv[1] == NULL) return -1;
    command = argv[1];

    if (strcmp(command, "--divide") == 0) {
        int64_t dividend, divisor;
        if (argc != 4 || parse_i64(argv[2], &dividend) != 0 ||
            parse_i64(argv[3], &divisor) != 0) {
            fputs("Usage: pytyliki --divide INTEGER INTEGER\n", stderr);
            return 2;
        }
        return print_division(dividend, divisor);
    }

    if (strcmp(command, "--partition") == 0) {
        uint64_t items, workers, grain = 1u;
        if ((argc != 4 && argc != 5) ||
            parse_u64(argv[2], &items) != 0 ||
            parse_u64(argv[3], &workers) != 0 ||
            (argc == 5 && parse_u64(argv[4], &grain) != 0)) {
            fputs("Usage: pytyliki --partition ITEMS WORKERS [MIN_GRAIN]\n",
                  stderr);
            return 2;
        }
        return print_partition(items, workers, grain);
    }

    if (strcmp(command, "--partition-auto") == 0) {
        uint64_t items, grain = 256u;
        pl_cpu_info cpu;
        if ((argc != 3 && argc != 4) ||
            parse_u64(argv[2], &items) != 0 ||
            (argc == 4 && parse_u64(argv[3], &grain) != 0)) {
            fputs("Usage: pytyliki --partition-auto ITEMS [MIN_GRAIN]\n",
                  stderr);
            return 2;
        }
        if (pl_cpu_profile(&cpu) != 0 || cpu.logical_cpus == 0u) {
            fputs("Could not determine available logical processors.\n", stderr);
            return 1;
        }
        return print_partition(items, cpu.logical_cpus, grain);
    }

    return -1;
}
