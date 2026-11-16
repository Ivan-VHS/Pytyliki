#ifndef PYTYLIKI_HW_RULES_H
#define PYTYLIKI_HW_RULES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_HW_RULE_CATALOG_CAPACITY 1000u
#define PL_HW_MAX_FINDINGS 64u

typedef enum {
    PL_HW_DOMAIN_ARCHITECTURE = 1,
    PL_HW_DOMAIN_EXECUTION,
    PL_HW_DOMAIN_CACHE_MEMORY,
    PL_HW_DOMAIN_POWER_THERMAL,
    PL_HW_DOMAIN_INTERCONNECT,
    PL_HW_DOMAIN_PHYSICAL_DESIGN,
    PL_HW_DOMAIN_RELIABILITY,
    PL_HW_DOMAIN_SECURITY,
    PL_HW_DOMAIN_VERIFICATION,
    PL_HW_DOMAIN_TOOLCHAIN
} pl_hw_domain;

typedef enum {
    PL_HW_SEVERITY_INFO = 1,
    PL_HW_SEVERITY_REVIEW,
    PL_HW_SEVERITY_WARNING
} pl_hw_severity;

/* Metrics must come from a trusted profiler/simulator. Unknown values stay
 * zero and are ignored unless the corresponding *_valid flag is set. */
typedef struct {
    double ipc;
    double l1d_miss_rate_pct;
    double l2_miss_rate_pct;
    double branch_mispredict_pct;
    double memory_stall_pct;
    double package_power_w;
    double temperature_c;
    double frequency_mhz;
    double utilization_pct;
    unsigned ipc_valid : 1;
    unsigned l1d_valid : 1;
    unsigned l2_valid : 1;
    unsigned branch_valid : 1;
    unsigned memory_stall_valid : 1;
    unsigned power_valid : 1;
    unsigned temperature_valid : 1;
    unsigned frequency_valid : 1;
    unsigned utilization_valid : 1;
} pl_hw_metrics;

typedef struct {
    unsigned id;
    pl_hw_domain domain;
    pl_hw_severity severity;
    const char *title;
    const char *evidence;
    const char *recommendation;
} pl_hw_finding;

typedef struct {
    size_t rules_evaluated;
    size_t findings_count;
    pl_hw_finding findings[PL_HW_MAX_FINDINGS];
} pl_hw_report;

/* v1 ships a small, explicit rule set. The 1..1000 ID space is reserved for
 * the complete catalog; rules not implemented yet are never reported as run. */
size_t pl_hw_rule_count(void);
int pl_hw_evaluate(const pl_hw_metrics *metrics, pl_hw_report *report);
const char *pl_hw_domain_name(pl_hw_domain domain);
const char *pl_hw_severity_name(pl_hw_severity severity);

#ifdef __cplusplus
}
#endif
#endif
