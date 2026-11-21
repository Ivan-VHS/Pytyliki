#include "careguard.h"

#include <assert.h>
#include <stddef.h>

int main(void) {
    CareGuardWorkload empty = {0u, 0u, 0u, 0u, 10u};
    CareGuardWorkload overloaded = {20u, 4u, 30u, 12u, 10u};
    CareGuardAssessment a = careguard_assess_project_load(&empty);
    CareGuardAssessment b = careguard_assess_project_load(&overloaded);
    uint32_t reserved = 0u, usable = 0u;

    assert(a.status == CAREGUARD_LOAD_NORMAL);
    assert(b.status == CAREGUARD_LOAD_PAUSE);
    assert(careguard_review_language("This is weakness") == 1u);
    assert(careguard_accept_help_request("I need help") == 1);
    assert(careguard_accept_help_request("   ") == 0);
    assert(careguard_select_support_path("urgent_human") == CAREGUARD_PATH_URGENT_HUMAN);
    assert(careguard_select_support_path("auto_call") == CAREGUARD_PATH_INVALID);
    assert(careguard_review_contextual_claim("claim", "issue/42") == 1);
    assert(careguard_review_contextual_claim("claim", "") == 0);
    assert(careguard_check_clinical_boundary("prescribe antidepressant") == 0);
    assert(careguard_check_clinical_boundary("pause build queue") == 1);
    assert(careguard_allocate_resilience_buffer(10u, 20u, &reserved, &usable) == 1);
    assert(reserved == 2u && usable == 8u);
    assert(careguard_allocate_resilience_buffer(10u, 80u, &reserved, &usable) == 0);
    assert(careguard_assess_project_load(NULL).status == CAREGUARD_LOAD_INVALID);
    return 0;
}
