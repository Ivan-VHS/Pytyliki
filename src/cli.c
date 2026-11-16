#include "core.h"
#include "clinical_rules.h"
#include "pipeline.h"
#include "policy.h"

#include <stdio.h>
#include <string.h>

static void usage(void) {
    puts("Pytyliki CPU Lab 0.5.0");
    puts("Usage: pytyliki [--profile] [--benchmark] [--analyze FILE]");
    puts("                 [--json FILE] [--pipeline] [--target c11|c23]");
    puts("                 [--clinical-catalog] [--policy-check ROLE ACTION] [--help]");
    puts("Roles: viewer, analyst, contributor, maintainer, admin");
    puts("Actions: read, analyze, benchmark, propose-patch, apply-patch,");
    puts("         manage-access, change-policy, execute-inspected-code,");
    puts("         write-firmware, change-cpu-controls");
}

static void print_clinical_catalog(void) {
    size_t i;
    pl_clinical_rule rule;
    puts("Clinical workflow templates (not clinical instructions):");
    for (i = 0; i < pl_clinical_catalog_count(); ++i) {
        if (pl_clinical_catalog_get(i, &rule) == 0) {
            printf("- %s | %s | %s | source=%s | expert-signoff=%s\n",
                   rule.id, pl_clinical_domain_name(rule.domain), rule.title,
                   rule.approved_source,
                   rule.requires_expert_signoff ? "required" : "required-by-policy");
        }
    }
    puts("Configure approved local sources and obtain qualified clinical validation before use.");
}

static int print_pipeline(pl_pipeline_target target) {
    pl_pipeline_plan plan;
    size_t i;
    if (pl_pipeline_build(target, &plan) != 0) return 1;
    printf("Deterministic reference pipeline for %s:\n", pl_pipeline_target_name(plan.target));
    for (i = 0; i < plan.stage_count; ++i)
        printf("%u. %s\n", (unsigned)(i + 1u), pl_pipeline_stage_name(plan.stages[i]));
    puts("Semantic-equivalence tests: required");
    puts("Human review: required");
    puts("Note: this is a workflow plan; automatic transpilation is not implemented.");
    return 0;
}

int main(int argc, char **argv) {
    int profile = 0, bench = 0, analyze = 0, json_requested = 0;
    int pipeline_requested = 0, clinical_requested = 0, policy_requested = 0;
    const char *analyze_path = NULL, *json_path = NULL;
    pl_pipeline_target target = PL_TARGET_C11;
    pl_role role;
    pl_action action;
    int i;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--profile") == 0) profile = 1;
        else if (strcmp(argv[i], "--benchmark") == 0) bench = 1;
        else if (strcmp(argv[i], "--analyze") == 0 && i + 1 < argc) {
            analyze = 1;
            analyze_path = argv[++i];
        } else if (strcmp(argv[i], "--json") == 0 && i + 1 < argc) {
            json_requested = 1;
            json_path = argv[++i];
        } else if (strcmp(argv[i], "--pipeline") == 0) pipeline_requested = 1;
        else if (strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
            const char *name = argv[++i];
            if (strcmp(name, "c11") == 0) target = PL_TARGET_C11;
            else if (strcmp(name, "c23") == 0) target = PL_TARGET_C23;
            else {
                fprintf(stderr, "Unsupported target: %s\n", name);
                return 2;
            }
        } else if (strcmp(argv[i], "--clinical-catalog") == 0) clinical_requested = 1;
        else if (strcmp(argv[i], "--policy-check") == 0 && i + 2 < argc) {
            policy_requested = 1;
            if (pl_policy_parse_role(argv[++i], &role) != 0 ||
                pl_policy_parse_action(argv[++i], &action) != 0) {
                fputs("Unknown role or action. Use --help.\n", stderr);
                return 2;
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else {
            fprintf(stderr, "Unknown or incomplete option: %s\n", argv[i]);
            usage();
            return 2;
        }
    }

    if (!profile && !bench && !analyze && !json_requested &&
        !pipeline_requested && !clinical_requested && !policy_requested) {
        profile = 1;
        bench = 1;
    }

    if (pipeline_requested && print_pipeline(target) != 0) return 1;
    if (clinical_requested) print_clinical_catalog();
    if (policy_requested) {
        const int allowed = pl_policy_authorize(role, action);
        printf("Authorization: %s (role=%s, action=%s)\n",
               allowed ? "ALLOW" : "DENY",
               pl_policy_role_name(role), pl_policy_action_name(action));
        if (!allowed) return 3;
    }

    if (profile || bench || analyze || json_requested) {
        pl_cpu_info cpu;
        pl_benchmark b = {0};
        pl_analysis a = {0};
        char details[8192] = {0};

        if (pl_cpu_profile(&cpu) != 0) return 1;
        if (bench && pl_run_benchmark(UINT64_C(10000000), &b) != 0) return 1;
        if (analyze && pl_analyze_file(analyze_path, &a, details, sizeof(details)) != 0) {
            fprintf(stderr, "Could not read: %s\n", analyze_path);
            return 1;
        }
        if (profile)
            printf("CPU vendor: %s\nCPU model: %s\nLogical processors: %u\nFamily/model: %u/%u\n",
                   cpu.vendor, cpu.model, cpu.logical_cpus, cpu.family, cpu.model_id);
        if (bench)
            printf("Benchmark: %llu iterations, %.3f ms, %.1f iterations/ms\n",
                   (unsigned long long)b.iterations, b.elapsed_ms, b.score);
        if (analyze)
            printf("Static scan: %u file(s), %u finding(s), %u read error(s)\n%s",
                   a.files_scanned, a.findings, a.read_errors, details);
        if (json_requested) {
            if (pl_write_json(json_path, &cpu, &b, &a) != 0) {
                fprintf(stderr, "Could not write JSON report: %s\n", json_path);
                return 1;
            }
            printf("JSON report: %s\n", json_path);
        }
    }
    return 0;
}
