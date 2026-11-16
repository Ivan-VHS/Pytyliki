#ifndef PYTYLIKI_PROCESSOR_CATALOG_H
#define PYTYLIKI_PROCESSOR_CATALOG_H

#include <stddef.h>

typedef enum {
    PL_PROCESSOR_DOMAIN_ARCHITECTURE = 0,
    PL_PROCESSOR_DOMAIN_EXECUTION,
    PL_PROCESSOR_DOMAIN_CACHE_MEMORY,
    PL_PROCESSOR_DOMAIN_VERIFICATION,
    PL_PROCESSOR_DOMAIN_TOOLCHAIN,
    PL_PROCESSOR_DOMAIN_COUNT
} pl_processor_domain;

typedef struct {
    const char *id;
    pl_processor_domain domain;
    const char *title;
    const char *evidence_source;
    const char *version;
    int requires_human_review;
} pl_processor_rule;

size_t pl_processor_catalog_count(void);
int pl_processor_catalog_get(size_t index, pl_processor_rule *out);
int pl_processor_rule_validate(const pl_processor_rule *rule);
const char *pl_processor_domain_name(pl_processor_domain domain);

#endif
