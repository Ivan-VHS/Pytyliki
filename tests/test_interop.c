#include "interop.h"
#include "metric_registry.h"
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
    uint64_t plugin_count=0u;
    pytyliki_plugin_descriptor_v1 plugin;
    pytyliki_metric_query_v1 metric_query;
    pytyliki_metric_operation_v1 metric_results[2];
    pytyliki_metric_search_result_v1 metric_result;
    assert(pytyliki_abi_version()==PYTYLIKI_ABI_VERSION);

    /* The reverse Reaper bridge is optional, explicit and ABI-versioned. */
    if (pytyliki_v1_reaper_available() == 1) {
        const char expression[] = "1 + 2 * 3";
        char reaper_diagnostic[128] = {0};
        int64_t reaper_value = 0;
        uint64_t selfhood_ordinal = 0u;
        assert(pytyliki_v1_reaper_abi_version() == 1u);
        assert(pytyliki_v1_reaper_eval_i64(
            expression, (uint64_t)(sizeof expression - 1u),
            &reaper_value, reaper_diagnostic, sizeof reaper_diagnostic
        ) == 0);
        assert(reaper_value == 7);
        assert(pytyliki_v1_reaper_selfhood_lookup("Сам", &selfhood_ordinal) == 0);
        assert(selfhood_ordinal == 2u);
    } else {
        const char expression[] = "1 + 2";
        int64_t reaper_value = 0;
        assert(pytyliki_v1_reaper_abi_version() == 0u);
        assert(pytyliki_v1_reaper_eval_i64(
            expression, (uint64_t)(sizeof expression - 1u),
            &reaper_value, NULL, 0u
        ) == PYTYLIKI_ABI_BACKEND_UNAVAILABLE);
        uint64_t selfhood_ordinal = 0u;
        assert(pytyliki_v1_reaper_selfhood_lookup("Сам", &selfhood_ordinal) ==
               PYTYLIKI_ABI_BACKEND_UNAVAILABLE);
    }
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

    /* Plugin discovery is a metadata-only ABI: no plugin code is loaded or run. */
    assert(pytyliki_v1_plugin_count(&plugin_count)==PYTYLIKI_ABI_OK);
    assert(plugin_count==18u);
    assert(pytyliki_v1_plugin_at(0u,&plugin)==PYTYLIKI_ABI_OK);
    assert(strcmp(plugin.id,"operandu")==0);
    assert(pytyliki_v1_plugin_at(10u,&plugin)==PYTYLIKI_ABI_OK);
    assert(strcmp(plugin.id,"keel-koh")==0);
    assert(strcmp(plugin.name,"Keel Koh")==0);
    assert(pytyliki_v1_plugin_find("Keel Koh",&plugin)==PYTYLIKI_ABI_OK);
    assert(strcmp(plugin.id,"keel-koh")==0);
    assert(pytyliki_v1_plugin_find("PROFEVO",&plugin)==PYTYLIKI_ABI_OK);
    assert(strcmp(plugin.name,"Profevo")==0);
    assert(pytyliki_v1_plugin_find("missing",&plugin)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_plugin_at(plugin_count,&plugin)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_plugin_count(NULL)==PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_plugin_find(NULL,&plugin)==PYTYLIKI_ABI_INVALID_ARGUMENT);

    /* Typed jOOQ-style query parameters cross the versioned ABI without exposing C pointers. */
    memset(&metric_query, 0, sizeof(metric_query));
    /* Copy complete NUL-terminated literals into the fixed-size ABI fields. */
    memcpy(metric_query.query, "vector", sizeof("vector"));
    memcpy(metric_query.category, "vector-algebra", sizeof("vector-algebra"));
    metric_query.required_capabilities = PL_METRIC_CAP_STREAMING;
    metric_query.excluded_capabilities = PL_METRIC_CAP_ALLOCATES;
    metric_query.required_type_mask = 1u;
    metric_query.offset = 1u;
    metric_query.limit = 2u;
    assert(pytyliki_v1_metric_registry_search(
        &metric_query, metric_results, 2u, &metric_result
    ) == PYTYLIKI_ABI_OK);
    assert(metric_result.total_matches == 5u && metric_result.written == 2u);
    assert(strcmp(metric_results[0].name, "vector.norm.l2") == 0);
    assert(strcmp(metric_results[1].name, "vector.norm.l1") == 0);

    metric_query.offset = 0u;
    metric_query.limit = 1u;
    assert(pytyliki_v1_metric_registry_search(
        &metric_query, NULL, 0u, &metric_result
    ) == PYTYLIKI_ABI_OK);
    assert(metric_result.total_matches == 5u && metric_result.written == 0u);

    metric_query.limit = PL_METRIC_QUERY_MAX_PAGE + 1u;
    assert(pytyliki_v1_metric_registry_search(
        &metric_query, metric_results, 2u, &metric_result
    ) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    metric_query.limit = 1u;
    assert(pytyliki_v1_metric_registry_search(
        &metric_query, NULL, 2u, &metric_result
    ) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    memset(metric_query.query, 'x', sizeof(metric_query.query));
    assert(pytyliki_v1_metric_registry_search(
        &metric_query, metric_results, 2u, &metric_result
    ) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    return 0;
}
