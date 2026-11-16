#include "apsia.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

const char *pl_apsia_language_name(pl_apsia_language language) {
    static const char *const names[PL_APSIA_LANG_COUNT] = {
        "host", "Rust", "C++", "Zig", "CUDA", "WebAssembly", "SQL",
        "Python", "Go"
    };
    return (language >= 0 && language < PL_APSIA_LANG_COUNT)
        ? names[language] : "unknown";
}

const char *pl_apsia_workload_name(pl_apsia_workload workload) {
    static const char *const names[PL_APSIA_WORK_COUNT] = {
        "cpu-bound", "gpu-compute", "analytics", "sandboxed-plugin",
        "network-io", "scripting"
    };
    return (workload >= 0 && workload < PL_APSIA_WORK_COUNT)
        ? names[workload] : "unknown";
}

static void host_fallback(pl_apsia_decision *decision, const char *reason) {
    decision->language = PL_APSIA_LANG_HOST;
    decision->route_selected = 0u;
    decision->performance_approved = 0u;
    (void)snprintf(decision->reason, sizeof(decision->reason), "%s", reason);
}

int pl_apsia_route(const pl_apsia_request *request, pl_apsia_decision *decision) {
    pl_apsia_language candidate;
    double total_native_ns;
    double speedup;
    double ffi_cores;

    if (request == NULL || decision == NULL) return -1;
    memset(decision, 0, sizeof(*decision));
    decision->language = PL_APSIA_LANG_HOST;

    if (request->workload < 0 || request->workload >= PL_APSIA_WORK_COUNT ||
        request->sandbox_required > 1u ||
        !isfinite(request->host_execution_ns) ||
        !isfinite(request->target_execution_ns) ||
        !isfinite(request->ffi_overhead_ns) ||
        !isfinite(request->ffi_calls_per_second) ||
        !isfinite(request->minimum_speedup) ||
        !isfinite(request->maximum_ffi_cpu_cores) ||
        request->host_execution_ns <= 0.0 ||
        request->target_execution_ns <= 0.0 ||
        request->ffi_overhead_ns < 0.0 ||
        request->ffi_calls_per_second < 0.0 ||
        request->minimum_speedup < 1.0 ||
        request->maximum_ffi_cpu_cores < 0.0) {
        host_fallback(decision, "Invalid measurements or policy; retain host implementation.");
        return -1;
    }

    /* Sandboxing takes precedence over raw speed. WASM must be explicitly
     * available; otherwise the request is not routed to native code. */
    if (request->sandbox_required != 0u) {
        candidate = PL_APSIA_LANG_WASM;
    } else {
        switch (request->workload) {
            case PL_APSIA_WORK_CPU: candidate = PL_APSIA_LANG_RUST; break;
            case PL_APSIA_WORK_GPU: candidate = PL_APSIA_LANG_CUDA; break;
            case PL_APSIA_WORK_ANALYTICS: candidate = PL_APSIA_LANG_SQL; break;
            case PL_APSIA_WORK_PLUGIN: candidate = PL_APSIA_LANG_WASM; break;
            case PL_APSIA_WORK_NETWORK_IO: candidate = PL_APSIA_LANG_GO; break;
            case PL_APSIA_WORK_SCRIPT: candidate = PL_APSIA_LANG_PYTHON; break;
            default:
                host_fallback(decision, "Unknown workload; retain host implementation.");
                return 0;
        }
    }

    if ((request->available_targets & PL_APSIA_TARGET_BIT(candidate)) == 0u) {
        host_fallback(decision, "Selected target is unavailable; retain host implementation.");
        return 0;
    }

    total_native_ns = request->target_execution_ns + request->ffi_overhead_ns;
    if (!isfinite(total_native_ns) || total_native_ns <= 0.0) {
        host_fallback(decision, "Invalid combined execution cost; retain host implementation.");
        return -1;
    }
    speedup = request->host_execution_ns / total_native_ns;
    ffi_cores = (request->ffi_overhead_ns * request->ffi_calls_per_second) / 1.0e9;
    if (!isfinite(speedup) || !isfinite(ffi_cores)) {
        host_fallback(decision, "Cost estimate overflow; retain host implementation.");
        return -1;
    }

    decision->estimated_speedup = speedup;
    decision->estimated_ffi_cpu_cores = ffi_cores;
    if (speedup < request->minimum_speedup) {
        host_fallback(decision, "FFI-adjusted speedup is below the configured threshold.");
        decision->estimated_speedup = speedup;
        decision->estimated_ffi_cpu_cores = ffi_cores;
        return 0;
    }
    if (ffi_cores > request->maximum_ffi_cpu_cores) {
        host_fallback(decision, "Estimated FFI overhead exceeds the CPU budget.");
        decision->estimated_speedup = speedup;
        decision->estimated_ffi_cpu_cores = ffi_cores;
        return 0;
    }

    decision->language = candidate;
    decision->route_selected = 1u;
    decision->performance_approved = 1u;
    (void)snprintf(decision->reason, sizeof(decision->reason),
        "Selected %s for %s; sandbox and FFI-adjusted cost policy passed.",
        pl_apsia_language_name(candidate), pl_apsia_workload_name(request->workload));
    return 0;
}
