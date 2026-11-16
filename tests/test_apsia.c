#include "apsia.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void) {
 pl_apsia_request r={0}; pl_apsia_decision d;
 r.workload=PL_APSIA_WORK_CPU_BOUND; r.expected_speedup=2.0; r.ffi_overhead_fraction=0.1; r.host_optimization_fraction=0.2;
 assert(pl_apsia_route(&r,&d)==0 && d.language==PL_APSIA_LANG_RUST && d.use_polyglot==1 && d.granted_capabilities==0u && d.human_review_required==1);
 r.workload=PL_APSIA_WORK_GPU_TENSOR; r.gpu_available=1; r.requested_capabilities=PL_APSIA_CAP_GPU;
 assert(pl_apsia_route(&r,&d)==0 && d.language==PL_APSIA_LANG_CUDA);
 r.deterministic_required=1;
 assert(pl_apsia_route(&r,&d)==0 && d.language==PL_APSIA_LANG_HOST);
 r.deterministic_required=0; r.workload=PL_APSIA_WORK_UNTRUSTED_PLUGIN; r.gpu_available=0; r.requested_capabilities=0u; r.expected_speedup=1.0;
 assert(pl_apsia_route(&r,&d)==0 && d.language==PL_APSIA_LANG_WASM && d.sandbox_required==1);
 r.workload=PL_APSIA_WORK_ANALYTICS; r.expected_speedup=2.0;
 assert(pl_apsia_route(&r,&d)==0 && d.language==PL_APSIA_LANG_DUCKDB);
 r.workload=PL_APSIA_WORK_CPU_BOUND; r.host_optimization_fraction=0.70;
 assert(pl_apsia_route(&r,&d)==0 && d.language==PL_APSIA_LANG_HOST);
 r.host_optimization_fraction=0.2; r.expected_speedup=1.05; r.ffi_overhead_fraction=0.1;
 assert(pl_apsia_route(&r,&d)==0 && d.language==PL_APSIA_LANG_HOST);
 r.expected_speedup=NAN; assert(pl_apsia_route(&r,&d)!=0);
 r.expected_speedup=2.0; r.requested_capabilities=UINT32_C(0x80000000);
 assert(pl_apsia_route(&r,&d)!=0);
 pl_apsia_promotion_evidence e={0}; assert(pl_apsia_can_promote(&e)==0);
 e.contract_valid=1; e.differential_tests_passed=1; e.fuzzing_passed=1; e.sanitizers_passed=1; e.static_analysis_passed=1; e.benchmark_gain_verified=1; e.shadow_equivalence_passed=1; e.independent_review_approved=1; e.artifact_signature_valid=1; e.rollback_tested=1; e.validated_head_token=42u; e.current_head_token=42u;
 assert(pl_apsia_can_promote(&e)==1); e.current_head_token=43u; assert(pl_apsia_can_promote(&e)==0);
 e.current_head_token=42u; e.independent_review_approved=0; assert(pl_apsia_can_promote(&e)==0);
 puts("APSIA routing and promotion tests passed."); return 0;
}
