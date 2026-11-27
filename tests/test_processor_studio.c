#include "processor_studio.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define ALL_RIGHTS PL_PROCESSOR_STUDIO_RIGHT_ALL

static void expect_status(pl_processor_studio_status actual,
                          pl_processor_studio_status expected) {
    if (actual != expected) {
        fprintf(stderr, "expected %s, received %s\n",
                pl_processor_studio_status_name(expected),
                pl_processor_studio_status_name(actual));
        assert(actual == expected);
    }
}

static void initialize(pl_processor_studio *studio,
                       pl_processor_studio_config *config) {
    pl_processor_studio_error error;
    pl_processor_studio_default_config(config);
    expect_status(pl_processor_studio_init(studio, config, &error), PL_PROCESSOR_STUDIO_OK);
}

static void test_catalog(void) {
    size_t count = 0u;
    size_t index;
    const pl_processor_studio_domain_descriptor *catalog = pl_processor_studio_catalog(&count);
    assert(catalog != NULL);
    assert(count == PL_PROCESSOR_STUDIO_DOMAIN_COUNT);
    assert(strcmp(catalog[0].environment, "Environment") == 0);
    assert(strcmp(catalog[19].environment, "Runtime environment") == 0);
    for (index = 0u; index < count; index++) {
        assert(catalog[index].id == index + 1u);
        assert(catalog[index].seed == ((uint64_t)(index + 1u) * UINT64_C(0x0101)));
        assert(catalog[index].environment[0] != '\0');
        assert(catalog[index].engineering_focus[0] != '\0');
        assert(catalog[index].organization[0] != '\0');
    }
    assert(catalog[1].transform == PL_PROCESSOR_STUDIO_ROTL_XOR);
    assert(catalog[1].rotation == 3u);
    assert(catalog[19].rotation == 15u);
}

static void test_layout_validation(void) {
    pl_processor_studio_config config;
    pl_processor_studio_error error;
    pl_processor_studio_default_config(&config);
    expect_status(pl_processor_studio_validate_layout(&config.layout, &error), PL_PROCESSOR_STUDIO_OK);

    config.layout.overlay_slot = config.layout.tray_slot;
    expect_status(pl_processor_studio_validate_layout(&config.layout, &error), PL_PROCESSOR_STUDIO_REGION_OVERLAP);

    pl_processor_studio_default_config(&config);
    config.layout.domain_table += 1u;
    expect_status(pl_processor_studio_validate_layout(&config.layout, &error), PL_PROCESSOR_STUDIO_INVALID_LAYOUT);

    pl_processor_studio_default_config(&config);
    config.layout.domain_table_size--;
    expect_status(pl_processor_studio_validate_layout(&config.layout, &error), PL_PROCESSOR_STUDIO_INVALID_LAYOUT);

    pl_processor_studio_default_config(&config);
    config.layout.arena_base = UINT64_MAX - UINT64_C(0xFFF);
    config.layout.arena_size = UINT64_C(0x2000);
    expect_status(pl_processor_studio_validate_layout(&config.layout, &error), PL_PROCESSOR_STUDIO_ADDRESS_OVERFLOW);
}

static void test_full_domain_commit_and_plan(void) {
    pl_processor_studio studio;
    pl_processor_studio_config config;
    pl_processor_studio_report report;
    pl_processor_studio_error error;
    pl_processor_studio_assignment plan[PL_PROCESSOR_STUDIO_DOMAIN_COUNT];
    size_t count = 0u;
    size_t worker_counts[4] = {0u, 0u, 0u, 0u};
    size_t index;
    char json[32768];
    size_t written = 0u;

    initialize(&studio, &config);
    expect_status(pl_processor_studio_begin(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_plan(&studio, PL_PROCESSOR_STUDIO_DOMAIN_MASK,
                                           ALL_RIGHTS, &count, &error), PL_PROCESSOR_STUDIO_OK);
    assert(count == PL_PROCESSOR_STUDIO_DOMAIN_COUNT);
    expect_status(pl_processor_studio_execute(&studio, PL_PROCESSOR_STUDIO_DOMAIN_MASK,
                                              ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_commit(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);

    expect_status(pl_processor_studio_get_report(&studio, ALL_RIGHTS, &report, &error), PL_PROCESSOR_STUDIO_OK);
    assert(report.generation == 2u);
    assert(report.requested_domain_mask == PL_PROCESSOR_STUDIO_DOMAIN_MASK);
    assert(report.completed_domain_mask == PL_PROCESSOR_STUDIO_DOMAIN_MASK);
    assert(report.committed_domain_mask == PL_PROCESSOR_STUDIO_DOMAIN_MASK);
    assert(report.domain_fold != 0u);
    assert(report.sweep_digest != 0u);
    assert(report.tournament_value != 0u);
    assert(report.tournament_domain == 0u || report.tournament_domain == 17u || report.tournament_domain == 20u);
    assert(report.predicate_nonzero == 1u);
    assert(report.registers.r[0] == 20u);
    assert(report.registers.r[1] == (report.registers.regit & UINT64_C(0xFF)));
    assert(report.registers.r[2] == UINT64_C(0x1414));
    assert(report.registers.r[5] == report.registers.r[3] + report.registers.r[4]);
    assert(report.registers.r[6] == (report.registers.r[3] >= report.registers.r[4]
        ? report.registers.r[3] : report.registers.r[4]));
    assert(report.registers.regit == report.tournament_value);
    assert(report.registers.planned_jump_target == PL_PROCESSOR_STUDIO_PLANNED_JUMP_TARGET);
    assert(report.registers.flags == 1u);
    assert(report.planned_assignment_count == PL_PROCESSOR_STUDIO_DOMAIN_COUNT);
    assert(report.physical_memory_accessed == 0u);
    assert(report.mmio_accessed == 0u);
    assert(report.dma_submitted == 0u);
    assert(report.threads_spawned == 0u);

    expect_status(pl_processor_studio_copy_plan(&studio, ALL_RIGHTS, plan,
                                                PL_PROCESSOR_STUDIO_DOMAIN_COUNT,
                                                &count, &error), PL_PROCESSOR_STUDIO_OK);
    assert(count == PL_PROCESSOR_STUDIO_DOMAIN_COUNT);
    for (index = 0u; index < count; index++) {
        assert(plan[index].domain_id == index + 1u);
        assert(plan[index].worker_id < 4u);
        worker_counts[plan[index].worker_id]++;
    }
    for (index = 0u; index < 4u; index++) {
        assert(worker_counts[index] == 5u);
    }

    expect_status(pl_processor_studio_report_json(&studio, ALL_RIGHTS,
                                                  json, sizeof(json), &written, &error), PL_PROCESSOR_STUDIO_OK);
    assert(written > 1000u);
    assert(strstr(json, "pytyliki.processor-studio-report.v1") != NULL);
    assert(strstr(json, "pytyliki.reaper-processor-studio.v1") != NULL);
    assert(strstr(json, "\"domain_count\":20") != NULL);
    assert(strstr(json, "\"threads_spawned\":false") != NULL);
    assert(strstr(json, "\"mmio_accessed\":false") != NULL);
    assert(strstr(json, "\"regit\":") != NULL);
    assert(strstr(json, "\"assignments\":[") != NULL);
    assert(json[written] == '\0');
}

static void test_rights_and_generation_guards(void) {
    pl_processor_studio studio;
    pl_processor_studio_config config;
    pl_processor_studio_error error;
    initialize(&studio, &config);

    expect_status(pl_processor_studio_begin(&studio, 0u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_GENERATION_CONFLICT);
    config.root_rights = PL_PROCESSOR_STUDIO_RIGHT_INSPECT;
    expect_status(pl_processor_studio_init(&studio, &config, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_begin(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_PERMISSION_DENIED);
    expect_status(pl_processor_studio_get_report(&studio, PL_PROCESSOR_STUDIO_RIGHT_EXECUTE,
                                                 &(pl_processor_studio_report){0}, &error), PL_PROCESSOR_STUDIO_PERMISSION_DENIED);
}

static void test_partial_run_rollback_and_stale_commit(void) {
    pl_processor_studio studio;
    pl_processor_studio_config config;
    pl_processor_studio_error error;
    pl_processor_studio_report report;
    size_t count = 0u;
    const uint32_t partial_mask = (UINT32_C(1) << 0u) | (UINT32_C(1) << 19u);
    initialize(&studio, &config);

    expect_status(pl_processor_studio_begin(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_begin(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_TRANSACTION_ALREADY_OPEN);
    expect_status(pl_processor_studio_plan(&studio, partial_mask, ALL_RIGHTS, &count, &error), PL_PROCESSOR_STUDIO_OK);
    assert(count == 2u);
    expect_status(pl_processor_studio_execute(&studio, partial_mask, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    studio.generation = 2u;
    expect_status(pl_processor_studio_commit(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_GENERATION_CONFLICT);
    studio.generation = 1u;
    expect_status(pl_processor_studio_rollback(&studio, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_get_report(&studio, ALL_RIGHTS, &report, &error), PL_PROCESSOR_STUDIO_OK);
    assert(report.generation == 1u);
    assert(report.committed_domain_mask == 0u);
    assert(report.planned_assignment_count == 0u);

    expect_status(pl_processor_studio_begin(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_execute(&studio, UINT32_C(1) << 20u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_INVALID_DOMAIN_MASK);
    expect_status(pl_processor_studio_rollback(&studio, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
}

static void test_plan_mismatch_and_generation_exhaustion(void) {
    pl_processor_studio studio;
    pl_processor_studio_config config;
    pl_processor_studio_error error;
    size_t count = 0u;
    const uint32_t first_domain = UINT32_C(1);
    const uint32_t second_domain = UINT32_C(2);

    initialize(&studio, &config);
    expect_status(pl_processor_studio_begin(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_plan(&studio, first_domain, ALL_RIGHTS, &count, &error), PL_PROCESSOR_STUDIO_OK);
    expect_status(pl_processor_studio_execute(&studio, second_domain, ALL_RIGHTS, &error),
                  PL_PROCESSOR_STUDIO_INVALID_DOMAIN_MASK);
    expect_status(pl_processor_studio_rollback(&studio, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);

    studio.generation = UINT32_MAX;
    expect_status(pl_processor_studio_begin(&studio, UINT32_MAX, ALL_RIGHTS, &error),
                  PL_PROCESSOR_STUDIO_GENERATION_EXHAUSTED);
}

static void test_trace_ring_and_small_buffers(void) {
    pl_processor_studio studio;
    pl_processor_studio_config config;
    pl_processor_studio_error error;
    pl_processor_studio_trace_record trace[PL_PROCESSOR_STUDIO_MAX_TRACE];
    pl_processor_studio_report report;
    size_t count = 0u;
    size_t index;
    char small[8];
    size_t written = 123u;

    initialize(&studio, &config);
    for (index = 0u; index < 40u; index++) {
        expect_status(pl_processor_studio_begin(&studio, 1u, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
        expect_status(pl_processor_studio_rollback(&studio, ALL_RIGHTS, &error), PL_PROCESSOR_STUDIO_OK);
    }
    expect_status(pl_processor_studio_get_report(&studio, ALL_RIGHTS, &report, &error), PL_PROCESSOR_STUDIO_OK);
    assert(report.trace_dropped > 0u);
    expect_status(pl_processor_studio_copy_trace(&studio, ALL_RIGHTS, trace,
                                                 PL_PROCESSOR_STUDIO_MAX_TRACE, &count, &error), PL_PROCESSOR_STUDIO_OK);
    assert(count == PL_PROCESSOR_STUDIO_MAX_TRACE);
    for (index = 1u; index < count; index++) {
        assert(trace[index].sequence > trace[index - 1u].sequence);
    }
    expect_status(pl_processor_studio_copy_trace(&studio, ALL_RIGHTS, trace, 1u,
                                                 &count, &error), PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL);

    expect_status(pl_processor_studio_report_json(&studio, ALL_RIGHTS, small,
                                                  sizeof(small), &written, &error), PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL);
    assert(written == 0u);
    assert(small[0] == '\0');
}

int main(void) {
    test_catalog();
    test_layout_validation();
    test_full_domain_commit_and_plan();
    test_rights_and_generation_guards();
    test_partial_run_rollback_and_stale_commit();
    test_plan_mismatch_and_generation_exhaustion();
    test_trace_ring_and_small_buffers();
    puts("Pytyliki 20-domain Processor Studio tests passed");
    return 0;
}
