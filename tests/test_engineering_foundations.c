#include "clinical_rules.h"
#include "pipeline.h"
#include "policy.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

int main(void) {
    pl_pipeline_plan plan;
    pl_role role;
    pl_action action;
    pl_clinical_rule rule;
    assert(pl_pipeline_build(PL_TARGET_C11, &plan) == 0);
    assert(plan.stage_count == PL_STAGE_COUNT && plan.deterministic);
    assert(plan.semantic_equivalence_required && plan.human_review_required);
    assert(plan.stages[0] == PL_STAGE_PARSE);
    assert(plan.stages[PL_STAGE_COUNT - 1] == PL_STAGE_TEST);
    assert(pl_pipeline_build(PL_TARGET_C23, &plan) == 0);
    assert(pl_pipeline_build((pl_pipeline_target)99, &plan) != 0);
    assert(strcmp(pl_pipeline_target_name(PL_TARGET_C11), "ISO C11") == 0);
    assert(pl_policy_authorize(PL_ROLE_VIEWER, PL_ACTION_READ));
    assert(!pl_policy_authorize(PL_ROLE_VIEWER, PL_ACTION_BENCHMARK));
    assert(pl_policy_authorize(PL_ROLE_ADMIN, PL_ACTION_CHANGE_POLICY));
    assert(!pl_policy_authorize(PL_ROLE_ADMIN, PL_ACTION_WRITE_FIRMWARE));
    assert(!pl_policy_authorize(PL_ROLE_ADMIN, PL_ACTION_EXECUTE_INSPECTED_CODE));
    assert(pl_policy_parse_role("maintainer", &role) == 0 && role == PL_ROLE_MAINTAINER);
    assert(pl_policy_parse_action("apply-patch", &action) == 0 && action == PL_ACTION_APPLY_PATCH);
    assert(pl_policy_parse_role("root", &role) != 0);
    assert(pl_clinical_catalog_count() == 3u);
    assert(pl_clinical_catalog_get(0u, &rule) == 0);
    assert(pl_clinical_rule_validate(&rule) != 0);
    rule.approved_source = "approved-local-guideline:INF-CHAIN-001";
    assert(pl_clinical_rule_validate(&rule) == 0);
    assert(pl_clinical_catalog_get(99u, &rule) != 0);
    return 0;
}
