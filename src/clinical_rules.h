#ifndef PYTYLIKI_CLINICAL_RULES_H
#define PYTYLIKI_CLINICAL_RULES_H

#include <stddef.h>

typedef enum {
    PL_CLINICAL_INFECTION_CONTROL = 0,
    PL_CLINICAL_EMERGENCY_RESPONSE,
    PL_CLINICAL_ONCOLOGY_SCREENING,
    PL_CLINICAL_DOMAIN_COUNT
} pl_clinical_domain;

typedef struct {
    const char *id;
    pl_clinical_domain domain;
    const char *title;
    const char *approved_source;
    const char *version;
    int requires_expert_signoff;
} pl_clinical_rule;

/* The catalogue contains workflow templates, not clinical instructions. */
size_t pl_clinical_catalog_count(void);
int pl_clinical_catalog_get(size_t index, pl_clinical_rule *out);

/* Returns 0 only when a rule has complete metadata and a configured source.
 * A configured source still requires independent clinical validation. */
int pl_clinical_rule_validate(const pl_clinical_rule *rule);
const char *pl_clinical_domain_name(pl_clinical_domain domain);

#endif
