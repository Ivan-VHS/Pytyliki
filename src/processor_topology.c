#define _POSIX_C_SOURCE 200809L
#include "processor_topology.h"
#include "processor.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef WINVER
#define WINVER 0x0A00
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__)
#include <dirent.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
static volatile LONG topology_generation = 0;
static uint32_t next_generation(void) {
    return (uint32_t)InterlockedIncrement(&topology_generation);
}
#else
#include <stdatomic.h>
static atomic_uint topology_generation = ATOMIC_VAR_INIT(0u);
static uint32_t next_generation(void) {
    return atomic_fetch_add_explicit(&topology_generation, 1u,
                                     memory_order_relaxed) + 1u;
}
#endif

static void set_unknown_cpu(pl_topology_logical_cpu *cpu) {
    memset(cpu, 0, sizeof(*cpu));
    cpu->group_id = PL_TOPOLOGY_UNKNOWN;
    cpu->package_id = PL_TOPOLOGY_UNKNOWN;
    cpu->die_id = PL_TOPOLOGY_UNKNOWN;
    cpu->core_id = PL_TOPOLOGY_UNKNOWN;
    cpu->numa_node_id = PL_TOPOLOGY_UNKNOWN;
    cpu->evidence = PL_TOPOLOGY_EVIDENCE_UNKNOWN;
}

static void set_string(char *target, size_t capacity, const char *source) {
    if (capacity == 0u) return;
    (void)snprintf(target, capacity, "%s", source != NULL ? source : "");
}

static void capture_time(char output[32]) {
    time_t now = time(NULL);
    struct tm value;
    memset(&value, 0, sizeof(value));
#if defined(_WIN32)
    if (gmtime_s(&value, &now) != 0) {
        set_string(output, 32u, "unknown");
        return;
    }
#else
    if (gmtime_r(&now, &value) == NULL) {
        set_string(output, 32u, "unknown");
        return;
    }
#endif
    (void)strftime(output, 32u, "%Y-%m-%dT%H:%M:%SZ", &value);
}

static void set_isa_from_profile(pl_processor_topology_snapshot *out) {
    pl_processor_snapshot profile;
    int x86;
    memset(&profile, 0, sizeof(profile));
    if (pl_processor_scan(&profile) != 0) return;
    set_string(out->architecture, sizeof(out->architecture), profile.architecture);
    x86 = strcmp(out->architecture, "x86") == 0 ||
          strcmp(out->architecture, "x86-64") == 0;
    set_string(out->vendor, sizeof(out->vendor), profile.base.vendor);
    set_string(out->model, sizeof(out->model), profile.base.model);
    if (!x86) return;
#define SET_ISA(feature, hw, usable, source) do { \
    out->isa[(feature)].hardware_present = (int8_t)(hw); \
    out->isa[(feature)].os_usable = (int8_t)(usable); \
    out->isa[(feature)].evidence = (uint8_t)(source); \
} while (0)
    SET_ISA(PL_TOPOLOGY_ISA_SSE2, profile.sse2 ? 1 : 0,
            profile.sse2 ? 1 : 0, 1u);
    SET_ISA(PL_TOPOLOGY_ISA_SSE42, profile.sse42 ? 1 : 0,
            profile.sse42 ? 1 : 0, 1u);
    SET_ISA(PL_TOPOLOGY_ISA_AES, profile.aes ? 1 : 0,
            profile.aes ? 1 : 0, 1u);
    SET_ISA(PL_TOPOLOGY_ISA_AVX, profile.avx_hw ? 1 : 0,
            profile.avx ? 1 : 0, 2u);
    SET_ISA(PL_TOPOLOGY_ISA_AVX2, profile.avx2_hw ? 1 : 0,
            profile.avx2 ? 1 : 0, 2u);
    SET_ISA(PL_TOPOLOGY_ISA_AVX512F, profile.avx512f_hw ? 1 : 0,
            profile.avx512f ? 1 : 0, 2u);
    SET_ISA(PL_TOPOLOGY_ISA_FMA, profile.fma_hw ? 1 : 0,
            profile.fma ? 1 : 0, 2u);
    SET_ISA(PL_TOPOLOGY_ISA_VIRTUALIZATION,
            profile.virtualization ? 1 : 0, -1, 1u);
#undef SET_ISA
}

static size_t find_cpu(const pl_processor_topology_snapshot *out,
                       uint32_t group_id, uint32_t os_id) {
    for (size_t i = 0u; i < out->logical_cpu_count; ++i) {
        if (out->logical_cpus[i].group_id == group_id &&
            out->logical_cpus[i].os_id == os_id) return i;
    }
    return out->logical_cpu_count;
}

static size_t add_cpu(pl_processor_topology_snapshot *out,
                      uint32_t group_id, uint32_t os_id, int online) {
    size_t found = find_cpu(out, group_id, os_id);
    if (found < out->logical_cpu_count) {
        if (online) out->logical_cpus[found].online = 1u;
        return found;
    }
    if (out->logical_cpu_count >= PL_TOPOLOGY_MAX_LOGICAL_CPUS) return SIZE_MAX;
    found = out->logical_cpu_count++;
    set_unknown_cpu(&out->logical_cpus[found]);
    out->logical_cpus[found].logical_id = (uint32_t)found;
    out->logical_cpus[found].group_id = group_id;
    out->logical_cpus[found].os_id = os_id;
    out->logical_cpus[found].online = online ? 1u : 0u;
    if (online) ++out->online_logical_cpu_count;
    return found;
}

static void set_mask_bit(uint64_t mask[PL_TOPOLOGY_MASK_WORDS],
                         uint32_t logical_id) {
    if (logical_id < PL_TOPOLOGY_MAX_LOGICAL_CPUS)
        mask[logical_id / 64u] |= UINT64_C(1) << (logical_id % 64u);
}

static uint64_t parse_size_bytes(const char *text) {
    char *end = NULL;
    unsigned long long amount;
    unsigned long long multiplier = 1ULL;
    if (text == NULL || text[0] == '\0') return 0u;
    errno = 0;
    amount = strtoull(text, &end, 10);
    if (errno != 0 || end == text) return 0u;
    while (*end == ' ' || *end == '\t') ++end;
    if (*end == 'K' || *end == 'k') multiplier = 1024ULL;
    else if (*end == 'M' || *end == 'm') multiplier = 1024ULL * 1024ULL;
    else if (*end == 'G' || *end == 'g') multiplier = 1024ULL * 1024ULL * 1024ULL;
    else if (*end != '\0') return 0u;
    if (amount > ULLONG_MAX / multiplier) return 0u;
    return (uint64_t)(amount * multiplier);
}

#if defined(__linux__)
static int read_text(const char *path, char *buffer, size_t capacity) {
    FILE *file = fopen(path, "r");
    if (file == NULL) return 0;
    if (fgets(buffer, (int)capacity, file) == NULL) {
        (void)fclose(file);
        return 0;
    }
    (void)fclose(file);
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 1;
}

static uint32_t read_u32_file(const char *path) {
    char buffer[64];
    char *end = NULL;
    unsigned long value;
    if (!read_text(path, buffer, sizeof(buffer))) return PL_TOPOLOGY_UNKNOWN;
    errno = 0;
    value = strtoul(buffer, &end, 10);
    if (errno != 0 || end == buffer || *end != '\0' || value > UINT32_MAX)
        return PL_TOPOLOGY_UNKNOWN;
    return (uint32_t)value;
}

static int parse_cpu_list(const char *text, uint64_t mask[PL_TOPOLOGY_MASK_WORDS],
                          uint32_t *count) {
    const char *cursor = text;
    uint32_t total = 0u;
    memset(mask, 0, sizeof(uint64_t) * PL_TOPOLOGY_MASK_WORDS);
    while (*cursor != '\0') {
        char *end = NULL;
        unsigned long first, last;
        while (*cursor == ',' || isspace((unsigned char)*cursor)) ++cursor;
        if (*cursor == '\0') break;
        errno = 0;
        first = strtoul(cursor, &end, 10);
        if (errno != 0 || end == cursor || first >= PL_TOPOLOGY_MAX_LOGICAL_CPUS)
            return 0;
        cursor = end;
        last = first;
        if (*cursor == '-') {
            ++cursor;
            errno = 0;
            last = strtoul(cursor, &end, 10);
            if (errno != 0 || end == cursor || last < first ||
                last >= PL_TOPOLOGY_MAX_LOGICAL_CPUS) return 0;
            cursor = end;
        }
        for (unsigned long cpu = first; cpu <= last; ++cpu) {
            uint64_t before = mask[cpu / 64u];
            set_mask_bit(mask, (uint32_t)cpu);
            if (before != mask[cpu / 64u] ||
                (before & (UINT64_C(1) << (cpu % 64u))) == 0u) ++total;
        }
        if (*cursor != '\0' && *cursor != ',' && !isspace((unsigned char)*cursor))
            return 0;
    }
    if (count != NULL) *count = total;
    return total != 0u;
}

static int same_cache(const pl_topology_cache *cache,
                      uint32_t level, pl_topology_cache_kind kind,
                      uint64_t size_bytes, uint32_t line_bytes,
                      const uint64_t mask[PL_TOPOLOGY_MASK_WORDS]) {
    return cache->level == level && cache->kind == kind &&
           cache->size_bytes == size_bytes && cache->line_bytes == line_bytes &&
           memcmp(cache->sharing_mask, mask,
                  sizeof(uint64_t) * PL_TOPOLOGY_MASK_WORDS) == 0;
}

static void scan_linux_numa(pl_processor_topology_snapshot *out) {
    DIR *directory = opendir("/sys/devices/system/node");
    struct dirent *entry;
    if (directory == NULL) return;
    while ((entry = readdir(directory)) != NULL) {
        unsigned node_id;
        char *end = NULL;
        char path[256];
        char text[512];
        int cpu_list_observed = 0;
        int memory_total_observed = 0;
        int memory_available_observed = 0;
        pl_topology_numa_node node;
        if (strncmp(entry->d_name, "node", 4u) != 0) continue;
        errno = 0;
        node_id = (unsigned)strtoul(entry->d_name + 4u, &end, 10);
        if (errno != 0 || end == entry->d_name + 4u || *end != '\0') continue;
        if (out->numa_node_count >= PL_TOPOLOGY_MAX_NUMA_NODES) break;
        memset(&node, 0, sizeof(node));
        node.node_id = node_id;
        node.evidence = PL_TOPOLOGY_EVIDENCE_PARTIAL;
        (void)snprintf(path, sizeof(path), "/sys/devices/system/node/%s/cpulist", entry->d_name);
        if (read_text(path, text, sizeof(text)) &&
            parse_cpu_list(text, node.logical_mask, &node.logical_count)) {
            cpu_list_observed = 1;
            for (size_t i = 0u; i < out->logical_cpu_count; ++i) {
                const uint32_t id = out->logical_cpus[i].os_id;
                if (id < PL_TOPOLOGY_MAX_LOGICAL_CPUS &&
                    (node.logical_mask[id / 64u] & (UINT64_C(1) << (id % 64u))) != 0u)
                    out->logical_cpus[i].numa_node_id = node_id;
            }
        }
        (void)snprintf(path, sizeof(path), "/sys/devices/system/node/%s/meminfo", entry->d_name);
        {
            FILE *meminfo = fopen(path, "r");
            char line[256];
            if (meminfo != NULL) {
                while (fgets(line, sizeof(line), meminfo) != NULL) {
                    char *value = NULL;
                    uint64_t bytes;
                    if (strstr(line, "MemTotal:") != NULL) {
                        value = strstr(line, "MemTotal:") + 9;
                    } else if (strstr(line, "MemAvailable:") != NULL) {
                        value = strstr(line, "MemAvailable:") + 13;
                    } else if (!memory_available_observed &&
                               strstr(line, "MemFree:") != NULL) {
                        value = strstr(line, "MemFree:") + 8;
                    }
                    if (value == NULL) continue;
                    {
                        unsigned long long kib = strtoull(value, NULL, 10);
                        if (kib > UINT64_MAX / UINT64_C(1024)) continue;
                        bytes = (uint64_t)kib * UINT64_C(1024);
                    }
                    if (strstr(line, "MemTotal:") != NULL) {
                        node.memory_total_bytes = bytes;
                        memory_total_observed = 1;
                    } else {
                        node.memory_available_bytes = bytes;
                        node.memory_available_observed = 1u;
                        if (strstr(line, "MemAvailable:") != NULL)
                            memory_available_observed = 1;
                    }
                }
                (void)fclose(meminfo);
            }
        }
        if (cpu_list_observed && memory_total_observed && memory_available_observed)
            node.evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
        else if (cpu_list_observed)
            node.evidence = PL_TOPOLOGY_EVIDENCE_PARTIAL;
        out->numa_nodes[out->numa_node_count++] = node;
    }
    (void)closedir(directory);
}

static void scan_linux_caches(pl_processor_topology_snapshot *out) {
    for (size_t cpu_index = 0u; cpu_index < out->logical_cpu_count; ++cpu_index) {
        const uint32_t os_id = out->logical_cpus[cpu_index].os_id;
        char directory_path[256];
        DIR *directory;
        struct dirent *entry;
        (void)snprintf(directory_path, sizeof(directory_path),
                       "/sys/devices/system/cpu/cpu%u/cache", os_id);
        directory = opendir(directory_path);
        if (directory == NULL) continue;
        while ((entry = readdir(directory)) != NULL) {
            char path[320];
            char value[256];
            uint32_t level, line_bytes, sharing_count = 0u;
            uint64_t size_bytes;
            uint64_t mask[PL_TOPOLOGY_MASK_WORDS];
            pl_topology_cache_kind kind = PL_TOPOLOGY_CACHE_UNKNOWN;
            pl_topology_cache *cache = NULL;
            if (strncmp(entry->d_name, "index", 5u) != 0) continue;
            (void)snprintf(path, sizeof(path), "%s/%s/level", directory_path, entry->d_name);
            level = read_u32_file(path);
            if (level == PL_TOPOLOGY_UNKNOWN || level == 0u || level > 4u) continue;
            (void)snprintf(path, sizeof(path), "%s/%s/type", directory_path, entry->d_name);
            if (read_text(path, value, sizeof(value))) {
                if (strcmp(value, "Data") == 0) kind = PL_TOPOLOGY_CACHE_DATA;
                else if (strcmp(value, "Instruction") == 0) kind = PL_TOPOLOGY_CACHE_INSTRUCTION;
                else if (strcmp(value, "Unified") == 0) kind = PL_TOPOLOGY_CACHE_UNIFIED;
            }
            (void)snprintf(path, sizeof(path), "%s/%s/size", directory_path, entry->d_name);
            size_bytes = read_text(path, value, sizeof(value)) ? parse_size_bytes(value) : 0u;
            (void)snprintf(path, sizeof(path), "%s/%s/coherency_line_size", directory_path, entry->d_name);
            line_bytes = read_u32_file(path);
            (void)snprintf(path, sizeof(path), "%s/%s/shared_cpu_list", directory_path, entry->d_name);
            if (!read_text(path, value, sizeof(value)) ||
                !parse_cpu_list(value, mask, &sharing_count)) continue;
            for (size_t i = 0u; i < out->cache_count; ++i) {
                if (same_cache(&out->caches[i], level, kind, size_bytes,
                               line_bytes, mask)) {
                    cache = &out->caches[i];
                    break;
                }
            }
            if (cache == NULL) {
                if (out->cache_count >= PL_TOPOLOGY_MAX_CACHES) continue;
                cache = &out->caches[out->cache_count];
                memset(cache, 0, sizeof(*cache));
                cache->cache_id = out->cache_count;
                cache->level = level;
                cache->kind = kind;
                cache->size_bytes = size_bytes;
                cache->line_bytes = line_bytes;
                memcpy(cache->sharing_mask, mask, sizeof(mask));
                cache->sharing_count = sharing_count;
                cache->evidence = (size_bytes != 0u && line_bytes != PL_TOPOLOGY_UNKNOWN &&
                                   kind != PL_TOPOLOGY_CACHE_UNKNOWN)
                                      ? PL_TOPOLOGY_EVIDENCE_OBSERVED
                                      : PL_TOPOLOGY_EVIDENCE_PARTIAL;
                ++out->cache_count;
            }
        }
        (void)closedir(directory);
    }
}

static pl_topology_status scan_linux(pl_processor_topology_snapshot *out) {
    DIR *directory = opendir("/sys/devices/system/cpu");
    struct dirent *entry;
    struct utsname system_info;
    if (directory == NULL) return PL_TOPOLOGY_IO_ERROR;
    while ((entry = readdir(directory)) != NULL) {
        char *end = NULL;
        unsigned long os_id;
        char path[256];
        pl_topology_logical_cpu *cpu;
        size_t index;
        if (strncmp(entry->d_name, "cpu", 3u) != 0 ||
            !isdigit((unsigned char)entry->d_name[3])) continue;
        errno = 0;
        os_id = strtoul(entry->d_name + 3u, &end, 10);
        if (errno != 0 || end == entry->d_name + 3u || *end != '\0') continue;
        if (os_id >= PL_TOPOLOGY_MAX_LOGICAL_CPUS) {
            (void)closedir(directory);
            return PL_TOPOLOGY_LIMIT_EXCEEDED;
        }
        index = add_cpu(out, 0u, (uint32_t)os_id, 1);
        if (index == SIZE_MAX) { (void)closedir(directory); return PL_TOPOLOGY_LIMIT_EXCEEDED; }
        cpu = &out->logical_cpus[index];
        cpu->logical_id = (uint32_t)os_id;
        (void)snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%lu/online", os_id);
        cpu->online = (uint8_t)(read_u32_file(path) == 0u ? 0u : 1u);
        (void)snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%lu/topology/physical_package_id", os_id);
        cpu->package_id = read_u32_file(path);
        (void)snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%lu/topology/die_id", os_id);
        cpu->die_id = read_u32_file(path);
        (void)snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%lu/topology/core_id", os_id);
        cpu->core_id = read_u32_file(path);
        if (cpu->package_id != PL_TOPOLOGY_UNKNOWN &&
            cpu->die_id != PL_TOPOLOGY_UNKNOWN &&
            cpu->core_id != PL_TOPOLOGY_UNKNOWN &&
            cpu->numa_node_id != PL_TOPOLOGY_UNKNOWN)
            cpu->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
        else cpu->evidence = PL_TOPOLOGY_EVIDENCE_PARTIAL;
    }
    (void)closedir(directory);
    out->online_logical_cpu_count = 0u;
    for (size_t i = 0u; i < out->logical_cpu_count; ++i)
        if (out->logical_cpus[i].online != 0u) ++out->online_logical_cpu_count;
    scan_linux_numa(out);
    for (size_t i = 0u; i < out->logical_cpu_count; ++i) {
        pl_topology_logical_cpu *cpu = &out->logical_cpus[i];
        cpu->evidence = (cpu->package_id != PL_TOPOLOGY_UNKNOWN &&
                         cpu->die_id != PL_TOPOLOGY_UNKNOWN &&
                         cpu->core_id != PL_TOPOLOGY_UNKNOWN &&
                         cpu->numa_node_id != PL_TOPOLOGY_UNKNOWN)
                            ? PL_TOPOLOGY_EVIDENCE_OBSERVED
                            : PL_TOPOLOGY_EVIDENCE_PARTIAL;
    }
    scan_linux_caches(out);
    if (uname(&system_info) == 0) set_string(out->platform_release, sizeof(out->platform_release), system_info.release);
    return PL_TOPOLOGY_OK;
}
#endif

#if defined(_WIN32)
static uint32_t mask_popcount(const uint64_t mask[PL_TOPOLOGY_MASK_WORDS]) {
    uint32_t count = 0u;
    for (size_t i = 0u; i < PL_TOPOLOGY_MASK_WORDS; ++i) {
        uint64_t bits = mask[i];
        while (bits != 0u) { count += (uint32_t)(bits & 1u); bits >>= 1u; }
    }
    return count;
}

static size_t find_or_add_windows_mask(pl_processor_topology_snapshot *out,
                                       WORD group, KAFFINITY mask,
                                       uint32_t package_id, uint32_t core_id,
                                       uint32_t numa_id, int cache_record,
                                       uint32_t cache_level, uint64_t cache_size,
                                       uint32_t line_size, pl_topology_cache_kind cache_kind) {
    uint64_t logical_mask[PL_TOPOLOGY_MASK_WORDS] = {0u};
    for (unsigned bit = 0u; bit < sizeof(KAFFINITY) * 8u; ++bit) {
        if ((mask & ((KAFFINITY)1u << bit)) != 0u) {
            size_t index = add_cpu(out, (uint32_t)group, bit, 1);
            pl_topology_logical_cpu *cpu;
            if (index == SIZE_MAX) return SIZE_MAX;
            cpu = &out->logical_cpus[index];
            if (package_id != PL_TOPOLOGY_UNKNOWN) cpu->package_id = package_id;
            if (core_id != PL_TOPOLOGY_UNKNOWN) cpu->core_id = core_id;
            if (numa_id != PL_TOPOLOGY_UNKNOWN) cpu->numa_node_id = numa_id;
            cpu->evidence = (cpu->package_id != PL_TOPOLOGY_UNKNOWN &&
                             cpu->core_id != PL_TOPOLOGY_UNKNOWN)
                                ? PL_TOPOLOGY_EVIDENCE_OBSERVED
                                : PL_TOPOLOGY_EVIDENCE_PARTIAL;
            set_mask_bit(logical_mask, cpu->logical_id);
        }
    }
    if (numa_id != PL_TOPOLOGY_UNKNOWN) {
        pl_topology_numa_node *node = NULL;
        for (size_t i = 0u; i < out->numa_node_count; ++i)
            if (out->numa_nodes[i].node_id == numa_id) node = &out->numa_nodes[i];
        if (node == NULL && out->numa_node_count < PL_TOPOLOGY_MAX_NUMA_NODES) {
            node = &out->numa_nodes[out->numa_node_count++];
            memset(node, 0, sizeof(*node));
            node->node_id = numa_id;
        }
        if (node != NULL) {
            for (size_t i = 0u; i < PL_TOPOLOGY_MASK_WORDS; ++i)
                node->logical_mask[i] |= logical_mask[i];
            node->logical_count = mask_popcount(node->logical_mask);
            node->evidence = PL_TOPOLOGY_EVIDENCE_OBSERVED;
        }
    }
    if (cache_record) {
        pl_topology_cache *cache = NULL;
        for (size_t i = 0u; i < out->cache_count; ++i) {
            const pl_topology_cache *candidate = &out->caches[i];
            if (candidate->level == cache_level && candidate->size_bytes == cache_size &&
                candidate->line_bytes == line_size && candidate->kind == cache_kind &&
                memcmp(candidate->sharing_mask, logical_mask, sizeof(logical_mask)) == 0) {
                cache = &out->caches[i];
                break;
            }
        }
        if (cache == NULL && out->cache_count < PL_TOPOLOGY_MAX_CACHES) {
            cache = &out->caches[out->cache_count];
            memset(cache, 0, sizeof(*cache));
            cache->cache_id = out->cache_count++;
            cache->level = cache_level;
            cache->size_bytes = cache_size;
            cache->line_bytes = line_size;
            cache->kind = cache_kind;
            cache->evidence = (cache_size != 0u && line_size != 0u)
                                  ? PL_TOPOLOGY_EVIDENCE_OBSERVED
                                  : PL_TOPOLOGY_EVIDENCE_PARTIAL;
            memcpy(cache->sharing_mask, logical_mask, sizeof(logical_mask));
            cache->sharing_count = mask_popcount(logical_mask);
        }
    }
    return 0u;
}

static pl_topology_status scan_windows(pl_processor_topology_snapshot *out) {
    DWORD bytes = 0u;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *records = NULL;
    uint32_t core_id = 0u;
    uint32_t package_id = 0u;
    uint32_t die_id = 0u;
    if (!GetLogicalProcessorInformationEx(RelationAll, NULL, &bytes) &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER) return PL_TOPOLOGY_IO_ERROR;
    if (bytes == 0u || bytes > 16u * 1024u * 1024u) return PL_TOPOLOGY_LIMIT_EXCEEDED;
    records = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)malloc(bytes);
    if (records == NULL) return PL_TOPOLOGY_IO_ERROR;
    if (!GetLogicalProcessorInformationEx(RelationAll, records, &bytes)) {
        free(records);
        return PL_TOPOLOGY_IO_ERROR;
    }
    for (DWORD offset = 0u; offset < bytes;) {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *item =
            (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)((unsigned char *)records + offset);
        if (item->Size == 0u || offset + item->Size > bytes) {
            free(records);
            return PL_TOPOLOGY_IO_ERROR;
        }
        if (item->Relationship == RelationProcessorPackage) {
            for (WORD i = 0u; i < item->Processor.GroupCount; ++i) {
                (void)find_or_add_windows_mask(out,
                    item->Processor.GroupMask[i].Group,
                    item->Processor.GroupMask[i].Mask,
                    package_id, PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN,
                    0, 0u, 0u, 0u, PL_TOPOLOGY_CACHE_UNKNOWN);
            }
            ++package_id;
        } else if (item->Relationship == RelationProcessorDie) {
            for (WORD i = 0u; i < item->Processor.GroupCount; ++i) {
                (void)find_or_add_windows_mask(out,
                    item->Processor.GroupMask[i].Group,
                    item->Processor.GroupMask[i].Mask,
                    PL_TOPOLOGY_UNKNOWN, die_id, PL_TOPOLOGY_UNKNOWN,
                    0, 0u, 0u, 0u, PL_TOPOLOGY_CACHE_UNKNOWN);
            }
            ++die_id;
        } else if (item->Relationship == RelationProcessorCore) {
            for (WORD i = 0u; i < item->Processor.GroupCount; ++i) {
                (void)find_or_add_windows_mask(out,
                    item->Processor.GroupMask[i].Group,
                    item->Processor.GroupMask[i].Mask,
                    PL_TOPOLOGY_UNKNOWN, core_id, PL_TOPOLOGY_UNKNOWN,
                    0, 0u, 0u, 0u, PL_TOPOLOGY_CACHE_UNKNOWN);
            }
            ++core_id;
        } else if (item->Relationship == RelationNumaNode) {
            (void)find_or_add_windows_mask(out,
                item->NumaNode.GroupMask.Group,
                item->NumaNode.GroupMask.Mask,
                PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN,
                item->NumaNode.NodeNumber, 0, 0u, 0u, 0u,
                PL_TOPOLOGY_CACHE_UNKNOWN);
        } else if (item->Relationship == RelationCache) {
            pl_topology_cache_kind kind = PL_TOPOLOGY_CACHE_UNKNOWN;
            if (item->Cache.Type == CacheData) kind = PL_TOPOLOGY_CACHE_DATA;
            else if (item->Cache.Type == CacheInstruction) kind = PL_TOPOLOGY_CACHE_INSTRUCTION;
            else if (item->Cache.Type == CacheUnified) kind = PL_TOPOLOGY_CACHE_UNIFIED;
            (void)find_or_add_windows_mask(out,
                item->Cache.GroupMask.Group, item->Cache.GroupMask.Mask,
                PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN, PL_TOPOLOGY_UNKNOWN,
                1, item->Cache.Level, item->Cache.CacheSize,
                item->Cache.LineSize, kind);
        }
        offset += item->Size;
    }
    free(records);
    for (size_t i = 0u; i < out->numa_node_count; ++i) {
        ULONGLONG available = 0u;
        if (out->numa_nodes[i].node_id <= UINT16_MAX &&
            GetNumaAvailableMemoryNodeEx((USHORT)out->numa_nodes[i].node_id, &available)) {
            out->numa_nodes[i].memory_available_bytes = (uint64_t)available;
            out->numa_nodes[i].memory_available_observed = 1u;
        }
        out->numa_nodes[i].evidence =
            out->numa_nodes[i].memory_available_bytes != 0u
                ? PL_TOPOLOGY_EVIDENCE_PARTIAL
                : PL_TOPOLOGY_EVIDENCE_PARTIAL;
    }
    out->online_logical_cpu_count = out->logical_cpu_count;
    return out->logical_cpu_count == 0u ? PL_TOPOLOGY_IO_ERROR : PL_TOPOLOGY_OK;
}
#endif

pl_topology_status pl_processor_topology_validate(
    const pl_processor_topology_snapshot *snapshot) {
    uint32_t online = 0u;
    if (snapshot == NULL) return PL_TOPOLOGY_INVALID_ARGUMENT;
    if (snapshot->generation == 0u || snapshot->logical_cpu_count == 0u ||
        snapshot->logical_cpu_count > PL_TOPOLOGY_MAX_LOGICAL_CPUS ||
        snapshot->online_logical_cpu_count > snapshot->logical_cpu_count ||
        snapshot->cache_count > PL_TOPOLOGY_MAX_CACHES ||
        snapshot->numa_node_count > PL_TOPOLOGY_MAX_NUMA_NODES ||
        snapshot->completeness > PL_TOPOLOGY_UNKNOWN_STATE)
        return PL_TOPOLOGY_LIMIT_EXCEEDED;

    for (uint32_t i = 0u; i < snapshot->logical_cpu_count; ++i) {
        const pl_topology_logical_cpu *cpu = &snapshot->logical_cpus[i];
        if (cpu->logical_id >= PL_TOPOLOGY_MAX_LOGICAL_CPUS || cpu->online > 1u ||
            cpu->evidence > PL_TOPOLOGY_EVIDENCE_OBSERVED)
            return PL_TOPOLOGY_INVALID_REFERENCE;
        if (cpu->online != 0u) ++online;
        for (uint32_t j = i + 1u; j < snapshot->logical_cpu_count; ++j) {
            if (cpu->logical_id == snapshot->logical_cpus[j].logical_id ||
                (cpu->group_id == snapshot->logical_cpus[j].group_id &&
                 cpu->os_id == snapshot->logical_cpus[j].os_id))
                return PL_TOPOLOGY_DUPLICATE_CPU;
        }
        if (cpu->numa_node_id != PL_TOPOLOGY_UNKNOWN) {
            int found = 0;
            for (uint32_t n = 0u; n < snapshot->numa_node_count; ++n) {
                const pl_topology_numa_node *node = &snapshot->numa_nodes[n];
                if (node->node_id == cpu->numa_node_id &&
                    (node->logical_mask[cpu->logical_id / 64u] &
                     (UINT64_C(1) << (cpu->logical_id % 64u))) != 0u)
                    found = 1;
            }
            if (!found) return PL_TOPOLOGY_INVALID_REFERENCE;
        }
    }
    if (online != snapshot->online_logical_cpu_count)
        return PL_TOPOLOGY_INVALID_REFERENCE;

    for (uint32_t i = 0u; i < snapshot->cache_count; ++i) {
        const pl_topology_cache *cache = &snapshot->caches[i];
        uint32_t count = 0u;
        if (cache->cache_id != i || cache->level == 0u || cache->level > 4u ||
            (unsigned)cache->kind > (unsigned)PL_TOPOLOGY_CACHE_UNKNOWN ||
            cache->evidence > PL_TOPOLOGY_EVIDENCE_OBSERVED)
            return PL_TOPOLOGY_INVALID_MASK;
        for (size_t word = 0u; word < PL_TOPOLOGY_MASK_WORDS; ++word) {
            uint64_t bits = cache->sharing_mask[word];
            while (bits != 0u) { count += (uint32_t)(bits & 1u); bits >>= 1u; }
        }
        if (count != cache->sharing_count ||
            (cache->evidence == PL_TOPOLOGY_EVIDENCE_OBSERVED && count == 0u))
            return PL_TOPOLOGY_INVALID_MASK;
        for (uint32_t id = 0u; id < PL_TOPOLOGY_MAX_LOGICAL_CPUS; ++id) {
            if ((cache->sharing_mask[id / 64u] & (UINT64_C(1) << (id % 64u))) != 0u) {
                int found = 0;
                for (uint32_t cpu = 0u; cpu < snapshot->logical_cpu_count; ++cpu)
                    if (snapshot->logical_cpus[cpu].logical_id == id) found = 1;
                if (!found) return PL_TOPOLOGY_INVALID_MASK;
            }
        }
    }

    for (uint32_t i = 0u; i < snapshot->numa_node_count; ++i) {
        const pl_topology_numa_node *node = &snapshot->numa_nodes[i];
        uint32_t count = 0u;
        for (size_t word = 0u; word < PL_TOPOLOGY_MASK_WORDS; ++word) {
            uint64_t bits = node->logical_mask[word];
            while (bits != 0u) { count += (uint32_t)(bits & 1u); bits >>= 1u; }
        }
        if (count != node->logical_count ||
            node->evidence > PL_TOPOLOGY_EVIDENCE_OBSERVED ||
            node->memory_available_observed > 1u ||
            node->memory_total_observed > 1u ||
            (node->memory_total_observed != 0u &&
             node->memory_available_observed != 0u &&
             node->memory_available_bytes > node->memory_total_bytes))
            return PL_TOPOLOGY_INVALID_MASK;
        if (node->evidence == PL_TOPOLOGY_EVIDENCE_OBSERVED && count == 0u)
            return PL_TOPOLOGY_INVALID_MASK;
        for (uint32_t j = i + 1u; j < snapshot->numa_node_count; ++j)
            if (node->node_id == snapshot->numa_nodes[j].node_id)
                return PL_TOPOLOGY_INVALID_REFERENCE;
        for (uint32_t id = 0u; id < PL_TOPOLOGY_MAX_LOGICAL_CPUS; ++id) {
            if ((node->logical_mask[id / 64u] & (UINT64_C(1) << (id % 64u))) != 0u) {
                int found = 0;
                for (uint32_t cpu = 0u; cpu < snapshot->logical_cpu_count; ++cpu) {
                    if (snapshot->logical_cpus[cpu].logical_id == id &&
                        snapshot->logical_cpus[cpu].numa_node_id == node->node_id)
                        found = 1;
                }
                if (!found) return PL_TOPOLOGY_INVALID_MASK;
            }
        }
    }

    for (size_t i = 0u; i < PL_TOPOLOGY_ISA_FEATURE_COUNT; ++i) {
        const pl_topology_isa_state *isa = &snapshot->isa[i];
        if (isa->hardware_present < -1 || isa->hardware_present > 1 ||
            isa->os_usable < -1 || isa->os_usable > 1 || isa->evidence > 2u ||
            (isa->os_usable == 1 && isa->hardware_present != 1))
            return PL_TOPOLOGY_INVALID_REFERENCE;
    }
    return PL_TOPOLOGY_OK;
}

pl_topology_status pl_processor_topology_scan(pl_processor_topology_snapshot *out) {
    if (out == NULL) return PL_TOPOLOGY_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    for (size_t i = 0u; i < PL_TOPOLOGY_ISA_FEATURE_COUNT; ++i) {
        out->isa[i].hardware_present = -1;
        out->isa[i].os_usable = -1;
    }
    out->generation = next_generation();
#if defined(_WIN32)
    set_string(out->provider, sizeof(out->provider), "Windows");
    set_string(out->interface_name, sizeof(out->interface_name), "windows-processor-topology-api");
    {
        DWORD groups = GetActiveProcessorGroupCount();
        for (WORD group = 0u; group < groups; ++group) {
            DWORD active = GetActiveProcessorCount(group);
            if (active > PL_TOPOLOGY_MAX_LOGICAL_CPUS ||
                out->logical_cpu_count + active > PL_TOPOLOGY_MAX_LOGICAL_CPUS)
                return PL_TOPOLOGY_LIMIT_EXCEEDED;
            for (DWORD cpu = 0u; cpu < active; ++cpu) {
                if (add_cpu(out, group, cpu, 1) == SIZE_MAX)
                    return PL_TOPOLOGY_LIMIT_EXCEEDED;
            }
        }
        {
            pl_topology_status status = scan_windows(out);
            if (status != PL_TOPOLOGY_OK) return status;
        }
    }
#elif defined(__linux__)
    set_string(out->provider, sizeof(out->provider), "Linux");
    set_string(out->interface_name, sizeof(out->interface_name), "linux-sysfs");
    {
        pl_topology_status status = scan_linux(out);
        if (status != PL_TOPOLOGY_OK) return status;
    }
#else
    set_string(out->provider, sizeof(out->provider), "unsupported");
    set_string(out->interface_name, sizeof(out->interface_name), "unknown");
    return PL_TOPOLOGY_UNSUPPORTED;
#endif
    capture_time(out->captured_at);
    set_isa_from_profile(out);
    out->completeness = PL_TOPOLOGY_COMPLETE;
    if (out->cache_count == 0u || out->numa_node_count == 0u)
        out->completeness = PL_TOPOLOGY_PARTIAL;
    for (uint32_t i = 0u; i < out->cache_count; ++i)
        if (out->caches[i].evidence != PL_TOPOLOGY_EVIDENCE_OBSERVED)
            out->completeness = PL_TOPOLOGY_PARTIAL;
    for (uint32_t i = 0u; i < out->numa_node_count; ++i)
        if (out->numa_nodes[i].evidence != PL_TOPOLOGY_EVIDENCE_OBSERVED)
            out->completeness = PL_TOPOLOGY_PARTIAL;
    for (uint32_t i = 0u; i < out->logical_cpu_count; ++i) {
        const pl_topology_logical_cpu *cpu = &out->logical_cpus[i];
        if (cpu->package_id == PL_TOPOLOGY_UNKNOWN ||
            cpu->die_id == PL_TOPOLOGY_UNKNOWN ||
            cpu->core_id == PL_TOPOLOGY_UNKNOWN ||
            cpu->numa_node_id == PL_TOPOLOGY_UNKNOWN ||
            cpu->evidence != PL_TOPOLOGY_EVIDENCE_OBSERVED) {
            out->completeness = PL_TOPOLOGY_PARTIAL;
            break;
        }
    }
    if (pl_processor_topology_validate(out) != PL_TOPOLOGY_OK)
        return PL_TOPOLOGY_INVALID_REFERENCE;
    return PL_TOPOLOGY_OK;
}

static int write_optional_u32(FILE *output, uint32_t value) {
    if (value == PL_TOPOLOGY_UNKNOWN) return fputs("null", output) >= 0;
    return fprintf(output, "%" PRIu32, value) >= 0;
}

static int write_json_string(FILE *output, const char *value) {
    if (fputc('"', output) == EOF) return 0;
    for (const unsigned char *p = (const unsigned char *)value; *p != 0u; ++p) {
        if (*p == '"' || *p == '\\') {
            if (fputc('\\', output) == EOF || fputc(*p, output) == EOF) return 0;
        } else if (*p < 0x20u) {
            if (fprintf(output, "\\u%04x", (unsigned)*p) < 0) return 0;
        } else if (fputc(*p, output) == EOF) return 0;
    }
    return fputc('"', output) != EOF;
}

static int write_optional_u64(FILE *output, uint64_t value) {
    if (value == 0u) return fputs("null", output) >= 0;
    return fprintf(output, "%" PRIu64, value) >= 0;
}

static int write_mask_ids(FILE *output, const pl_processor_topology_snapshot *snapshot,
                          const uint64_t mask[PL_TOPOLOGY_MASK_WORDS]) {
    int first = 1;
    for (uint32_t i = 0u; i < snapshot->logical_cpu_count; ++i) {
        uint32_t id = snapshot->logical_cpus[i].logical_id;
        if ((mask[id / 64u] & (UINT64_C(1) << (id % 64u))) != 0u) {
            if ((!first && fputc(',', output) == EOF) ||
                fprintf(output, "%" PRIu32, id) < 0) return 0;
            first = 0;
        }
    }
    return 1;
}

pl_topology_status pl_processor_topology_write_json(
    const pl_processor_topology_snapshot *snapshot, FILE *output) {
    static const char *const feature_names[PL_TOPOLOGY_ISA_FEATURE_COUNT] = {
        "sse2", "sse4_2", "aes", "avx", "avx2", "avx512f", "fma", "virtualization"
    };
    if (output == NULL || pl_processor_topology_validate(snapshot) != PL_TOPOLOGY_OK)
        return PL_TOPOLOGY_INVALID_ARGUMENT;
    if (fprintf(output, "{\"schema\":\"%s\",\"version\":2,\"generation\":%" PRIu32
                 ",\"captured_at\":", PL_TOPOLOGY_SCHEMA, snapshot->generation) < 0 ||
        !write_json_string(output, snapshot->captured_at) ||
        fputs(",\"collector\":{\"provider\":", output) < 0 ||
        !write_json_string(output, snapshot->provider) ||
        fputs(",\"interface\":", output) < 0 ||
        !write_json_string(output, snapshot->interface_name) ||
        fputs(",\"read_only\":true,\"platform_release\":", output) < 0 ||
        !write_json_string(output, snapshot->platform_release) ||
        fputs("},\"completeness\":", output) < 0) return PL_TOPOLOGY_IO_ERROR;
    if (fputs(snapshot->completeness == PL_TOPOLOGY_COMPLETE ? "\"complete\"" : "\"partial\"",
              output) < 0 ||
        fprintf(output, ",\"topology\":{\"logical_processor_count\":%" PRIu32
                ",\"online_logical_processor_count\":%" PRIu32 ",\"logical_processors\":[",
                snapshot->logical_cpu_count, snapshot->online_logical_cpu_count) < 0)
        return PL_TOPOLOGY_IO_ERROR;
    for (uint32_t i = 0u; i < snapshot->logical_cpu_count; ++i) {
        const pl_topology_logical_cpu *cpu = &snapshot->logical_cpus[i];
        if ((i != 0u && fputc(',', output) == EOF) ||
            fprintf(output, "{\"logical_id\":%" PRIu32 ",\"os_id\":%" PRIu32
                    ",\"group_id\":", cpu->logical_id, cpu->os_id) < 0 ||
            !write_optional_u32(output, cpu->group_id) ||
            fputs(",\"online\":", output) < 0 ||
            fputs(cpu->online ? "true" : "false", output) < 0 ||
            fputs(",\"package_id\":", output) < 0 ||
            !write_optional_u32(output, cpu->package_id) ||
            fputs(",\"die_id\":", output) < 0 ||
            !write_optional_u32(output, cpu->die_id) ||
            fputs(",\"core_id\":", output) < 0 ||
            !write_optional_u32(output, cpu->core_id) ||
            fputs(",\"numa_node_id\":", output) < 0 ||
            !write_optional_u32(output, cpu->numa_node_id) ||
            fputs(cpu->evidence == PL_TOPOLOGY_EVIDENCE_OBSERVED
                    ? ",\"evidence\":\"observed\"}"
                    : (cpu->evidence == PL_TOPOLOGY_EVIDENCE_PARTIAL
                       ? ",\"evidence\":\"partially-observed\"}"
                       : ",\"evidence\":\"unknown\"}"), output) < 0)
            return PL_TOPOLOGY_IO_ERROR;
    }
    if (fputs("],\"caches\":[", output) < 0) return PL_TOPOLOGY_IO_ERROR;
    for (uint32_t i = 0u; i < snapshot->cache_count; ++i) {
        const pl_topology_cache *cache = &snapshot->caches[i];
        if ((i != 0u && fputc(',', output) == EOF) ||
            fprintf(output, "{\"cache_id\":%" PRIu32 ",\"level\":%" PRIu32
                    ",\"kind\":\"%s\",\"size_bytes\":",
                    cache->cache_id, cache->level,
                    cache->kind == PL_TOPOLOGY_CACHE_DATA ? "data" :
                    cache->kind == PL_TOPOLOGY_CACHE_INSTRUCTION ? "instruction" :
                    cache->kind == PL_TOPOLOGY_CACHE_UNIFIED ? "unified" : "unknown") < 0)
            return PL_TOPOLOGY_IO_ERROR;
        if (!write_optional_u64(output, cache->size_bytes) ||
            fputs(",\"line_bytes\":", output) < 0 ||
            !write_optional_u32(output, cache->line_bytes) ||
            fputs(",\"sharing_logical_ids\":[", output) < 0 ||
            !write_mask_ids(output, snapshot, cache->sharing_mask) ||
            fputs(cache->evidence == PL_TOPOLOGY_EVIDENCE_OBSERVED
                    ? "],\"evidence\":\"observed\"}"
                    : "],\"evidence\":\"partially-observed\"}", output) < 0)
            return PL_TOPOLOGY_IO_ERROR;
    }
    if (fputs("],\"numa_nodes\":[", output) < 0) return PL_TOPOLOGY_IO_ERROR;
    for (uint32_t i = 0u; i < snapshot->numa_node_count; ++i) {
        const pl_topology_numa_node *node = &snapshot->numa_nodes[i];
        if ((i != 0u && fputc(',', output) == EOF) ||
            fprintf(output, "{\"node_id\":%" PRIu32 ",\"logical_ids\":[", node->node_id) < 0 ||
            !write_mask_ids(output, snapshot, node->logical_mask) ||
            fputs("],\"memory_total_bytes\":", output) < 0 ||
            (node->memory_total_observed != 0u
                ? fprintf(output, "%" PRIu64, node->memory_total_bytes) < 0
                : fputs("null", output) < 0) ||
            fputs(",\"memory_available_bytes\":", output) < 0 ||
            (node->memory_available_observed != 0u
                ? fprintf(output, "%" PRIu64, node->memory_available_bytes) < 0
                : fputs("null", output) < 0) ||
            fprintf(output, ",\"evidence\":\"%s\"}",
                    node->evidence == PL_TOPOLOGY_EVIDENCE_OBSERVED ? "observed" : "partially-observed") < 0)
            return PL_TOPOLOGY_IO_ERROR;
    }
    if (fputs("]},\"isa\":{\"architecture\":", output) < 0 ||
        !write_json_string(output, snapshot->architecture[0] ? snapshot->architecture : "unknown") ||
        fputs(",\"vendor\":", output) < 0 ||
        (snapshot->vendor[0] ? !write_json_string(output, snapshot->vendor) : fputs("null", output) < 0) ||
        fputs(",\"model\":", output) < 0 ||
        (snapshot->model[0] ? !write_json_string(output, snapshot->model) : fputs("null", output) < 0) ||
        fputs(",\"features\":{", output) < 0) return PL_TOPOLOGY_IO_ERROR;
    for (size_t i = 0u; i < PL_TOPOLOGY_ISA_FEATURE_COUNT; ++i) {
        const pl_topology_isa_state *feature = &snapshot->isa[i];
        const char *hardware = feature->hardware_present < 0 ? "null" :
                               (feature->hardware_present ? "true" : "false");
        const char *usable = feature->os_usable < 0 ? "null" :
                             (feature->os_usable ? "true" : "false");
        if ((i != 0u && fputc(',', output) == EOF) ||
            fprintf(output, "\"%s\":{\"hardware_present\":%s,\"os_usable\":%s,\"evidence\":\"%s\"}",
                    feature_names[i], hardware, usable,
                    feature->evidence == 2u ? "os-state" :
                    feature->evidence == 1u ? "cpuid" : "unknown") < 0)
            return PL_TOPOLOGY_IO_ERROR;
    }
    if (fputs("}},\"execution_performed\":false,\"hardware_access_performed\":false,\"processor_state_changed\":false}\n",
              output) < 0 || ferror(output)) return PL_TOPOLOGY_IO_ERROR;
    return PL_TOPOLOGY_OK;
}

const char *pl_topology_status_name(pl_topology_status status) {
    switch (status) {
    case PL_TOPOLOGY_OK: return "ok";
    case PL_TOPOLOGY_INVALID_ARGUMENT: return "invalid-argument";
    case PL_TOPOLOGY_LIMIT_EXCEEDED: return "limit-exceeded";
    case PL_TOPOLOGY_DUPLICATE_CPU: return "duplicate-cpu";
    case PL_TOPOLOGY_INVALID_REFERENCE: return "invalid-reference";
    case PL_TOPOLOGY_INVALID_MASK: return "invalid-mask";
    case PL_TOPOLOGY_IO_ERROR: return "io-error";
    case PL_TOPOLOGY_UNSUPPORTED: return "unsupported";
    }
    return "unknown";
}
