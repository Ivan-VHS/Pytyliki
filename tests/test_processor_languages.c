#include "processor_languages.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_catalogue_is_exact_and_unique(void) {
    const pl_language_profile *items = NULL;
    const size_t count = pl_language_catalog(&items);
    assert(count == PL_LANGUAGE_COUNT);
    assert(count == 66u);
    assert(items != NULL);

    for (size_t i = 0u; i < count; ++i) {
        assert(items[i].id != NULL && items[i].id[0] != '\0');
        assert(items[i].name != NULL && items[i].name[0] != '\0');
        assert(items[i].family != NULL && items[i].family[0] != '\0');
        assert(items[i].extension != NULL && items[i].extension[0] == '.');
        assert(items[i].toolchain != NULL && items[i].toolchain[0] != '\0');
        assert(items[i].support_note != NULL && items[i].support_note[0] != '\0');
        for (size_t j = i + 1u; j < count; ++j) {
            assert(strcmp(items[i].id, items[j].id) != 0);
            assert(strcmp(items[i].name, items[j].name) != 0);
            assert(strcmp(items[i].extension, items[j].extension) != 0);
        }
        assert(pl_language_find(items[i].id) == &items[i]);
        assert(pl_language_find(items[i].name) == &items[i]);
    }
    assert(pl_language_catalog(NULL) == 0u);
    assert(pl_language_find("rUsT") == pl_language_find("sys-rust"));
    assert(pl_language_find("not-a-language") == NULL);
}

static void test_language_detection(void) {
    const pl_language_profile *profile = NULL;
    const char wat[] = "  (module (func))";
    assert(pl_language_detect("module.nasm", "", 0u, &profile) == 0);
    assert(profile == pl_language_find("asm-x86_64-nasm"));
    assert(pl_language_detect("module.unknown", wat, sizeof(wat) - 1u, &profile) == 0);
    assert(profile == pl_language_find("vm-wasm-text"));
    assert(pl_language_detect("source.asm", NULL, 0u, &profile) == 2);
    assert(profile == NULL);
    assert(pl_language_detect("source.s", NULL, 0u, &profile) == 2);
    assert(pl_language_detect("module.unknown", "unknown", 7u, &profile) == 1);
    assert(profile == NULL);
    assert(pl_language_detect("module.unknown", NULL, 0u, &profile) == 1);
    assert(pl_language_detect(NULL, NULL, 0u, NULL) == -1);
    assert(pl_language_detect(NULL, "x", PL_LANGUAGE_SOURCE_MAX + 1u, &profile) == -1);
}

static void test_bounded_structural_inspection(void) {
    const char valid[] = "int main(void) { return call(1, 2); }\n";
    const char mismatch[] = "int main(] {";
    const char open_comment[] = "int x; /* never closed";
    const char open_string[] = "const char *message = \"unfinished;";
    pl_language_report report;

    assert(pl_language_inspect("C", valid, sizeof(valid) - 1u, &report) == 0);
    assert(report.structurally_valid == 1);
    assert(report.line_count == 2u);
    assert(report.token_count > 5u);
    assert(report.max_delimiter_depth == 2u);
    assert(pl_language_inspect("sys-c", mismatch, sizeof(mismatch) - 1u, &report) == -3);
    assert(report.structurally_valid == 0);
    assert(pl_language_inspect("sys-c", open_comment, sizeof(open_comment) - 1u, &report) == -5);
    assert(report.unterminated_comment == 1);
    assert(pl_language_inspect("sys-c", open_string, sizeof(open_string) - 1u, &report) == -4);
    assert(report.unterminated_string == 1);
    assert(pl_language_inspect("unknown", valid, sizeof(valid) - 1u, &report) == -1);
    assert(pl_language_inspect("sys-c", NULL, 0u, &report) == 0);
    assert(report.source_bytes == 0u);
    assert(pl_language_inspect("sys-c", valid, PL_LANGUAGE_SOURCE_MAX + 1u, &report) == -1);
}

static void test_target_compatibility(void) {
    int compatible = -1;
    assert(pl_language_check_target("asm-x86-masm", "x86", &compatible) == 0);
    assert(compatible == 1);
    assert(pl_language_check_target("asm-x86-masm", "aarch64", &compatible) == 0);
    assert(compatible == 0);
    assert(pl_language_check_target("sys-rust", "wasm32", &compatible) == 0);
    assert(compatible == 1);
    assert(pl_language_check_target("hdl-verilog", "hardware", &compatible) == 0);
    assert(compatible == 1);
    assert(pl_language_check_target("hdl-verilog", "x86_64", &compatible) == 0);
    assert(compatible == 0);
    assert(pl_language_check_target("missing", "x86_64", &compatible) == -1);
    assert(pl_language_check_target("sys-c", "imaginary", &compatible) == -1);
}

static void test_build_plan_is_explicit_and_inert(void) {
    pl_language_build_plan plan;
    int found_target = 0;
    int found_compile_only = 0;

    assert(pl_language_build_plan_create("sys-c", "x86_64",
        "src/main.c", "build/main.o", &plan) == 0);
    assert(strcmp(plan.executable, "clang") == 0);
    assert(plan.plan_only == 1);
    assert(plan.external_toolchain_required == 1);
    for (size_t i = 0u; i < plan.argument_count; ++i) {
        if (strncmp(plan.arguments[i], "--target=", 9u) == 0) found_target = 1;
        if (strcmp(plan.arguments[i], "-c") == 0) found_compile_only = 1;
    }
    assert(found_target == 1);
    assert(found_compile_only == 1);

    assert(pl_language_build_plan_create("asm-ebpf", "bpf",
        "src/program.bpf", "build/program.o", &plan) == 0);
    assert(strcmp(plan.executable, "llvm-mc") == 0);
    assert(strcmp(plan.arguments[0], "-triple=bpf") == 0);

    assert(pl_language_build_plan_create("sys-d-betterc", "x86_64",
        "src/module.d", "build/module.o", &plan) == 0);
    assert(strcmp(plan.executable, "ldc2") == 0);
    assert(strcmp(plan.arguments[0], "-betterC") == 0);
    assert(strcmp(plan.arguments[2], "-mtriple=x86_64-unknown-linux-gnu") == 0);

    assert(pl_language_build_plan_create("ir-qbe", "x86_64",
        "src/module.ssa", "build/module.o", &plan) == 0);
    assert(strcmp(plan.arguments[0], "-t") == 0);
    assert(strcmp(plan.arguments[1], "amd64_sysv") == 0);
    assert(pl_language_check_target("ir-qbe", "aarch64", &compatible) == 0);
    assert(compatible == 0);

    assert(pl_language_build_plan_create("asm-8051-asem", "8051",
        "src/start.a51", "build/start.obj", &plan) == -4);
    assert(pl_language_build_plan_create("sys-c", "x86_64",
        "-malicious.c", "build/out.o", &plan) == -1);
    assert(pl_language_build_plan_create("sys-c", "unlisted",
        "src/main.c", "build/out.o", &plan) == -2);
    assert(pl_language_build_plan_create("hdl-verilog", "x86_64",
        "rtl/top.v", "build/top", &plan) == -3);
}

int main(void) {
    test_catalogue_is_exact_and_unique();
    test_language_detection();
    test_bounded_structural_inspection();
    test_target_compatibility();
    test_build_plan_is_explicit_and_inert();
    puts("All processor language workbench tests passed.");
    return 0;
}
