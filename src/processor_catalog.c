#include "processor_catalog.h"
#include <string.h>

static const pl_processor_rule catalogue[] = {
    {
        "CPU-DEP-001",
        PL_PROCESSOR_DOMAIN_EXECUTION,
        "Dependency-chain inspection workflow",
        "NOT_CONFIGURED",
        "v1",
        1
    },
    {
        "CPU-CACHE-001",
        PL_PROCESSOR_DOMAIN_CACHE_MEMORY,
        "Cache-locality profiling workflow",
        "NOT_CONFIGURED",
        "v1",
        1
    },
    {
        "CPU-VERIFY-001",
        PL_PROCESSOR_DOMAIN_VERIFICATION,
        "Benchmark-integrity verification workflow",
        "NOT_CONFIGURED",
        "v1",
        1
    }
};

size_t pl_processor_catalog_count(void) {
    return sizeof(catalogue) / sizeof(catalogue[0]);
}

int pl_processor_catalog_get(size_t index, pl_processor_rule *out) {
    if (out == NULL || index >= pl_processor_catalog_count()) return -1;
    *out = catalogue[index];
    return 0;
}

int pl_processor_rule_validate(const pl_processor_rule *rule) {
    if (rule == NULL || rule->id == NULL || rule->title == NULL ||
        rule->evidence_source == NULL || rule->version == NULL) return -1;
    if (rule->id[0] == '\0' || rule->title[0] == '\0' ||
        rule->version[0] == '\0' || rule->requires_human_review == 0) return -1;
    if ((int)rule->domain < (int)PL_PROCESSOR_DOMAIN_ARCHITECTURE ||
        rule->domain >= PL_PROCESSOR_DOMAIN_COUNT) return -1;
    if (rule->evidence_source[0] == '\0' ||
        strcmp(rule->evidence_source, "NOT_CONFIGURED") == 0) return -1;
    return 0;
}

const char *pl_processor_domain_name(pl_processor_domain domain) {
    switch (domain) {
        case PL_PROCESSOR_DOMAIN_ARCHITECTURE: return "architecture";
        case PL_PROCESSOR_DOMAIN_EXECUTION: return "execution";
        case PL_PROCESSOR_DOMAIN_CACHE_MEMORY: return "cache-memory";
        case PL_PROCESSOR_DOMAIN_VERIFICATION: return "verification";
        case PL_PROCESSOR_DOMAIN_TOOLCHAIN: return "toolchain";
        default: return "unknown";
    }
}
