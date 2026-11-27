#define _GNU_SOURCE
#include "processor_placement.h"
#include <stddef.h>
#include <string.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#include <stdlib.h>
#elif defined(__linux__)
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#else
#endif

#if defined(_WIN32)
static int cpu_set_exists_and_available(ULONG cpu_set_id) {
    DWORD bytes = 0u;
    SYSTEM_CPU_SET_INFORMATION *items;
    DWORD offset = 0u;
    int found = 0;
    if (GetSystemCpuSetInformation(NULL, 0u, &bytes, GetCurrentProcess(), 0u) ||
        GetLastError() != ERROR_INSUFFICIENT_BUFFER || bytes == 0u) {
        return 0;
    }
    items = (SYSTEM_CPU_SET_INFORMATION *)malloc(bytes);
    if (items == NULL) return 0;
    if (!GetSystemCpuSetInformation(items, bytes, &bytes, GetCurrentProcess(), 0u)) {
        free(items);
        return 0;
    }
    while (offset + sizeof(SYSTEM_CPU_SET_INFORMATION) <= bytes) {
        SYSTEM_CPU_SET_INFORMATION *item =
            (SYSTEM_CPU_SET_INFORMATION *)((unsigned char *)items + offset);
        if (item->Size == 0u || offset + item->Size > bytes) break;
        if (item->Type == CpuSetInformation &&
            item->CpuSet.Id == cpu_set_id) {
            found = !item->CpuSet.RealTime &&
                (!item->CpuSet.Allocated || item->CpuSet.AllocatedToTargetProcess);
            break;
        }
        offset += item->Size;
    }
    free(items);
    return found;
}
int pl_cpu_placement_first_allowed(uint32_t *processor_id) {
    DWORD bytes = 0u;
    SYSTEM_CPU_SET_INFORMATION *items;
    DWORD offset = 0u;
    if (processor_id == NULL) return PL_CPU_PLACEMENT_INVALID_ARGUMENT;
    if (GetSystemCpuSetInformation(NULL, 0u, &bytes, GetCurrentProcess(), 0u) ||
        GetLastError() != ERROR_INSUFFICIENT_BUFFER || bytes == 0u) {
        return PL_CPU_PLACEMENT_OS_ERROR;
    }
    items = (SYSTEM_CPU_SET_INFORMATION *)malloc(bytes);
    if (items == NULL) return PL_CPU_PLACEMENT_OS_ERROR;
    if (!GetSystemCpuSetInformation(items, bytes, &bytes, GetCurrentProcess(), 0u)) {
        free(items);
        return PL_CPU_PLACEMENT_OS_ERROR;
    }
    while (offset + sizeof(SYSTEM_CPU_SET_INFORMATION) <= bytes) {
        SYSTEM_CPU_SET_INFORMATION *item =
            (SYSTEM_CPU_SET_INFORMATION *)((unsigned char *)items + offset);
        if (item->Size == 0u || offset + item->Size > bytes) break;
        if (item->Type == CpuSetInformation &&
            !item->CpuSet.RealTime &&
            (!item->CpuSet.Allocated || item->CpuSet.AllocatedToTargetProcess)) {
            *processor_id = item->CpuSet.Id;
            free(items);
            return PL_CPU_PLACEMENT_OK;
        }
        offset += item->Size;
    }
    free(items);
    return PL_CPU_PLACEMENT_NOT_AVAILABLE;
}
int pl_cpu_placement_apply_current_thread(uint32_t processor_id, int consent,
                                          pl_cpu_placement_snapshot *previous) {
    ULONG returned = 0u;
    HANDLE thread = GetCurrentThread();
    if (previous == NULL) return PL_CPU_PLACEMENT_INVALID_ARGUMENT;
    memset(previous, 0, sizeof(*previous));
    if (consent != 1) return PL_CPU_PLACEMENT_CONSENT_REQUIRED;
    if (!cpu_set_exists_and_available((ULONG)processor_id))
        return PL_CPU_PLACEMENT_NOT_AVAILABLE;
    if (!GetThreadSelectedCpuSets(thread, NULL, 0u, &returned)) {
        DWORD error = GetLastError();
        if (error != ERROR_INSUFFICIENT_BUFFER && error != ERROR_BAD_LENGTH)
            return PL_CPU_PLACEMENT_OS_ERROR;
    }
    if (returned > PL_CPU_PLACEMENT_MAX_CPUSETS)
        return PL_CPU_PLACEMENT_CAPACITY;
    if (returned != 0u) {
        if (!GetThreadSelectedCpuSets(thread, previous->selected_cpu_set_ids,
                                      PL_CPU_PLACEMENT_MAX_CPUSETS, &returned))
            return PL_CPU_PLACEMENT_OS_ERROR;
        previous->selected_count = (uint32_t)returned;
    }
    previous->backend = PL_CPU_PLACEMENT_BACKEND_WINDOWS_CPU_SET;
    previous->valid = 1u;
    if (!SetThreadSelectedCpuSets(thread, (const ULONG *)&processor_id, 1u)) {
        memset(previous, 0, sizeof(*previous));
        return GetLastError() == ERROR_ACCESS_DENIED
            ? PL_CPU_PLACEMENT_PERMISSION_DENIED : PL_CPU_PLACEMENT_OS_ERROR;
    }
    return PL_CPU_PLACEMENT_OK;
}
int pl_cpu_placement_restore_current_thread(
    const pl_cpu_placement_snapshot *previous) {
    if (previous == NULL || previous->valid != 1u ||
        previous->backend != PL_CPU_PLACEMENT_BACKEND_WINDOWS_CPU_SET ||
        previous->selected_count > PL_CPU_PLACEMENT_MAX_CPUSETS)
        return PL_CPU_PLACEMENT_SNAPSHOT_INVALID;
    if (!SetThreadSelectedCpuSets(GetCurrentThread(),
            previous->selected_count != 0u ? previous->selected_cpu_set_ids : NULL,
            previous->selected_count))
        return GetLastError() == ERROR_ACCESS_DENIED
            ? PL_CPU_PLACEMENT_PERMISSION_DENIED : PL_CPU_PLACEMENT_OS_ERROR;
    return PL_CPU_PLACEMENT_OK;
}
#elif defined(__linux__)
static size_t affinity_set_bytes(void) {
    return CPU_ALLOC_SIZE(PL_CPU_PLACEMENT_MAX_LOGICAL);
}
static cpu_set_t *affinity_set_new(void) {
    cpu_set_t *set = CPU_ALLOC(PL_CPU_PLACEMENT_MAX_LOGICAL);
    if (set != NULL) CPU_ZERO_S(affinity_set_bytes(), set);
    return set;
}
int pl_cpu_placement_first_allowed(uint32_t *processor_id) {
    cpu_set_t *set;
    size_t bytes = affinity_set_bytes();
    if (processor_id == NULL) return PL_CPU_PLACEMENT_INVALID_ARGUMENT;
    set = affinity_set_new();
    if (set == NULL) return PL_CPU_PLACEMENT_OS_ERROR;
    if (sched_getaffinity(0, bytes, set) != 0) {
        int error = errno;
        CPU_FREE(set);
        return error == EPERM || error == EACCES
            ? PL_CPU_PLACEMENT_PERMISSION_DENIED : PL_CPU_PLACEMENT_OS_ERROR;
    }
    for (uint32_t cpu = 0u; cpu < PL_CPU_PLACEMENT_MAX_LOGICAL; ++cpu) {
        if (CPU_ISSET_S(cpu, bytes, set)) {
            *processor_id = cpu;
            CPU_FREE(set);
            return PL_CPU_PLACEMENT_OK;
        }
    }
    CPU_FREE(set);
    return PL_CPU_PLACEMENT_NOT_AVAILABLE;
}
int pl_cpu_placement_apply_current_thread(uint32_t processor_id, int consent,
                                          pl_cpu_placement_snapshot *previous) {
    cpu_set_t *old_set;
    cpu_set_t *new_set;
    const size_t bytes = affinity_set_bytes();
    if (previous == NULL || processor_id >= PL_CPU_PLACEMENT_MAX_LOGICAL)
        return PL_CPU_PLACEMENT_INVALID_ARGUMENT;
    memset(previous, 0, sizeof(*previous));
    if (consent != 1) return PL_CPU_PLACEMENT_CONSENT_REQUIRED;
    old_set = affinity_set_new();
    new_set = affinity_set_new();
    if (old_set == NULL || new_set == NULL) {
        if (old_set != NULL) CPU_FREE(old_set);
        if (new_set != NULL) CPU_FREE(new_set);
        return PL_CPU_PLACEMENT_OS_ERROR;
    }
    if (sched_getaffinity(0, bytes, old_set) != 0) {
        int error = errno;
        CPU_FREE(old_set);
        CPU_FREE(new_set);
        return error == EPERM || error == EACCES
            ? PL_CPU_PLACEMENT_PERMISSION_DENIED : PL_CPU_PLACEMENT_OS_ERROR;
    }
    if (!CPU_ISSET_S(processor_id, bytes, old_set)) {
        CPU_FREE(old_set);
        CPU_FREE(new_set);
        return PL_CPU_PLACEMENT_NOT_AVAILABLE;
    }
    for (uint32_t cpu = 0u; cpu < PL_CPU_PLACEMENT_MAX_LOGICAL; ++cpu) {
        if (CPU_ISSET_S(cpu, bytes, old_set))
            previous->previous_affinity[cpu / 64u] |= UINT64_C(1) << (cpu % 64u);
    }
    previous->backend = PL_CPU_PLACEMENT_BACKEND_LINUX_SCHED;
    previous->valid = 1u;
    CPU_SET_S(processor_id, bytes, new_set);
    if (sched_setaffinity(0, bytes, new_set) != 0) {
        int error = errno;
        memset(previous, 0, sizeof(*previous));
        CPU_FREE(old_set);
        CPU_FREE(new_set);
        return error == EPERM || error == EACCES
            ? PL_CPU_PLACEMENT_PERMISSION_DENIED : PL_CPU_PLACEMENT_OS_ERROR;
    }
    CPU_FREE(old_set);
    CPU_FREE(new_set);
    return PL_CPU_PLACEMENT_OK;
}
int pl_cpu_placement_restore_current_thread(
    const pl_cpu_placement_snapshot *previous) {
    cpu_set_t *old_set;
    const size_t bytes = affinity_set_bytes();
    if (previous == NULL || previous->valid != 1u ||
        previous->backend != PL_CPU_PLACEMENT_BACKEND_LINUX_SCHED)
        return PL_CPU_PLACEMENT_SNAPSHOT_INVALID;
    old_set = affinity_set_new();
    if (old_set == NULL) return PL_CPU_PLACEMENT_OS_ERROR;
    for (uint32_t cpu = 0u; cpu < PL_CPU_PLACEMENT_MAX_LOGICAL; ++cpu) {
        if ((previous->previous_affinity[cpu / 64u] &
             (UINT64_C(1) << (cpu % 64u))) != 0u)
            CPU_SET_S(cpu, bytes, old_set);
    }
    if (sched_setaffinity(0, bytes, old_set) != 0) {
        int error = errno;
        CPU_FREE(old_set);
        return error == EPERM || error == EACCES
            ? PL_CPU_PLACEMENT_PERMISSION_DENIED : PL_CPU_PLACEMENT_OS_ERROR;
    }
    CPU_FREE(old_set);
    return PL_CPU_PLACEMENT_OK;
}
#else
int pl_cpu_placement_first_allowed(uint32_t *processor_id) {
    (void)processor_id;
    return PL_CPU_PLACEMENT_UNSUPPORTED;
}
int pl_cpu_placement_apply_current_thread(uint32_t processor_id, int consent,
                                          pl_cpu_placement_snapshot *previous) {
    (void)processor_id;
    (void)consent;
    (void)previous;
    return PL_CPU_PLACEMENT_UNSUPPORTED;
}
int pl_cpu_placement_restore_current_thread(
    const pl_cpu_placement_snapshot *previous) {
    (void)previous;
    return PL_CPU_PLACEMENT_UNSUPPORTED;
}
#endif

const char *pl_cpu_placement_status_name(pl_cpu_placement_status status) {
    switch (status) {
    case PL_CPU_PLACEMENT_OK: return "ok";
    case PL_CPU_PLACEMENT_INVALID_ARGUMENT: return "invalid_argument";
    case PL_CPU_PLACEMENT_CONSENT_REQUIRED: return "consent_required";
    case PL_CPU_PLACEMENT_UNSUPPORTED: return "unsupported";
    case PL_CPU_PLACEMENT_NOT_AVAILABLE: return "not_available";
    case PL_CPU_PLACEMENT_PERMISSION_DENIED: return "permission_denied";
    case PL_CPU_PLACEMENT_OS_ERROR: return "os_error";
    case PL_CPU_PLACEMENT_SNAPSHOT_INVALID: return "snapshot_invalid";
    case PL_CPU_PLACEMENT_CAPACITY: return "capacity";
    default: return "unknown";
    }
}
const char *pl_cpu_placement_backend_name(pl_cpu_placement_backend backend) {
    switch (backend) {
    case PL_CPU_PLACEMENT_BACKEND_LINUX_SCHED: return "linux-sched-affinity";
    case PL_CPU_PLACEMENT_BACKEND_WINDOWS_CPU_SET: return "windows-cpu-sets";
    default: return "none";
    }
}
