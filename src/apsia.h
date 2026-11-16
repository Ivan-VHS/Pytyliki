#ifndef PYTYLIKI_APSIA_H
#define PYTYLIKI_APSIA_H
#include <stdint.h>
typedef enum { PL_APSIA_WORK_CPU_BOUND=0, PL_APSIA_WORK_GPU_TENSOR, PL_APSIA_WORK_UNTRUSTED_PLUGIN, PL_APSIA_WORK_CONCURRENT_IO, PL_APSIA_WORK_ANALYTICS, PL_APSIA_WORK_EMBEDDED_SCRIPT, PL_APSIA_WORK_COUNT } pl_apsia_workload;
typedef enum { PL_APSIA_LANG_HOST=0, PL_APSIA_LANG_RUST, PL_APSIA_LANG_CPP, PL_APSIA_LANG_ZIG, PL_APSIA_LANG_CUDA, PL_APSIA_LANG_WASM, PL_APSIA_LANG_GO, PL_APSIA_LANG_ELIXIR, PL_APSIA_LANG_SQL, PL_APSIA_LANG_DUCKDB, PL_APSIA_LANG_LUA, PL_APSIA_LANG_PYTHON } pl_apsia_language;
enum { PL_APSIA_CAP_NETWORK=1u<<0, PL_APSIA_CAP_FILESYSTEM=1u<<1, PL_APSIA_CAP_CLOCK=1u<<2, PL_APSIA_CAP_GPU=1u<<3, PL_APSIA_CAP_SHARED_MEMORY=1u<<4 };
typedef struct { pl_apsia_workload workload; double expected_speedup; double ffi_overhead_fraction; double host_optimization_fraction; uint32_t requested_capabilities; int gpu_available; int deterministic_required; int sandbox_required; } pl_apsia_request;
typedef struct { pl_apsia_language language; uint32_t granted_capabilities; int use_polyglot; int sandbox_required; int zero_copy_candidate; int human_review_required; const char *reason; } pl_apsia_decision;
/* Advisory only: never compiles, loads, or executes generated code. */
int pl_apsia_route(const pl_apsia_request *request, pl_apsia_decision *decision);
const char *pl_apsia_language_name(pl_apsia_language language);
typedef struct { int contract_valid; int differential_tests_passed; int fuzzing_passed; int sanitizers_passed; int static_analysis_passed; int benchmark_gain_verified; int shadow_equivalence_passed; int independent_review_approved; int artifact_signature_valid; int rollback_tested; uint64_t validated_head_token; uint64_t current_head_token; } pl_apsia_promotion_evidence;
/* Missing or stale evidence always blocks promotion. */
int pl_apsia_can_promote(const pl_apsia_promotion_evidence *evidence);
#endif
