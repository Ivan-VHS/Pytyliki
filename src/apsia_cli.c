#include "apsia.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_number(const char *text, double *value) {
    char *end = NULL;
    double parsed;
    if (text == NULL || text[0] == '\0' || value == NULL) return -1;
    errno = 0;
    parsed = strtod(text, &end);
    if (end == text || *end != '\0' || errno == ERANGE || !isfinite(parsed))
        return -1;
    *value = parsed;
    return 0;
}

static int parse_workload(const char *text, pl_apsia_workload *workload) {
    static const char *const names[PL_APSIA_WORK_COUNT] = {
        "cpu", "gpu", "plugin", "network-io", "analytics", "script"
    };
    unsigned i;
    if (text == NULL || workload == NULL) return -1;
    for (i = 0u; i < PL_APSIA_WORK_COUNT; ++i) {
        if (strcmp(text, names[i]) == 0) {
            *workload = (pl_apsia_workload)i;
            return 0;
        }
    }
    return -1;
}

static int parse_u32(const char *text, uint32_t *value) {
    char *end = NULL;
    unsigned long parsed;
    if (text == NULL || text[0] == '\0' || value == NULL) return -1;
    errno = 0;
    parsed = strtoul(text, &end, 0);
    if (end == text || *end != '\0' || errno == ERANGE || parsed > UINT32_MAX)
        return -1;
    *value = (uint32_t)parsed;
    return 0;
}

static void usage(void) {
    puts("Pytyliki APSIA advisory route planner\n"
         "Usage: pytyliki-apsia WORKLOAD EXPECTED_SPEEDUP FFI_OVERHEAD HOST_OPTIMIZATION [options]\n"
         "WORKLOAD: cpu | gpu | plugin | network-io | analytics | script\n"
         "Options: --sandbox --gpu --deterministic --capabilities MASK\n"
         "Fractions must be in [0,1]; routing is advisory and grants no capabilities.");
}

int main(int argc, char **argv) {
    pl_apsia_request request = {0};
    pl_apsia_decision decision;
    int i;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }
    if (argc < 5) {
        usage();
        return 2;
    }
    if (parse_workload(argv[1], &request.workload) != 0 ||
        parse_number(argv[2], &request.expected_speedup) != 0 ||
        parse_number(argv[3], &request.ffi_overhead_fraction) != 0 ||
        parse_number(argv[4], &request.host_optimization_fraction) != 0) {
        fputs("Invalid workload or numeric argument.\n", stderr);
        return 2;
    }
    for (i = 5; i < argc; ++i) {
        if (strcmp(argv[i], "--sandbox") == 0) request.sandbox_required = 1;
        else if (strcmp(argv[i], "--gpu") == 0) request.gpu_available = 1;
        else if (strcmp(argv[i], "--deterministic") == 0) request.deterministic_required = 1;
        else if (strcmp(argv[i], "--capabilities") == 0 && i + 1 < argc) {
            if (parse_u32(argv[++i], &request.requested_capabilities) != 0) {
                fputs("Invalid capability mask.\n", stderr);
                return 2;
            }
        } else {
            fprintf(stderr, "Unknown or incomplete option: %s\n", argv[i]);
            return 2;
        }
    }
    if (request.ffi_overhead_fraction < 0.0 || request.ffi_overhead_fraction > 1.0 ||
        request.host_optimization_fraction < 0.0 || request.host_optimization_fraction > 1.0) {
        fputs("Fractions must be in [0,1].\n", stderr);
        return 2;
    }
    if (pl_apsia_route(&request, &decision) != 0) {
        fputs("APSIA rejected invalid input.\n", stderr);
        return 3;
    }
    printf("Workload: %s\nCandidate: %s\nPolyglot route: %s\n"
           "Sandbox required: %s\nZero-copy candidate: %s\n"
           "Granted capabilities: 0x%08x\nHuman review required: %s\nReason: %s\n",
           argv[1], pl_apsia_language_name(decision.language),
           decision.use_polyglot ? "yes" : "no",
           decision.sandbox_required ? "yes" : "no",
           decision.zero_copy_candidate ? "yes" : "no",
           (unsigned)decision.granted_capabilities,
           decision.human_review_required ? "yes" : "no", decision.reason);
    return 0;
}
