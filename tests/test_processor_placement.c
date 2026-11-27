#include "processor_placement.h"
#include "processor_thread_recovery_arch.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    pl_cpu_placement_snapshot previous;
    uint32_t cpu_id = 0u;
    memset(&previous, 0xA5, sizeof(previous));
    pl_tr_cpu_relax();
    assert(pl_cpu_placement_apply_current_thread(0u, 0, &previous) ==
           PL_CPU_PLACEMENT_CONSENT_REQUIRED);
    assert(pl_cpu_placement_first_allowed(&cpu_id) == PL_CPU_PLACEMENT_OK);
    assert(pl_cpu_placement_apply_current_thread(cpu_id, 1, &previous) ==
           PL_CPU_PLACEMENT_OK);
    assert(previous.valid == 1u);
    assert(previous.backend != PL_CPU_PLACEMENT_BACKEND_NONE);
    assert(pl_cpu_placement_restore_current_thread(&previous) == PL_CPU_PLACEMENT_OK);
    assert(pl_cpu_placement_restore_current_thread(NULL) ==
           PL_CPU_PLACEMENT_SNAPSHOT_INVALID);
    puts("Pytyliki reversible current-thread CPU placement tests passed.");
    return 0;
}
