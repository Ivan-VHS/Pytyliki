#include "berry_core.h"
#include "processor.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

int main(void) {
    const pl_berry_core_command *catalog = NULL;
    pl_processor_snapshot snapshot;
    pl_berry_core_result result;
    pl_berry_core_request request = {PL_BERRY_MALINA, 0u};
    assert(pl_berry_core_catalog(&catalog) == PL_BERRY_CORE_COMMAND_COUNT);
    assert(catalog != NULL);
    for (unsigned i = 0u; i < PL_BERRY_CORE_COMMAND_COUNT; ++i) {
        assert(catalog[i].name != NULL && catalog[i].name[0] != '\0');
        assert(catalog[i].virtue != NULL && catalog[i].virtue[0] != '\0');
        assert(catalog[i].operation != NULL && catalog[i].operation[0] != '\0');
        request.command = (pl_berry_core_id)i;
        request.target_logical_core = 0u;
        assert(pl_berry_core_dispatch(&request, &result) == 0);
        assert(result.accepted && result.planned_only);
        assert(!result.hardware_modified);
        assert(result.target_logical_core == 0u);
    }
    assert(pl_processor_scan(&snapshot) == 0);
    request.command = PL_BERRY_MALINA;
    request.target_logical_core = snapshot.base.logical_cpus;
    assert(pl_berry_core_dispatch(&request, &result) == -3);
    assert(result.detail[0] != '\0');
    request.target_logical_core = 0u;
    request.command = (pl_berry_core_id)PL_BERRY_CORE_COMMAND_COUNT;
    assert(pl_berry_core_dispatch(&request, &result) == -1);
    assert(pl_berry_core_dispatch(NULL, &result) == -1);
    assert(pl_berry_core_dispatch(&request, NULL) == -1);
    puts("All berry core command tests passed.");
    return 0;
}
