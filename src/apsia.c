#include "apsia.h"

#include <math.h>
#include <stddef.h>

const char *pl_apsia_language_name(pl_apsia_language language) {
    static const char *const names[] = {
        "host", "Rust", "C++", "Zig", "CUDA", "WebAssembly",
        "Go", "Elixir", "SQL", "DuckDB", "Lua", "Python"
    };

    if ((int)language < 0 || language > PL_APSIA_LANG_PYTHON) {
        return "unknown";
    }
    return names[(unsigned)language];
}

int pl_apsia_route(const pl_apsia_request *request, pl_apsia_decision *decision) {
    const uint32_t known_caps =
        PL_APSIA_CAP_NETWORK | PL_APSIA_CAP_FILESYSTEM | PL_APSIA_CAP_CLOCK |
        PL_APSIA_CAP_GPU | PL_APSIA_CAP_SHARED_MEMORY;
    pl_apsia_language selected = PL_APSIA_LANG_HOST;
    const char *reason = "retain host implementation";
    int sandbox;

    if (request == NULL || decision == NULL) {
        return -1;
    }
    if ((int)request->workload < 0 || request->workload >= PL_APSIA_WORK_COUNT ||
        !isfinite(request->expected_speedup) ||
        !isfinite(request->ffi_overhead_fraction) ||
        !isfinite(request->host_optimization_fraction) ||
        request->expected_speedup < 0.0 ||
        request->ffi_overhead_fraction < 0.0 ||
        request->ffi_overhead_fraction > 1.0 ||
        request->host_optimization_fraction < 0.0 ||
        request->host_optimization_fraction > 1.0 ||
        (request->requested_capabilities & ~known_caps) != 0u ||
        (request->gpu_available != 0 && request->gpu_available != 1) ||
        (request->wasm_available != 0 && request->wasm_available != 1) ||
        (request->deterministic_required != 0 && request->deterministic_required != 1) ||
        (request->sandbox_required != 0 && request->sandbox_required != 1)) {
        return -1;
    }

    sandbox = request->sandbox_required ||
              request->workload == PL_APSIA_WORK_UNTRUSTED_PLUGIN;
    if (sandbox) {
        /* Never advertise a sandbox route unless the caller explicitly declares
         * a compatible WebAssembly/WASI backend available. */
        if (!request->wasm_available) {
            return -2;
        }
        selected = PL_APSIA_LANG_WASM;
        reason = "sandbox required: declared WebAssembly candidate";
    } else if (request->host_optimization_fraction >= 0.70) {
        reason = "host optimization preferred: >=70% of opportunity";
    } else if (request->expected_speedup <= 1.0 + request->ffi_overhead_fraction) {
        reason = "estimated gain does not exceed FFI overhead";
    } else {
        switch (request->workload) {
        case PL_APSIA_WORK_CPU_BOUND:
            selected = PL_APSIA_LANG_RUST;
            reason = "CPU-bound workload: Rust candidate";
            break;
        case PL_APSIA_WORK_GPU_TENSOR:
            if (request->deterministic_required) {
                reason = "deterministic result required: retain host implementation";
            } else if (request->gpu_available &&
                       (request->requested_capabilities & PL_APSIA_CAP_GPU) != 0u) {
                selected = PL_APSIA_LANG_CUDA;
                reason = "GPU explicitly declared available: CUDA candidate";
            } else {
                reason = "GPU backend not declared available: retain host";
            }
            break;
        case PL_APSIA_WORK_CONCURRENT_IO:
            selected = PL_APSIA_LANG_GO;
            reason = "concurrent I/O workload: Go candidate";
            break;
        case PL_APSIA_WORK_ANALYTICS:
            selected = PL_APSIA_LANG_DUCKDB;
            reason = "analytical workload: DuckDB candidate";
            break;
        case PL_APSIA_WORK_EMBEDDED_SCRIPT:
            selected = PL_APSIA_LANG_LUA;
            reason = "embedded scripting workload: Lua candidate";
            break;
        default:
            return -1;
        }
    }

    decision->language = selected;
    decision->use_polyglot = selected != PL_APSIA_LANG_HOST;
    decision->sandbox_required = sandbox;
    decision->human_review_required = 1;
    decision->zero_copy_candidate =
        decision->use_polyglot &&
        (request->requested_capabilities & PL_APSIA_CAP_SHARED_MEMORY) != 0u;
    /* Routing never grants capabilities; a separate reviewed broker must do so. */
    decision->granted_capabilities = 0u;
    decision->reason = reason;
    return 0;
}

int pl_apsia_can_promote(const pl_apsia_promotion_evidence *evidence) {
    if (evidence == NULL || evidence->validated_head_token == 0u ||
        evidence->current_head_token == 0u ||
        evidence->validated_head_token != evidence->current_head_token) {
        return 0;
    }

    return evidence->contract_valid == 1 &&
           evidence->differential_tests_passed == 1 &&
           evidence->fuzzing_passed == 1 &&
           evidence->sanitizers_passed == 1 &&
           evidence->static_analysis_passed == 1 &&
           evidence->benchmark_gain_verified == 1 &&
           evidence->shadow_equivalence_passed == 1 &&
           evidence->independent_review_approved == 1 &&
           evidence->artifact_signature_valid == 1 &&
           evidence->rollback_tested == 1;
}
