#include "processor_topology.h"
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {
    pl_processor_topology_snapshot snapshot;
    if(argc!=2 || strcmp(argv[1],"--json")!=0) {
        fprintf(stderr,"usage: pytyliki-topology --json\n");
        return 2;
    }
    pl_topology_status status=pl_processor_topology_scan(&snapshot);
    if(status!=PL_TOPOLOGY_OK) {
        fprintf(stderr,"topology scan failed: %s\n",pl_topology_status_name(status));
        return 1;
    }
    return pl_processor_topology_write_json(&snapshot,stdout)==PL_TOPOLOGY_OK ? 0 : 1;
}
