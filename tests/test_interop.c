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
    pl_ni_snapshot native_snapshot;
    pl_ppf_request placement_request;
    pl_ppf_report placement_report;
    uint32_t native_contract_version = 0u;
    uint64_t native_structure_size = 0u;
    pl_native_range native_range = {UINT64_C(0x1000), 64u, 32u, 16u};
    pl_native_register native_register;
    pl_dma_resource dma_resource = {
        UINT64_C(9), UINT64_C(1), UINT64_C(0x2000),
        UINT64_C(128), PL_DMA_CPU_OWNED, 1u
    };
    uint64_t native_address = 0u;
    assert(pytyliki_abi_version()==PYTYLIKI_ABI_VERSION);
    assert(pytyliki_v1_norm_f64(a,2u,&value)==0&&fabs(value-5.0)<1e-12);
    assert(pytyliki_v1_dot_f64(a,y,2u,&value)==0&&fabs(value-4.0)<1e-12);
    assert(pytyliki_v1_distance_f64(a,b,2u,&value)==0&&fabs(value-5.0)<1e-12);

    /* The native hardware FFI is read-only and caller-owned. Placement is a pure proposal. */
    assert(pytyliki_v1_native_snapshot_descriptor(
        &native_contract_version, &native_structure_size) == PYTYLIKI_ABI_OK);
    assert(native_contract_version == PL_NI_ABI_VERSION);
    assert(native_structure_size == (uint64_t)sizeof(native_snapshot));
    assert(pytyliki_v1_native_snapshot_collect(
        &native_snapshot, native_structure_size) == PYTYLIKI_ABI_OK);
    assert(native_snapshot.abi_version == PL_NI_ABI_VERSION);
    assert(pytyliki_v1_native_snapshot_collect(
        &native_snapshot, native_structure_size - 1u) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    memset(&placement_request, 0, sizeof(placement_request));
    placement_request.preferred_numa_node = UINT16_MAX;
    placement_request.current_cpu_index = UINT16_MAX;
    placement_request.require_online = 1u;
    for (size_t wi = 0u; wi < 5u; ++wi) placement_request.weights[wi] = 1u;
    assert(pytyliki_v1_native_placement(
        &native_snapshot, &placement_request, NULL, 0u, &placement_report
    ) == PYTYLIKI_ABI_OK);
    assert(placement_report.candidate_count == native_snapshot.cpu_count);
    assert(placement_report.has_selection == 1u);
    assert(placement_report.topology_generation == native_snapshot.provenance.generation);
    assert(pytyliki_v1_native_snapshot_descriptor(NULL, &native_structure_size) ==
        PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_native_range_validate(&native_range) == PYTYLIKI_ABI_OK);
    native_range.base = UINT64_MAX - 3u;
    native_range.length = 8u;
    assert(pytyliki_v1_native_range_validate(&native_range) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    memset(&native_register, 0, sizeof(native_register));
    native_register.base = UINT64_C(0x4000);
    native_register.size = UINT64_C(0x100);
    native_register.allowed_value_mask = UINT64_C(0xff);
    native_register.generation = 3u;
    native_register.address_bits = 32u;
    native_register.width_bits = 32u;
    native_register.alignment = 4u;
    native_register.access = PL_NATIVE_ACCESS_READ | PL_NATIVE_ACCESS_WRITE;
    native_register.address_space = PL_NATIVE_SPACE_MMIO;
    assert(pytyliki_v1_native_register_validate(
        &native_register, 4u, 1u, 0x5au, 3u, &native_address
    ) == PYTYLIKI_ABI_OK);
    assert(native_address == UINT64_C(0x4004));
    assert(pytyliki_v1_native_register_validate(
        &native_register, 4u, 1u, 0x100u, 3u, &native_address
    ) == PYTYLIKI_ABI_ACCESS_DENIED);
    assert(pytyliki_v1_native_dma_cpu_access_validate(&dma_resource, 1u) == PYTYLIKI_ABI_OK);
    assert(pytyliki_v1_native_dma_transition(
        &dma_resource, 1u, PL_DMA_DEVICE_OWNED
    ) == PYTYLIKI_ABI_OK);
    assert(pytyliki_v1_native_dma_cpu_access_validate(&dma_resource, 2u) == PYTYLIKI_ABI_ACCESS_DENIED);
    assert(pytyliki_v1_native_dma_transition(
        &dma_resource, 2u, PL_DMA_COMPLETED
    ) == PYTYLIKI_ABI_OK);
    assert(pytyliki_v1_native_dma_transition(
        &dma_resource, 3u, PL_DMA_CPU_OWNED
    ) == PYTYLIKI_ABI_OK);
    assert(pytyliki_v1_native_dma_cpu_access_validate(&dma_resource, 4u) == PYTYLIKI_ABI_OK);
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
