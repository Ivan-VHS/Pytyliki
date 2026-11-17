#include "apsia.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_number(const char *text, double *value) {
    char *end = NULL;
    double parsed;

    if (text == NULL || value == NULL || text[0] == '\0') {
        return -1;
    }
    errno = 0;
    parsed = strtod(text, &end);
    if (end == text || *end != '\0' || errno == ERANGE || !isfinite(parsed)) {
        return -1;
    }
    *value = parsed;
    return 0;
}

static int parse_workload(const char *text, pl_apsia_workload *workload) {
    static const char *const names[PL_APSIA_WORK_COUNT] = {
        "cpu", "gpu", "plugin", "network-io", "analytics", "script"
    };
    unsigned i;

    if (text == NULL || workload == NULL) {
        return -1;
    }
    for (i = 0u; i < PL_APSIA_WORK_COUNT; ++i) {
        if (strcmp(text, names[i]) == 0) {
            *workload = (pl_apsia_workload)i;
            return 0;
        }
    }
    return -1;
}

static int parse_capabilities(const char *text, uint32_t *capabilities) {
    char copy[256];
    char *cursor;
    size_t length;

    if (text == NULL || capabilities == NULL) {
        return -1;
    }
    length = strlen(text);
    if (length == 0u || length >= sizeof(copy)) {
        return -1;
    }
    memcpy(copy, text, length + 1u);
    *capabilities = 0u;
    cursor = copy;

    for (;;) {
        char *comma = strchr(cursor, ',');
        if (comma != NULL) {
            *comma = '\0';
        }
        if (cursor[0] == '\0') {
            return -1;
        }
        if (strcmp(cursor, "network") == 0) {
            *capabilities |= PL_APSIA_CAP_NETWORK;
        } else if (strcmp(cursor, "filesystem") == 0) {
            *capabilities |= PL_APSIA_CAP_FILESYSTEM;
        } else if (strcmp(cursor, "clock") == 0) {
            *capabilities |= PL_APSIA_CAP_CLOCK;
        } else if (strcmp(cursor, "gpu") == 0) {
            *capabilities |= PL_APSIA_CAP_GPU;
        } else if (strcmp(cursor, "shared-memory") == 0) {
            *capabilities |= PL_APSIA_CAP_SHARED_MEMORY;
        } else {
            return -1;
        }
        if (comma == NULL) {
            break;
        }
        cursor = comma + 1;
        if (cursor[0] == '\0') {
            return -1;
        }
    }
    return *capabilities != 0u ? 0 : -1;
}

static void usage(void) {
    puts("Pytyliki APSIA advisory route planner\n"
         "Usage: pytyliki-apsia WORKLOAD EXPECTED_SPEEDUP FFI_OVERHEAD_FRACTION HOST_OPTIMIZATION_FRACTION [options]\n"
         "WORKLOAD: cpu | gpu | plugin | network-io | analytics | script\n"
         "Options: --gpu --wasm --deterministic --sandbox --capabilities LIST\n"
         "--gpu and --wasm explicitly declare candidate backend availability; they do not probe hardware or runtimes.\n"
         "Capabilities: network,filesystem,clock,gpu,shared-memory\n"
         "Example: pytyliki-apsia gpu 2.0 0.05 0.10 --gpu");
}

int main(int argc, char **argv) {
    pl_apsia_request request = {0};
    pl_apsia_decision decision;
    int i;
    int route_status;

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
        parse_number(argv[4], &request.host_optimization_fraction) != 0 ||
        request.expected_speedup < 0.0 ||
        request.ffi_overhead_fraction < 0.0 || request.ffi_overhead_fraction > 1.0 ||
        request.host_optimization_fraction < 0.0 ||
        request.host_optimization_fraction > 1.0) {
        fputs("Invalid workload or numeric policy input.\n", stderr);
        return 2;
    }

    for (i = 5; i < argc; ++i) {
        if (strcmp(argv[i], "--gpu") == 0) {
            request.gpu_available = 1;
            request.requested_capabilities |= PL_APSIA_CAP_GPU;
        } else if (strcmp(argv[i], "--wasm") == 0) {
            request.wasm_available = 1;
        } else if (strcmp(argv[i], "--deterministic") == 0) {
            request.deterministic_required = 1;
        } else if (strcmp(argv[i], "--sandbox") == 0) {
            request.sandbox_required = 1;
        } else if (strcmp(argv[i], "--capabilities") == 0 && i + 1 < argc) {
            uint32_t parsed_capabilities = 0u;
            if (parse_capabilities(argv[++i], &parsed_capabilities) != 0) {
                fputs("Invalid capability list.\n", stderr);
                return 2;
            }
            /* Add rather than replace: option order must not clear --gpu. */
            request.requested_capabilities |= parsed_capabilities;
        } else {
            fprintf(stderr, "Unknown or incomplete option: %s\n", argv[i]);
            usage();
            return 2;
        }
    }

    route_status = pl_apsia_route(&request, &decision);
    if (route_status == -2) {
        fputs("Required WebAssembly backend was not declared available; refusing sandbox route.\n", stderr);
        return 3;
    }
    if (route_status != 0) {
        fputs("APSIA rejected invalid routing input.\n", stderr);
        return 3;
    }

    printf("Workload: %s\nLanguage: %s\nPolyglot: %s\n"
           "Sandbox required: %s\nZero-copy candidate: %s\n"
           "Granted capabilities: 0x%08x\nHuman review required: %s\nDecision: %s\n",
           argv[1], pl_apsia_language_name(decision.language),
           decision.use_polyglot ? "yes" : "no",
           decision.sandbox_required ? "yes" : "no",
           decision.zero_copy_candidate ? "yes" : "no",
           (unsigned)decision.granted_capabilities,
           decision.human_review_required ? "yes" : "no", decision.reason);
    return 0;
}
