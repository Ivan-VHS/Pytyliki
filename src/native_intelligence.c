#define _GNU_SOURCE
#include "native_intelligence.h"
#include "processor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <intrin.h>
#else
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#endif

#define PL_NI_UNKNOWN_ID UINT16_MAX
#define PL_NI_COST_MAX UINT32_C(1000)
#define PL_NI_KNOWN_ISA_MASK (PL_NI_ISA_SSE2 | PL_NI_ISA_SSE42 | PL_NI_ISA_AES | \
                              PL_NI_ISA_AVX | PL_NI_ISA_AVX2 | PL_NI_ISA_FMA | \
                              PL_NI_ISA_AVX512F)

static void copy_text(char *dst, size_t capacity, const char *text) {
    if (capacity != 0u) (void)snprintf(dst, capacity, "%s", text != NULL ? text : "");
}

static void set_bit(uint64_t *mask, size_t bit) {
    if (mask != NULL && bit < PL_NI_MAX_CPUS)
        mask[bit / 64u] |= UINT64_C(1) << (bit % 64u);
}

static int bit_is_set(const uint64_t *mask, size_t bit) {
    return mask != NULL && bit < PL_NI_MAX_CPUS &&
           (mask[bit / 64u] & (UINT64_C(1) << (bit % 64u))) != 0u;
}

static int add_cpu(pl_ni_snapshot *snapshot, uint16_t group, uint16_t os_id) {
    uint32_t i;
    for (i = 0u; i < snapshot->cpu_count; ++i) {
        if (snapshot->cpus[i].group_id == group &&
            snapshot->cpus[i].os_cpu_id == os_id) return (int)i;
    }
    if (snapshot->cpu_count >= PL_NI_MAX_CPUS) return -1;
    i = snapshot->cpu_count++;
    memset(&snapshot->cpus[i], 0, sizeof(snapshot->cpus[i]));
    snapshot->cpus[i].group_id = group;
    snapshot->cpus[i].os_cpu_id = os_id;
    snapshot->cpus[i].package_id = PL_NI_UNKNOWN_ID;
    snapshot->cpus[i].die_id = PL_NI_UNKNOWN_ID;
    snapshot->cpus[i].core_id = PL_NI_UNKNOWN_ID;
    snapshot->cpus[i].numa_node = PL_NI_UNKNOWN_ID;
    snapshot->cpus[i].thread_index = PL_NI_UNKNOWN_ID;
    snapshot->cpus[i].flags = PL_NI_CPU_ONLINE;
    return (int)i;
}

static int cpu_index_by_os_id(const pl_ni_snapshot *snapshot, uint16_t group,
                              uint16_t os_id) {
    uint32_t i;
    for (i = 0u; i < snapshot->cpu_count; ++i)
        if (snapshot->cpus[i].group_id == group &&
            snapshot->cpus[i].os_cpu_id == os_id) return (int)i;
    return -1;
}

static int add_numa(pl_ni_snapshot *snapshot, uint16_t id) {
    uint32_t i;
    for (i = 0u; i < snapshot->numa_count; ++i)
        if (snapshot->numa[i].id == id) return (int)i;
    if (snapshot->numa_count >= PL_NI_MAX_NUMA_NODES) return -1;
    i = snapshot->numa_count++;
    memset(&snapshot->numa[i], 0, sizeof(snapshot->numa[i]));
    snapshot->numa[i].id = id;
    return (int)i;
}

static int add_cache(pl_ni_snapshot *snapshot, const pl_ni_cache *cache) {
    uint32_t i;
    if (snapshot->cache_count >= PL_NI_MAX_CACHES) return -1;
    i = snapshot->cache_count++;
    snapshot->caches[i] = *cache;
    return (int)i;
}

static uint64_t isa_from_processor(const pl_processor_snapshot *p) {
    uint64_t value = 0u;
    if (p->sse2) value |= PL_NI_ISA_SSE2;
    if (p->sse42) value |= PL_NI_ISA_SSE42;
    if (p->aes) value |= PL_NI_ISA_AES;
    if (p->avx) value |= PL_NI_ISA_AVX;
    if (p->avx2) value |= PL_NI_ISA_AVX2;
    if (p->fma) value |= PL_NI_ISA_FMA;
    if (p->avx512f) value |= PL_NI_ISA_AVX512F;
    return value;
}

const char *pl_ni_status_string(pl_ni_status status) {
    static const char *const names[] = {
        "ok", "invalid_argument", "unsupported", "io_error",
        "capacity_exceeded", "inconsistent", "stale_generation",
        "out_of_range", "access_denied", "invalid_state"
    };
    return (unsigned)status < sizeof(names) / sizeof(names[0])
        ? names[(unsigned)status] : "unknown";
}

uint64_t pl_ni_snapshot_fingerprint(const pl_ni_snapshot *snapshot) {
    const unsigned char *bytes;
    size_t length, i;
    uint64_t hash = UINT64_C(14695981039346656037);
    if (snapshot == NULL || snapshot->cpu_count > PL_NI_MAX_CPUS ||
        snapshot->cache_count > PL_NI_MAX_CACHES ||
        snapshot->numa_count > PL_NI_MAX_NUMA_NODES)
        return 0u;
    bytes = (const unsigned char *)&snapshot->abi_version;
    length = offsetof(pl_ni_snapshot, provenance) - offsetof(pl_ni_snapshot, abi_version);
    for (i = 0u; i < length; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    bytes = (const unsigned char *)snapshot->cpus;
    length = (size_t)snapshot->cpu_count * sizeof(snapshot->cpus[0]);
    for (i = 0u; i < length; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    bytes = (const unsigned char *)snapshot->caches;
    length = (size_t)snapshot->cache_count * sizeof(snapshot->caches[0]);
    for (i = 0u; i < length; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    bytes = (const unsigned char *)snapshot->numa;
    length = (size_t)snapshot->numa_count * sizeof(snapshot->numa[0]);
    for (i = 0u; i < length; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    bytes = (const unsigned char *)snapshot->numa_distance;
    length = (size_t)snapshot->numa_count * PL_NI_MAX_NUMA_NODES *
             sizeof(snapshot->numa_distance[0][0]);
    for (i = 0u; i < length; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

pl_ni_status pl_ni_snapshot_validate(const pl_ni_snapshot *snapshot) {
    uint32_t i, j;
    if (snapshot == NULL || snapshot->abi_version != PL_NI_ABI_VERSION ||
        snapshot->cpu_count == 0u || snapshot->cpu_count > PL_NI_MAX_CPUS ||
        snapshot->cache_count > PL_NI_MAX_CACHES ||
        snapshot->numa_count > PL_NI_MAX_NUMA_NODES ||
        snapshot->available_memory_bytes > snapshot->total_memory_bytes ||
        (snapshot->isa_mask & ~PL_NI_KNOWN_ISA_MASK) != 0u ||
        snapshot->provenance.schema_version != PL_NI_ABI_VERSION ||
        snapshot->provenance.topology_state < PL_NI_FACT_KNOWN ||
        snapshot->provenance.topology_state > PL_NI_FACT_CONFLICTING ||
        snapshot->provenance.allowed_state < PL_NI_FACT_KNOWN ||
        snapshot->provenance.allowed_state > PL_NI_FACT_CONFLICTING ||
        snapshot->provenance.isa_state < PL_NI_FACT_KNOWN ||
        snapshot->provenance.isa_state > PL_NI_FACT_CONFLICTING ||
        snapshot->provenance.memory_state < PL_NI_FACT_KNOWN ||
        snapshot->provenance.memory_state > PL_NI_FACT_CONFLICTING ||
        snapshot->provenance.isa_scope > PL_NI_ISA_SCOPE_PER_LOGICAL_CPU ||
        snapshot->provenance.platform[0] == '\0' ||
        snapshot->provenance.topology_source[0] == '\0' ||
        (snapshot->provenance.allowed_state == PL_NI_FACT_KNOWN &&
         snapshot->provenance.allowed_source[0] == '\0') ||
        snapshot->provenance.snapshot_fingerprint == 0u ||
        snapshot->provenance.generation != snapshot->provenance.snapshot_fingerprint ||
        snapshot->provenance.snapshot_fingerprint != pl_ni_snapshot_fingerprint(snapshot))
        return PL_NI_INVALID_ARGUMENT;
    for (i = 0u; i < snapshot->cpu_count; ++i) {
        const pl_ni_cpu *cpu = &snapshot->cpus[i];
        if ((cpu->flags & ~(PL_NI_CPU_ONLINE | PL_NI_CPU_ALLOWED |
                            PL_NI_CPU_ALLOWED_KNOWN)) != 0u ||
            ((cpu->flags & PL_NI_CPU_ALLOWED) != 0u &&
             (cpu->flags & PL_NI_CPU_ALLOWED_KNOWN) == 0u) ||
            (cpu->isa_mask & ~PL_NI_KNOWN_ISA_MASK) != 0u)
            return PL_NI_INCONSISTENT;
        for (j = i + 1u; j < snapshot->cpu_count; ++j)
            if (cpu->group_id == snapshot->cpus[j].group_id &&
                cpu->os_cpu_id == snapshot->cpus[j].os_cpu_id)
                return PL_NI_INCONSISTENT;
    }
    for (i = 0u; i < snapshot->cache_count; ++i) {
        const pl_ni_cache *cache = &snapshot->caches[i];
        uint32_t sharers = 0u;
        if (cache->level == 0u || cache->size_bytes == 0u ||
            cache->sharing_count == 0u) return PL_NI_INCONSISTENT;
        for (j = 0u; j < snapshot->cpu_count; ++j)
            if (bit_is_set(cache->sharing_mask, j)) ++sharers;
        if (sharers != cache->sharing_count) return PL_NI_INCONSISTENT;
        for (j = snapshot->cpu_count; j < PL_NI_MAX_CPUS; ++j)
            if (bit_is_set(cache->sharing_mask, j)) return PL_NI_INCONSISTENT;
    }
    for (i = 0u; i < snapshot->numa_count; ++i) {
        const pl_ni_numa_node *node = &snapshot->numa[i];
        uint32_t count = 0u;
        for (j = 0u; j < snapshot->cpu_count; ++j)
            if (bit_is_set(node->cpu_mask, j)) ++count;
        if (count != node->cpu_count) return PL_NI_INCONSISTENT;
    }
    return PL_NI_OK;
}

#if defined(_WIN32)
static int apply_group_mask(pl_ni_snapshot *snapshot, WORD group,
                            KAFFINITY mask, uint16_t package,
                            uint16_t die, uint16_t core, uint16_t numa,
                            int relation_kind) {
    unsigned bit, ordinal = 0u;
    for (bit = 0u; bit < sizeof(KAFFINITY) * CHAR_BIT; ++bit) {
        int index;
        if ((mask & ((KAFFINITY)1u << bit)) == 0u) continue;
        index = add_cpu(snapshot, (uint16_t)group, (uint16_t)bit);
        if (index < 0) return 0;
        if (relation_kind == 1 && package != PL_NI_UNKNOWN_ID)
            snapshot->cpus[index].package_id = package;
        if (relation_kind == 2) {
            snapshot->cpus[index].core_id = core;
            snapshot->cpus[index].thread_index = (uint16_t)ordinal;
        }
        if (relation_kind == 3) snapshot->cpus[index].die_id = die;
        if (relation_kind == 4) snapshot->cpus[index].numa_node = numa;
        ++ordinal;
    }
    return 1;
}

static pl_ni_status collect_windows(pl_ni_snapshot *snapshot) {
    DWORD required = 0u, length = 0u;
    unsigned char *buffer = NULL;
    unsigned char *cursor;
    uint16_t core_id = 0u, package_id = 0u, die_id = 0u;
    WORD group_count, group;
    if (!GetLogicalProcessorInformationEx(RelationAll, NULL, &required) &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER) return PL_NI_IO_ERROR;
    if (required == 0u) return PL_NI_IO_ERROR;
    buffer = (unsigned char *)malloc((size_t)required);
    if (buffer == NULL) return PL_NI_IO_ERROR;
    length = required;
    if (!GetLogicalProcessorInformationEx(RelationAll,
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &length)) {
        free(buffer);
        return PL_NI_IO_ERROR;
    }
    cursor = buffer;
    while ((size_t)(cursor - buffer) < (size_t)length) {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)cursor;
        if (info->Size == 0u || (size_t)(cursor - buffer) + info->Size > length) {
            free(buffer);
            return PL_NI_INCONSISTENT;
        }
        if (info->Relationship == RelationProcessorPackage) {
            PROCESSOR_RELATIONSHIP *p = &info->Processor;
            size_t required_size;
            if (p->GroupCount == 0u) { free(buffer); return PL_NI_INCONSISTENT; }
            required_size = sizeof(PROCESSOR_RELATIONSHIP) +
                ((size_t)p->GroupCount - 1u) * sizeof(GROUP_AFFINITY);
            if (required_size > info->Size) { free(buffer); return PL_NI_INCONSISTENT; }
            for (group = 0u; group < p->GroupCount; ++group)
                if (!apply_group_mask(snapshot, p->GroupMask[group].Group,
                    p->GroupMask[group].Mask, package_id, 0u, 0u,
                    PL_NI_UNKNOWN_ID, 1)) {
                    free(buffer); return PL_NI_CAPACITY_EXCEEDED;
                }
            ++package_id;
        } else if (info->Relationship == RelationProcessorCore) {
            PROCESSOR_RELATIONSHIP *p = &info->Processor;
            size_t required_size;
            if (p->GroupCount == 0u) { free(buffer); return PL_NI_INCONSISTENT; }
            required_size = sizeof(PROCESSOR_RELATIONSHIP) +
                ((size_t)p->GroupCount - 1u) * sizeof(GROUP_AFFINITY);
            if (required_size > info->Size) { free(buffer); return PL_NI_INCONSISTENT; }
            for (group = 0u; group < p->GroupCount; ++group)
                if (!apply_group_mask(snapshot, p->GroupMask[group].Group,
                    p->GroupMask[group].Mask, 0u, 0u, core_id,
                    PL_NI_UNKNOWN_ID, 2)) {
                    free(buffer); return PL_NI_CAPACITY_EXCEEDED;
                }
            ++core_id;
        } else if (info->Relationship == RelationProcessorDie) {
            PROCESSOR_RELATIONSHIP *p = &info->Processor;
            size_t required_size;
            if (p->GroupCount == 0u) { free(buffer); return PL_NI_INCONSISTENT; }
            required_size = sizeof(PROCESSOR_RELATIONSHIP) +
                ((size_t)p->GroupCount - 1u) * sizeof(GROUP_AFFINITY);
            if (required_size > info->Size) { free(buffer); return PL_NI_INCONSISTENT; }
            for (group = 0u; group < p->GroupCount; ++group)
                if (!apply_group_mask(snapshot, p->GroupMask[group].Group,
                    p->GroupMask[group].Mask, 0u, die_id, 0u,
                    PL_NI_UNKNOWN_ID, 3)) {
                    free(buffer); return PL_NI_CAPACITY_EXCEEDED;
                }
            ++die_id;
        } else if (info->Relationship == RelationNumaNode ||
                   info->Relationship == RelationNumaNodeEx) {
            NUMA_NODE_RELATIONSHIP *p = &info->NumaNode;
            WORD masks_count = p->GroupCount != 0u ? p->GroupCount : 1u;
            GROUP_AFFINITY *masks = p->GroupCount != 0u ? p->GroupMasks : &p->GroupMask;
            size_t required_size = sizeof(NUMA_NODE_RELATIONSHIP);
            if (p->GroupCount != 0u)
                required_size += ((size_t)p->GroupCount - 1u) * sizeof(GROUP_AFFINITY);
            if (required_size > info->Size) { free(buffer); return PL_NI_INCONSISTENT; }
            int node_index = add_numa(snapshot, (uint16_t)p->NodeNumber);
            if (node_index < 0) { free(buffer); return PL_NI_CAPACITY_EXCEEDED; }
            {
                WORD mask_index;
                for (mask_index = 0u; mask_index < masks_count; ++mask_index) {
                    unsigned bit;
                    for (bit = 0u; bit < sizeof(KAFFINITY) * CHAR_BIT; ++bit) {
                        if ((masks[mask_index].Mask & ((KAFFINITY)1u << bit)) != 0u) {
                            int cpu = add_cpu(snapshot, masks[mask_index].Group, (uint16_t)bit);
                            if (cpu < 0) { free(buffer); return PL_NI_CAPACITY_EXCEEDED; }
                            snapshot->cpus[cpu].numa_node = (uint16_t)p->NodeNumber;
                            set_bit(snapshot->numa[node_index].cpu_mask, (size_t)cpu);
                        }
                    }
                }
            }
        } else if (info->Relationship == RelationCache) {
            CACHE_RELATIONSHIP *p = &info->Cache;
            pl_ni_cache cache;
            WORD masks_count = p->GroupCount != 0u ? p->GroupCount : 1u;
            GROUP_AFFINITY *masks = p->GroupCount != 0u ? p->GroupMasks : &p->GroupMask;
            size_t required_size = sizeof(CACHE_RELATIONSHIP);
            WORD mask_index;
            int cache_index;
            memset(&cache, 0, sizeof(cache));
            if (p->GroupCount > 1u)
                required_size += ((size_t)p->GroupCount - 1u) * sizeof(GROUP_AFFINITY);
            if (required_size > info->Size) { free(buffer); return PL_NI_INCONSISTENT; }
            cache.level = p->Level;
            cache.kind = p->Type == CacheData ? 1u :
                         p->Type == CacheInstruction ? 2u :
                         p->Type == CacheUnified ? 3u : 4u;
            cache.group_id = masks_count == 1u ? masks[0].Group : PL_NI_UNKNOWN_ID;
            cache.size_bytes = p->CacheSize;
            cache.line_bytes = p->LineSize;
            cache.associativity = p->Associativity;
            for (mask_index = 0u; mask_index < masks_count; ++mask_index) {
                unsigned bit;
                for (bit = 0u; bit < sizeof(KAFFINITY) * CHAR_BIT; ++bit) {
                    if ((masks[mask_index].Mask & ((KAFFINITY)1u << bit)) != 0u) {
                        int cpu = add_cpu(snapshot, masks[mask_index].Group, (uint16_t)bit);
                        if (cpu < 0) { free(buffer); return PL_NI_CAPACITY_EXCEEDED; }
                        set_bit(cache.sharing_mask, (size_t)cpu);
                        ++cache.sharing_count;
                    }
                }
            }
            cache_index = add_cache(snapshot, &cache);
            if (cache_index < 0) { free(buffer); return PL_NI_CAPACITY_EXCEEDED; }
        }
        cursor += info->Size;
    }
    free(buffer);
    group_count = GetActiveProcessorGroupCount();
    for (group = 0u; group < group_count; ++group) {
        DWORD active = GetActiveProcessorCount(group);
        DWORD cpu;
        for (cpu = 0u; cpu < active; ++cpu) {
            int index = add_cpu(snapshot, group, (uint16_t)cpu);
            if (index < 0) return PL_NI_CAPACITY_EXCEEDED;
        }
    }
    {
        WORD groups[64];
        USHORT actual = 64u;
        int allowed_known = 0;
        if (GetProcessGroupAffinity(GetCurrentProcess(), &actual, groups) &&
            actual == 1u) {
            DWORD_PTR process_mask = 0u, system_mask = 0u;
            if (GetProcessAffinityMask(GetCurrentProcess(), &process_mask, &system_mask)) {
                uint32_t i;
                for (i = 0u; i < snapshot->cpu_count; ++i) {
                    pl_ni_cpu *cpu = &snapshot->cpus[i];
                    cpu->flags |= PL_NI_CPU_ALLOWED_KNOWN;
                    if (cpu->group_id == groups[0] &&
                        cpu->os_cpu_id < sizeof(DWORD_PTR) * CHAR_BIT &&
                        (process_mask & ((DWORD_PTR)1u << cpu->os_cpu_id)) != 0u)
                        cpu->flags |= PL_NI_CPU_ALLOWED;
                }
                allowed_known = 1;
            }
        }
        snapshot->provenance.allowed_state =
            allowed_known ? PL_NI_FACT_KNOWN : PL_NI_FACT_UNAVAILABLE;
        copy_text(snapshot->provenance.allowed_source,
                  sizeof(snapshot->provenance.allowed_source),
                  allowed_known ? "GetProcessGroupAffinity + GetProcessAffinityMask"
                                : "unavailable for multi-group process");
    }
    for (group = 0u; group < snapshot->numa_count; ++group) {
        unsigned i;
        for (i = 0u; i < snapshot->cpu_count; ++i)
            if (snapshot->cpus[i].numa_node == snapshot->numa[group].id)
                ++snapshot->numa[group].cpu_count;
    }
    copy_text(snapshot->provenance.topology_source,
              sizeof(snapshot->provenance.topology_source),
              "GetLogicalProcessorInformationEx");
    copy_text(snapshot->provenance.memory_source,
              sizeof(snapshot->provenance.memory_source), "GlobalMemoryStatusEx");
    copy_text(snapshot->provenance.platform, sizeof(snapshot->provenance.platform), "windows");
    return snapshot->cpu_count != 0u ? PL_NI_OK : PL_NI_IO_ERROR;
}
#else
static int read_text(const char *path, char *buffer, size_t capacity) {
    FILE *file;
    size_t count;
    if (path == NULL || buffer == NULL || capacity < 2u) return 0;
    file = fopen(path, "rb");
    if (file == NULL) return 0;
    count = fread(buffer, 1u, capacity - 1u, file);
    if (ferror(file)) { fclose(file); return 0; }
    fclose(file);
    buffer[count] = '\0';
    while (count != 0u && (buffer[count - 1u] == '\n' ||
           buffer[count - 1u] == '\r' || buffer[count - 1u] == ' '))
        buffer[--count] = '\0';
    return count != 0u;
}

static int read_u32(const char *path, uint32_t *value) {
    char buffer[96], *end = NULL;
    unsigned long parsed;
    if (!read_text(path, buffer, sizeof(buffer))) return 0;
    errno = 0;
    parsed = strtoul(buffer, &end, 10);
    if (errno != 0 || end == buffer || *end != '\0' || parsed > UINT32_MAX) return 0;
    *value = (uint32_t)parsed;
    return 1;
}

static uint32_t parse_size_bytes(const char *text) {
    char *end = NULL;
    unsigned long long value;
    unsigned long long scale = 1u;
    value = strtoull(text, &end, 10);
    if (end == text) return 0u;
    if (*end == 'K' || *end == 'k') scale = 1024u;
    else if (*end == 'M' || *end == 'm') scale = 1024u * 1024u;
    else if (*end == 'G' || *end == 'g') scale = 1024u * 1024u * 1024u;
    if (value > UINT32_MAX / scale) return UINT32_MAX;
    return (uint32_t)(value * scale);
}

static void list_to_mask(const pl_ni_snapshot *snapshot, const char *list,
                         uint64_t *mask, uint32_t *count) {
    const char *cursor = list;
    if (count != NULL) *count = 0u;
    while (*cursor != '\0') {
        char *end = NULL;
        unsigned long first = strtoul(cursor, &end, 10);
        unsigned long last = first;
        if (end == cursor) break;
        cursor = end;
        if (*cursor == '-') {
            cursor++;
            last = strtoul(cursor, &end, 10);
            if (end == cursor) break;
            cursor = end;
        }
        if (first < PL_NI_MAX_CPUS && last < PL_NI_MAX_CPUS &&
            last >= first && last - first < PL_NI_MAX_CPUS) {
            unsigned long id;
            for (id = first; id <= last; ++id) {
                int index = cpu_index_by_os_id(snapshot, 0u, (uint16_t)id);
                if (index >= 0 && !bit_is_set(mask, (size_t)index)) {
                    set_bit(mask, (size_t)index);
                    if (count != NULL) ++*count;
                }
            }
        }
        if (*cursor == ',') ++cursor;
        else if (*cursor != '\0') break;
    }
}

static uint16_t thread_index_from_list(const char *list, uint16_t target) {
    const char *cursor = list;
    uint32_t ordinal = 0u;
    while (*cursor != '\0') {
        char *end = NULL;
        unsigned long first = strtoul(cursor, &end, 10);
        unsigned long last = first;
        unsigned long id;
        if (end == cursor) break;
        cursor = end;
        if (*cursor == '-') {
            ++cursor;
            last = strtoul(cursor, &end, 10);
            if (end == cursor) break;
            cursor = end;
        }
        if (last >= first && last - first < PL_NI_MAX_CPUS) {
            for (id = first; id <= last; ++id) {
                if (id == target && ordinal <= UINT16_MAX)
                    return (uint16_t)ordinal;
                ++ordinal;
            }
        }
        if (*cursor == ',') ++cursor;
        else if (*cursor != '\0') break;
    }
    return PL_NI_UNKNOWN_ID;
}

static void collect_linux_numa_distances(pl_ni_snapshot *snapshot) {
    uint32_t i;
    for (i = 0u; i < snapshot->numa_count; ++i) {
        char path[160], buffer[1024];
        const char *cursor;
        uint16_t target_id = 0u;
        (void)snprintf(path, sizeof(path),
            "/sys/devices/system/node/node%u/distance",
            (unsigned)snapshot->numa[i].id);
        if (!read_text(path, buffer, sizeof(buffer))) continue;
        cursor = buffer;
        while (*cursor != '\0') {
            char *end = NULL;
            unsigned long distance;
            while (*cursor == ' ' || *cursor == '\t') ++cursor;
            if (*cursor == '\0') break;
            distance = strtoul(cursor, &end, 10);
            if (end == cursor) break;
            cursor = end;
            if (distance <= UINT16_MAX) {
                uint32_t j;
                for (j = 0u; j < snapshot->numa_count; ++j) {
                    if (snapshot->numa[j].id == target_id) {
                        snapshot->numa_distance[i][j] = (uint16_t)distance;
                        break;
                    }
                }
            }
            if (target_id != UINT16_MAX) ++target_id;
        }
    }
}

static int line_has_flag(const char *flags, const char *wanted) {
    size_t wanted_length = strlen(wanted);
    const char *cursor = flags;
    while (*cursor != '\0') {
        const char *start;
        size_t length;
        while (*cursor == ' ' || *cursor == '\t') ++cursor;
        start = cursor;
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' &&
               *cursor != '\r' && *cursor != '\n') ++cursor;
        length = (size_t)(cursor - start);
        if (length == wanted_length && memcmp(start, wanted, length) == 0)
            return 1;
    }
    return 0;
}

static uint64_t parse_linux_isa_flags(const char *line) {
    const char *colon = strchr(line, ':');
    const char *flags = colon != NULL ? colon + 1 : "";
    uint64_t mask = 0u;
    if (line_has_flag(flags, "sse2")) mask |= PL_NI_ISA_SSE2;
    if (line_has_flag(flags, "sse4_2")) mask |= PL_NI_ISA_SSE42;
    if (line_has_flag(flags, "aes")) mask |= PL_NI_ISA_AES;
    if (line_has_flag(flags, "avx")) mask |= PL_NI_ISA_AVX;
    if (line_has_flag(flags, "avx2")) mask |= PL_NI_ISA_AVX2;
    if (line_has_flag(flags, "fma")) mask |= PL_NI_ISA_FMA;
    if (line_has_flag(flags, "avx512f")) mask |= PL_NI_ISA_AVX512F;
    return mask;
}

static int collect_linux_isa(pl_ni_snapshot *snapshot) {
    FILE *file = fopen("/proc/cpuinfo", "rb");
    char line[4096];
    uint16_t current_cpu = PL_NI_UNKNOWN_ID;
    uint32_t seen = 0u, online = 0u;
    uint64_t intersection = PL_NI_KNOWN_ISA_MASK;
    if (file == NULL) return 0;
    for (uint32_t i = 0u; i < snapshot->cpu_count; ++i)
        if ((snapshot->cpus[i].flags & PL_NI_CPU_ONLINE) != 0u) ++online;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "processor", 9u) == 0) {
            char *colon = strchr(line, ':');
            if (colon != NULL) {
                unsigned long id = strtoul(colon + 1, NULL, 10);
                current_cpu = id < UINT16_MAX ? (uint16_t)id : PL_NI_UNKNOWN_ID;
            }
        } else if (strncmp(line, "flags", 5u) == 0 ||
                   strncmp(line, "Features", 8u) == 0) {
            int index = cpu_index_by_os_id(snapshot, 0u, current_cpu);
            if (index >= 0) {
                uint64_t mask = parse_linux_isa_flags(line);
                snapshot->cpus[index].isa_mask = mask;
                intersection &= mask;
                ++seen;
            }
        }
    }
    fclose(file);
    if (online != 0u && seen == online) {
        snapshot->isa_mask = intersection;
        snapshot->provenance.isa_state = PL_NI_FACT_KNOWN;
        snapshot->provenance.isa_scope = PL_NI_ISA_SCOPE_PER_LOGICAL_CPU;
        copy_text(snapshot->provenance.isa_source,
                  sizeof(snapshot->provenance.isa_source),
                  "/proc/cpuinfo per-CPU kernel-enabled flags");
        return 1;
    }
    for (uint32_t i = 0u; i < snapshot->cpu_count; ++i)
        snapshot->cpus[i].isa_mask = 0u;
    snapshot->provenance.isa_scope = PL_NI_ISA_SCOPE_EXECUTING_CPU;
    return 0;
}

static pl_ni_status collect_linux(pl_ni_snapshot *snapshot) {
    DIR *directory;
    struct dirent *entry;
    char path[256], buffer[4096];
    directory = opendir("/sys/devices/system/cpu");
    if (directory == NULL) return PL_NI_IO_ERROR;
    while ((entry = readdir(directory)) != NULL) {
        char *end = NULL;
        unsigned long os_id;
        int index;
        uint32_t value;
        if (strncmp(entry->d_name, "cpu", 3u) != 0 ||
            entry->d_name[3] < '0' || entry->d_name[3] > '9') continue;
        os_id = strtoul(entry->d_name + 3, &end, 10);
        if (end == entry->d_name + 3 || *end != '\0')
            continue;
        if (os_id >= PL_NI_MAX_CPUS) {
            closedir(directory);
            return PL_NI_CAPACITY_EXCEEDED;
        }
        index = add_cpu(snapshot, 0u, (uint16_t)os_id);
        if (index < 0) { closedir(directory); return PL_NI_CAPACITY_EXCEEDED; }
        if (strcmp(entry->d_name, "cpu0") != 0) {
            (void)snprintf(path, sizeof(path),
                "/sys/devices/system/cpu/%s/online", entry->d_name);
            if (read_u32(path, &value) && value == 0u)
                snapshot->cpus[index].flags &= ~PL_NI_CPU_ONLINE;
            else if (!read_u32(path, &value))
                snapshot->cpus[index].flags &= ~PL_NI_CPU_ALLOWED_KNOWN;
        }
        (void)snprintf(path, sizeof(path),
            "/sys/devices/system/cpu/%s/topology/physical_package_id", entry->d_name);
        if (read_u32(path, &value) && value <= UINT16_MAX)
            snapshot->cpus[index].package_id = (uint16_t)value;
        (void)snprintf(path, sizeof(path),
            "/sys/devices/system/cpu/%s/topology/die_id", entry->d_name);
        if (read_u32(path, &value) && value <= UINT16_MAX)
            snapshot->cpus[index].die_id = (uint16_t)value;
        (void)snprintf(path, sizeof(path),
            "/sys/devices/system/cpu/%s/topology/core_id", entry->d_name);
        if (read_u32(path, &value) && value <= UINT16_MAX)
            snapshot->cpus[index].core_id = (uint16_t)value;
    }
    closedir(directory);
    {
        cpu_set_t allowed_set;
        int allowed_set_known;
        CPU_ZERO(&allowed_set);
        allowed_set_known =
            sched_getaffinity(0, sizeof(allowed_set), &allowed_set) == 0;
        snapshot->provenance.allowed_state =
            allowed_set_known ? PL_NI_FACT_KNOWN : PL_NI_FACT_UNAVAILABLE;
        copy_text(snapshot->provenance.allowed_source,
                  sizeof(snapshot->provenance.allowed_source),
                  allowed_set_known ? "sched_getaffinity current thread"
                                    : "unavailable");
        for (uint32_t i = 0u; i < snapshot->cpu_count; ++i) {
        pl_ni_cpu *cpu = &snapshot->cpus[i];
        (void)snprintf(path, sizeof(path),
            "/sys/devices/system/cpu/cpu%u/topology/thread_siblings_list",
            (unsigned)cpu->os_cpu_id);
        if (read_text(path, buffer, sizeof(buffer)))
            cpu->thread_index = thread_index_from_list(buffer, cpu->os_cpu_id);
        cpu->flags &= ~(PL_NI_CPU_ALLOWED | PL_NI_CPU_ALLOWED_KNOWN);
        if (allowed_set_known && cpu->os_cpu_id < CPU_SETSIZE) {
            cpu->flags |= PL_NI_CPU_ALLOWED_KNOWN;
            if (CPU_ISSET(cpu->os_cpu_id, &allowed_set))
                cpu->flags |= PL_NI_CPU_ALLOWED;
        }
        }
    }
    directory = opendir("/sys/devices/system/node");
    if (directory != NULL) {
        while ((entry = readdir(directory)) != NULL) {
            char *end = NULL;
            unsigned long node_id;
            uint64_t mask[PL_NI_CPU_WORDS] = {0};
            uint32_t listed = 0u;
            int node_index;
            if (strncmp(entry->d_name, "node", 4u) != 0 ||
                entry->d_name[4] < '0' || entry->d_name[4] > '9') continue;
            node_id = strtoul(entry->d_name + 4, &end, 10);
            if (end == entry->d_name + 4 || *end != '\0' || node_id > UINT16_MAX)
                continue;
            (void)snprintf(path, sizeof(path),
                "/sys/devices/system/node/%s/cpulist", entry->d_name);
            if (!read_text(path, buffer, sizeof(buffer))) continue;
            list_to_mask(snapshot, buffer, mask, &listed);
            if (listed == 0u) continue;
            node_index = add_numa(snapshot, (uint16_t)node_id);
            if (node_index < 0) { closedir(directory); return PL_NI_CAPACITY_EXCEEDED; }
            snapshot->numa[node_index].cpu_mask[0] = mask[0];
            snapshot->numa[node_index].cpu_mask[1] = mask[1];
            snapshot->numa[node_index].cpu_mask[2] = mask[2];
            snapshot->numa[node_index].cpu_mask[3] = mask[3];
            snapshot->numa[node_index].cpu_count = listed;
            for (uint32_t i = 0u; i < snapshot->cpu_count; ++i) {
                if (!bit_is_set(mask, i)) continue;
                if (snapshot->cpus[i].numa_node != PL_NI_UNKNOWN_ID &&
                    snapshot->cpus[i].numa_node != (uint16_t)node_id) {
                    closedir(directory);
                    return PL_NI_INCONSISTENT;
                }
                snapshot->cpus[i].numa_node = (uint16_t)node_id;
            }
        }
        closedir(directory);
    }
    for (uint32_t ni = 0u; ni < snapshot->numa_count; ++ni) {
        pl_ni_numa_node *node = &snapshot->numa[ni];
        char mem_path[160];
        (void)snprintf(mem_path, sizeof(mem_path),
            "/sys/devices/system/node/node%u/meminfo", (unsigned)node->id);
        if (read_text(mem_path, buffer, sizeof(buffer))) {
            char *marker = strstr(buffer, "MemTotal:");
            if (marker != NULL) {
                unsigned long long kib = strtoull(marker + 9, NULL, 10);
                if (kib <= UINT64_MAX / 1024u) node->memory_bytes = kib * 1024u;
            }
        }

    }
    collect_linux_numa_distances(snapshot);
    directory = opendir("/sys/devices/system/cpu");
    if (directory == NULL) return PL_NI_IO_ERROR;
    {
        while ((entry = readdir(directory)) != NULL) {
            unsigned long os_id;
            char *end = NULL;
            if (strncmp(entry->d_name, "cpu", 3u) != 0 ||
                entry->d_name[3] < '0' || entry->d_name[3] > '9') continue;
            os_id = strtoul(entry->d_name + 3, &end, 10);
            if (end == entry->d_name + 3 || *end != '\0' || os_id >= PL_NI_MAX_CPUS)
                continue;
            for (uint32_t cache_index = 0u; cache_index < 8u; ++cache_index) {
                pl_ni_cache cache;
                uint32_t level = 0u, line = 0u;
                char base[256], type[32], size_text[32], shared[4096];
                memset(&cache, 0, sizeof(cache));
                (void)snprintf(base, sizeof(base),
                    "/sys/devices/system/cpu/%s/cache/index%u", entry->d_name, cache_index);
                (void)snprintf(path, sizeof(path), "%s/level", base);
                if (!read_u32(path, &level)) continue;
                (void)snprintf(path, sizeof(path), "%s/type", base);
                if (!read_text(path, type, sizeof(type))) continue;
                (void)snprintf(path, sizeof(path), "%s/size", base);
                if (!read_text(path, size_text, sizeof(size_text))) continue;
                (void)snprintf(path, sizeof(path), "%s/coherency_line_size", base);
                (void)read_u32(path, &line);
                (void)snprintf(path, sizeof(path), "%s/shared_cpu_list", base);
                if (!read_text(path, shared, sizeof(shared))) continue;
                cache.level = (uint8_t)level;
                cache.kind = strcmp(type, "Data") == 0 ? 1u :
                             strcmp(type, "Instruction") == 0 ? 2u :
                             strcmp(type, "Unified") == 0 ? 3u : 4u;
                cache.size_bytes = parse_size_bytes(size_text);
                cache.line_bytes = line;
                list_to_mask(snapshot, shared, cache.sharing_mask, &cache.sharing_count);
                if (cache.size_bytes == 0u || cache.sharing_count == 0u) continue;
                {
                    int duplicate = 0;
                    for (uint32_t ci = 0u; ci < snapshot->cache_count; ++ci) {
                        if (snapshot->caches[ci].level == cache.level &&
                            snapshot->caches[ci].kind == cache.kind &&
                            snapshot->caches[ci].size_bytes == cache.size_bytes &&
                            memcmp(snapshot->caches[ci].sharing_mask, cache.sharing_mask,
                                   sizeof(cache.sharing_mask)) == 0) duplicate = 1;
                    }
                    if (!duplicate && add_cache(snapshot, &cache) < 0) {
                        closedir(directory);
                        return PL_NI_CAPACITY_EXCEEDED;
                    }
                }
            }
        }
        closedir(directory);
    }
    copy_text(snapshot->provenance.topology_source,
              sizeof(snapshot->provenance.topology_source),
              "Linux sysfs cpu topology/cache/node ABI");
    copy_text(snapshot->provenance.memory_source,
              sizeof(snapshot->provenance.memory_source), "sysconf");
    copy_text(snapshot->provenance.platform, sizeof(snapshot->provenance.platform), "linux");
    return snapshot->cpu_count != 0u ? PL_NI_OK : PL_NI_IO_ERROR;
}
#endif

pl_ni_status pl_ni_collect(pl_ni_snapshot *out) {
    pl_processor_snapshot processor;
    pl_ni_status status;
    if (out == NULL) return PL_NI_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    out->abi_version = PL_NI_ABI_VERSION;
    out->provenance.schema_version = PL_NI_ABI_VERSION;
#if defined(_WIN32)
    {
        MEMORYSTATUSEX memory;
        memset(&memory, 0, sizeof(memory));
        memory.dwLength = sizeof(memory);
        if (GlobalMemoryStatusEx(&memory)) {
            out->total_memory_bytes = memory.ullTotalPhys;
            out->available_memory_bytes = memory.ullAvailPhys;
            out->provenance.memory_state = PL_NI_FACT_KNOWN;
        } else out->provenance.memory_state = PL_NI_FACT_UNAVAILABLE;
    }
    status = collect_windows(out);
#else
    {
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGESIZE);
        long available = -1;
        if (pages > 0 && page_size > 0 &&
            (uint64_t)pages <= UINT64_MAX / (uint64_t)page_size)
            out->total_memory_bytes = (uint64_t)pages * (uint64_t)page_size;
#ifdef _SC_AVPHYS_PAGES
        available = sysconf(_SC_AVPHYS_PAGES);
#endif
        if (available >= 0 && page_size > 0 &&
            (uint64_t)available <= UINT64_MAX / (uint64_t)page_size)
            out->available_memory_bytes = (uint64_t)available * (uint64_t)page_size;
        if (out->available_memory_bytes > out->total_memory_bytes)
            out->available_memory_bytes = out->total_memory_bytes;
        out->provenance.memory_state = out->total_memory_bytes != 0u
            ? PL_NI_FACT_KNOWN : PL_NI_FACT_UNAVAILABLE;
    }
    status = collect_linux(out);
#endif
    if (status != PL_NI_OK) return status;
    out->provenance.topology_state = PL_NI_FACT_KNOWN;
    if (pl_processor_scan(&processor) == 0) {
        out->isa_mask = isa_from_processor(&processor);
        out->provenance.isa_state = PL_NI_FACT_KNOWN;
        out->provenance.isa_scope = PL_NI_ISA_SCOPE_EXECUTING_CPU;
        copy_text(out->provenance.isa_source,
                  sizeof(out->provenance.isa_source),
                  "CPUID plus OS vector-state validation on collector CPU");
    } else {
        out->provenance.isa_scope = PL_NI_ISA_SCOPE_UNKNOWN;
        out->provenance.isa_state = PL_NI_FACT_UNAVAILABLE;
        copy_text(out->provenance.isa_source,
                  sizeof(out->provenance.isa_source), "unavailable");
    }
#if !defined(_WIN32)
    (void)collect_linux_isa(out);
#endif
    for (uint32_t i = 0u; i < out->numa_count; ++i)
        for (uint32_t j = 0u; j < out->numa_count; ++j)
            if (i == j && out->numa_distance[i][j] == 0u)
                out->numa_distance[i][j] = 10u;
    {
        uint64_t fingerprint = pl_ni_snapshot_fingerprint(out);
        out->provenance.snapshot_fingerprint = fingerprint;
        out->provenance.generation = fingerprint;
    }
    return pl_ni_snapshot_validate(out);
}

static uint32_t clamp_cost(uint32_t value) {
    return value > PL_NI_COST_MAX ? PL_NI_COST_MAX : value;
}

static int cpu_core_occupancy(const pl_ni_snapshot *snapshot,
                               const pl_ppf_request *request,
                               uint32_t candidate) {
    const pl_ni_cpu *cpu = &snapshot->cpus[candidate];
    uint32_t i;
    if (cpu->package_id == PL_NI_UNKNOWN_ID ||
        cpu->die_id == PL_NI_UNKNOWN_ID ||
        cpu->core_id == PL_NI_UNKNOWN_ID)
        return -1;
    for (i = 0u; i < snapshot->cpu_count; ++i) {
        const pl_ni_cpu *other = &snapshot->cpus[i];
        if (i != candidate && other->package_id == cpu->package_id &&
            other->die_id == cpu->die_id && other->core_id == cpu->core_id) {
            if (bit_is_set(request->occupied_cpu_mask, i)) return 1;
        }
    }
    return 0;
}

static uint32_t cpu_shared_cache_penalty(const pl_ni_snapshot *snapshot,
                                         uint32_t cpu_index) {
    uint32_t i, penalty = 0u;
    int found = 0;
    for (i = 0u; i < snapshot->cache_count; ++i) {
        const pl_ni_cache *cache = &snapshot->caches[i];
        if (cache->level >= 3u && bit_is_set(cache->sharing_mask, cpu_index)) {
            uint32_t next = cache->sharing_count > 1u
                ? (cache->sharing_count - 1u) * 20u : 0u;
            if (next > penalty) penalty = next;
            found = 1;
        }
    }
    return found ? clamp_cost(penalty) : UINT32_MAX;
}

pl_ni_status pl_ppf_place(const pl_ni_snapshot *snapshot,
                          const pl_ppf_request *request,
                          const pl_ppf_telemetry *telemetry,
                          size_t telemetry_count, pl_ppf_report *out) {
    uint32_t i, best = UINT32_MAX;
    uint64_t best_cost = UINT64_MAX;
    uint64_t policy = UINT64_C(14695981039346656037);
    if (snapshot == NULL || request == NULL || out == NULL ||
        pl_ni_snapshot_validate(snapshot) != PL_NI_OK ||
        (telemetry == NULL && telemetry_count != 0u) ||
        (telemetry != NULL && telemetry_count != snapshot->cpu_count))
        return PL_NI_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    out->schema_version = PL_NI_ABI_VERSION;
    out->candidate_count = snapshot->cpu_count;
    out->topology_generation = snapshot->provenance.generation;
    out->topology_fingerprint = snapshot->provenance.snapshot_fingerprint;
    {
        const uint64_t scalar_values[] = {
            request->preferred_numa_node, request->current_cpu_index,
            request->required_isa, request->require_online,
            request->require_allowed, request->require_known_allowed,
            request->exclusive_core, request->weights[0], request->weights[1],
            request->weights[2], request->weights[3], request->weights[4]
        };
        size_t item;
        for (item = 0u; item < sizeof(scalar_values) / sizeof(scalar_values[0]); ++item) {
            uint64_t value = scalar_values[item];
            unsigned byte;
            for (byte = 0u; byte < 8u; ++byte) {
                policy ^= (unsigned char)(value & UINT64_C(0xff));
                policy *= UINT64_C(1099511628211);
                value >>= 8u;
            }
        }
        for (item = 0u; item < PL_NI_CPU_WORDS; ++item) {
            uint64_t value = request->occupied_cpu_mask[item];
            unsigned byte;
            for (byte = 0u; byte < 8u; ++byte) {
                policy ^= (unsigned char)(value & UINT64_C(0xff));
                policy *= UINT64_C(1099511628211);
                value >>= 8u;
            }
        }
    }
    out->policy_fingerprint = policy;
    for (i = 0u; i < 5u; ++i) out->weights[i] = request->weights[i];
    if (telemetry != NULL) {
        size_t ti, byte;
        uint64_t telemetry_hash = UINT64_C(14695981039346656037);
        for (ti = 0u; ti < telemetry_count; ++ti) {
            const uint32_t fields[4] = {
                telemetry[ti].cache_pressure,
                telemetry[ti].bandwidth_pressure,
                telemetry[ti].migration_cost,
                telemetry[ti].valid_mask
            };
            for (byte = 0u; byte < 4u; ++byte) {
                unsigned shift;
                for (shift = 0u; shift < 4u; ++shift) {
                    telemetry_hash ^= (unsigned char)(fields[byte] >> (shift * 8u));
                    telemetry_hash *= UINT64_C(1099511628211);
                }
            }
        }
        out->telemetry_fingerprint = telemetry_hash;
    }
    for (i = 0u; i < snapshot->cpu_count; ++i) {
        const pl_ni_cpu *cpu = &snapshot->cpus[i];
        pl_ppf_candidate *candidate = &out->candidates[i];
        uint32_t w0 = request->weights[0], w1 = request->weights[1];
        uint32_t w2 = request->weights[2], w3 = request->weights[3];
        uint32_t w4 = request->weights[4];
        uint64_t weighted;
        candidate->cpu_index = i;
        candidate->evidence_flags = PL_PPF_EVIDENCE_TOPOLOGY | PL_PPF_EVIDENCE_ESTIMATED;
        if (request->require_online && (cpu->flags & PL_NI_CPU_ONLINE) == 0u) {
            candidate->rejection = PL_NI_ACCESS_DENIED; continue;
        }
        if (request->require_allowed &&
            (cpu->flags & PL_NI_CPU_ALLOWED) == 0u) {
            candidate->rejection = PL_NI_ACCESS_DENIED; continue;
        }
        if (request->require_known_allowed &&
            (cpu->flags & PL_NI_CPU_ALLOWED_KNOWN) == 0u) {
            candidate->rejection = PL_NI_UNSUPPORTED; continue;
        }
        if (request->required_isa != 0u &&
            snapshot->provenance.isa_scope != PL_NI_ISA_SCOPE_PER_LOGICAL_CPU &&
            snapshot->provenance.isa_scope != PL_NI_ISA_SCOPE_SYSTEM_COMMON) {
            candidate->rejection = PL_NI_UNSUPPORTED; continue;
        }
        if ((request->required_isa & ~cpu->isa_mask) != 0u) {
            candidate->rejection = PL_NI_UNSUPPORTED; continue;
        }
        {
            int core_occupancy = cpu_core_occupancy(snapshot, request, i);
            if (request->exclusive_core && core_occupancy < 0) {
                candidate->rejection = PL_NI_UNSUPPORTED; continue;
            }
            if (request->exclusive_core && core_occupancy > 0) {
                candidate->rejection = PL_NI_ACCESS_DENIED; continue;
            }
        }
        if (bit_is_set(request->occupied_cpu_mask, i)) {
            candidate->rejection = PL_NI_ACCESS_DENIED;
            continue;
        }
        candidate->accepted = 1u;
        candidate->rejection = PL_NI_OK;
        if (request->preferred_numa_node != PL_NI_UNKNOWN_ID) {
            uint32_t preferred_index = snapshot->numa_count;
            uint32_t actual_index = snapshot->numa_count;
            for (uint32_t ni = 0u; ni < snapshot->numa_count; ++ni) {
                if (snapshot->numa[ni].id == request->preferred_numa_node)
                    preferred_index = ni;
                if (snapshot->numa[ni].id == cpu->numa_node)
                    actual_index = ni;
            }
            if (cpu->numa_node == request->preferred_numa_node) {
                candidate->numa_cost = 0u;
            } else if (preferred_index == snapshot->numa_count ||
                       actual_index == snapshot->numa_count) {
                candidate->numa_cost = 500u;
            } else {
                uint16_t distance =
                    snapshot->numa_distance[preferred_index][actual_index];
                candidate->numa_cost = distance == 0u
                    ? 250u : clamp_cost((uint32_t)distance * 10u);
            }
        }
        candidate->cache_cost = cpu_shared_cache_penalty(snapshot, i);
        if (candidate->cache_cost == UINT32_MAX) {
            candidate->cache_cost = PL_NI_COST_MAX / 2u;
            candidate->evidence_flags |= PL_PPF_EVIDENCE_MISSING_TOPOLOGY;
        }
        {
            int core_occupancy = cpu_core_occupancy(snapshot, request, i);
            if (core_occupancy < 0) {
                candidate->smt_cost = PL_NI_COST_MAX / 2u;
                candidate->evidence_flags |= PL_PPF_EVIDENCE_MISSING_TOPOLOGY;
            } else candidate->smt_cost = core_occupancy > 0 ? 1000u : 0u;
        }
        if (telemetry != NULL && (telemetry[i].valid_mask & PL_PPF_TELEMETRY_CACHE)) {
            candidate->cache_cost = clamp_cost(telemetry[i].cache_pressure);
            candidate->evidence_flags |= PL_PPF_EVIDENCE_TELEMETRY;
        }
        if (telemetry != NULL && (telemetry[i].valid_mask & PL_PPF_TELEMETRY_BANDWIDTH)) {
            candidate->bandwidth_cost = clamp_cost(telemetry[i].bandwidth_pressure);
            candidate->evidence_flags |= PL_PPF_EVIDENCE_TELEMETRY;
        } else {
            candidate->bandwidth_cost = PL_NI_COST_MAX / 2u;
            candidate->evidence_flags |= PL_PPF_EVIDENCE_MISSING_TELEMETRY;
        }
        if (telemetry != NULL && (telemetry[i].valid_mask & PL_PPF_TELEMETRY_MIGRATION)) {
            candidate->migration_cost = clamp_cost(telemetry[i].migration_cost);
            candidate->evidence_flags |= PL_PPF_EVIDENCE_TELEMETRY;
        } else if (request->current_cpu_index == UINT16_MAX) {
            candidate->migration_cost = PL_NI_COST_MAX / 2u;
            candidate->evidence_flags |= PL_PPF_EVIDENCE_MISSING_TELEMETRY;
        } else candidate->migration_cost =
            request->current_cpu_index == i ? 0u : 50u;
        candidate->total_cost =
            (uint64_t)candidate->numa_cost * w0 +
            (uint64_t)candidate->cache_cost * w1 +
            (uint64_t)candidate->smt_cost * w2 +
            (uint64_t)candidate->bandwidth_cost * w3 +
            (uint64_t)candidate->migration_cost * w4;
        weighted = candidate->total_cost;
        if (best == UINT32_MAX || weighted < best_cost ||
            (weighted == best_cost && i < best)) {
            best = i;
            best_cost = weighted;
        }
    }
    if (best != UINT32_MAX) {
        out->has_selection = 1u;
        out->selected_cpu_index = best;
    }
    return PL_NI_OK;
}

static int power_of_two(uint32_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}

pl_ni_status pl_native_range_validate(const pl_native_range *range) {
    uint64_t end;
    if (range == NULL || range->length == 0u ||
        range->address_bits == 0u || range->address_bits > 64u ||
        !power_of_two(range->alignment) ||
        (range->base % range->alignment) != 0u ||
        range->length > UINT64_MAX - range->base)
        return PL_NI_INVALID_ARGUMENT;
    end = range->base + range->length;
    if (range->address_bits < 64u &&
        end > (UINT64_C(1) << range->address_bits))
        return PL_NI_OUT_OF_RANGE;
    return PL_NI_OK;
}

pl_ni_status pl_native_register_validate(const pl_native_register *reg,
                                          uint64_t offset, uint32_t is_write,
                                          uint64_t value, uint64_t expected_generation,
                                          uint64_t *absolute_address) {
    uint64_t width, address;
    uint32_t required;
    if (absolute_address != NULL) *absolute_address = 0u;
    if (reg == NULL || absolute_address == NULL ||
        reg->address_space < PL_NATIVE_SPACE_VIRTUAL ||
        reg->address_space > PL_NATIVE_SPACE_MMIO ||
        (is_write != 0u && is_write != 1u))
        return PL_NI_INVALID_ARGUMENT;
    if (reg->generation != expected_generation) return PL_NI_STALE_GENERATION;
    if (reg->width_bits != 8u && reg->width_bits != 16u &&
        reg->width_bits != 32u && reg->width_bits != 64u)
        return PL_NI_INVALID_ARGUMENT;
    {
        pl_native_range declared_range;
        pl_ni_status range_status;
        declared_range.base = reg->base;
        declared_range.length = reg->size;
        declared_range.address_bits = reg->address_bits;
        declared_range.alignment = reg->alignment;
        range_status = pl_native_range_validate(&declared_range);
        if (range_status != PL_NI_OK) return range_status;
    }
    width = reg->width_bits / 8u;
    if (!power_of_two(reg->alignment) || (reg->base % reg->alignment) != 0u ||
        offset > reg->size || width > reg->size - offset ||
        offset > UINT64_MAX - reg->base)
        return PL_NI_OUT_OF_RANGE;
    address = reg->base + offset;
    if (address > UINT64_MAX - width ||
        (reg->address_bits < 64u &&
         address + width > (UINT64_C(1) << reg->address_bits)) ||
        address % width != 0u || address % reg->alignment != 0u)
        return PL_NI_OUT_OF_RANGE;
    required = is_write ? (PL_NATIVE_ACCESS_WRITE | PL_NATIVE_ACCESS_W1C |
                           PL_NATIVE_ACCESS_W1S) : PL_NATIVE_ACCESS_READ;
    if ((reg->access & ~(PL_NATIVE_ACCESS_READ | PL_NATIVE_ACCESS_WRITE |
                         PL_NATIVE_ACCESS_W1C | PL_NATIVE_ACCESS_W1S)) != 0u)
        return PL_NI_INVALID_ARGUMENT;
    if ((reg->access & required) == 0u)
        return PL_NI_ACCESS_DENIED;
    if (is_write && (value & ~reg->allowed_value_mask) != 0u)
        return PL_NI_ACCESS_DENIED;
    *absolute_address = address;
    return PL_NI_OK;
}

pl_ni_status pl_dma_transition(pl_dma_resource *resource,
                               uint64_t expected_generation,
                               pl_dma_state next_state) {
    int valid = 0;
    if (resource == NULL || resource->resource_id == 0u ||
        resource->length == 0u || resource->length > UINT64_MAX - resource->base ||
        resource->direction < 1u || resource->direction > 3u)
        return PL_NI_INVALID_ARGUMENT;
    if (resource->generation != expected_generation) return PL_NI_STALE_GENERATION;
    if (resource->state == PL_DMA_CPU_OWNED && next_state == PL_DMA_DEVICE_OWNED) valid = 1;
    else if (resource->state == PL_DMA_DEVICE_OWNED &&
             (next_state == PL_DMA_COMPLETED || next_state == PL_DMA_FAULTED)) valid = 1;
    else if (resource->state == PL_DMA_COMPLETED && next_state == PL_DMA_CPU_OWNED) valid = 1;
    else if ((resource->state == PL_DMA_CPU_OWNED ||
              resource->state == PL_DMA_COMPLETED) && next_state == PL_DMA_UNMAPPED) valid = 1;
    if (!valid) return PL_NI_INVALID_STATE;
    if (resource->generation == UINT64_MAX) return PL_NI_OUT_OF_RANGE;
    resource->state = (uint32_t)next_state;
    ++resource->generation;
    return PL_NI_OK;
}

pl_ni_status pl_dma_cpu_access_validate(const pl_dma_resource *resource,
                                        uint64_t expected_generation) {
    if (resource == NULL || resource->resource_id == 0u ||
        resource->length == 0u || resource->length > UINT64_MAX - resource->base ||
        resource->direction < 1u || resource->direction > 3u)
        return PL_NI_INVALID_ARGUMENT;
    if (resource->generation != expected_generation) return PL_NI_STALE_GENERATION;
    return resource->state == PL_DMA_CPU_OWNED
        ? PL_NI_OK : PL_NI_ACCESS_DENIED;
}
