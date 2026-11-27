#include "hardware_syntax.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const valid_constructs[PL_HW_SYNTAX_CONSTRUCT_COUNT] = {
    "table irq_dispatch { | IRQ_ID | ACTION_HOOK | }",
    "stream cpu_pulse from RING |> filter(x > 0) |> sink(OUT);",
    "stack_proc boot_guard(seed: u64) { PUSH_IMM 1; LOAD_REG R0; RET_TOP }",
    "fsm core_power_state initial IDLE { state IDLE: on tick -> goto IDLE }",
    "contract bus_alignment_rule for AXI_BUS { require addr % 64 == 0; }",
    ".section .text, \"ax\"\n.global foo\nfoo:\n    LDW R0, [R4 + 0]\n    RET\n",
    "react on cache.evict(line_addr) { ring_push(TRACE_BUF, line_addr); }",
    "matrix trace_mode_matrix { mode = [DRY]; cell (DRY, RING_0) => pin_registers(); }",
    "fn fold_addr_tree(addr: u64) -> u64 { return addr; }",
    "schedule quantum_wheel every 250_cycles { core[0]: slice(40_cycles) -> run_ingest(); }",
    "rule apocrypha_noise_gate priority 100 { when input.kind == NOISE then drop_raw_noise(input); }",
    "atomic_tx reg_bank_swap(bank_a: RegSet, bank_b: RegSet) { save_checkpoint(); rollback(); }",
    "dag pipeline_graph { node load_stage: load(); node save_stage: save(); load_stage -> save_stage; }",
    "macro_def vector_lane_shift(lane, count) { emit(lane); }",
    "hot_patch overlay_swap { detach old_overlay; attach new_overlay; }",
    "probe ipc_watch on core[0] window 1024_cycles { measure ipc = retired_insts / cycles; }",
    "device dma_ring_worker(ch: DmaChannel) { on ready { start_transfer(ch); } }",
    "invariant hardware_sanity { always temp_c < 95.0; }",
    "memory_map cpu_window_layout { slot REG_MIRROR @ 0x10000000 : size 4KB : attr [RW]; }",
    "trace_script step_debugger target core[0] { set_bp(0x80000120); continue_until_bp(); }"
};

static void test_catalog_matches_language_constructs(void) {
    const pl_hw_construct_info *catalog = NULL;
    size_t count = pl_hw_syntax_catalog(&catalog);
    size_t index;
    assert(count == PL_HW_SYNTAX_CONSTRUCT_COUNT);
    assert(catalog != NULL);
    for (index = 0u; index < count; ++index) {
        assert(catalog[index].keyword != NULL);
        assert(catalog[index].family != NULL);
        assert(catalog[index].execution_model != NULL);
        assert(catalog[index].safety_notes != NULL);
        assert(strcmp(pl_hw_construct_name((pl_hw_construct_kind)index),
                      catalog[index].keyword) == 0);
    }
}

static void test_all_twenty_constructs_parse(void) {
    size_t index;
    for (index = 0u; index < PL_HW_SYNTAX_CONSTRUCT_COUNT; ++index) {
        pl_hw_construct_node node;
        pl_hw_syntax_error error;
        assert(pl_hw_syntax_parse_one(valid_constructs[index], &node, &error));
        assert((unsigned)node.kind == index);
        assert(node.name[0] != '\0');
        assert(node.source_length == strlen(valid_constructs[index]));
    }
}

static void test_special_body_models(void) {
    pl_hw_construct_node node;
    pl_hw_syntax_error error;
    assert(pl_hw_syntax_parse_one(valid_constructs[1], &node, &error));
    assert(node.body_kind == PL_HW_BODY_PIPELINE);
    assert(pl_hw_syntax_parse_one("fn answer() -> u64 = 42;", &node, &error));
    assert(node.kind == PL_HW_CONSTRUCT_FUNCTION);
    assert(node.body_kind == PL_HW_BODY_EXPRESSION);
    assert(pl_hw_syntax_parse_one(valid_constructs[5], &node, &error));
    assert(node.body_kind == PL_HW_BODY_ASSEMBLY);
}

static void test_rejects_invalid_structure(void) {
    static const char *const invalid[] = {
        "",
        "unknown thing { }",
        "table { missing_name }",
        "table irq_dispatch {",
        "table irq_dispatch { value = (1 + 2]; }",
        "stream cpu_pulse from RING |> sink(OUT)",
        "device dma_worker { on ready { start(); } } trailing",
        "fsm state_machine { state READY: on tick -> goto READY;",
        "contract bus_rule { require aligned; } )",
        "hot_patch overlay { text = \"unterminated; }",
        "fn f() { return 1; } fn g() { return 2; }",
        "memory_map layout { slot A @ 0x1000 : size 4KB; } garbage"
    };
    size_t index;
    for (index = 0u; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        pl_hw_construct_node node;
        pl_hw_syntax_error error;
        assert(!pl_hw_syntax_parse_one(invalid[index], &node, &error));
        assert(error.message[0] != '\0');
    }
}

static void test_comments_strings_and_nesting(void) {
    const char *source =
        "device dma_worker(ch: DmaChannel) { // event handler\n"
        "  on ready { let label = \"brace } inside string\"; /* nested */ }\n"
        "}";
    pl_hw_construct_node node;
    pl_hw_syntax_error error;
    assert(pl_hw_syntax_parse_one(source, &node, &error));
    assert(node.kind == PL_HW_CONSTRUCT_DEVICE);
}

static void test_limits_and_null_arguments(void) {
    char *oversized = (char *)malloc(PL_HW_SYNTAX_MAX_SOURCE_BYTES + 2u);
    pl_hw_construct_node node;
    pl_hw_syntax_error error;
    assert(oversized != NULL);
    memset(oversized, 'x', PL_HW_SYNTAX_MAX_SOURCE_BYTES + 1u);
    oversized[PL_HW_SYNTAX_MAX_SOURCE_BYTES + 1u] = '\0';
    assert(!pl_hw_syntax_parse_one(oversized, &node, &error));
    assert(strstr(error.message, "one-megabyte") != NULL);
    free(oversized);
    assert(!pl_hw_syntax_parse_one(NULL, &node, &error));
    assert(!pl_hw_syntax_parse_one("table t { }", NULL, &error));
}

static void test_json_reports_structural_only_status(void) {
    pl_hw_construct_node node;
    pl_hw_syntax_error error;
    FILE *stream = tmpfile();
    char buffer[2048];
    size_t count;
    assert(stream != NULL);
    assert(pl_hw_syntax_parse_one(valid_constructs[0], &node, &error));
    assert(pl_hw_syntax_write_json(&node, stream));
    rewind(stream);
    count = fread(buffer, 1u, sizeof(buffer) - 1u, stream);
    buffer[count] = '\0';
    assert(strstr(buffer, PL_HW_SYNTAX_SCHEMA) != NULL);
    assert(strstr(buffer, "\"kind\":\"table\"") != NULL);
    assert(strstr(buffer, "\"source_execution_performed\":false") != NULL);
    fclose(stream);
}

int main(void) {
    test_catalog_matches_language_constructs();
    test_all_twenty_constructs_parse();
    test_special_body_models();
    test_rejects_invalid_structure();
    test_comments_strings_and_nesting();
    test_limits_and_null_arguments();
    test_json_reports_structural_only_status();
    puts("Pytyliki hardware construct syntax tests passed");
    return 0;
}
