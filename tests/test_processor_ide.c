#include "processor_ide.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

static pl_ll_target make_target(void) {
    pl_ll_target target;
    memset(&target, 0, sizeof(target));
    target.abi_version = PL_LL_ABI_VERSION;
    (void)strcpy(target.id, "test-x86_64");
    target.address_bits = 48u;
    target.word_bits = 64u;
    target.byte_order = PL_LL_LITTLE_ENDIAN;
    target.bus_width_bits = 64u;
    target.cache_line_bytes = 64u;
    target.cpu_count = 8u;
    target.clock_min_hz = UINT64_C(1000000);
    target.clock_max_hz = UINT64_C(5000000000);
    target.supported_power_states =
        PL_LL_POWER_MASK(PL_LL_POWER_IDLE) |
        PL_LL_POWER_MASK(PL_LL_POWER_PERFORMANCE);
    return target;
}

static void test_workspace_registry_reuse(void) {
    const pl_system_enhancement *items = NULL;
    assert(pl_ide_workspace_catalog(&items) == PL_SYSTEM_ENHANCEMENT_COUNT);
    assert(items != NULL);
    for (size_t index = 0u; index < PL_SYSTEM_ENHANCEMENT_COUNT; ++index) {
        const pl_system_enhancement *found = pl_ide_workspace_find((unsigned)index + 1u);
        assert(found == &items[index]);
        assert(pl_system_enhancement_validate(found) == 0);
    }
    assert(pl_ide_workspace_catalog(NULL) == 0u);
    assert(pl_ide_workspace_find(0u) == NULL);
    assert(pl_ide_workspace_find(PL_SYSTEM_ENHANCEMENT_COUNT + 1u) == NULL);
}

static void test_safe_session_modes(void) {
    const pl_ll_target target = make_target();
    pl_ide_session session;
    assert(pl_ide_session_init(&session, "pytyliki", "session-inspect", 5u,
                               PL_IDE_MODE_INSPECT, &target, 0u) == PL_IDE_OK);
    assert(pl_ide_session_validate(&session) == PL_IDE_OK);
    assert(session.mode == PL_IDE_MODE_INSPECT);
    assert(strcmp(session.project_id, "pytyliki") == 0);

    assert(pl_ide_session_init(&session, "pytyliki", "session-default", 5u,
                               PL_IDE_MODE_DEFAULT, &target, 0u) == PL_IDE_OK);
    assert(session.mode == PL_IDE_MODE_INSPECT);

    assert(pl_ide_session_init(&session, "pytyliki", "session-sim", 19u,
                               PL_IDE_MODE_SIMULATE, &target, 0u) == PL_IDE_OK);
    assert(pl_ide_session_validate(&session) == PL_IDE_OK);

    assert(pl_ide_session_init(&session, "pytyliki", "session-plan", 18u,
                               PL_IDE_MODE_PLAN, &target, 0u) == PL_IDE_OK);
    assert(pl_ide_session_validate(&session) == PL_IDE_OK);
}

static void test_debug_requires_provider_capability_and_approval(void) {
    const pl_ll_target target = make_target();
    pl_ide_session session;
    const uint32_t complete =
        PL_IDE_FLAG_TARGET_PROVIDER |
        PL_IDE_FLAG_DEBUG_PROVIDER |
        PL_IDE_FLAG_TARGET_AUTHORIZATION_DECLARED |
        PL_IDE_FLAG_OPERATOR_APPROVED;

    assert(pl_ide_session_init(&session, "pytyliki", "session-debug", 5u,
        PL_IDE_MODE_DEBUG, &target, 0u) == PL_IDE_PROVIDER_REQUIRED);
    assert(pl_ide_session_init(&session, "pytyliki", "session-debug", 5u,
        PL_IDE_MODE_DEBUG, &target, PL_IDE_FLAG_TARGET_PROVIDER |
        PL_IDE_FLAG_DEBUG_PROVIDER) == PL_IDE_CAPABILITY_REQUIRED);
    assert(pl_ide_session_init(&session, "pytyliki", "session-debug", 5u,
        PL_IDE_MODE_DEBUG, &target, PL_IDE_FLAG_TARGET_PROVIDER |
        PL_IDE_FLAG_DEBUG_PROVIDER | PL_IDE_FLAG_TARGET_AUTHORIZATION_DECLARED) ==
        PL_IDE_OPERATOR_APPROVAL_REQUIRED);
    assert(pl_ide_session_init(&session, "pytyliki", "session-debug", 5u,
        PL_IDE_MODE_DEBUG, &target, complete) == PL_IDE_OK);
}

static void test_physical_modes_require_capability(void) {
    const pl_ll_target target = make_target();
    pl_ide_session session;
    const uint32_t approved =
        PL_IDE_FLAG_TARGET_PROVIDER |
        PL_IDE_FLAG_TARGET_AUTHORIZATION_DECLARED |
        PL_IDE_FLAG_OPERATOR_APPROVED;

    assert(pl_ide_session_init(&session, "pytyliki", "session-hil", 5u,
        PL_IDE_MODE_HARDWARE_IN_LOOP, &target, 0u) == PL_IDE_PROVIDER_REQUIRED);
    assert(pl_ide_session_init(&session, "pytyliki", "session-hil", 5u,
        PL_IDE_MODE_HARDWARE_IN_LOOP, &target, approved) == PL_IDE_OK);
    assert(pl_ide_session_init(&session, "pytyliki", "session-priv", 5u,
        PL_IDE_MODE_PRIVILEGED, &target, approved) == PL_IDE_OK);
}

static void test_invalid_session_metadata(void) {
    const pl_ll_target target = make_target();
    pl_ide_session session;
    pl_ll_target invalid = target;

    assert(pl_ide_session_init(&session, "bad id", "session-1", 1u,
        PL_IDE_MODE_INSPECT, &target, 0u) == PL_IDE_INVALID_IDENTIFIER);
    assert(pl_ide_session_init(&session, "pytyliki", "session-1", 21u,
        PL_IDE_MODE_INSPECT, &target, 0u) == PL_IDE_INVALID_WORKSPACE);
    assert(pl_ide_session_init(&session, "pytyliki", "session-1", 1u,
        (pl_ide_mode)99, &target, 0u) == PL_IDE_INVALID_MODE);
    assert(pl_ide_session_init(&session, "pytyliki", "session-1", 1u,
        PL_IDE_MODE_INSPECT, &target, UINT32_C(1) << 31) == PL_IDE_INVALID_FLAGS);

    invalid.address_bits = 0u;
    assert(pl_ide_session_init(&session, "pytyliki", "session-1", 1u,
        PL_IDE_MODE_INSPECT, &invalid, 0u) == PL_IDE_INVALID_TARGET);
    assert(pl_ide_session_init(NULL, "pytyliki", "session-1", 1u,
        PL_IDE_MODE_INSPECT, &target, 0u) == PL_IDE_INVALID_ARGUMENT);
}

int main(void) {
    test_workspace_registry_reuse();
    test_safe_session_modes();
    test_debug_requires_provider_capability_and_approval();
    test_physical_modes_require_capability();
    test_invalid_session_metadata();
    assert(strcmp(pl_ide_status_string(PL_IDE_OK), "ok") == 0);
    assert(strcmp(pl_ide_status_string((pl_ide_status)99), "unknown") == 0);
    puts("Processor IDE session contract tests passed");
    return 0;
}
