#include "apsia.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int number(const char *text, double *out) {
    char *end = NULL;
    double value;
    if (text == NULL || text[0] == '\0') return -1;
    errno = 0;
    value = strtod(text, &end);
    if (end == text || *end != '\0' || errno == ERANGE || !isfinite(value)) return -1;
    *out = value;
    return 0;
}

static int integer_flag(const char *text, int *out) {
    if (strcmp(text, "0") == 0) { *out = 0; return 0; }
    if (strcmp(text, "1") == 0) { *out = 1; return 0; }
    return -1;
}

static int workload(const char *text, pl_apsia_workload *out) {
    static const char *const names[] = { "cpu", "gpu", "plugin", "io", "analytics", "script" };
    for (unsigned i = 0u; i < PL_APSIA_WORK_COUNT; ++i) {
        if (strcmp(text, names[i]) == 0) { *out = (pl_apsia_workload)i; return 0; }
    }
    return -1;
}

static void usage(void) {
    puts("Pytyliki APSIA advisory route planner\n"
         "Usage: pytyliki-apsia WORKLOAD EXPECTED_SPEEDUP FFI_OVERHEAD HOST_OPTIMIZATION CAPABILITIES GPU_AVAILABLE DETERMINISTIC SANDBOX\n"
         "WORKLOAD: cpu | gpu | plugin | io | analytics | script\n"
         "CAPABILITIES: integer bitmask (0 when no capabilities are requested)\n"
         "Boolean fields: 0 or 1. Routing is advisory; it never loads or executes code.");
}

int main(int argc, char **argv) {
    pl_apsia_request request = {0};
    pl_apsia_decision decision;
    char *end = NULL;
    unsigned long caps;
    if (argc == 2 && strcmp(argv[1], "--help") == 0) { usage(); return 0; }
    if (argc != 9) { usage(); return 2; }
    if (workload(argv[1], &request.workload) != 0 ||
        number(argv[2], &request.expected_speedup) != 0 ||
        number(argv[3], &request.ffi_overhead_fraction) != 0 ||
        number(argv[4], &request.host_optimization_fraction) != 0) {
        fputs("Invalid workload or numeric input.\n", stderr);
        return 2;
    }
    errno = 0;
    caps = strtoul(argv[5], &end, 0);
    if (argv[5][0] == '\0' || end == argv[5] || *end != '\0' || errno == ERANGE || caps > UINT32_MAX)
        return 2;
    request.requested_capabilities = (uint32_t)caps;
    if (integer_flag(argv[6], &request.gpu_available) != 0 ||
        integer_flag(argv[7], &request.deterministic_required) != 0 ||
        integer_flag(argv[8], &request.sandbox_required) != 0) return 2;
    if (pl_apsia_route(&request, &decision) != 0) {
        fputs("APSIA rejected invalid input.\n", stderr);
        return 3;
    }
    printf("Workload: %u\nLanguage candidate: %s\nPolyglot candidate: %s\n"
           "Sandbox required: %s\nHuman review required: %s\nGranted capabilities: 0x%08x\nReason: %s\n",
           (unsigned)request.workload, pl_apsia_language_name(decision.language),
           decision.use_polyglot ? "yes" : "no",
           decision.sandbox_required ? "yes" : "no",
           decision.human_review_required ? "yes" : "no",
           (unsigned)decision.granted_capabilities, decision.reason);
    return 0;
}
