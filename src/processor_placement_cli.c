#include "processor_placement.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_u64(const char *text, uint64_t *value) {
    char *end = NULL;
    unsigned long long parsed;
    if (text == NULL || text[0] == '\0' || text[0] == '-') return 0;
    errno = 0;
    parsed = strtoull(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0') return 0;
    *value = (uint64_t)parsed;
    return 1;
}

static int parse_task(const char *argument, pl_placement_task *task) {
    char buffer[256];
    char *parts[9];
    char *cursor;
    size_t count = 0u;
    uint64_t values[9];
    if (argument == NULL || strlen(argument) >= sizeof(buffer)) return 0;
    (void)snprintf(buffer, sizeof(buffer), "%s", argument);
    cursor = buffer;
    while (count < 9u) {
        char *comma = strchr(cursor, ',');
        if (comma != NULL) *comma = '\0';
        parts[count++] = cursor;
        if (comma == NULL) break;
        cursor = comma + 1;
    }
    if (count != 9u || strchr(parts[8], ',') != NULL) return 0;
    if (strcmp(parts[2], "unknown") == 0) values[2] = PL_TOPOLOGY_UNKNOWN;
    else if (!parse_u64(parts[2], &values[2])) return 0;
    for (size_t i = 0u; i < 9u; ++i) {
        if (i == 2u && values[2] == PL_TOPOLOGY_UNKNOWN) continue;
        if (!parse_u64(parts[i], &values[i])) return 0;
    }
    if (values[0] == 0u || values[0] > UINT32_MAX ||
        values[1] > UINT8_MAX ||
        (values[2] != PL_TOPOLOGY_UNKNOWN && values[2] > UINT32_MAX) ||
        values[3] > 10000u || values[6] > UINT32_MAX ||
        values[8] > 1u)
        return 0;

    memset(task, 0, sizeof(*task));
    task->task_id = (uint32_t)values[0];
    task->priority = (uint8_t)values[1];
    task->preferred_numa_node_id = (uint32_t)values[2];
    task->requested_load_basis_points = (uint32_t)values[3];
    task->working_set_bytes = values[4];
    task->memory_bandwidth_bytes_per_second = values[5];
    task->required_isa_mask = (uint32_t)values[6];
    task->affinity_mask[0] = values[7];
    task->require_local_numa = (uint8_t)values[8];
    return 1;
}

int main(int argc, char **argv) {
    pl_processor_topology_snapshot topology;
    pl_placement_task tasks[PL_PLACEMENT_MAX_TASKS];
    pl_placement_pressure pressure;
    pl_placement_result result;
    size_t task_count = 0u;

    if (argc < 3) {
        fprintf(stderr,
            "usage: pytyliki-placement --task ID,PRIORITY,NUMA|unknown,LOAD_BP,"
            "WORKSET_BYTES,BANDWIDTH_BPS,ISA_MASK,AFFINITY_MASK,REQUIRE_LOCAL [--task ...]\n");
        return 2;
    }
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--task") != 0 || i + 1 >= argc ||
            task_count >= PL_PLACEMENT_MAX_TASKS) {
            fprintf(stderr, "invalid arguments near: %s\n", argv[i]);
            return 2;
        }
        if (!parse_task(argv[++i], &tasks[task_count])) {
            fprintf(stderr, "invalid task descriptor: %s\n", argv[i]);
            return 2;
        }
        ++task_count;
    }
    {
        pl_topology_status status = pl_processor_topology_scan(&topology);
        if (status != PL_TOPOLOGY_OK) {
            fprintf(stderr, "topology scan failed: %s\n",
                    pl_topology_status_name(status));
            return 1;
        }
    }
    memset(&pressure, 0, sizeof(pressure));
    pressure.generation = topology.generation;
    {
        pl_placement_status status = pl_processor_placement_plan(
            &topology, tasks, task_count, &pressure, &result);
        if (status != PL_PLACEMENT_OK) {
            fprintf(stderr, "placement failed: %s\n",
                    pl_placement_status_name(status));
            return 1;
        }
    }
    if (pl_processor_placement_write_json(&result, stdout) != PL_PLACEMENT_OK)
        return 1;
    return 0;
}
