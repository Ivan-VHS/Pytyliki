#define _POSIX_C_SOURCE 200809L
#include "processor_topology.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#elif defined(__linux__)
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

static uint32_t topology_generation = 1u;

static void set_unknown_topology(pl_processor_topology_snapshot *snapshot) {
    uint32_t index;
    memset(snapshot, 0, sizeof(*snapshot));
    for (index = 0u; index < PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS; ++index) {
        snapshot->logical_processors[index].logical_id = index;
        snapshot->logical_processors[index].group_id = PL_TOPOLOGY_UNKNOWN_ID;
        snapshot->logical_processors[index].package_id = PL_TOPOLOGY_UNKNOWN_ID;
        snapshot->logical_processors[index].die_id = PL_TOPOLOGY_UNKNOWN_ID;
        snapshot->logical_processors[index].core_id = PL_TOPOLOGY_UNKNOWN_ID;
        snapshot->logical_processors[index].numa_node_id = PL_TOPOLOGY_UNKNOWN_ID;
        snapshot->logical_processors[index].evidence = PL_TOPOLOGY_EVIDENCE_UNKNOWN;
    }
}

static void set_timestamp(char output[32]) {
    time_t now = time(NULL);
    struct tm value;
    if (now == (time_t)-1) {
        (void)snprintf(output, 32u, "1970-01-01T00:00:00Z");
        return;
    }
#if defined(_WIN32)
    if (gmtime_s(&value, &now) != 0) {
        (void)snprintf(output, 32u, "1970-01-01T00:00:00Z");
        return;
    }
#else
    if (gmtime_r(&now, &value) == NULL) {
        (void)snprintf(output, 32u, "1970-01-01T00:00:00Z");
        return;
    }
#endif
    if (strftime(output, 32u, "%Y-%m-%dT%H:%M:%SZ", &value) == 0u)
        (void)snprintf(output, 32u, "1970-01-01T00:00:00Z");
}

static void set_isa_evidence(pl_processor_topology_snapshot *out,
                             const pl_processor_snapshot *cpu) {
    if (strcmp(cpu->architecture, "x86-64") == 0 ||
        strcmp(cpu->architecture, "x86") == 0) {
        out->isa_known = PL_TOPOLOGY_ISA_SSE2 | PL_TOPOLOGY_ISA_SSE42 |
            PL_TOPOLOGY_ISA_AVX | PL_TOPOLOGY_ISA_AVX2 |
            PL_TOPOLOGY_ISA_AVX512F | PL_TOPOLOGY_ISA_AES |
            PL_TOPOLOGY_ISA_FMA | PL_TOPOLOGY_ISA_VIRTUALIZE;
        if (cpu->sse2) {
            out->isa_hardware_present |= PL_TOPOLOGY_ISA_SSE2;
            out->isa_os_usable |= PL_TOPOLOGY_ISA_SSE2;
        }
        if (cpu->sse42) {
            out->isa_hardware_present |= PL_TOPOLOGY_ISA_SSE42;
            out->isa_os_usable |= PL_TOPOLOGY_ISA_SSE42;
        }
        if (cpu->avx_hw) out->isa_hardware_present |= PL_TOPOLOGY_ISA_AVX;
        if (cpu->avx) out->isa_os_usable |= PL_TOPOLOGY_ISA_AVX;
        if (cpu->avx2) {
            out->isa_hardware_present |= PL_TOPOLOGY_ISA_AVX2;
            out->isa_os_usable |= PL_TOPOLOGY_ISA_AVX2;
        }
        if (cpu->avx512f) {
            out->isa_hardware_present |= PL_TOPOLOGY_ISA_AVX512F;
            out->isa_os_usable |= PL_TOPOLOGY_ISA_AVX512F;
        }
        if (cpu->aes) {
            out->isa_hardware_present |= PL_TOPOLOGY_ISA_AES;
            out->isa_os_usable |= PL_TOPOLOGY_ISA_AES;
        }
        if (cpu->fma) {
            out->isa_hardware_present |= PL_TOPOLOGY_ISA_FMA;
            out->isa_os_usable |= PL_TOPOLOGY_ISA_FMA;
        }
        if (cpu->virtualization) {
            out->isa_hardware_present |= PL_TOPOLOGY_ISA_VIRTUALIZE;
            out->isa_os_usable |= PL_TOPOLOGY_ISA_VIRTUALIZE;
        }
    }
}

static void copy_cpu_profile(pl_processor_topology_snapshot *out,
                             const pl_processor_snapshot *cpu) {
    (void)snprintf(out->architecture, sizeof(out->architecture), "%s",
                   cpu->architecture);
    (void)snprintf(out->vendor, sizeof(out->vendor), "%s", cpu->base.vendor);
    (void)snprintf(out->model, sizeof(out->model), "%s", cpu->base.model);
    set_isa_evidence(out, cpu);
}

#if defined(__linux__)
static int read_line(const char *path, char *buffer, size_t capacity) {
    FILE *file = fopen(path, "r");
    if (file == NULL) return 0;
    if (fgets(buffer, (int)capacity, file) == NULL) {
        fclose(file);
        return 0;
    }
    fclose(file);
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 1;
}

static int read_unsigned_file(const char *path, uint32_t *value) {
    char line[128];
    char *end = NULL;
    unsigned long parsed;
    if (!read_line(path, line, sizeof(line))) return 0;
    parsed = strtoul(line, &end, 10);
    if (end == line || (*end != '\0' && *end != '\n') || parsed > UINT32_MAX)
        return 0;
    *value = (uint32_t)parsed;
    return 1;
}

static int parse_size_bytes(const char *text, uint64_t *bytes) {
    char *end = NULL;
    unsigned long long number = strtoull(text, &end, 10);
    uint64_t multiplier = 1u;
    if (end == text) return 0;
    while (*end == ' ' || *end == '\t') ++end;
    if (*end == 'K' || *end == 'k') multiplier = UINT64_C(1024);
    else if (*end == 'M' || *end == 'm') multiplier = UINT64_C(1024) * 1024u;
    else if (*end == 'G' || *end == 'g') multiplier = UINT64_C(1024) * 1024u * 1024u;
    if ((uint64_t)number > UINT64_MAX / multiplier) return 0;
    *bytes = (uint64_t)number * multiplier;
    return 1;
}

static int parse_cpu_list(const char *text, uint32_t *ids, size_t capacity,
                          size_t *count) {
    const char *cursor = text;
    size_t used = 0u;
    if (count == NULL) return 0;
    *count = 0u;
    while (*cursor != '\0') {
        char *end = NULL;
        unsigned long first = strtoul(cursor, &end, 10);
        unsigned long last = first;
        if (end == cursor || first > 65535u) return 0;
        cursor = end;
        if (*cursor == '-') {
            ++cursor;
            last = strtoul(cursor, &end, 10);
            if (end == cursor || last < first || last > 65535u) return 0;
            cursor = end;
        }
        if (last - first + 1u > capacity - used) return 0;
        while (first <= last) ids[used++] = (uint32_t)first++;
        if (*cursor == ',') ++cursor;
        else if (*cursor != '\0') return 0;
    }
    *count = used;
    return 1;
}

static int logical_index_for_os_id(const pl_processor_topology_snapshot *s,
                                   uint32_t os_id) {
    uint32_t i;
    for (i = 0u; i < s->logical_processor_count; ++i)
        if (s->logical_processors[i].os_id == os_id) return (int)i;
    return -1;
}

static void mask_set(uint64_t mask[PL_TOPOLOGY_MASK_WORDS], uint32_t index) {
    if (index < PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS)
        mask[index / 64u] |= UINT64_C(1) << (index % 64u);
}

static int mask_equal(const uint64_t a[PL_TOPOLOGY_MASK_WORDS],
                      const uint64_t b[PL_TOPOLOGY_MASK_WORDS]) {
    uint32_t i;
    for (i = 0u; i < PL_TOPOLOGY_MASK_WORDS; ++i)
        if (a[i] != b[i]) return 0;
    return 1;
}

static void collect_linux_caches(pl_processor_topology_snapshot *out,
                                 uint32_t *unique_id) {
    uint32_t cpu_index;
    for (cpu_index = 0u; cpu_index < out->logical_processor_count; ++cpu_index) {
        uint32_t cache_index;
        for (cache_index = 0u; cache_index < 16u; ++cache_index) {
            char path[256], line[128];
            uint32_t level = 0u, line_size = 0u, os_cpu_ids[PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS];
            uint64_t size_bytes = 0u, sharing[PL_TOPOLOGY_MASK_WORDS] = {0};
            size_t os_count = 0u, j;
            pl_topology_cache_kind kind = PL_TOPOLOGY_CACHE_UNKNOWN;
            int found = 0;
            (void)snprintf(path, sizeof(path),
                "/sys/devices/system/cpu/cpu%u/cache/index%u/level",
                out->logical_processors[cpu_index].os_id, cache_index);
            if (!read_unsigned_file(path, &level)) break;
            (void)snprintf(path, sizeof(path),
                "/sys/devices/system/cpu/cpu%u/cache/index%u/type",
                out->logical_processors[cpu_index].os_id, cache_index);
            if (!read_line(path, line, sizeof(line))) continue;
            if (strcmp(line, "Data") == 0) kind = PL_TOPOLOGY_CACHE_DATA;
            else if (strcmp(line, "Instruction") == 0) kind = PL_TOPOLOGY_CACHE_INSTRUCTION;
            else if (strcmp(line, "Unified") == 0) kind = PL_TOPOLOGY_CACHE_UNIFIED;
            else if (strcmp(line, "Trace") == 0) kind = PL_TOPOLOGY_CACHE_TRACE;
            (void)snprintf(path, sizeof(path),
                "/sys/devices/system/cpu/cpu%u/cache/index%u/size",
                out->logical_processors[cpu_index].os_id, cache_index);
            if (!read_line(path, line, sizeof(line)) || !parse_size_bytes(line, &size_bytes))
                continue;
            (void)snprintf(path, sizeof(path),
                "/sys/devices/system/cpu/cpu%u/cache/index%u/coherency_line_size",
                out->logical_processors[cpu_index].os_id, cache_index);
            (void)read_unsigned_file(path, &line_size);
            (void)snprintf(path, sizeof(path),
                "/sys/devices/system/cpu/cpu%u/cache/index%u/shared_cpu_list",
                out->logical_processors[cpu_index].os_id, cache_index);
            if (!read_line(path, line, sizeof(line)) ||
                !parse_cpu_list(line, os_cpu_ids, PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS, &os_count))
                continue;
            for (j = 0u; j < os_count; ++j) {
                int logical = logical_index_for_os_id(out, os_cpu_ids[j]);
                if (logical >= 0) mask_set(sharing, (uint32_t)logical);
            }
            for (j = 0u; j < out->cache_count; ++j) {
                const pl_topology_cache *old = &out->caches[j];
                if (old->level == level && old->kind == kind &&
                    old->size_bytes == size_bytes && old->line_bytes == line_size &&
                    mask_equal(old->sharing_mask, sharing)) {
                    found = 1;
                    break;
                }
            }
            if (!found && out->cache_count < PL_TOPOLOGY_MAX_CACHES) {
                pl_topology_cache *cache = &out->caches[out->cache_count++];
                memset(cache, 0, sizeof(*cache));
                cache->cache_id = (*unique_id)++;
                cache->level = level;
                cache->kind = kind;
                cache->size_bytes = size_bytes;
                cache->line_bytes = line_size;
                memcpy(cache->sharing_mask, sharing, sizeof(sharing));
                cache->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
            }
        }
    }
}

static void collect_linux_numa(pl_processor_topology_snapshot *out) {
    uint32_t node_id;
    for (node_id = 0u; node_id < PL_TOPOLOGY_MAX_NUMA_NODES; ++node_id) {
        char path[256], line[512];
        uint32_t os_ids[PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS];
        size_t os_count = 0u, i;
        pl_topology_numa_node node;
        (void)snprintf(path, sizeof(path),
                       "/sys/devices/system/node/node%u/cpulist", node_id);
        if (!read_line(path, line, sizeof(line))) continue;
        if (!parse_cpu_list(line, os_ids, PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS, &os_count))
            continue;
        memset(&node, 0, sizeof(node));
        node.node_id = node_id;
        node.evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
        for (i = 0u; i < os_count; ++i) {
            int logical = logical_index_for_os_id(out, os_ids[i]);
            if (logical >= 0) {
                mask_set(node.logical_mask, (uint32_t)logical);
                out->logical_processors[logical].numa_node_id = node_id;
            }
        }
        (void)snprintf(path, sizeof(path),
                       "/sys/devices/system/node/node%u/meminfo", node_id);
        if (read_line(path, line, sizeof(line))) {
            char *total = strstr(line, "MemTotal:");
            char *free_mem = strstr(line, "MemFree:");
            unsigned long long kib;
            if (total != NULL && sscanf(total, "MemTotal: %llu kB", &kib) == 1) {
                node.memory_total_bytes = (uint64_t)kib * UINT64_C(1024);
                node.has_memory_total = 1u;
            }
            if (free_mem != NULL && sscanf(free_mem, "MemFree: %llu kB", &kib) == 1) {
                node.memory_available_bytes = (uint64_t)kib * UINT64_C(1024);
                node.has_memory_available = 1u;
            }
        }
        if (out->numa_node_count < PL_TOPOLOGY_MAX_NUMA_NODES)
            out->numa_nodes[out->numa_node_count++] = node;
    }
}

static pl_topology_status collect_linux(pl_processor_topology_snapshot *out) {
    long configured = sysconf(_SC_NPROCESSORS_CONF);
    uint32_t os_id, count = 0u, online = 0u, cache_id = 0u;
    if (configured <= 0) return PL_TOPOLOGY_IO_ERROR;
    if (configured > (long)PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS)
        return PL_TOPOLOGY_LIMIT_EXCEEDED;
    for (os_id = 0u; os_id < (uint32_t)configured; ++os_id) {
        char path[256], line[128];
        pl_topology_logical_processor *cpu;
        uint32_t value;
        int is_online = 1;
        if (os_id != 0u) {
            (void)snprintf(path, sizeof(path),
                           "/sys/devices/system/cpu/cpu%u/online", os_id);
            if (read_line(path, line, sizeof(line))) is_online = (line[0] == '1');
        }
        (void)snprintf(path, sizeof(path),
                       "/sys/devices/system/cpu/cpu%u/topology/physical_package_id", os_id);
        if (access(path, F_OK) != 0) continue;
        cpu = &out->logical_processors[count];
        cpu->logical_id = count;
        cpu->os_id = os_id;
        cpu->online = (uint8_t)(is_online != 0);
        cpu->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
        if (is_online) ++online;
        if (read_unsigned_file(path, &value)) cpu->package_id = value;
        (void)snprintf(path, sizeof(path),
                       "/sys/devices/system/cpu/cpu%u/topology/die_id", os_id);
        if (read_unsigned_file(path, &value)) cpu->die_id = value;
        (void)snprintf(path, sizeof(path),
                       "/sys/devices/system/cpu/cpu%u/topology/core_id", os_id);
        if (read_unsigned_file(path, &value)) cpu->core_id = value;
        {
            uint32_t node_id;
            for (node_id = 0u; node_id < PL_TOPOLOGY_MAX_NUMA_NODES; ++node_id) {
                (void)snprintf(path, sizeof(path),
                    "/sys/devices/system/cpu/cpu%u/node%u", os_id, node_id);
                if (access(path, F_OK) == 0) {
                    cpu->numa_node_id = node_id;
                    break;
                }
            }
        }
        ++count;
    }
    out->logical_processor_count = count;
    out->online_logical_processor_count = online;
    if (count == 0u) return PL_TOPOLOGY_IO_ERROR;
    collect_linux_numa(out);
    collect_linux_caches(out, &cache_id);
    (void)snprintf(out->collector, sizeof(out->collector), "linux-sysfs");
    out->complete = 1u;
    for (os_id = 0u; os_id < count; ++os_id) {
        const pl_topology_logical_processor *cpu = &out->logical_processors[os_id];
        if (cpu->package_id == PL_TOPOLOGY_UNKNOWN_ID ||
            cpu->core_id == PL_TOPOLOGY_UNKNOWN_ID) out->complete = 0u;
    }
    return PL_TOPOLOGY_OK;
}
#endif

#if defined(_WIN32)
static int find_windows_cpu(pl_processor_topology_snapshot *out,
                            WORD group, BYTE number) {
    uint32_t i;
    for (i = 0u; i < out->logical_processor_count; ++i)
        if (out->logical_processors[i].group_id == (uint32_t)group &&
            out->logical_processors[i].os_id == (uint32_t)number) return (int)i;
    return -1;
}

static void apply_windows_group_mask(pl_processor_topology_snapshot *out,
                                     WORD group, KAFFINITY mask,
                                     uint32_t *field, uint32_t value,
                                     int set_numa) {
    unsigned bit;
    for (bit = 0u; bit < sizeof(KAFFINITY) * 8u; ++bit) {
        if ((mask & ((KAFFINITY)1u << bit)) != 0u) {
            int index = find_windows_cpu(out, group, (BYTE)bit);
            if (index >= 0) {
                if (set_numa) out->logical_processors[index].numa_node_id = value;
                else *(&field[index]) = value;
            }
        }
    }
}

static pl_topology_status collect_windows(pl_processor_topology_snapshot *out) {
    WORD group_count = GetActiveProcessorGroupCount();
    uint32_t dense = 0u, i;
    DWORD bytes = 0u;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX records = NULL;
    BYTE *cursor;
    uint32_t package_id = 0u, core_id = 0u, cache_id = 0u;
    if (group_count == 0u) return PL_TOPOLOGY_IO_ERROR;
    for (i = 0u; i < group_count; ++i) {
        DWORD active = GetActiveProcessorCount((WORD)i);
        DWORD bit;
        for (bit = 0u; bit < active; ++bit) {
            pl_topology_logical_processor *cpu;
            if (dense >= PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS)
                return PL_TOPOLOGY_LIMIT_EXCEEDED;
            cpu = &out->logical_processors[dense];
            cpu->logical_id = dense;
            cpu->os_id = bit;
            cpu->group_id = i;
            cpu->online = 1u;
            cpu->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
            ++dense;
        }
    }
    out->logical_processor_count = dense;
    out->online_logical_processor_count = dense;
    (void)GetLogicalProcessorInformationEx(RelationAll, NULL, &bytes);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || bytes == 0u)
        return PL_TOPOLOGY_IO_ERROR;
    records = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bytes);
    if (records == NULL) return PL_TOPOLOGY_INTERNAL_ERROR;
    if (!GetLogicalProcessorInformationEx(RelationAll, records, &bytes)) {
        free(records);
        return PL_TOPOLOGY_IO_ERROR;
    }
    cursor = (BYTE *)records;
    while (cursor < (BYTE *)records + bytes) {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX item =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)cursor;
        WORD group_index;
        if (item->Size == 0u || cursor + item->Size > (BYTE *)records + bytes) {
            free(records);
            return PL_TOPOLOGY_INTERNAL_ERROR;
        }
        if (item->Relationship == RelationProcessorPackage ||
            item->Relationship == RelationProcessorCore) {
            WORD masks = item->Processor.GroupCount;
            for (group_index = 0u; group_index < masks; ++group_index) {
                const GROUP_AFFINITY *affinity = &item->Processor.GroupMask[group_index];
                unsigned bit;
                for (bit = 0u; bit < sizeof(KAFFINITY) * 8u; ++bit) {
                    if ((affinity->Mask & ((KAFFINITY)1u << bit)) != 0u) {
                        int logical = find_windows_cpu(out, affinity->Group, (BYTE)bit);
                        if (logical >= 0) {
                            if (item->Relationship == RelationProcessorPackage)
                                out->logical_processors[logical].package_id = package_id;
                            else out->logical_processors[logical].core_id = core_id;
                        }
                    }
                }
            }
            if (item->Relationship == RelationProcessorPackage) ++package_id;
            else ++core_id;
        } else if (item->Relationship == RelationNumaNode) {
            WORD masks = item->NumaNode.GroupCount;
            for (group_index = 0u; group_index < masks; ++group_index) {
                const GROUP_AFFINITY *affinity = &item->NumaNode.GroupMasks[group_index];
                unsigned bit;
                for (bit = 0u; bit < sizeof(KAFFINITY) * 8u; ++bit) {
                    if ((affinity->Mask & ((KAFFINITY)1u << bit)) != 0u) {
                        int logical = find_windows_cpu(out, affinity->Group, (BYTE)bit);
                        if (logical >= 0)
                            out->logical_processors[logical].numa_node_id =
                                item->NumaNode.NodeNumber;
                    }
                }
            }
            if (out->numa_node_count < PL_TOPOLOGY_MAX_NUMA_NODES) {
                pl_topology_numa_node *node = &out->numa_nodes[out->numa_node_count++];
                memset(node, 0, sizeof(*node));
                node->node_id = item->NumaNode.NodeNumber;
                node->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
                for (i = 0u; i < out->logical_processor_count; ++i)
                    if (out->logical_processors[i].numa_node_id == node->node_id)
                        node->logical_mask[i / 64u] |= UINT64_C(1) << (i % 64u);
            }
        } else if (item->Relationship == RelationCache) {
            const CACHE_RELATIONSHIP *cache = &item->Cache;
            if (out->cache_count < PL_TOPOLOGY_MAX_CACHES) {
                pl_topology_cache *dest = &out->caches[out->cache_count++];
                unsigned bit;
                memset(dest, 0, sizeof(*dest));
                dest->cache_id = cache_id++;
                dest->level = cache->Level;
                dest->kind = cache->Type == CacheData ? PL_TOPOLOGY_CACHE_DATA :
                    cache->Type == CacheInstruction ? PL_TOPOLOGY_CACHE_INSTRUCTION :
                    cache->Type == CacheUnified ? PL_TOPOLOGY_CACHE_UNIFIED :
                    PL_TOPOLOGY_CACHE_UNKNOWN;
                dest->size_bytes = cache->CacheSize;
                dest->line_bytes = cache->LineSize;
                dest->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
                for (bit = 0u; bit < sizeof(KAFFINITY) * 8u; ++bit) {
                    if ((cache->GroupMask.Mask & ((KAFFINITY)1u << bit)) != 0u) {
                        int logical = find_windows_cpu(out, cache->GroupMask.Group, (BYTE)bit);
                        if (logical >= 0)
                            dest->sharing_mask[(uint32_t)logical / 64u] |=
                                UINT64_C(1) << ((uint32_t)logical % 64u);
                    }
                }
            }
        }
        cursor += item->Size;
    }
    free(records);
    (void)snprintf(out->collector, sizeof(out->collector), "windows-processor-topology-api");
    out->complete = 1u;
    for (i = 0u; i < out->logical_processor_count; ++i)
        if (out->logical_processors[i].package_id == PL_TOPOLOGY_UNKNOWN_ID ||
            out->logical_processors[i].core_id == PL_TOPOLOGY_UNKNOWN_ID)
            out->complete = 0u;
    return PL_TOPOLOGY_OK;
}
#endif

pl_topology_status pl_processor_topology_collect(
    pl_processor_topology_snapshot *out,
    pl_processor_snapshot *cpu_profile
) {
    pl_processor_snapshot local_cpu;
    pl_topology_status status;
    if (out == NULL) return PL_TOPOLOGY_INVALID_ARGUMENT;
    set_unknown_topology(out);
    if (cpu_profile == NULL) cpu_profile = &local_cpu;
    if (pl_processor_scan(cpu_profile) != 0) return PL_TOPOLOGY_IO_ERROR;
    copy_cpu_profile(out, cpu_profile);
    out->generation = topology_generation++;
    if (out->generation == 0u) out->generation = topology_generation++;
    set_timestamp(out->captured_at);
#if defined(_WIN32)
    status = collect_windows(out);
#elif defined(__linux__)
    status = collect_linux(out);
#else
    status = PL_TOPOLOGY_UNSUPPORTED_PLATFORM;
#endif
    if (status != PL_TOPOLOGY_OK) return status;
    if (out->numa_node_count == 0u || out->cache_count == 0u) out->complete = 0u;
    return PL_TOPOLOGY_OK;
}

static void json_string(FILE *output, const char *value) {
    const unsigned char *cursor = (const unsigned char *)(value ? value : "");
    fputc('"', output);
    while (*cursor != '\0') {
        switch (*cursor) {
        case '"': fputs("\\\"", output); break;
        case '\\': fputs("\\\\", output); break;
        case '\n': fputs("\\n", output); break;
        case '\r': fputs("\\r", output); break;
        case '\t': fputs("\\t", output); break;
        default:
            if (*cursor < 0x20u) fprintf(output, "\\u%04x", (unsigned)*cursor);
            else fputc((int)*cursor, output);
        }
        ++cursor;
    }
    fputc('"', output);
}

const char *pl_topology_cache_kind_name(pl_topology_cache_kind kind) {
    switch (kind) {
    case PL_TOPOLOGY_CACHE_DATA: return "data";
    case PL_TOPOLOGY_CACHE_INSTRUCTION: return "instruction";
    case PL_TOPOLOGY_CACHE_UNIFIED: return "unified";
    case PL_TOPOLOGY_CACHE_TRACE: return "trace";
    case PL_TOPOLOGY_CACHE_UNKNOWN:
    default: return "unknown";
    }
}

const char *pl_topology_evidence_name(pl_topology_evidence evidence) {
    switch (evidence) {
    case PL_TOPOLOGY_EVIDENCE_OBSERVED: return "observed";
    case PL_TOPOLOGY_EVIDENCE_PARTIAL: return "partially-observed";
    case PL_TOPOLOGY_EVIDENCE_UNKNOWN:
    default: return "unknown";
    }
}

const char *pl_topology_status_name(pl_topology_status status) {
    switch (status) {
    case PL_TOPOLOGY_OK: return "ok";
    case PL_TOPOLOGY_INVALID_ARGUMENT: return "invalid-argument";
    case PL_TOPOLOGY_UNSUPPORTED_PLATFORM: return "unsupported-platform";
    case PL_TOPOLOGY_LIMIT_EXCEEDED: return "limit-exceeded";
    case PL_TOPOLOGY_IO_ERROR: return "io-error";
    case PL_TOPOLOGY_INTERNAL_ERROR: return "internal-error";
    }
    return "unknown";
}

pl_topology_status pl_processor_topology_write_json(
    const pl_processor_topology_snapshot *snapshot, FILE *output
) {
    uint32_t i, j;
    if (snapshot == NULL || output == NULL || snapshot->generation == 0u ||
        snapshot->logical_processor_count == 0u ||
        snapshot->logical_processor_count > PL_TOPOLOGY_MAX_LOGICAL_PROCESSORS ||
        snapshot->cache_count > PL_TOPOLOGY_MAX_CACHES ||
        snapshot->numa_node_count > PL_TOPOLOGY_MAX_NUMA_NODES)
        return PL_TOPOLOGY_INVALID_ARGUMENT;
    fputs("{\"schema\":\"pytyliki.processor-topology-evidence.v2\",\"version\":2", output);
    fprintf(output, ",\"generation\":%u,\"captured_at\":", snapshot->generation);
    json_string(output, snapshot->captured_at);
    fputs(",\"collector\":{\"provider\":\"Pytyliki\",\"interface\":", output);
    json_string(output, snapshot->collector);
    fputs(",\"read_only\":true,\"platform_release\":null}", output);
    fprintf(output, ",\"completeness\":\"%s\",\"topology\":{"
                    "\"logical_processor_count\":%u,\"online_logical_processor_count\":%u,\"logical_processors\":[",
        snapshot->complete ? "complete" : "partial",
        snapshot->logical_processor_count, snapshot->online_logical_processor_count);
    for (i = 0u; i < snapshot->logical_processor_count; ++i) {
        const pl_topology_logical_processor *cpu = &snapshot->logical_processors[i];
        if (i != 0u) fputc(',', output);
        fprintf(output, "{\"logical_id\":%u,\"os_id\":%u,\"online\":%s,"
                        "\"package_id\":",
                cpu->logical_id, cpu->os_id, cpu->online ? "true" : "false");
        if (cpu->package_id == PL_TOPOLOGY_UNKNOWN_ID) fputs("null", output);
        else fprintf(output, "%u", cpu->package_id);
        fputs(",\"die_id\":", output);
        if (cpu->die_id == PL_TOPOLOGY_UNKNOWN_ID) fputs("null", output);
        else fprintf(output, "%u", cpu->die_id);
        fputs(",\"core_id\":", output);
        if (cpu->core_id == PL_TOPOLOGY_UNKNOWN_ID) fputs("null", output);
        else fprintf(output, "%u", cpu->core_id);
        fputs(",\"numa_node_id\":", output);
        if (cpu->numa_node_id == PL_TOPOLOGY_UNKNOWN_ID) fputs("null", output);
        else fprintf(output, "%u", cpu->numa_node_id);
        fprintf(output, ",\"group_id\":");
        if (cpu->group_id == PL_TOPOLOGY_UNKNOWN_ID) fputs("null", output);
        else fprintf(output, "%u", cpu->group_id);
        fputs(",\"evidence\":", output);
        json_string(output, pl_topology_evidence_name(cpu->evidence));
        fputc('}', output);
    }
    fputs("],\"caches\":[", output);
    for (i = 0u; i < snapshot->cache_count; ++i) {
        const pl_topology_cache *cache = &snapshot->caches[i];
        int first = 1;
        if (i != 0u) fputc(',', output);
        fprintf(output, "{\"cache_id\":%u,\"level\":%u,\"kind\":",
                cache->cache_id, cache->level);
        json_string(output, pl_topology_cache_kind_name(cache->kind));
        fprintf(output, ",\"size_bytes\":%" PRIu64 ",\"line_bytes\":%u,\"sharing_logical_ids\":[",
                cache->size_bytes, cache->line_bytes);
        for (j = 0u; j < snapshot->logical_processor_count; ++j) {
            if ((cache->sharing_mask[j / 64u] &
                 (UINT64_C(1) << (j % 64u))) != 0u) {
                if (!first) fputc(',', output);
                fprintf(output, "%u", j);
                first = 0;
            }
        }
        fputs("],\"evidence\":", output);
        json_string(output, pl_topology_evidence_name(cache->evidence));
        fputc('}', output);
    }
    fputs("],\"numa_nodes\":[", output);
    for (i = 0u; i < snapshot->numa_node_count; ++i) {
        const pl_topology_numa_node *node = &snapshot->numa_nodes[i];
        int first = 1;
        if (i != 0u) fputc(',', output);
        fprintf(output, "{\"node_id\":%u,\"logical_ids\":[", node->node_id);
        for (j = 0u; j < snapshot->logical_processor_count; ++j) {
            if ((node->logical_mask[j / 64u] &
                 (UINT64_C(1) << (j % 64u))) != 0u) {
                if (!first) fputc(',', output);
                fprintf(output, "%u", j);
                first = 0;
            }
        }
        fputs("],\"memory_total_bytes\":", output);
        if (node->has_memory_total) fprintf(output, "%" PRIu64, node->memory_total_bytes);
        else fputs("null", output);
        fputs(",\"memory_available_bytes\":", output);
        if (node->has_memory_available) fprintf(output, "%" PRIu64, node->memory_available_bytes);
        else fputs("null", output);
        fputs(",\"evidence\":", output);
        json_string(output, pl_topology_evidence_name(node->evidence));
        fputc('}', output);
    }
    fprintf(output,
        "]},\"isa\":{\"architecture\":");
    json_string(output, snapshot->architecture);
    fputs(",\"vendor\":", output); json_string(output, snapshot->vendor);
    fputs(",\"model\":", output); json_string(output, snapshot->model);
    fputs(",\"features\":{", output);
#define WRITE_FEATURE(name, flag) do { \
    if (!first_feature) fputc(',', output); first_feature = 0; \
    fprintf(output, "\\\"%s\\\":{\\\"hardware_present\\\":", name); \
    if ((snapshot->isa_known & (flag)) == 0u) fputs("null", output); \
    else fputs((snapshot->isa_hardware_present & (flag)) ? "true" : "false", output); \
    fputs(",\\\"os_usable\\\":", output); \
    if ((snapshot->isa_known & (flag)) == 0u) fputs("null", output); \
    else fputs((snapshot->isa_os_usable & (flag)) ? "true" : "false", output); \
    fputs(",\\\"evidence\\\":\\\"cpuid\\\"}", output); \
} while (0)
    {
        int first_feature = 1;
        WRITE_FEATURE("sse2", PL_TOPOLOGY_ISA_SSE2);
        WRITE_FEATURE("sse4_2", PL_TOPOLOGY_ISA_SSE42);
        WRITE_FEATURE("avx", PL_TOPOLOGY_ISA_AVX);
        WRITE_FEATURE("avx2", PL_TOPOLOGY_ISA_AVX2);
        WRITE_FEATURE("avx512f", PL_TOPOLOGY_ISA_AVX512F);
        WRITE_FEATURE("aes", PL_TOPOLOGY_ISA_AES);
        WRITE_FEATURE("fma", PL_TOPOLOGY_ISA_FMA);
        WRITE_FEATURE("virtualization", PL_TOPOLOGY_ISA_VIRTUALIZE);
    }
#undef WRITE_FEATURE
    fputs("}},\"execution_performed\":false,\"hardware_access_performed\":false,"
          "\"processor_state_changed\":false}\n", output);
    return ferror(output) ? PL_TOPOLOGY_IO_ERROR : PL_TOPOLOGY_OK;
}
