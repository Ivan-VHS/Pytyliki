#include "interop.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <string.h>
int main(void) {
    const double a[]={3.0,4.0}, b[]={0.0,0.0}, y[]={0.0,1.0};
    double value=0.0; int32_t orthogonal=0;
    pytyliki_embedded_target_v1 embedded_target;
    pytyliki_embedded_source_report_v1 embedded_report;
    const char embedded_source[] = "#include <stdint.h>\nvolatile int *GPIO;\n";
    assert(pytyliki_abi_version()==PYTYLIKI_ABI_VERSION);
    assert(pytyliki_v1_norm_f64(a,2u,&value)==0&&fabs(value-5.0)<1e-12);
    assert(pytyliki_v1_dot_f64(a,y,2u,&value)==0&&fabs(value-4.0)<1e-12);
    assert(pytyliki_v1_distance_f64(a,b,2u,&value)==0&&fabs(value-5.0)<1e-12);
    assert(pytyliki_v1_orthogonal_f64((double[]){1.0,0.0},y,2u,1e-12,&orthogonal)==0&&orthogonal==1);
    assert(pytyliki_v1_embedded_target_count() >= 7u);
    assert(pytyliki_v1_embedded_target_at(0u, &embedded_target) == PYTYLIKI_ABI_OK);
    assert(strcmp(embedded_target.id, "native-c11") == 0);
    assert(pytyliki_v1_embedded_target_at(pytyliki_v1_embedded_target_count(), &embedded_target) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_embedded_inspect_source(embedded_source, sizeof(embedded_source) - 1u, &embedded_report) == PYTYLIKI_ABI_OK);
    assert(embedded_report.include_count == 1u && embedded_report.register_markers >= 2u);
    assert(pytyliki_v1_embedded_inspect_source(NULL, 0u, &embedded_report) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_norm_f64(NULL,2u,&value)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_norm_f64(a,4097u,&value)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_distance_f64(a,b,2u,NULL)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    return 0;
}
