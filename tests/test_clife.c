#include "clife.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const char payload[] = "known-good-project-state";
    char restored[64];
    size_t written = 0u;
    pl_clife_checkpoint checkpoint;
    pl_clife_runtime_state state = {0.80, 0.90, 0.90, 5u, 0};
    pl_clife_fault_event fault = {PL_CLIFE_FAULT_ARTIFACT_INTEGRITY, 0.10, 0};

    assert(pl_clife_checkpoint_capture(&checkpoint, "pytyliki-test", 4u,
                                       payload, sizeof(payload)) == 0);
    assert(pl_clife_checkpoint_verify(&checkpoint));
    assert(pl_clife_checkpoint_restore(&checkpoint, "pytyliki-test", restored,
                                       sizeof(restored), &written) == 0);
    assert(written == sizeof(payload));
    assert(memcmp(restored, payload, sizeof(payload)) == 0);
    assert(pl_clife_checkpoint_restore(&checkpoint, "other-project", restored,
                                       sizeof(restored), &written) != 0);
    assert(pl_clife_checkpoint_restore(&checkpoint, "pytyliki-test", restored,
                                       2u, &written) != 0);

    assert(pl_clife_handle_fault(&state, &fault, &checkpoint, 0.0) ==
           PL_CLIFE_VERIFIED_RESTORE);
    assert(fault.mitigated == 1);

    state.artifact_integrity = 0.20;
    state.resource_health = 0.30;
    state.execution_health = 0.20;
    assert(pl_clife_branch_awakening(&state, NULL, 0.95) ==
           PL_CLIFE_EMERGENCY_SAFE_STOP);
    assert(state.recovery_locked == 1);
    assert(pl_clife_branch_awakening(&state, NULL, 2.0) == PL_CLIFE_INVALID_STATE);

    pl_clife_checkpoint_free(&checkpoint);
    assert(!pl_clife_checkpoint_verify(&checkpoint));
    puts("Pytyliki CLife tests: PASS");
    return 0;
}
