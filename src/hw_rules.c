#include "hw_rules.h"

#include <string.h>

typedef struct {
    unsigned id;
    pl_hw_domain domain;
    pl_hw_severity severity;
    const char *title;
    const char *recommendation;
    int metric;
    double threshold;
    int greater_is_issue;
} rule_def;

enum {
    M_IPC, M_L1D, M_L2, M_BRANCH, M_MEM_STALL,
    M_POWER, M_TEMP, M_FREQ, M_UTIL
};

static const rule_def rules[] = {
    {1, PL_HW_DOMAIN_EXECUTION, PL_HW_SEVERITY_REVIEW, "Low measured IPC", "Profile front-end starvation, dependency chains, execution-port pressure and workload parallelism.", M_IPC, 0.75, 0},
    {2, PL_HW_DOMAIN_CACHE_MEMORY, PL_HW_SEVERITY_WARNING, "Elevated L1 data-cache miss rate", "Inspect access locality, working-set size, alignment and hardware-prefetch behavior.", M_L1D, 5.0, 1},
    {3, PL_HW_DOMAIN_CACHE_MEMORY, PL_HW_SEVERITY_WARNING, "Elevated L2 cache miss rate", "Profile cache capacity, conflict behavior and memory-level parallelism before changing cache policy.", M_L2, 20.0, 1},
    {4, PL_HW_DOMAIN_ARCHITECTURE, PL_HW_SEVERITY_REVIEW, "Branch prediction pressure", "Use branch-level profiling; consider reducing unpredictable control flow or evaluating predictor design in RTL.", M_BRANCH, 5.0, 1},
    {5, PL_HW_DOMAIN_CACHE_MEMORY, PL_HW_SEVERITY_WARNING, "Memory stall pressure", "Measure bandwidth, latency, NUMA placement and outstanding memory requests.", M_MEM_STALL, 30.0, 1},
    {6, PL_HW_DOMAIN_POWER_THERMAL, PL_HW_SEVERITY_REVIEW, "High package power observation", "Compare against the processor's documented power envelope and workload baseline; do not force voltage changes.", M_POWER, 0.0, 1},
    {7, PL_HW_DOMAIN_POWER_THERMAL, PL_HW_SEVERITY_WARNING, "High reported temperature", "Check cooling, airflow, sensor validity and manufacturer thermal limits; avoid automatic fan or voltage writes.", M_TEMP, 90.0, 1},
    {8, PL_HW_DOMAIN_POWER_THERMAL, PL_HW_SEVERITY_REVIEW, "Low frequency under high utilization", "Check operating-system power policy, thermal/power limits and workload behavior using read-only telemetry.", M_FREQ, 0.0, 0},
    {9, PL_HW_DOMAIN_EXECUTION, PL_HW_SEVERITY_INFO, "Low utilization observation", "Check whether the workload is serial, blocked on I/O, synchronized or limited by memory latency.", M_UTIL, 20.0, 0},
    {10, PL_HW_DOMAIN_TOOLCHAIN, PL_HW_SEVERITY_REVIEW, "Insufficient profiler evidence", "Collect valid hardware-counter measurements before drawing architectural conclusions.", -1, 0.0, 0},
    {11, PL_HW_DOMAIN_VERIFICATION, PL_HW_SEVERITY_WARNING, "No physical-design evidence supplied", "Any proposed RTL change must pass lint, formal equivalence, timing, power and area analysis.", -1, 0.0, 0},
    {12, PL_HW_DOMAIN_RELIABILITY, PL_HW_SEVERITY_REVIEW, "No reliability characterization supplied", "For silicon work, include voltage-temperature corners, aging, electromigration and yield analysis.", -1, 0.0, 0}
};

static int metric_value(const pl_hw_metrics *m, int id, double *value) {
    switch (id) {
    case M_IPC: if (m->ipc_valid) { *value = m->ipc; return 1; } break;
    case M_L1D: if (m->l1d_valid) { *value = m->l1d_miss_rate_pct; return 1; } break;
    case M_L2: if (m->l2_valid) { *value = m->l2_miss_rate_pct; return 1; } break;
    case M_BRANCH: if (m->branch_valid) { *value = m->branch_mispredict_pct; return 1; } break;
    case M_MEM_STALL: if (m->memory_stall_valid) { *value = m->memory_stall_pct; return 1; } break;
    case M_POWER: if (m->power_valid) { *value = m->package_power_w; return 1; } break;
    case M_TEMP: if (m->temperature_valid) { *value = m->temperature_c; return 1; } break;
    case M_FREQ: if (m->frequency_valid && m->utilization_valid) { *value = m->frequency_mhz; return 1; } break;
    case M_UTIL: if (m->utilization_valid) { *value = m->utilization_pct; return 1; } break;
    default: break;
    }
    return 0;
}

size_t pl_hw_rule_count(void) {
    return sizeof(rules) / sizeof(rules[0]);
}

int pl_hw_evaluate(const pl_hw_metrics *metrics, pl_hw_report *report) {
    size_t i;
    if (metrics == NULL || report == NULL) return -1;
    memset(report, 0, sizeof(*report));
    report->rules_evaluated = pl_hw_rule_count();

    for (i = 0; i < pl_hw_rule_count(); ++i) {
        const rule_def *r = &rules[i];
        double value = 0.0;
        int has_value = metric_value(metrics, r->metric, &value);
        int issue = 0;
        pl_hw_finding *f;

        if (r->metric >= 0) {
            if (!has_value) continue;
            issue = r->greater_is_issue ? value > r->threshold : value < r->threshold;
        } else if (r->id == 10) {
            issue = !(metrics->ipc_valid || metrics->l1d_valid || metrics->l2_valid ||
                      metrics->branch_valid || metrics->memory_stall_valid);
        } else {
            issue = 1; /* Explicit process gate: no PPA/formal evidence in this input. */
        }

        if (!issue || report->findings_count >= PL_HW_MAX_FINDINGS) continue;
        f = &report->findings[report->findings_count++];
        f->id = r->id;
        f->domain = r->domain;
        f->severity = r->severity;
        f->title = r->title;
        f->recommendation = r->recommendation;
        f->evidence = has_value ? "Threshold triggered by supplied profiler metric." :
                                  "Required evidence was not supplied; this is a process warning, not a measured defect.";
    }
    return 0;
}

const char *pl_hw_domain_name(pl_hw_domain domain) {
    switch (domain) {
    case PL_HW_DOMAIN_ARCHITECTURE: return "architecture";
    case PL_HW_DOMAIN_EXECUTION: return "execution";
    case PL_HW_DOMAIN_CACHE_MEMORY: return "cache-memory";
    case PL_HW_DOMAIN_POWER_THERMAL: return "power-thermal";
    case PL_HW_DOMAIN_INTERCONNECT: return "interconnect";
    case PL_HW_DOMAIN_PHYSICAL_DESIGN: return "physical-design";
    case PL_HW_DOMAIN_RELIABILITY: return "reliability";
    case PL_HW_DOMAIN_SECURITY: return "security";
    case PL_HW_DOMAIN_VERIFICATION: return "verification";
    case PL_HW_DOMAIN_TOOLCHAIN: return "toolchain";
    default: return "unknown";
    }
}

const char *pl_hw_severity_name(pl_hw_severity severity) {
    switch (severity) {
    case PL_HW_SEVERITY_INFO: return "info";
    case PL_HW_SEVERITY_REVIEW: return "review";
    case PL_HW_SEVERITY_WARNING: return "warning";
    default: return "unknown";
    }
}
