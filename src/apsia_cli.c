#include "apsia.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_number(const char *text, double *value) {
    char *end = NULL;
    double parsed;
    if (text == NULL || text[0] == '\\0' || value == NULL) return -1;
    errno = 0;
    parsed = strtod(text, &end);
    if (end == text || *end != '\\0' || errno == ERANGE || !isfinite(parsed)) return -1;
    *value = parsed;
    return 0;
}

static int parse_workload(const char *name, pl_apsia_workload *workload) {
    if (name == NULL || workload == NULL) return -1;
    if (strcmp(name, "cpu") == 0) *workload = PL_APSIA_WORK_CPU_BOUND;
    else if (strcmp(name, "gpu") == 0) *workload = PL_APSIA_WORK_GPU_TENSOR;
    else if (strcmp(name, "plugin") == 0) *workload = PL_APSIA_WORK_UNTRUSTED_PLUGIN;
    else if (strcmp(name, "io") == 0) *workload = PL_APSIA_WORK_CONCURRENT_IO;
    else if (strcmp(name, "analytics") == 0) *workload = PL_APSIA_WORK_ANALYTICS;
    else if (strcmp(name, "script") == 0) *workload = PL_APSIA_WORK_EMBEDDED_SCRIPT;
    else return -1;
    return 0;
}

static void usage(void) {
    puts("Pytyliki APSIA advisory route planner\\n"
         "Usage: pytyliki-apsia WORKLOAD EXPECTED_SPEEDUP FFI_OVERHEAD_FRACTION HOST_OPTIMIZATION_FRACTION [--sandbox]\\n"
         "WORKLOAD: cpu | gpu | plugin | io | analytics | script\\n"
         "Fractions must be in [0,1]; speedup must be non-negative.\\n"
         "Example: pytyliki-apsia cpu 2.0 0.05 0.10");
}

int main(int argc, char **argv) {
    pl_apsia_request request = {0};
    pl_apsia_decision decision;
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }
    if (argc != 5 && argc != 6) {
        usage();
        return 2;
    }
    if (argc == 6 && strcmp(argv[5], "--sandbox") != 0) {
        usage();
        return 2;
    }
    if (parse_workload(argv[1], &request.workload) != 0 ||
        parse_number(argv[2], &request.expected_speedup) != 0 ||
        parse_number(argv[3], &request.ffi_overhead_fraction) != 0 ||
        parse_number(argv[4], &request.host_optimization_fraction) != 0 ||
        request.expected_speedup < 0.0 ||
        request.ffi_overhead_fraction < 0.0 || request.ffi_overhead_fraction > 1.0 ||
        request.host_optimization_fraction < 0.0 || request.host_optimization_fraction > 1.0) {
        fputs("Invalid workload or measurement.\\n", stderr);
        return 2;
    }
    request.gpu_available = request.workload == PL_APSIA_WORK_GPU_TENSOR;
    if (request.gpu_available) request.requested_capabilities |= PL_APSIA_CAP_GPU;
    request.sandbox_required = (argc == 6 || request.workload == PL_APSIA_WORK_UNTRUSTED_PLUGIN);
    request.deterministic_required = 0;

    if (pl_apsia_route(&request, &decision) != 0) {
        fputs("APSIA rejected invalid input.\\n", stderr);
        return 3;
    }
    printf("Workload: %u\\nTarget: %s\\nPolyglot candidate: %s\\nSandbox required: %s\\nHuman review required: %s\\nGranted capabilities: 0x%08x\\nReason: %s\\n",
           (unsigned)request.workload,
           pl_apsia_language_name(decision.language),
           decision.use_polyglot ? "yes" : "no",
           decision.sandbox_required ? "yes" : "no",
           decision.human_review_required ? "yes" : "no",
           decision.granted_capabilities,
           decision.reason);
    return 0;
}
