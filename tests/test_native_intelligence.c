#include "native_intelligence.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void set_bit(uint64_t *mask, unsigned bit) {
    mask[bit / 64u] |= UINT64_C(1) << (bit % 64u);
}

static void fixture(pl_ni_snapshot *snapshot) {
    pl_ni_numa_node *node;
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->abi_version = PL_NI_ABI_VERSION;
    snapshot->cpu_count = 4u;
    snapshot->cache_count = 1u;
    snapshot->numa_count = 1u;
    snapshot->total_memory_bytes = UINT64_C(16) * 1024u * 1024u * 1024u;
    snapshot->available_memory_bytes = UINT64_C(8) * 1024u * 1024u * 1024u;
    snapshot->isa_mask = PL_NI_ISA_SSE2 | PL_NI_ISA_AVX;
    for (unsigned i = 0u; i < 4u; ++i) {
        snapshot->cpus[i].group_id = 0u;
        snapshot->cpus[i].os_cpu_id = (uint16_t)i;
        snapshot->cpus[i].package_id = 0u;
        snapshot->cpus[i].die_id = 0u;
        snapshot->cpus[i].core_id = (uint16_t)(i / 2u);
        snapshot->cpus[i].numa_node = 0u;
        snapshot->cpus[i].thread_index = (uint16_t)(i % 2u);
        snapshot->cpus[i].flags = PL_NI_CPU_ONLINE | PL_NI_CPU_ALLOWED |
                                  PL_NI_CPU_ALLOWED_KNOWN;
        snapshot->cpus[i].isa_mask = snapshot->isa_mask;
    }
    snapshot->caches[0].level = 3u;
    snapshot->caches[0].kind = 3u;
    snapshot->caches[0].size_bytes = 8u * 1024u * 1024u;
    snapshot->caches[0].line_bytes = 64u;
    snapshot->caches[0].sharing_count = 4u;
    for (unsigned i = 0u; i < 4u; ++i)
        set_bit(snapshot->caches[0].sharing_mask, i);
    node = &snapshot->numa[0];
    node->id = 0u;
    node->cpu_count = 4u;
    for (unsigned i = 0u; i < 4u; ++i) set_bit(node->cpu_mask, i);
    snapshot->numa_distance[0][0] = 10u;
    snapshot->provenance.schema_version = PL_NI_ABI_VERSION;
    snapshot->provenance.topology_state = PL_NI_FACT_KNOWN;
    snapshot->provenance.isa_state = PL_NI_FACT_KNOWN;
    snapshot->provenance.isa_scope = PL_NI_ISA_SCOPE_PER_LOGICAL_CPU;
    snapshot->provenance.memory_state = PL_NI_FACT_KNOWN;
    (void)snprintf(snapshot->provenance.platform, sizeof(snapshot->provenance.platform), "fixture");
    (void)snprintf(snapshot->provenance.topology_source, sizeof(snapshot->provenance.topology_source), "fixture-topology");
    (void)snprintf(snapshot->provenance.isa_source, sizeof(snapshot->provenance.isa_source), "fixture-isa");
    (void)snprintf(snapshot->provenance.memory_source, sizeof(snapshot->provenance.memory_source), "fixture-memory");
    snapshot->provenance.snapshot_fingerprint = pl_ni_snapshot_fingerprint(snapshot);
    snapshot->provenance.generation = snapshot->provenance.snapshot_fingerprint;
}

static void test_collector_is_read_only_and_returns_provenance(void) {
    pl_ni_snapshot snapshot;
    pl_ni_status status = pl_ni_collect(&snapshot);
    assert(status == PL_NI_OK);
    assert(snapshot.cpu_count != 0u);
    assert(snapshot.provenance.schema_version == PL_NI_ABI_VERSION);
    assert(snapshot.provenance.platform[0] != '\0');
    assert(snapshot.provenance.topology_source[0] != '\0');
    assert(snapshot.provenance.snapshot_fingerprint != 0u);
    assert(snapshot.provenance.generation == snapshot.provenance.snapshot_fingerprint);
    assert(pl_ni_snapshot_validate(&snapshot) == PL_NI_OK);
}

static void test_ppf_constraints_cost_and_tie_break(void) {
    pl_ni_snapshot snapshot;
    pl_ppf_request request;
    pl_ppf_telemetry telemetry[4];
    pl_ppf_report report;
    fixture(&snapshot);
    memset(&request, 0, sizeof(request));
    request.preferred_numa_node = 0u;
    request.current_cpu_index = 3u;
    request.required_isa = PL_NI_ISA_SSE2;
    request.require_online = 1u;
    request.require_allowed = 1u;
    request.require_known_allowed = 1u;
    request.exclusive_core = 1u;
    request.weights[0] = 4u;
    request.weights[1] = 1u;
    request.weights[2] = 10u;
    request.weights[3] = 3u;
    request.weights[4] = 1u;
    memset(telemetry, 0, sizeof(telemetry));
    for (unsigned i = 0u; i < 4u; ++i) {
        telemetry[i].valid_mask = PL_PPF_TELEMETRY_CACHE |
                                  PL_PPF_TELEMETRY_BANDWIDTH |
                                  PL_PPF_TELEMETRY_MIGRATION;
        telemetry[i].cache_pressure = 100u;
        telemetry[i].bandwidth_pressure = 50u;
        telemetry[i].migration_cost = i == 0u ? 0u : 200u;
    }
    set_bit(request.occupied_cpu_mask, 0u);
    assert(pl_ppf_place(&snapshot, &request, telemetry, 4u, &report) == PL_NI_OK);
    assert(report.candidate_count == 4u);
    assert(report.has_selection == 1u);
    assert(report.selected_cpu_index == 2u);
    assert(report.topology_generation == snapshot.provenance.generation);
    assert(report.candidates[0].rejection == PL_NI_ACCESS_DENIED);
    assert(report.candidates[1].rejection == PL_NI_ACCESS_DENIED);
    telemetry[2].cache_pressure = 100u;
    telemetry[2].migration_cost = 1000u;
    telemetry[0].cache_pressure = 300u;
    telemetry[1].migration_cost = 0u;
    memset(request.occupied_cpu_mask, 0, sizeof(request.occupied_cpu_mask));
    request.exclusive_core = 0u;
    assert(pl_ppf_place(&snapshot, &request, telemetry, 4u, &report) == PL_NI_OK);
    assert(report.has_selection == 1u);
    assert(report.selected_cpu_index == 1u);
    request.current_cpu_index = UINT16_MAX;
    assert(pl_ppf_place(&snapshot, &request, NULL, 0u, &report) == PL_NI_OK);
    assert(report.has_selection == 1u);
    assert(report.candidates[report.selected_cpu_index].bandwidth_cost == 500u);
    assert(report.candidates[report.selected_cpu_index].migration_cost == 500u);
    assert((report.candidates[report.selected_cpu_index].evidence_flags &
            PL_PPF_EVIDENCE_MISSING_TELEMETRY) != 0u);
}

static void test_typed_ranges_and_register_access(void) {
    pl_native_range range = {0x1000u, 0x100u, 32u, 16u};
    pl_native_register reg;
    uint64_t address = 0u;
    assert(pl_native_range_validate(&range) == PL_NI_OK);
    range.base = UINT64_C(0xFFFFFFF0);
    range.length = 64u;
    assert(pl_native_range_validate(&range) == PL_NI_OUT_OF_RANGE);
    range.address_bits = 64u;
    range.base = UINT64_MAX - 7u;
    assert(pl_native_range_validate(&range) == PL_NI_INVALID_ARGUMENT);

    memset(&reg, 0, sizeof(reg));
    reg.base = 0x4000u;
    reg.size = 0x100u;
    reg.allowed_value_mask = 0xFFu;
    reg.generation = 7u;
    reg.address_bits = 32u;
    reg.width_bits = 32u;
    reg.alignment = 4u;
    reg.access = PL_NATIVE_ACCESS_READ | PL_NATIVE_ACCESS_WRITE;
    reg.address_space = PL_NATIVE_SPACE_MMIO;
    assert(pl_native_register_validate(&reg, 4u, 1u, 0xA5u, 7u, &address) == PL_NI_OK);
    assert(address == 0x4004u);
    assert(pl_native_register_validate(&reg, 4u, 1u, 0x100u, 7u, &address) == PL_NI_ACCESS_DENIED);
    assert(pl_native_register_validate(&reg, 4u, 1u, 1u, 6u, &address) == PL_NI_STALE_GENERATION);
    assert(pl_native_register_validate(&reg, 0xFFu, 0u, 0u, 7u, &address) == PL_NI_OUT_OF_RANGE);
    reg.access = PL_NATIVE_ACCESS_READ;
    assert(pl_native_register_validate(&reg, 0u, 1u, 0u, 7u, &address) == PL_NI_ACCESS_DENIED);
    reg.base = UINT64_MAX - 3u;
    reg.size = 4u;
    reg.alignment = 4u;
    assert(pl_native_register_validate(&reg, 0u, 0u, 0u, 7u, &address) == PL_NI_INVALID_ARGUMENT);
}

static void test_dma_ownership_and_generations(void) {
    pl_dma_resource resource = {
        UINT64_C(42), UINT64_C(9), UINT64_C(0x100000),
        UINT64_C(4096), PL_DMA_CPU_OWNED, 2u
    };
    assert(pl_dma_cpu_access_validate(&resource, 9u) == PL_NI_OK);
    assert(pl_dma_transition(&resource, 9u, PL_DMA_DEVICE_OWNED) == PL_NI_OK);
    assert(resource.generation == 10u);
    assert(pl_dma_cpu_access_validate(&resource, 10u) == PL_NI_ACCESS_DENIED);
    assert(pl_dma_transition(&resource, 9u, PL_DMA_COMPLETED) == PL_NI_STALE_GENERATION);
    assert(pl_dma_transition(&resource, 10u, PL_DMA_COMPLETED) == PL_NI_OK);
    assert(pl_dma_cpu_access_validate(&resource, 11u) == PL_NI_ACCESS_DENIED);
    assert(pl_dma_transition(&resource, 11u, PL_DMA_CPU_OWNED) == PL_NI_OK);
    assert(pl_dma_cpu_access_validate(&resource, 12u) == PL_NI_OK);
    assert(pl_dma_transition(&resource, 12u, PL_DMA_UNMAPPED) == PL_NI_OK);
    assert(pl_dma_cpu_access_validate(&resource, 13u) == PL_NI_ACCESS_DENIED);
}

int main(void) {
    test_collector_is_read_only_and_returns_provenance();
    test_ppf_constraints_cost_and_tie_break();
    test_typed_ranges_and_register_access();
    test_dma_ownership_and_generations();
    puts("native intelligence collector, placement and resource tests passed");
    return 0;
}
