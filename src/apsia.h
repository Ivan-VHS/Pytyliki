#ifndef PYTYLIKI_APSIA_H
#define PYTYLIKI_APSIA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* APSIA routing is a deterministic planning layer. It does not compile,
 * load, execute, or promote generated code. */
typedef enum {
    PL_APSIA_WORK_CPU = 0,
    PL_APSIA_WORK_GPU,
    PL_APSIA_WORK_ANALYTICS,
    PL_APSIA_WORK_PLUGIN,
    PL_APSIA_WORK_NETWORK_IO,
    PL_APSIA_WORK_SCRIPT,
    PL_APSIA_WORK_COUNT
} pl_apsia_workload;

typedef enum {
    PL_APSIA_LANG_HOST = 0,
    PL_APSIA_LANG_RUST,
    PL_APSIA_LANG_CPP,
    PL_APSIA_LANG_ZIG,
    PL_APSIA_LANG_CUDA,
    PL_APSIA_LANG_WASM,
    PL_APSIA_LANG_SQL,
    PL_APSIA_LANG_PYTHON,
    PL_APSIA_LANG_GO,
    PL_APSIA_LANG_COUNT
} pl_apsia_language;

#define PL_APSIA_TARGET_BIT(language) (UINT32_C(1) << (unsigned)(language))

typedef struct {
    pl_apsia_workload workload;
    uint32_t available_targets;
    uint32_t sandbox_required;
    double host_execution_ns;
    double target_execution_ns;
    double ffi_overhead_ns;
    double ffi_calls_per_second;
    double minimum_speedup;
    double maximum_ffi_cpu_cores;
} pl_apsia_request;

typedef struct {
    pl_apsia_language language;
    uint32_t route_selected;
    uint32_t performance_approved;
    uint32_t request_rejected;
    double estimated_speedup;
    double estimated_ffi_cpu_cores;
    char reason[160];
} pl_apsia_decision;

/* Returns 0 for a valid plan, -1 for invalid input. If sandbox_required is
 * set, an unavailable or uneconomic sandbox rejects the request; it never
 * falls back to executing the plugin in the host process. */
int pl_apsia_route(const pl_apsia_request *request, pl_apsia_decision *decision);
const char *pl_apsia_language_name(pl_apsia_language language);
const char *pl_apsia_workload_name(pl_apsia_workload workload);

#ifdef __cplusplus
}
#endif
#endif
