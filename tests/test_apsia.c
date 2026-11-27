#include "apsia.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    pl_apsia_request request = {0};
    pl_apsia_decision decision;
    pl_apsia_promotion_evidence evidence = {0};

    request.workload = PL_APSIA_WORK_CPU_BOUND;
    request.expected_speedup = 2.0;
    request.ffi_overhead_fraction = 0.1;
    request.host_optimization_fraction = 0.2;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_RUST);
    assert(decision.use_polyglot == 1);
    assert(decision.granted_capabilities == 0u);
    assert(decision.human_review_required == 1);

    request.workload = PL_APSIA_WORK_GPU_TENSOR;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_HOST);
    request.gpu_available = 1;
    request.requested_capabilities = PL_APSIA_CAP_GPU;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_CUDA);
    request.deterministic_required = 1;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_HOST);

    request.deterministic_required = 0;
    request.workload = PL_APSIA_WORK_UNTRUSTED_PLUGIN;
    request.gpu_available = 0;
    request.requested_capabilities = 0u;
    request.expected_speedup = 1.0;
    request.wasm_available = 0;
    assert(pl_apsia_route(&request, &decision) == -2);
    request.wasm_available = 1;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_WASM);
    assert(decision.sandbox_required == 1);
    request.wasm_available = 0;
    request.sandbox_required = 1;
    assert(pl_apsia_route(&request, &decision) == -2);

    request.sandbox_required = 0;
    request.workload = PL_APSIA_WORK_ANALYTICS;
    request.expected_speedup = 2.0;
    request.wasm_available = 0;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_DAKDB);

    request.workload = PL_APSIA_WORK_CPU_BOUND;
    request.host_optimization_fraction = 0.70;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_HOST);
    request.host_optimization_fraction = 0.2;
    request.expected_speedup = 1.05;
    request.ffi_overhead_fraction = 0.1;
    assert(pl_apsia_route(&request, &decision) == 0);
    assert(decision.language == PL_APSIA_LANG_HOST);

    request.expected_speedup = NAN;
    assert(pl_apsia_route(&request, &decision) == -1);
    request.expected_speedup = 2.0;
    request.requested_capabilities = UINT32_C(0x80000000);
    assert(pl_apsia_route(&request, &decision) == -1);

    assert(pl_apsia_can_promote(&evidence) == 0);
    evidence.contract_valid = 1;
    evidence.differential_tests_passed = 1;
    evidence.fuzzing_passed = 1;
    evidence.sanitizers_passed = 1;
    evidence.static_analysis_passed = 1;
    evidence.benchmark_gain_verified = 1;
    evidence.shadow_equivalence_passed = 1;
    evidence.independent_review_approved = 1;
    evidence.artifact_signature_valid = 1;
    evidence.rollback_tested = 1;
    evidence.validated_head_token = 42u;
    evidence.current_head_token = 42u;
    assert(pl_apsia_can_promote(&evidence) == 1);
    evidence.current_head_token = 43u;
    assert(pl_apsia_can_promote(&evidence) == 0);
    evidence.current_head_token = 42u;
    evidence.independent_review_approved = 0;
    assert(pl_apsia_can_promote(&evidence) == 0);

    puts("APSIA routing and promotion tests passed.");
    return 0;
}
