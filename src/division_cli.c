#include "division_cli.h"

#include "core.h"
#include "division.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PL_DIVISION_CLI_MAX_WORKERS 64u

static int parse_i64(const char *text, int64_t *out) {
    char *end = NULL;
    long long value;
    if (text == NULL || text[0] == '\0' || out == NULL ||
        text[0] == '+' || text[0] == ' ') return -1;
    errno = 0;
    value = strtoll(text, &end, 10);
    if (errno == ERANGE || end == text || end == NULL || *end != '\0' ||
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
    if (pl_euclidean_divmod(dividend, divisor, &result) != 0) {
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
    int64_t counts[PL_DIVISION_CLI_MAX_WORKERS];
    uint64_t offset = 0u;
    uint64_t workers;
    if (minimum_grain == 0u || available_workers == 0u) {
        fputs("Partition request rejected: workers and minimum grain must be positive.\n", stderr);
        return 2;
    }
    if (items == 0u) {
        puts("No work items: no partitions required.");
        return 0;
    }
    if (items > (uint64_t)INT64_MAX) {
        fputs("Partition request exceeds the signed 64-bit balanced-split limit.\n", stderr);
        return 2;
    }

    workers = items / minimum_grain + (items % minimum_grain != 0u ? 1u : 0u);
    if (workers == 0u) workers = 1u;
    if (workers > available_workers) workers = available_workers;
    if (workers > PL_DIVISION_CLI_MAX_WORKERS) workers = PL_DIVISION_CLI_MAX_WORKERS;
    if (pl_division_split_evenly((int64_t)items, (size_t)workers,
                                 counts, PL_DIVISION_CLI_MAX_WORKERS) != 0) {
        fputs("Partition plan could not be created.\n", stderr);
        return 2;
    }

    printf("Balanced partition plan: items=%" PRIu64
           " available-workers=%" PRIu64 " minimum-grain=%" PRIu64
           " recommended-workers=%" PRIu64 "\n",
           items, available_workers, minimum_grain, workers);
    for (uint64_t i = 0u; i < workers; ++i) {
        const uint64_t count = (uint64_t)counts[i];
        printf("  worker-%" PRIu64 ": offset=%" PRIu64 " count=%" PRIu64 "\n",
               i, offset, count);
        if (UINT64_MAX - offset < count) {
            fputs("Partition offset overflow.\n", stderr);
            return 2;
        }
        offset += count;
    }
    if (offset != items) {
        fputs("Partition plan failed its total-work invariant.\n", stderr);
        return 1;
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
            fputs("Usage: pytyliki --partition-auto ITEMS [MIN_GRAIN]\n", stderr);
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
