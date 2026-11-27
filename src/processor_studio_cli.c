#include "processor_studio.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define STUDIO_CLI_JSON_CAPACITY 32768u

static int print_error(const char *operation, pl_processor_studio_status status,
                       const pl_processor_studio_error *error) {
    fprintf(stderr, "%s: %s", operation, pl_processor_studio_status_name(status));
    if (error != NULL && error->message[0] != '\0') {
        fprintf(stderr, ": %s", error->message);
    }
    fputc('\n', stderr);
    return 1;
}

static int print_catalog(void) {
    size_t count = 0u;
    size_t index;
    const pl_processor_studio_domain_descriptor *catalog = pl_processor_studio_catalog(&count);
    printf("Pytyliki 20-domain Processor Studio catalog (%zu domains)\n", count);
    printf("IDX  ENVIRONMENT                 ENGINEERING FOCUS       ORGANIZATION\n");
    for (index = 0u; index < count; index++) {
        printf("%02u   %-28s %-24s %s\n",
               catalog[index].id,
               catalog[index].environment,
               catalog[index].engineering_focus,
               catalog[index].organization);
    }
    return 0;
}

static int run_studio(int emit_json, int show_plan) {
    pl_processor_studio studio;
    pl_processor_studio_config config;
    pl_processor_studio_error error;
    pl_processor_studio_report report;
    pl_processor_studio_assignment assignments[PL_PROCESSOR_STUDIO_DOMAIN_COUNT];
    size_t assignment_count = 0u;
    pl_processor_studio_status status;
    size_t index;

    pl_processor_studio_default_config(&config);
    status = pl_processor_studio_init(&studio, &config, &error);
    if (status != PL_PROCESSOR_STUDIO_OK) {
        return print_error("initialize", status, &error);
    }
    status = pl_processor_studio_begin(&studio, 1u, PL_PROCESSOR_STUDIO_RIGHT_ALL, &error);
    if (status != PL_PROCESSOR_STUDIO_OK) {
        return print_error("begin", status, &error);
    }
    status = pl_processor_studio_plan(&studio, PL_PROCESSOR_STUDIO_DOMAIN_MASK,
                                      PL_PROCESSOR_STUDIO_RIGHT_ALL,
                                      &assignment_count, &error);
    if (status != PL_PROCESSOR_STUDIO_OK) {
        (void)pl_processor_studio_rollback(&studio, PL_PROCESSOR_STUDIO_RIGHT_ALL, NULL);
        return print_error("plan", status, &error);
    }
    status = pl_processor_studio_execute(&studio, PL_PROCESSOR_STUDIO_DOMAIN_MASK,
                                         PL_PROCESSOR_STUDIO_RIGHT_ALL, &error);
    if (status != PL_PROCESSOR_STUDIO_OK) {
        (void)pl_processor_studio_rollback(&studio, PL_PROCESSOR_STUDIO_RIGHT_ALL, NULL);
        return print_error("execute", status, &error);
    }
    status = pl_processor_studio_commit(&studio, 1u, PL_PROCESSOR_STUDIO_RIGHT_ALL, &error);
    if (status != PL_PROCESSOR_STUDIO_OK) {
        (void)pl_processor_studio_rollback(&studio, PL_PROCESSOR_STUDIO_RIGHT_ALL, NULL);
        return print_error("commit", status, &error);
    }

    if (emit_json != 0) {
        char json[STUDIO_CLI_JSON_CAPACITY];
        size_t written = 0u;
        status = pl_processor_studio_report_json(&studio, PL_PROCESSOR_STUDIO_RIGHT_ALL,
                                                 json, sizeof(json), &written, &error);
        if (status != PL_PROCESSOR_STUDIO_OK) {
            return print_error("serialize report", status, &error);
        }
        if (fwrite(json, 1u, written, stdout) != written || fputc('\n', stdout) == EOF) {
            fputs("write report: output error\n", stderr);
            return 1;
        }
        return 0;
    }

    status = pl_processor_studio_get_report(&studio, PL_PROCESSOR_STUDIO_RIGHT_ALL,
                                            &report, &error);
    if (status != PL_PROCESSOR_STUDIO_OK) {
        return print_error("read report", status, &error);
    }
    printf("20-domain Processor Studio committed: generation=%" PRIu32 " mask=0x%05" PRIx32 "\n",
           report.generation, report.committed_domain_mask);
    printf("fold=0x%016" PRIx64 " matrix=0x%016" PRIx64 " sweep=0x%016" PRIx64 "\n",
           report.domain_fold, report.matrix_fold, report.sweep_digest);
    printf("tournament=0x%016" PRIx64 " winner=domain-%02u low-byte=%02x parity=%u nonzero=%s\n",
           report.tournament_value, report.tournament_domain,
           report.low_byte_signature, report.bit_parity,
           report.predicate_nonzero != 0u ? "true" : "false");
    printf("logical queue=AXI_BUS workers=%" PRIu32 " advisory assignments=%" PRIu32 "\n",
           report.worker_count, report.planned_assignment_count);
    puts("hardware effects: physical-memory=none mmio=none dma=none threads=none");

    if (show_plan != 0) {
        status = pl_processor_studio_copy_plan(&studio, PL_PROCESSOR_STUDIO_RIGHT_ALL,
                                               assignments, PL_PROCESSOR_STUDIO_DOMAIN_COUNT,
                                               &assignment_count, &error);
        if (status != PL_PROCESSOR_STUDIO_OK) {
            return print_error("read plan", status, &error);
        }
        puts("advisory assignment plan (no threads are started):");
        for (index = 0u; index < assignment_count; index++) {
            printf("  domain-%02u -> worker-%02u (cost=%" PRIu32 ")\n",
                   assignments[index].domain_id, assignments[index].worker_id,
                   assignments[index].estimated_cost);
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--catalog") == 0) {
        return print_catalog();
    }
    if (argc == 2 && strcmp(argv[1], "--run") == 0) {
        return run_studio(0, 0);
    }
    if (argc == 2 && strcmp(argv[1], "--plan") == 0) {
        return run_studio(0, 1);
    }
    if (argc == 2 && strcmp(argv[1], "--json") == 0) {
        return run_studio(1, 0);
    }
    fprintf(stderr, "usage: %s --catalog|--run|--plan|--json\n", argv[0]);
    return 2;
}
