#include "apsia.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_number(const char *text, double *out) {
    char *end = NULL;
    double value;

    if (text == NULL || text[0] == '\0' || out == NULL) return -1;
    errno = 0;
    value = strtod(text, &end);
    if (end == text || *end != '\0' || errno == ERANGE || !isfinite(value)) return -1;
    *out = value;
    return 0;
}

static int parse_flag(const char *text, int *out) {
    if (text == NULL || out == NULL) return -1;
    if (strcmp(text, "0") == 0) { *out = 0; return 0; }
    if (strcmp(text, "1") == 0) { *out = 1; return 0; }
    return -1;
}

static int parse_workload(const char *text, pl_apsia_workload *out) {
    static const char *const names[] = { "cpu", "gpu", "plugin", "io", "analytics", "script" };

    if (text == NULL || out == NULL) return -1;
    for (unsigned i = 0u; i < PL_APSIA_WORK_COUNT; ++i) {
        if (strcmp(text, names[i]) == 0) {
            *out = (pl_apsia_workload)i;
            return 0;
        }
    }
    return -1;
}

static void usage(void) {
    puts("Pytyliki APSIA advisory route planner\n"
         "Usage: pytyliki-apsia WORKLOAD EXPECTED_SPEEDUP FFI_OVERHEAD HOST_OPTIMIZATION [OPTIONS]\n"
         "WORKLOAD: cpu | gpu | plugin | io | analytics | script\n"
         "Options: --gpu --wasm --deterministic --sandbox\n"
         "         --capabilities NAME[,NAME...]\n"
         "Capabilities: network, filesystem, clock, gpu, shared-memory\n"
         "Legacy positional form: WORKLOAD SPEEDUP FFI HOST_OPT CAPS GPU DETERMINISTIC SANDBOX\n"
         "Extended positional form adds WASM after GPU.\n"
         "Routing is advisory; it never loads or executes generated code.");
}

static int capability_from_token(const char *token, size_t length, uint32_t *out) {
    if (token == NULL || out == NULL) return -1;
    if (length == 7u && memcmp(token, "network", 7u) == 0) {
        *out = PL_APSIA_CAP_NETWORK;
    } else if (length == 10u && memcmp(token, "filesystem", 10u) == 0) {
        *out = PL_APSIA_CAP_FILESYSTEM;
    } else if (length == 5u && memcmp(token, "clock", 5u) == 0) {
        *out = PL_APSIA_CAP_CLOCK;
    } else if (length == 3u && memcmp(token, "gpu", 3u) == 0) {
        *out = PL_APSIA_CAP_GPU;
    } else if (length == 13u && memcmp(token, "shared-memory", 13u) == 0) {
        *out = PL_APSIA_CAP_SHARED_MEMORY;
    } else {
        return -1;
    }
    return 0;
}

static int parse_capabilities(const char *text, uint32_t *out) {
    const char *token;
    const char *cursor;
    uint32_t capabilities = 0u;

    if (text == NULL || out == NULL || text[0] == '\0') return -1;

    token = text;
    cursor = text;
    for (;;) {
        uint32_t bit;
        if (*cursor == ',' || *cursor == '\0') {
            if (cursor == token || capability_from_token(token, (size_t)(cursor - token), &bit) != 0) {
                return -1;
            }
            capabilities |= bit;
            if (*cursor == '\0') break;
            token = cursor + 1;
        }
        if (*cursor == '\0') break;
        ++cursor;
    }

    *out = capabilities;
    return 0;
}

static int parse_legacy_request(int argc, char **argv, pl_apsia_request *request) {
    char *end = NULL;
    unsigned long capabilities;

    if (argc != 9 && argc != 10) return -1;
    if (parse_workload(argv[1], &request->workload) != 0 ||
        parse_number(argv[2], &request->expected_speedup) != 0 ||
        parse_number(argv[3], &request->ffi_overhead_fraction) != 0 ||
        parse_number(argv[4], &request->host_optimization_fraction) != 0) {
        return -1;
    }

    errno = 0;
    capabilities = strtoul(argv[5], &end, 0);
    if (argv[5][0] == '\0' || end == argv[5] || *end != '\0' ||
        errno == ERANGE || capabilities > UINT32_MAX) {
        return -1;
    }
    request->requested_capabilities = (uint32_t)capabilities;
    if (parse_flag(argv[6], &request->gpu_available) != 0) return -1;
    if (argc == 9) {
        request->wasm_available = 0;
        if (parse_flag(argv[7], &request->deterministic_required) != 0 ||
            parse_flag(argv[8], &request->sandbox_required) != 0) return -1;
    } else if (parse_flag(argv[7], &request->wasm_available) != 0 ||
               parse_flag(argv[8], &request->deterministic_required) != 0 ||
               parse_flag(argv[9], &request->sandbox_required) != 0) {
        return -1;
    }
    return 0;
}

static int parse_option_request(int argc, char **argv, pl_apsia_request *request) {
    if (parse_workload(argv[1], &request->workload) != 0 ||
        parse_number(argv[2], &request->expected_speedup) != 0 ||
        parse_number(argv[3], &request->ffi_overhead_fraction) != 0 ||
        parse_number(argv[4], &request->host_optimization_fraction) != 0) {
        return -1;
    }

    for (int i = 5; i < argc; ++i) {
        if (strcmp(argv[i], "--gpu") == 0) {
            request->gpu_available = 1;
            request->requested_capabilities |= PL_APSIA_CAP_GPU;
        } else if (strcmp(argv[i], "--wasm") == 0) {
            request->wasm_available = 1;
        } else if (strcmp(argv[i], "--deterministic") == 0) {
            request->deterministic_required = 1;
        } else if (strcmp(argv[i], "--sandbox") == 0) {
            request->sandbox_required = 1;
        } else if (strcmp(argv[i], "--capabilities") == 0) {
            uint32_t capabilities;
            if (++i >= argc || parse_capabilities(argv[i], &capabilities) != 0) return -1;
            request->requested_capabilities |= capabilities;
        } else if (strncmp(argv[i], "--capabilities=", 15u) == 0) {
            uint32_t capabilities;
            if (parse_capabilities(argv[i] + 15u, &capabilities) != 0) return -1;
            request->requested_capabilities |= capabilities;
        } else {
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    pl_apsia_request request = {0};
    pl_apsia_decision decision;
    int parsed;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }
    if (argc < 5) {
        usage();
        return 2;
    }

    /* Keep the original numeric form usable for existing scripts. */
    parsed = (argc == 9 || argc == 10)
        ? parse_legacy_request(argc, argv, &request)
        : -1;
    if (parsed != 0) parsed = parse_option_request(argc, argv, &request);
    if (parsed != 0) {
        fputs("Invalid APSIA workload, numeric input or option.\n", stderr);
        usage();
        return 2;
    }

    if (pl_apsia_route(&request, &decision) != 0) {
        fputs("APSIA rejected the request; verify the declared backends and bounds.\n", stderr);
        return 3;
    }

    printf("Workload: %u\nLanguage: %s\nPolyglot candidate: %s\n"
           "Sandbox required: %s\nHuman review required: %s\n"
           "Zero-copy candidate: %s\nGranted capabilities: 0x%08x\nReason: %s\n",
           (unsigned)request.workload,
           pl_apsia_language_name(decision.language),
           decision.use_polyglot ? "yes" : "no",
           decision.sandbox_required ? "yes" : "no",
           decision.human_review_required ? "yes" : "no",
           decision.zero_copy_candidate ? "yes" : "no",
           (unsigned)decision.granted_capabilities,
           decision.reason);
    return 0;
}
