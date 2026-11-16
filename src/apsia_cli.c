#include "apsia.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_number(const char *text, double *value) {
    char *end = NULL;
    double parsed;
    errno = 0;
    parsed = strtod(text, &end);
    if (text == NULL || text[0] == '\0' || end == text || *end != '\0' ||
        errno == ERANGE || !isfinite(parsed)) return -1;
    *value = parsed;
    return 0;
}

static int parse_workload(const char *text, pl_apsia_workload *workload) {
    static const char *const names[PL_APSIA_WORK_COUNT] = {
        "cpu", "gpu", "analytics", "plugin", "network-io", "script"
    };
    unsigned i;
    for (i = 0u; i < PL_APSIA_WORK_COUNT; ++i) {
        if (strcmp(text, names[i]) == 0) {
            *workload = (pl_apsia_workload)i;
            return 0;
        }
    }
    return -1;
}

static int add_target(const char *name, uint32_t *mask) {
    static const char *const names[PL_APSIA_LANG_COUNT] = {
        "host", "rust", "cpp", "zig", "cuda", "wasm", "sql", "python", "go"
    };
    unsigned i;
    for (i = 1u; i < PL_APSIA_LANG_COUNT; ++i) {
        if (strcmp(name, names[i]) == 0) {
            *mask |= PL_APSIA_TARGET_BIT(i);
            return 0;
        }
    }
    return -1;
}

static int parse_targets(const char *text, uint32_t *mask) {
    char copy[256];
    char *token;
    size_t length = strlen(text);
    if (length == 0u || length >= sizeof(copy)) return -1;
    memcpy(copy, text, length + 1u);
    *mask = 0u;
    token = strtok(copy, ",");
    while (token != NULL) {
        if (add_target(token, mask) != 0) return -1;
        token = strtok(NULL, ",");
    }
    return *mask != 0u ? 0 : -1;
}

static void usage(void) {
    puts("Pytyliki APSIA route planner\n"
         "Usage: pytyliki-apsia WORKLOAD HOST_NS TARGET_NS FFI_NS FFI_CALLS_PER_SEC TARGETS [--sandbox]\n"
         "WORKLOAD: cpu | gpu | analytics | plugin | network-io | script\n"
         "TARGETS: comma-separated rust,cpp,zig,cuda,wasm,sql,python,go\n"
         "Example: pytyliki-apsia cpu 1000 300 100 100000 rust,wasm");
}

int main(int argc, char **argv) {
    pl_apsia_request request = {0};
    pl_apsia_decision decision;
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }
    if (argc != 7 && argc != 8) {
        usage();
        return 2;
    }
    if (argc == 8 && strcmp(argv[7], "--sandbox") != 0) {
        usage();
        return 2;
    }
    if (parse_workload(argv[1], &request.workload) != 0 ||
        parse_number(argv[2], &request.host_execution_ns) != 0 ||
        parse_number(argv[3], &request.target_execution_ns) != 0 ||
        parse_number(argv[4], &request.ffi_overhead_ns) != 0 ||
        parse_number(argv[5], &request.ffi_calls_per_second) != 0 ||
        parse_targets(argv[6], &request.available_targets) != 0) {
        fputs("Invalid workload, numeric measurement, or target list.\n", stderr);
        return 2;
    }
    request.sandbox_required = (argc == 8 || request.workload == PL_APSIA_WORK_PLUGIN) ? 1u : 0u;
    request.minimum_speedup = 1.5;
    request.maximum_ffi_cpu_cores = 0.25;

    if (pl_apsia_route(&request, &decision) != 0) {
        fprintf(stderr, "APSIA rejected invalid input: %s\n", decision.reason);
        return 3;
    }
    printf("Workload: %s\nTarget: %s\nRoute selected: %s\n"
           "Estimated speedup: %.3fx\nEstimated FFI CPU cores: %.6f\nReason: %s\n",
           pl_apsia_workload_name(request.workload),
           pl_apsia_language_name(decision.language),
           decision.route_selected ? "yes" : "no",
           decision.estimated_speedup,
           decision.estimated_ffi_cpu_cores,
           decision.reason);
    if (decision.request_rejected != 0u) return 4;
    return 0;
}
