#ifndef PYTYLIKI_IRONCLAD_SYSTEMS_H
#define PYTYLIKI_IRONCLAD_SYSTEMS_H

#include <stddef.h>

#define PL_SYSTEM_ENHANCEMENT_COUNT 20u
#define PL_SYSTEM_NAME_MAX 32u

typedef enum {
    PL_SYSTEM_RELIABILITY = 0,
    PL_SYSTEM_PERFORMANCE,
    PL_SYSTEM_MODULARITY,
    PL_SYSTEM_DIAGNOSTICS,
    PL_SYSTEM_LOW_LEVEL,
    PL_SYSTEM_ASYNC,
    PL_SYSTEM_API,
    PL_SYSTEM_USABILITY,
    PL_SYSTEM_RECOVERY,
    PL_SYSTEM_PORTABILITY,
    PL_SYSTEM_COMPILATION,
    PL_SYSTEM_SECURITY,
    PL_SYSTEM_TESTING,
    PL_SYSTEM_DISTRIBUTED,
    PL_SYSTEM_MEMORY,
    PL_SYSTEM_EXTENSIBILITY,
    PL_SYSTEM_PROFILING,
    PL_SYSTEM_AUTOMATION,
    PL_SYSTEM_EXPERIMENTAL,
    PL_SYSTEM_OBSERVABILITY
} pl_system_domain;

typedef struct {
    unsigned id;
    const char *name;
    pl_system_domain domain;
    const char *objective;
    const char *acceptance;
} pl_system_enhancement;

/* Deterministic, read-only roadmap registry; no entry claims implementation by itself. */
size_t pl_system_enhancements(const pl_system_enhancement **out);
const char *pl_system_domain_name(pl_system_domain domain);
int pl_system_enhancement_validate(const pl_system_enhancement *item);

#endif
