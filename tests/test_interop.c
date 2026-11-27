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
    uint64_t embedded_count=0u;
    pytyliki_embedded_profile_v1 embedded_profile;
    pytyliki_embedded_sections_v1 embedded_sections={1024u,256u,128u,512u,64u,2048u};
    pytyliki_embedded_budget_v1 embedded_budget;
    assert(pytyliki_abi_version()==PYTYLIKI_ABI_VERSION);
    assert(pytyliki_v1_norm_f64(a,2u,&value)==0&&fabs(value-5.0)<1e-12);
    assert(pytyliki_v1_dot_f64(a,y,2u,&value)==0&&fabs(value-4.0)<1e-12);
    assert(pytyliki_v1_distance_f64(a,b,2u,&value)==0&&fabs(value-5.0)<1e-12);
    assert(pytyliki_v1_orthogonal_f64((double[]){1.0,0.0},y,2u,1e-12,&orthogonal)==0&&orthogonal==1);
    assert(pytyliki_v1_embedded_profile_count(&embedded_count)==PYTYLIKI_ABI_OK);
    assert(embedded_count==3u);
    assert(pytyliki_v1_embedded_profile_at(0u,&embedded_profile)==PYTYLIKI_ABI_OK);
    assert(strcmp(embedded_profile.id,"cortex-m4-reference")==0);
    assert(embedded_profile.clock_hz==UINT64_C(84000000));
    assert(embedded_profile.flash_size_bytes==UINT64_C(512)*1024u);
    assert(pytyliki_v1_embedded_profile_at(embedded_count,&embedded_profile)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_embedded_budget_evaluate("cortex-m4-reference",&embedded_sections,&embedded_budget)==PYTYLIKI_ABI_OK);
    assert(embedded_budget.flash_used_bytes==1408u && embedded_budget.flash_fits==1);
    assert(embedded_budget.sram_used_bytes==2752u && embedded_budget.sram_fits==1);
    assert(pytyliki_v1_embedded_budget_evaluate("unknown",&embedded_sections,&embedded_budget)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_embedded_budget_evaluate("cortex-m4-reference",NULL,&embedded_budget)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_embedded_profile_count(NULL)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_norm_f64(NULL,2u,&value)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_norm_f64(a,4097u,&value)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_distance_f64(a,b,2u,NULL)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    {
        int32_t available = -1;
        int64_t native_result = 987;
        uint64_t native_ordinal = 0u;
        char native_error[128] = {0};
        assert(pytyliki_v1_reaper_available(&available) == PYTYLIKI_ABI_OK);
#ifdef PYTYLIKI_HAS_REAPER
        assert(available == 1);
        assert(pytyliki_v1_reaper_eval_i64("6 * 7", 5u, &native_result,
                                          native_error, sizeof native_error) == PYTYLIKI_ABI_OK);
        assert(native_result == 42);
        assert(pytyliki_v1_reaper_selfhood_lookup("Сам", &native_ordinal) == PYTYLIKI_ABI_OK);
        assert(native_ordinal == 2u);
        assert(pytyliki_v1_reaper_selfhood_lookup("not-found", &native_ordinal) == PYTYLIKI_ABI_NOT_FOUND);
#else
        assert(available == 0);
        assert(pytyliki_v1_reaper_eval_i64("6 * 7", 5u, &native_result,
                                          native_error, sizeof native_error) == PYTYLIKI_ABI_BACKEND_UNAVAILABLE);
        assert(native_result == 987);
        assert(strstr(native_error, "REAPER_SOURCE_DIR") != NULL);
        assert(pytyliki_v1_reaper_selfhood_lookup("Сам", &native_ordinal) == PYTYLIKI_ABI_BACKEND_UNAVAILABLE);
#endif
        assert(pytyliki_v1_reaper_available(NULL) == PYTYLIKI_ABI_INVALID_ARGUMENT);
        assert(pytyliki_v1_reaper_selfhood_lookup(NULL, &native_ordinal) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    }
    return 0;
}
