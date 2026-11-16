#include "apsia.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>

static pl_apsia_request base_request(void) {
    pl_apsia_request r = {0};
    r.workload = PL_APSIA_WORK_CPU;
    r.available_targets = UINT32_MAX;
    r.host_execution_ns = 1000.0;
    r.target_execution_ns = 300.0;
    r.ffi_overhead_ns = 100.0;
    r.ffi_calls_per_second = 100000.0;
    r.minimum_speedup = 1.5;
    r.maximum_ffi_cpu_cores = 0.25;
    return r;
}

int main(void) {
    pl_apsia_request r = base_request();
    pl_apsia_decision d;

    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.route_selected == 1u);
    assert(d.performance_approved == 1u);
    assert(d.request_rejected == 0u);
    assert(d.language == PL_APSIA_LANG_RUST);
    assert(d.estimated_speedup == 2.5);

    r.workload = PL_APSIA_WORK_GPU;
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.language == PL_APSIA_LANG_CUDA);

    r.workload = PL_APSIA_WORK_ANALYTICS;
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.language == PL_APSIA_LANG_SQL);

    r.workload = PL_APSIA_WORK_NETWORK_IO;
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.language == PL_APSIA_LANG_GO);

    r.sandbox_required = 1u;
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.language == PL_APSIA_LANG_WASM);
    assert(d.request_rejected == 0u);

    r.available_targets &= ~PL_APSIA_TARGET_BIT(PL_APSIA_LANG_WASM);
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.route_selected == 0u);
    assert(d.language == PL_APSIA_LANG_HOST);
    assert(d.request_rejected == 1u);

    r = base_request();
    r.workload = PL_APSIA_WORK_PLUGIN;
    r.available_targets &= ~PL_APSIA_TARGET_BIT(PL_APSIA_LANG_WASM);
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.route_selected == 0u);
    assert(d.request_rejected == 1u); /* plugins never fall back unsandboxed */

    r = base_request();
    r.ffi_overhead_ns = 600.0;
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.route_selected == 0u);
    assert(d.language == PL_APSIA_LANG_HOST);
    assert(d.request_rejected == 0u);

    r.sandbox_required = 1u;
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.route_selected == 0u);
    assert(d.request_rejected == 1u);

    r = base_request();
    r.ffi_calls_per_second = 1000000000.0;
    assert(pl_apsia_route(&r, &d) == 0);
    assert(d.route_selected == 0u);

    r = base_request();
    r.host_execution_ns = NAN;
    assert(pl_apsia_route(&r, &d) != 0);
    assert(d.language == PL_APSIA_LANG_HOST);
    assert(d.request_rejected == 1u);
    assert(pl_apsia_route(NULL, &d) != 0);
    assert(pl_apsia_route(&r, NULL) != 0);

    puts("APSIA routing tests passed.");
    return 0;
}
