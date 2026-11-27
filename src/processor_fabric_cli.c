#include "processor_fabric.h"

#include <stdio.h>
#include <string.h>

static void usage(FILE *stream) {
    fputs(
        "Pytyliki Processor Fabric ABI v2\n"
        "Usage:\n"
        "  pytyliki-fabric --version\n"
        "  pytyliki-fabric --catalogues\n"
        "  pytyliki-fabric --catalogues-json\n"
        "\n"
        "Catalogue enumeration is metadata-only; it does not access hardware.\n",
        stream
    );
}

int main(int argc, char **argv) {
    pl_processor_fabric_catalogues catalogues;

    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pytyliki Processor Fabric 2.0.0 (bounded advisory planner)");
        return 0;
    }
    if (argc != 2 ||
        (strcmp(argv[1], "--catalogues") != 0 &&
         strcmp(argv[1], "--catalogues-json") != 0)) {
        usage(stderr);
        return 2;
    }
    if (pl_processor_fabric_collect_catalogues(&catalogues) != PL_PROCESSOR_FABRIC_OK) {
        fputs("pytyliki-fabric: catalogue aggregation failed\n", stderr);
        return 1;
    }

    if (strcmp(argv[1], "--catalogues-json") == 0) {
        printf(
            "{\"schema\":\"pytyliki.processor-fabric-catalogues.v1\","
            "\"cpu_suite_modules\":%u,\"processor_rules\":%u,"
            "\"hardware_constructs\":%u,\"system_sigil_operators\":%u,"
            "\"iron_core_markers\":%u,\"hardware_access_performed\":false}\n",
            catalogues.cpu_suite_modules, catalogues.processor_rules,
            catalogues.hardware_constructs, catalogues.system_sigil_operators,
            catalogues.iron_core_markers
        );
        return 0;
    }

    puts("Pytyliki Processor Fabric catalogue aggregation");
    printf("CPU Suite modules: %u\n", catalogues.cpu_suite_modules);
    printf("Processor rules: %u\n", catalogues.processor_rules);
    printf("Hardware constructs: %u\n", catalogues.hardware_constructs);
    printf("SystemArray sigil operators: %u\n", catalogues.system_sigil_operators);
    printf("Iron Core markers: %u\n", catalogues.iron_core_markers);
    puts("Mode: metadata-only; no process scheduling or hardware access");
    return 0;
}
