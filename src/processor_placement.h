#ifndef PYTYLIKI_PROCESSOR_PLACEMENT_H
#define PYTYLIKI_PROCESSOR_PLACEMENT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PL_CPU_PLACEMENT_MAX_LOGICAL 8192u
#define PL_CPU_PLACEMENT_MASK_WORDS (PL_CPU_PLACEMENT_MAX_LOGICAL / 64u)
#define PL_CPU_PLACEMENT_MAX_CPUSETS 64u
typedef enum {
    PL_CPU_PLACEMENT_OK = 0,
    PL_CPU_PLACEMENT_INVALID_ARGUMENT = -1,
    PL_CPU_PLACEMENT_CONSENT_REQUIRED = -2,
    PL_CPU_PLACEMENT_UNSUPPORTED = -3,
    PL_CPU_PLACEMENT_NOT_AVAILABLE = -4,
    PL_CPU_PLACEMENT_PERMISSION_DENIED = -5,
    PL_CPU_PLACEMENT_OS_ERROR = -6,
    PL_CPU_PLACEMENT_SNAPSHOT_INVALID = -7,
    PL_CPU_PLACEMENT_CAPACITY = -8
} pl_cpu_placement_status;
typedef enum {
    PL_CPU_PLACEMENT_BACKEND_NONE = 0,
    PL_CPU_PLACEMENT_BACKEND_LINUX_SCHED = 1,
    PL_CPU_PLACEMENT_BACKEND_WINDOWS_CPU_SET = 2
} pl_cpu_placement_backend;
typedef struct {
    uint32_t backend;
    uint32_t valid;
    uint32_t selected_count;
    uint32_t reserved;
    uint32_t selected_cpu_set_ids[PL_CPU_PLACEMENT_MAX_CPUSETS];
    uint64_t previous_affinity[PL_CPU_PLACEMENT_MASK_WORDS];
} pl_cpu_placement_snapshot;
/*
 * Returns the first currently eligible processor identifier.
 * Linux returns a logical CPU index; Windows returns a CPU Set ID.
 */
int pl_cpu_placement_first_allowed(uint32_t *processor_id);
/*
 * Changes placement only for the calling application thread.
 * consent must be exactly 1. The previous placement is captured before change.
 */
int pl_cpu_placement_apply_current_thread(uint32_t processor_id, int consent,
                                           pl_cpu_placement_snapshot *previous);
/* Restores the calling thread to the captured placement. */
int pl_cpu_placement_restore_current_thread(
    const pl_cpu_placement_snapshot *previous);
const char *pl_cpu_placement_status_name(pl_cpu_placement_status status);
const char *pl_cpu_placement_backend_name(pl_cpu_placement_backend backend);
#ifdef __cplusplus
}
#endif
#endif
