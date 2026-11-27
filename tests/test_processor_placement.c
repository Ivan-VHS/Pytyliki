#include "processor_placement.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    pl_processor_topology_snapshot topology;
    pl_placement_task tasks[3];
    pl_placement_pressure pressure;
    pl_placement_result result;
    memset(&topology,0,sizeof(topology));
    memset(tasks,0,sizeof(tasks));
    memset(&pressure,0,sizeof(pressure));
    topology.generation=11u;
    topology.logical_cpu_count=4u;
    topology.online_logical_cpu_count=4u;
    topology.completeness=PL_TOPOLOGY_COMPLETE;
    for(uint32_t i=0u;i<4u;++i) {
        topology.logical_cpus[i].logical_id=i;
        topology.logical_cpus[i].os_id=i;
        topology.logical_cpus[i].group_id=0u;
        topology.logical_cpus[i].package_id=0u;
        topology.logical_cpus[i].core_id=i/2u;
        topology.logical_cpus[i].numa_node_id=i<2u?0u:1u;
        topology.logical_cpus[i].online=1u;
        topology.logical_cpus[i].evidence=PL_TOPOLOGY_EVIDENCE_OBSERVED;
    }
    topology.numa_node_count=2u;
    topology.numa_nodes[0].node_id=0u; topology.numa_nodes[0].evidence=PL_TOPOLOGY_EVIDENCE_OBSERVED;
    topology.numa_nodes[1].node_id=1u; topology.numa_nodes[1].evidence=PL_TOPOLOGY_EVIDENCE_OBSERVED;
    topology.cache_count=2u;
    for(uint32_t i=0u;i<2u;++i) {
        topology.caches[i].cache_id=i; topology.caches[i].level=3u;
        topology.caches[i].kind=PL_TOPOLOGY_CACHE_UNIFIED;
        topology.caches[i].size_bytes=8u*1024u*1024u;
        topology.caches[i].line_bytes=64u;
        topology.caches[i].evidence=PL_TOPOLOGY_EVIDENCE_OBSERVED;
        topology.caches[i].sharing_mask[0]=i==0u?3u:12u;
        topology.caches[i].sharing_count=2u;
    }
    topology.isa[PL_TOPOLOGY_ISA_SSE2].hardware_present=1;
    topology.isa[PL_TOPOLOGY_ISA_SSE2].os_usable=1;
    pressure.generation=11u;
    pressure.cpu_load_basis_points[0]=3000u;
    pressure.cpu_load_basis_points[1]=1000u;
    pressure.numa_bandwidth_capacity_bytes_per_second[0]=1000000000u;
    pressure.numa_bandwidth_capacity_bytes_per_second[1]=1000000000u;
    pressure.numa_bandwidth_used_bytes_per_second[0]=800000000u;
    tasks[0].task_id=3u; tasks[0].priority=1u; tasks[0].preferred_numa_node_id=0u;
    tasks[0].working_set_bytes=2u*1024u*1024u;
    tasks[0].memory_bandwidth_bytes_per_second=100000000u;
    tasks[0].requested_load_basis_points=1000u;
    tasks[0].require_local_numa=1u;
    tasks[1].task_id=1u; tasks[1].priority=9u; tasks[1].preferred_numa_node_id=1u;
    tasks[1].working_set_bytes=1024u*1024u; tasks[1].requested_load_basis_points=1000u;
    tasks[1].require_local_numa=1u;
    tasks[2].task_id=2u; tasks[2].priority=5u; tasks[2].preferred_numa_node_id=PL_TOPOLOGY_UNKNOWN;
    tasks[2].working_set_bytes=4096u; tasks[2].requested_load_basis_points=1000u;
    assert(pl_processor_placement_plan(&topology,tasks,3u,&pressure,&result)==PL_PLACEMENT_OK);
    assert(result.assignment_count==3u);
    assert(result.assignments[0].task_id==1u);
    assert(result.assignments[0].state==PL_PLACEMENT_ASSIGNED);
    assert(result.assignments[0].logical_cpu_id>=2u);
    assert(result.assignments[1].task_id==2u);
    assert(result.assignments[2].task_id==3u);
    assert(result.report.execution_performed==0u);
    assert(result.report.affinity_changed==0u);
    assert(result.report.scheduling_changed==0u);
    pressure.generation=10u;
    assert(pl_processor_placement_plan(&topology,tasks,3u,&pressure,&result)==PL_PLACEMENT_STALE_GENERATION);
    puts("processor placement v2 tests passed");
    return 0;
}
