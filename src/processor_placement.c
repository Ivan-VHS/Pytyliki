#include "processor_placement.h"

#include <inttypes.h>
#include <string.h>

static int feature_usable(const pl_processor_topology_snapshot *topology,uint32_t bit) {
    return bit<PL_TOPOLOGY_ISA_FEATURE_COUNT &&
           topology->isa[bit].os_usable==1;
}
static int cpu_has_features(const pl_processor_topology_snapshot *topology,uint32_t required) {
    for(uint32_t bit=0u;bit<PL_TOPOLOGY_ISA_FEATURE_COUNT;++bit)
        if((required&(UINT32_C(1)<<bit))!=0u && !feature_usable(topology,bit)) return 0;
    return (required>>PL_TOPOLOGY_ISA_FEATURE_COUNT)==0u;
}
static int cpu_allowed(const pl_placement_task *task,uint32_t logical_id) {
    int unrestricted=1;
    for(size_t i=0u;i<PL_TOPOLOGY_MASK_WORDS;++i)
        if(task->affinity_mask[i]!=0u) unrestricted=0;
    if(unrestricted) return 1;
    return (task->affinity_mask[logical_id/64u]&(UINT64_C(1)<<(logical_id%64u)))!=0u;
}
static uint32_t clamp_cost(uint64_t value) {
    return value>10000u?10000u:(uint32_t)value;
}
static uint64_t saturating_add(uint64_t a,uint64_t b) {
    return a>UINT64_MAX-b?UINT64_MAX:a+b;
}
static uint32_t ratio_cost(uint64_t used,uint64_t requested,uint64_t capacity) {
    uint64_t total=saturating_add(used,requested);
    if(capacity==0u) return 5000u;
    if(total>=capacity) return 10000u;
    if(total>UINT64_MAX/10000u) return 10000u;
    return (uint32_t)((total*10000u)/capacity);
}
static size_t find_numa(const pl_processor_topology_snapshot *topology,uint32_t id) {
    for(size_t i=0u;i<topology->numa_node_count;++i)
        if(topology->numa_nodes[i].node_id==id) return i;
    return topology->numa_node_count;
}
static size_t find_cpu(const pl_processor_topology_snapshot *topology,uint32_t logical_id) {
    for(size_t i=0u;i<topology->logical_cpu_count;++i)
        if(topology->logical_cpus[i].logical_id==logical_id) return i;
    return topology->logical_cpu_count;
}
static int cache_contains(const pl_topology_cache *cache,uint32_t logical_id) {
    return (cache->sharing_mask[logical_id/64u]&(UINT64_C(1)<<(logical_id%64u)))!=0u;
}
static void score_cpu(const pl_processor_topology_snapshot *topology,
                      const pl_placement_task *task,
                      const pl_placement_pressure *pressure,
                      size_t cpu_index,pl_placement_assignment *score) {
    const pl_topology_logical_cpu *cpu=&topology->logical_cpus[cpu_index];
    uint32_t id=cpu->logical_id;
    uint64_t weighted;
    uint32_t sibling_count=0u,sibling_load=0u;
    uint64_t cache_capacity=0u,cache_used=0u;
    uint32_t best_cache_level=0u;
    score->logical_cpu_id=id;
    score->cpu_load_cost=clamp_cost((uint64_t)pressure->cpu_load_basis_points[id]+task->requested_load_basis_points);
    score->capacity_cost=score->cpu_load_cost;
    if(score->cpu_load_cost>=10000u) score->reason_mask|=PL_PLACEMENT_REASON_CAPACITY_PRESSURE;

    if(task->preferred_numa_node_id!=PL_TOPOLOGY_UNKNOWN) {
        if(cpu->numa_node_id==PL_TOPOLOGY_UNKNOWN) {
            score->reason_mask|=PL_PLACEMENT_REASON_NUMA_UNKNOWN;
            score->numa_locality_cost=5000u;
        } else if(cpu->numa_node_id!=task->preferred_numa_node_id) {
            score->reason_mask|=PL_PLACEMENT_REASON_REMOTE_NUMA;
            score->numa_locality_cost=8000u;
        }
    }

    for(uint32_t i=0u;i<topology->cache_count;++i) {
        const pl_topology_cache *cache=&topology->caches[i];
        if(cache_contains(cache,id) &&
           (cache->kind==PL_TOPOLOGY_CACHE_UNIFIED || cache->kind==PL_TOPOLOGY_CACHE_DATA) &&
           cache->level>best_cache_level) {
            best_cache_level=cache->level;
            cache_capacity=cache->size_bytes;
            cache_used=pressure->cache_working_set_bytes[i];
        }
    }
    if(best_cache_level!=0u && cache_capacity!=0u) {
        score->cache_pressure_cost=ratio_cost(cache_used,task->working_set_bytes,cache_capacity);
        if(score->cache_pressure_cost!=0u) score->reason_mask|=PL_PLACEMENT_REASON_SHARED_CACHE;
    } else {
        score->cache_pressure_cost=task->working_set_bytes==0u?0u:2500u;
        if(task->working_set_bytes!=0u) score->reason_mask|=PL_PLACEMENT_REASON_CACHE_UNKNOWN;
    }

    if(cpu->package_id!=PL_TOPOLOGY_UNKNOWN && cpu->core_id!=PL_TOPOLOGY_UNKNOWN) {
        for(size_t i=0u;i<topology->logical_cpu_count;++i) {
            const pl_topology_logical_cpu *other=&topology->logical_cpus[i];
            if(other->logical_id!=id && other->online!=0u &&
               other->package_id==cpu->package_id && other->core_id==cpu->core_id) {
                ++sibling_count;
                sibling_load+=pressure->cpu_load_basis_points[other->logical_id];
            }
        }
    }
    if(sibling_count!=0u) {
        score->smt_contention_cost=clamp_cost(
            (uint64_t)sibling_load/sibling_count+(uint64_t)sibling_count*1500u);
        score->reason_mask|=PL_PLACEMENT_REASON_SHARED_CORE;
    }

    if(cpu->numa_node_id==PL_TOPOLOGY_UNKNOWN) {
        score->bandwidth_pressure_cost=task->memory_bandwidth_bytes_per_second==0u?0u:3000u;
        if(task->memory_bandwidth_bytes_per_second!=0u)
            score->reason_mask|=PL_PLACEMENT_REASON_BANDWIDTH_UNKNOWN;
    } else {
        size_t node=find_numa(topology,cpu->numa_node_id);
        if(node>=topology->numa_node_count ||
           pressure->numa_bandwidth_capacity_bytes_per_second[node]==0u) {
            score->bandwidth_pressure_cost=task->memory_bandwidth_bytes_per_second==0u?0u:3000u;
            if(task->memory_bandwidth_bytes_per_second!=0u)
                score->reason_mask|=PL_PLACEMENT_REASON_BANDWIDTH_UNKNOWN;
        } else {
            score->bandwidth_pressure_cost=ratio_cost(
                pressure->numa_bandwidth_used_bytes_per_second[node],
                task->memory_bandwidth_bytes_per_second,
                pressure->numa_bandwidth_capacity_bytes_per_second[node]);
            if(score->bandwidth_pressure_cost>=7000u)
                score->reason_mask|=PL_PLACEMENT_REASON_BANDWIDTH_PRESSURE;
        }
    }
    weighted=(uint64_t)score->cpu_load_cost*2u+
             (uint64_t)score->numa_locality_cost*3u+
             (uint64_t)score->cache_pressure_cost*2u+
             (uint64_t)score->smt_contention_cost*2u+
             (uint64_t)score->bandwidth_pressure_cost*3u+
             (uint64_t)score->capacity_cost*2u;
    score->total_cost=(uint32_t)(weighted/14u);
}
static int task_before(const pl_placement_task *a,const pl_placement_task *b) {
    return a->priority>b->priority || (a->priority==b->priority && a->task_id<b->task_id);
}
static void update_pressure(const pl_processor_topology_snapshot *topology,
                            const pl_placement_task *task,size_t cpu_index,
                            pl_placement_pressure *pressure) {
    const pl_topology_logical_cpu *selected=&topology->logical_cpus[cpu_index];
    uint32_t id=selected->logical_id;
    pressure->cpu_load_basis_points[id]=clamp_cost(
        (uint64_t)pressure->cpu_load_basis_points[id]+task->requested_load_basis_points);
    for(uint32_t i=0u;i<topology->cache_count;++i)
        if(cache_contains(&topology->caches[i],id))
            pressure->cache_working_set_bytes[i]=saturating_add(
                pressure->cache_working_set_bytes[i],task->working_set_bytes);
    if(selected->numa_node_id!=PL_TOPOLOGY_UNKNOWN) {
        size_t node=find_numa(topology,selected->numa_node_id);
        if(node<topology->numa_node_count)
            pressure->numa_bandwidth_used_bytes_per_second[node]=saturating_add(
                pressure->numa_bandwidth_used_bytes_per_second[node],
                task->memory_bandwidth_bytes_per_second);
    }
}
pl_placement_status pl_processor_placement_plan(
    const pl_processor_topology_snapshot *topology,
    const pl_placement_task *tasks,size_t task_count,
    pl_placement_pressure *pressure,pl_placement_result *out) {
    size_t order[PL_PLACEMENT_MAX_TASKS];
    if(topology==NULL || pressure==NULL || out==NULL ||
       (task_count!=0u && tasks==NULL)) return PL_PLACEMENT_INVALID_ARGUMENT;
    if(task_count>PL_PLACEMENT_MAX_TASKS) return PL_PLACEMENT_LIMIT_EXCEEDED;
    if(pressure->generation!=topology->generation) return PL_PLACEMENT_STALE_GENERATION;
    if(pl_processor_topology_validate(topology)!=PL_TOPOLOGY_OK) return PL_PLACEMENT_INVALID_TOPOLOGY;
    memset(out,0,sizeof(*out));
    out->report.generation=topology->generation;
    for(size_t i=0u;i<task_count;++i) {
        if(tasks[i].task_id==0u || tasks[i].requested_load_basis_points>10000u ||
           tasks[i].require_local_numa>1u ||
           (tasks[i].require_local_numa!=0u &&
            tasks[i].preferred_numa_node_id==PL_TOPOLOGY_UNKNOWN))
            return PL_PLACEMENT_INVALID_ARGUMENT;
        order[i]=i;
        for(size_t j=i+1u;j<task_count;++j)
            if(tasks[i].task_id==tasks[j].task_id) return PL_PLACEMENT_INVALID_ARGUMENT;
    }
    for(size_t i=1u;i<task_count;++i) {
        size_t j=i;
        while(j>0u && task_before(&tasks[order[j]],&tasks[order[j-1u]])) {
            size_t tmp=order[j]; order[j]=order[j-1u]; order[j-1u]=tmp; --j;
        }
    }
    for(size_t pos=0u;pos<task_count;++pos) {
        const pl_placement_task *task=&tasks[order[pos]];
        pl_placement_assignment best;
        size_t best_index=topology->logical_cpu_count;
        int found=0,capacity_candidate=0,locality_unknown=0,compatible_seen=0;
        memset(&best,0,sizeof(best));
        best.task_id=task->task_id;
        best.state=PL_PLACEMENT_NO_COMPATIBLE_CPU;
        best.logical_cpu_id=PL_TOPOLOGY_UNKNOWN;
        for(size_t i=0u;i<topology->logical_cpu_count;++i) {
            const pl_topology_logical_cpu *record=&topology->logical_cpus[i];
            pl_placement_assignment candidate;
            uint32_t logical_id=record->logical_id;
            if(record->online==0u || !cpu_allowed(task,logical_id) ||
               !cpu_has_features(topology,task->required_isa_mask)) continue;
            compatible_seen=1;
            if(task->require_local_numa!=0u) {
                if(record->numa_node_id==PL_TOPOLOGY_UNKNOWN) {
                    locality_unknown=1;
                    continue;
                }
                if(record->numa_node_id!=task->preferred_numa_node_id) continue;
            }
            memset(&candidate,0,sizeof(candidate));
            score_cpu(topology,task,pressure,i,&candidate);
            if(candidate.capacity_cost>=10000u) { capacity_candidate=1; continue; }
            if(!found || candidate.total_cost<best.total_cost ||
               (candidate.total_cost==best.total_cost &&
                candidate.logical_cpu_id<best.logical_cpu_id)) {
                best=candidate;
                best.state=PL_PLACEMENT_ASSIGNED;
                best_index=i;
                found=1;
            }
        }
        if(found) {
            update_pressure(topology,task,best_index,pressure);
            ++out->report.assigned;
            out->report.total_estimated_cost+=best.total_cost;
        } else if(locality_unknown) {
            best.state=PL_PLACEMENT_REQUIRED_LOCALITY_UNKNOWN;
            best.reason_mask|=PL_PLACEMENT_REASON_NUMA_UNKNOWN;
            ++out->report.locality_unknown;
        } else if(capacity_candidate) {
            best.state=PL_PLACEMENT_CAPACITY_EXHAUSTED;
            ++out->report.capacity_exhausted;
        } else {
            best.state=PL_PLACEMENT_NO_COMPATIBLE_CPU;
            if(compatible_seen) best.reason_mask|=PL_PLACEMENT_REASON_REMOTE_NUMA;
            ++out->report.no_compatible_cpu;
        }
        out->assignments[out->assignment_count++]=best;
    }
    out->report.execution_performed=0u;
    out->report.affinity_changed=0u;
    out->report.scheduling_changed=0u;
    return PL_PLACEMENT_OK;
}

pl_placement_status pl_processor_placement_write_json(const pl_placement_result *result,FILE *output) {
    if(result==NULL || output==NULL || result->assignment_count>PL_PLACEMENT_MAX_TASKS)
        return PL_PLACEMENT_INVALID_ARGUMENT;
    if(fprintf(output,"{\"schema\":\"pytyliki.processor-placement-report.v2\","
        "\"generation\":%" PRIu32 ",\"summary\":{\"assigned\":%" PRIu32
        ",\"no_compatible_cpu\":%" PRIu32 ",\"capacity_exhausted\":%" PRIu32
        ",\"locality_unknown\":%" PRIu32 ",\"total_estimated_cost\":%" PRIu64
        "},\"assignments\":[",result->report.generation,result->report.assigned,
        result->report.no_compatible_cpu,result->report.capacity_exhausted,
        result->report.locality_unknown,result->report.total_estimated_cost)<0)
        return PL_PLACEMENT_INVALID_ARGUMENT;
    for(size_t i=0u;i<result->assignment_count;++i) {
        const pl_placement_assignment *a=&result->assignments[i];
        if((i!=0u && fputc(',',output)==EOF) ||
           fprintf(output,"{\"task_id\":%" PRIu32 ",\"logical_cpu_id\":",a->task_id)<0)
            return PL_PLACEMENT_INVALID_ARGUMENT;
        if(a->logical_cpu_id==PL_TOPOLOGY_UNKNOWN) {
            if(fputs("null",output)<0) return PL_PLACEMENT_INVALID_ARGUMENT;
        } else if(fprintf(output,"%" PRIu32,a->logical_cpu_id)<0)
            return PL_PLACEMENT_INVALID_ARGUMENT;
        if(fprintf(output,",\"state\":\"%s\",\"total_cost\":%" PRIu32
            ",\"cost\":{\"cpu_load\":%" PRIu32 ",\"numa_locality\":%" PRIu32
            ",\"cache_pressure\":%" PRIu32 ",\"smt_contention\":%" PRIu32
            ",\"bandwidth_pressure\":%" PRIu32 ",\"capacity\":%" PRIu32
            "},\"reason_mask\":%" PRIu32 "}",pl_placement_state_name(a->state),
            a->total_cost,a->cpu_load_cost,a->numa_locality_cost,a->cache_pressure_cost,
            a->smt_contention_cost,a->bandwidth_pressure_cost,a->capacity_cost,a->reason_mask)<0)
            return PL_PLACEMENT_INVALID_ARGUMENT;
    }
    if(fputs("],\"execution_performed\":false,\"affinity_changed\":false,"
             "\"scheduling_changed\":false}\n",output)<0 || ferror(output))
        return PL_PLACEMENT_INVALID_ARGUMENT;
    return PL_PLACEMENT_OK;
}

const char *pl_placement_state_name(pl_placement_state s) {
    switch(s) {
    case PL_PLACEMENT_ASSIGNED:return "assigned";
    case PL_PLACEMENT_NO_COMPATIBLE_CPU:return "no-compatible-cpu";
    case PL_PLACEMENT_CAPACITY_EXHAUSTED:return "capacity-exhausted";
    case PL_PLACEMENT_REQUIRED_LOCALITY_UNKNOWN:return "required-locality-unknown";
    }
    return "unknown";
}
const char *pl_placement_status_name(pl_placement_status s) {
    switch(s) {
    case PL_PLACEMENT_OK:return "ok";
    case PL_PLACEMENT_INVALID_ARGUMENT:return "invalid-argument";
    case PL_PLACEMENT_LIMIT_EXCEEDED:return "limit-exceeded";
    case PL_PLACEMENT_STALE_GENERATION:return "stale-generation";
    case PL_PLACEMENT_INVALID_TOPOLOGY:return "invalid-topology";
    }
    return "unknown";
}
