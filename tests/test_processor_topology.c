#include "processor_topology.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    pl_processor_topology_snapshot snapshot;
    assert(pl_processor_topology_scan(&snapshot)==PL_TOPOLOGY_OK);
    assert(snapshot.logical_cpu_count>0u);
    assert(snapshot.logical_cpu_count<=PL_TOPOLOGY_MAX_LOGICAL_CPUS);
    assert(snapshot.online_logical_cpu_count>0u);
    assert(pl_processor_topology_validate(&snapshot)==PL_TOPOLOGY_OK);
    for(uint32_t i=0u;i<snapshot.logical_cpu_count;++i) {
        assert(snapshot.logical_cpus[i].logical_id<PL_TOPOLOGY_MAX_LOGICAL_CPUS);
        assert(snapshot.logical_cpus[i].online<=1u);
    }
    {
        pl_processor_topology_snapshot invalid=snapshot;
        if(invalid.logical_cpu_count>1u) {
            invalid.logical_cpus[1].os_id=invalid.logical_cpus[0].os_id;
            invalid.logical_cpus[1].group_id=invalid.logical_cpus[0].group_id;
            assert(pl_processor_topology_validate(&invalid)==PL_TOPOLOGY_DUPLICATE_CPU);
        }
    }
    {
        FILE *stream=fopen("processor-topology-test-output.json","w+");
        assert(stream!=NULL);
        assert(pl_processor_topology_write_json(&snapshot,stream)==PL_TOPOLOGY_OK);
        assert(fseek(stream,0L,SEEK_SET)==0);
        char prefix[64]={0};
        assert(fread(prefix,1u,strlen("{\"schema\":\"pytyliki.processor-topology-evidence.v2\"}"),stream)>0u);
        assert(strstr(prefix,"pytyliki.processor-topology-evidence.v2")!=NULL);
        fclose(stream);\n        remove("processor-topology-test-output.json");
    }
    puts("processor topology collector tests passed");
    return 0;
}
