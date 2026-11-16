#include "clinical_rules.h"
#include <string.h>

static const pl_clinical_rule catalogue[] = {
    {
        "INF-CHAIN-001",
        PL_CLINICAL_INFECTION_CONTROL,
        "Break-the-chain workflow template",
        "NOT_CONFIGURED",
        "template-1",
        1
    },
    {
        "EMR-SEQUENCE-001",
        PL_CLINICAL_EMERGENCY_RESPONSE,
        "Emergency response workflow template",
        "NOT_CONFIGURED",
        "template-1",
        1
    },
    {
        "ONC-SCREEN-001",
        PL_CLINICAL_ONCOLOGY_SCREENING,
        "Oncology screening workflow template",
        "NOT_CONFIGURED",
        "template-1",
        1
    }
};

size_t pl_clinical_catalog_count(void) {
    return sizeof(catalogue) / sizeof(catalogue[0]);
}

int pl_clinical_catalog_get(size_t index, pl_clinical_rule *out) {
    if (out == NULL || index >= pl_clinical_catalog_count()) return -1;
    *out = catalogue[index];
    return 0;
}

int pl_clinical_rule_validate(const pl_clinical_rule *rule) {
    if (rule == NULL || rule->id == NULL || rule->title == NULL ||
        rule->approved_source == NULL || rule->version == NULL) return -1;
    if (rule->id[0] == '\0' || rule->title[0] == '\0' ||
        rule->version[0] == '\0' || rule->requires_expert_signoff == 0) return -1;
    if (rule->domain < 0 || rule->domain >= PL_CLINICAL_DOMAIN_COUNT) return -1;
    if (rule->approved_source[0] == '\0' ||
        strcmp(rule->approved_source, "NOT_CONFIGURED") == 0) return -1;
    return 0;
}

const char *pl_clinical_domain_name(pl_clinical_domain domain) {
    switch (domain) {
        case PL_CLINICAL_INFECTION_CONTROL: return "infection-control";
        case PL_CLINICAL_EMERGENCY_RESPONSE: return "emergency-response";
        case PL_CLINICAL_ONCOLOGY_SCREENING: return "oncology-screening";
        default: return "unknown";
    }
}
