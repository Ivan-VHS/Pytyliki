#include "core.h"
#include "hw_rules.h"
#include <stdio.h>
#include <string.h>

static void usage(void) {
    puts("Pytyliki CPU Lab 0.5.0\nUsage: pytyliki [--profile] [--benchmark] [--analyze FILE] [--hw-rules] [--json FILE] [--help]");
}

static void print_hw_report(void) {
    pl_hw_metrics metrics = {0};
    pl_hw_report report;
    size_t i;
    if (pl_hw_evaluate(&metrics, &report) != 0) {
        fputs("Hardware rule evaluation failed.\n", stderr);
        return;
    }
    printf("Hardware rules evaluated: %zu (implemented rules only)\n", report.rules_evaluated);
    puts("No live hardware-counter input was supplied; results below are evidence/process gates, not measured CPU defects.");
    for (i = 0; i < report.findings_count; ++i) {
        const pl_hw_finding *f = &report.findings[i];
        printf("  [%03u] %s / %s: %s\n      %s\n",
               f->id, pl_hw_domain_name(f->domain), pl_hw_severity_name(f->severity),
               f->title, f->recommendation);
    }
}

int main(int argc, char **argv) {
    int profile = 0, bench = 0, hw_rules = 0;
    const char *analyze = NULL, *json = NULL;
    int i;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--profile")) profile = 1;
        else if (!strcmp(argv[i], "--benchmark")) bench = 1;
        else if (!strcmp(argv[i], "--hw-rules")) hw_rules = 1;
        else if (!strcmp(argv[i], "--analyze") && i + 1 < argc) analyze = argv[++i];
        else if (!strcmp(argv[i], "--json") && i + 1 < argc) json = argv[++i];
        else if (!strcmp(argv[i], "--help")) { usage(); return 0; }
        else { fprintf(stderr, "Unknown or incomplete option: %s\n", argv[i]); usage(); return 2; }
    }
    if (!profile && !bench && !hw_rules && !analyze && !json) { profile = 1; bench = 1; }

    {
        pl_cpu_info cpu;
        pl_benchmark b = {0};
        pl_analysis a = {0};
        char details[8192] = {0};
        if (pl_cpu_profile(&cpu) != 0) return 1;
        if (bench && pl_run_benchmark(UINT64_C(10000000), &b) != 0) return 1;
        if (analyze && pl_analyze_file(analyze, &a, details, sizeof(details)) != 0) {
            fprintf(stderr, "Could not read: %s\n", analyze);
            return 1;
        }
        if (profile) printf("CPU vendor: %s\nLogical processors: %u\nFamily/model: %u/%u\n",
                            cpu.vendor, cpu.logical_cpus, cpu.family, cpu.model_id);
        if (bench) printf("Benchmark: %llu iterations, %.3f ms, %.1f iterations/ms\n",
                          (unsigned long long)b.iterations, b.elapsed_ms, b.score);
        if (analyze) printf("Static scan: %u file(s), %u finding(s), %u read error(s)\n%s",
                            a.files_scanned, a.findings, a.read_errors, details);
        if (hw_rules) print_hw_report();
        if (json) {
            if (pl_write_json(json, &cpu, &b, &a) != 0) {
                fprintf(stderr, "Could not write JSON report: %s\n", json);
                return 1;
            }
            printf("JSON report: %s\n", json);
        }
    }
    return 0;
}
