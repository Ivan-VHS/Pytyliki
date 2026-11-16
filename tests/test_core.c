#include "core.h"
#include "clinical_rules.h"
#include "pipeline.h"
#include "policy.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_cpu_and_benchmark(void) {
    pl_cpu_info cpu;
    pl_benchmark benchmark;
    assert(pl_cpu_profile(&cpu) == 0);
    assert(cpu.logical_cpus >= 1u);
    assert(cpu.vendor[0] != '\0');
    assert(cpu.model[0] != '\0');
    assert(pl_run_benchmark(UINT64_C(10000), &benchmark) == 0);
    assert(benchmark.iterations == UINT64_C(10000));
    assert(benchmark.elapsed_ms >= 0.0);
    assert(pl_run_benchmark(0, &benchmark) != 0);
    assert(pl_run_benchmark(UINT64_C(500000001), &benchmark) != 0);
}

static void test_static_scanner(void) {
    const char *path = "pytyliki_test_input.c";
    FILE *file = fopen(path, "wb");
    pl_analysis analysis = {0};
    char details[1024];
    assert(file != NULL);
    fputs("int main(){ strcpy(dst, src); system(cmd); }\n", file);
    assert(fclose(file) == 0);
    assert(pl_analyze_file(path, &analysis, details, sizeof(details)) == 0);
    assert(analysis.files_scanned == 1u);
    assert(analysis.findings == 2u);
    assert(strstr(details, "process execution API") != NULL);
    assert(remove(path) == 0);
}

static void test_policy(void) {
    pl_role role;
    pl_action action;
    assert(pl_policy_authorize(PL_ROLE_VIEWER, PL_ACTION_READ));
    assert(pl_policy_authorize(PL_ROLE_VIEWER, PL_ACTION_ANALYZE));
    assert(!pl_policy_authorize(PL_ROLE_VIEWER, PL_ACTION_BENCHMARK));
    assert(pl_policy_authorize(PL_ROLE_CONTRIBUTOR, PL_ACTION_PROPOSE_PATCH));
    assert(pl_policy_authorize(PL_ROLE_MAINTAINER, PL_ACTION_APPLY_PATCH));
    assert(!pl_policy_authorize(PL_ROLE_MAINTAINER, PL_ACTION_MANAGE_ACCESS));
    assert(pl_policy_authorize(PL_ROLE_ADMIN, PL_ACTION_CHANGE_POLICY));
    assert(!pl_policy_authorize(PL_ROLE_ADMIN, PL_ACTION_WRITE_FIRMWARE));
    assert(!pl_policy_authorize(PL_ROLE_ADMIN, PL_ACTION_EXECUTE_INSPECTED_CODE));
    assert(pl_policy_parse_role("analyst", &role) == 0 && role == PL_ROLE_ANALYST);
    assert(pl_policy_parse_action("benchmark", &action) == 0 && action == PL_ACTION_BENCHMARK);
    assert(pl_policy_parse_role("root", &role) != 0);
}

static void test_pipeline(void) {
    pl_pipeline_plan plan;
    assert(pl_pipeline_build(PL_TARGET_C11, &plan) == 0);
    assert(plan.stage_count == PL_STAGE_COUNT);
    assert(plan.deterministic);
    assert(plan.semantic_equivalence_required);
    assert(plan.human_review_required);
    assert(plan.is_reference_plan);
    assert(plan.stages[0] == PL_STAGE_PARSE);
    assert(plan.stages[PL_STAGE_COUNT - 1] == PL_STAGE_TEST);
    assert(pl_pipeline_build((pl_pipeline_target)99, &plan) != 0);
    assert(pl_pipeline_build(PL_TARGET_C23, &plan) == 0);
}

static void test_clinical_catalog(void) {
    size_t i;
    pl_clinical_rule rule;
    assert(pl_clinical_catalog_count() == 3u);
    for (i = 0; i < pl_clinical_catalog_count(); ++i) {
        assert(pl_clinical_catalog_get(i, &rule) == 0);
        assert(rule.requires_expert_signoff);
        assert(pl_clinical_rule_validate(&rule) != 0);
    }
    assert(pl_clinical_catalog_get(99u, &rule) != 0);
    assert(pl_clinical_catalog_get(0u, &rule) == 0);
    rule.approved_source = "approved-local-guideline:INF-CHAIN-001";
    assert(pl_clinical_rule_validate(&rule) == 0);
}

int main(void) {
    test_cpu_and_benchmark();
    test_static_scanner();
    test_policy();
    test_pipeline();
    test_clinical_catalog();
    puts("All Pytyliki core tests passed.");
    return 0;
}
