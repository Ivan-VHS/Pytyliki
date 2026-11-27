#include "processor_topology.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_collector_is_read_only_and_bounded(void) {
    pl_processor_topology_snapshot snapshot;
    pl_processor_snapshot cpu;
    pl_topology_status status = pl_processor_topology_collect(&snapshot, &cpu);
    if (status == PL_TOPOLOGY_UNSUPPORTED_PLATFORM) return;
    assert(status == PL_TOPOLOGY_OK);
    assert(snapshot.generation != 0u);
    assert(snapshot.logical_processor_count > 0u);
    assert(snapshot.online_logical_processor_count <= snapshot.logical_processor_count);
    assert(snapshot.logical_processor_count <= PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS);
    assert(strcmp(snapshot.collector, "linux-sysfs") == 0 ||
           strcmp(snapshot.collector, "windows-processor-topology-api") == 0);
    assert(snapshot.captured_at[0] != '\0');
}

static void test_json_envelope_and_unknown_values(void) {
    pl_processor_topology_snapshot snapshot;
    pl_topology_status status;
    FILE *stream;
    char buffer[16384];
    size_t count;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.generation = 3u;
    snapshot.logical_processor_count = 1u;
    snapshot.online_logical_processor_count = 1u;
    snapshot.logical_processors[0].logical_id = 0u;
    snapshot.logical_processors[0].os_id = 0u;
    snapshot.logical_processors[0].group_id = PL_TOPOLOGY_UNKNOWN_ID;
    snapshot.logical_processors[0].package_id = PL_TOPOLOGY_UNKNOWN_ID;
    snapshot.logical_processors[0].die_id = PL_TOPOLOGY_UNKNOWN_ID;
    snapshot.logical_processors[0].core_id = PL_TOPOLOGY_UNKNOWN_ID;
    snapshot.logical_processors[0].numa_node_id = PL_TOPOLOGY_UNKNOWN_ID;
    snapshot.logical_processors[0].online = 1u;
    (void)snprintf(snapshot.captured_at, sizeof(snapshot.captured_at),
                   "2026-01-01T00:00:00Z");
    (void)snprintf(snapshot.collector, sizeof(snapshot.collector), "fixture");
    (void)snprintf(snapshot.architecture, sizeof(snapshot.architecture), "unknown");
    stream = tmpfile();
    assert(stream != NULL);
    status = pl_processor_topology_write_json(&snapshot, stream);
    assert(status == PL_TOPOLOGY_OK);
    rewind(stream);
    count = fread(buffer, 1u, sizeof(buffer) - 1u, stream);
    buffer[count] = '\0';
    assert(strstr(buffer, "pytyliki.processor-topology-evidence.v2") != NULL);
    assert(strstr(buffer, "\"package_id\":null") != NULL);
    assert(strstr(buffer, "\"os_usable\":null") != NULL);
    assert(strstr(buffer, "\"hardware_access_performed\":false") != NULL);
    fclose(stream);
}

int main(void) {
    test_collector_is_read_only_and_bounded();
    test_json_envelope_and_unknown_values();
    puts("Pytyliki processor topology collector tests passed");
    return 0;
}
