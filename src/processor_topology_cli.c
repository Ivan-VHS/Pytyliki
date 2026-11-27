#include "processor_topology.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    pl_processor_topology_snapshot topology;
    pl_processor_snapshot cpu;
    pl_topology_status status;
    if (argc != 2 || strcmp(argv[1], "--json") != 0) {
        fputs("Usage: pytyliki-topology --json\n", stderr);
        return 2;
    }
    status = pl_processor_topology_collect(&topology, &cpu);
    if (status != PL_TOPOLOGY_OK) {
        fprintf(stderr, "pytyliki-topology: %s\n", pl_topology_status_name(status));
        return 1;
    }
    status = pl_processor_topology_write_json(&topology, stdout);
    if (status != PL_TOPOLOGY_OK) {
        fprintf(stderr, "pytyliki-topology: report failed: %s\n",
                pl_topology_status_name(status));
        return 1;
    }
    return 0;
}
