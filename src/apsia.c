#include "apsia.h"
#include <math.h>
const char *pl_apsia_language_name(pl_apsia_language language) {
 static const char *const names[]={"host","Rust","C++","Zig","CUDA","WebAssembly","Go","Elixir","SQL","DuckDB","Lua","Python"};
 if ((int)language<0 || language>PL_APSIA_LANG_PYTHON) return "unknown";
 return names[(unsigned)language];
}
int pl_apsia_route(const pl_apsia_request *r, pl_apsia_decision *d) {
 const uint32_t known_caps=PL_APSIA_CAP_NETWORK|PL_APSIA_CAP_FILESYSTEM|PL_APSIA_CAP_CLOCK|PL_APSIA_CAP_GPU|PL_APSIA_CAP_SHARED_MEMORY;
 pl_apsia_language selected=PL_APSIA_LANG_HOST; const char *reason="retain host implementation"; int sandbox=0;
 if (!r || !d) return -1;
 if ((int)r->workload<0 || r->workload>=PL_APSIA_WORK_COUNT || !isfinite(r->expected_speedup) || !isfinite(r->ffi_overhead_fraction) || !isfinite(r->host_optimization_fraction) || r->expected_speedup<0.0 || r->ffi_overhead_fraction<0.0 || r->ffi_overhead_fraction>1.0 || r->host_optimization_fraction<0.0 || r->host_optimization_fraction>1.0 || (r->requested_capabilities&~known_caps)!=0u || (r->gpu_available!=0 && r->gpu_available!=1) || (r->deterministic_required!=0 && r->deterministic_required!=1) || (r->sandbox_required!=0 && r->sandbox_required!=1)) return -1;
 /* Isolation is a requirement, not a performance optimization: force the
  * sandbox backend whenever the caller explicitly requests a sandbox. */
 if (r->sandbox_required || r->workload==PL_APSIA_WORK_UNTRUSTED_PLUGIN) {
  selected=PL_APSIA_LANG_WASM; sandbox=1; reason="sandbox required: WebAssembly candidate";
 } else if (r->host_optimization_fraction>=0.70) reason="host optimization preferred: >=70% of opportunity";
 else if (r->expected_speedup<=1.0+r->ffi_overhead_fraction) reason="estimated gain does not exceed FFI overhead";
 else switch(r->workload) {
 case PL_APSIA_WORK_CPU_BOUND: selected=PL_APSIA_LANG_RUST; reason="CPU-bound workload: Rust candidate"; break;
 case PL_APSIA_WORK_GPU_TENSOR:
  if (r->deterministic_required) reason="deterministic result required: retain host implementation";
  else if (r->gpu_available && (r->requested_capabilities&PL_APSIA_CAP_GPU)) { selected=PL_APSIA_LANG_CUDA; reason="GPU explicitly available: CUDA candidate"; }
  else reason="GPU backend unavailable: retain host";
  break;
 case PL_APSIA_WORK_CONCURRENT_IO: selected=PL_APSIA_LANG_GO; reason="concurrent I/O workload: Go candidate"; break;
 case PL_APSIA_WORK_ANALYTICS: selected=PL_APSIA_LANG_DUCKDB; reason="analytical workload: DuckDB candidate"; break;
 case PL_APSIA_WORK_EMBEDDED_SCRIPT: selected=PL_APSIA_LANG_LUA; reason="embedded scripting workload: Lua candidate"; break;
 default: return -1;
 }
 d->language=selected; d->use_polyglot=(selected!=PL_APSIA_LANG_HOST); d->sandbox_required=sandbox; d->human_review_required=1;
 d->zero_copy_candidate=(d->use_polyglot && (r->requested_capabilities&PL_APSIA_CAP_SHARED_MEMORY)!=0u);
 /* Routing never grants capabilities. A separate reviewed broker must do so. */
 d->granted_capabilities=0u; d->reason=reason; return 0;
}
int pl_apsia_can_promote(const pl_apsia_promotion_evidence *e) {
 if (!e || !e->validated_head_token || !e->current_head_token || e->validated_head_token!=e->current_head_token) return 0;
 return e->contract_valid==1 && e->differential_tests_passed==1 && e->fuzzing_passed==1 && e->sanitizers_passed==1 && e->static_analysis_passed==1 && e->benchmark_gain_verified==1 && e->shadow_equivalence_passed==1 && e->independent_review_approved==1 && e->artifact_signature_valid==1 && e->rollback_tested==1;
}
