#include "hw_rules.h"
#include <assert.h>
#include <string.h>

int main(void) {
    pl_hw_metrics metrics;
    pl_hw_report report;
    size_t i;
    int saw_l1 = 0, saw_temp = 0, saw_missing_ppa = 0;

    memset(&metrics, 0, sizeof(metrics));
    assert(pl_hw_rule_count() == 12u);
    assert(pl_hw_evaluate(&metrics, &report) == 0);
    assert(report.rules_evaluated == 12u);
    assert(report.findings_count >= 2u); /* Missing profiler + PPA evidence. */

    memset(&metrics, 0, sizeof(metrics));
    metrics.l1d_valid = 1; metrics.l1d_miss_rate_pct = 12.0;
    metrics.temperature_valid = 1; metrics.temperature_c = 94.0;
    metrics.ipc_valid = 1; metrics.ipc = 1.4;
    assert(pl_hw_evaluate(&metrics, &report) == 0);
    for (i = 0; i < report.findings_count; ++i) {
        if (report.findings[i].id == 2u) saw_l1 = 1;
        if (report.findings[i].id == 7u) saw_temp = 1;
        if (report.findings[i].id == 11u) saw_missing_ppa = 1;
    }
    assert(saw_l1 && saw_temp && saw_missing_ppa);
    assert(strcmp(pl_hw_domain_name(PL_HW_DOMAIN_CACHE_MEMORY), "cache-memory") == 0);
    assert(strcmp(pl_hw_severity_name(PL_HW_SEVERITY_WARNING), "warning") == 0);
    return 0;
}
