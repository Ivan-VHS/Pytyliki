#include "ras_provider.h"

const char *pl_ras_provider_name(void) {
    return "Windows WHEA provider is unavailable on this platform";
}

pl_ras_provider_status pl_ras_provider_poll(
    pl_ras_provider_cursor *cursor,
    pl_ras_batch *batch
) {
    if (cursor == NULL || batch == NULL) return PL_RAS_PROVIDER_INVALID_ARGUMENT;
    for (size_t i = 0u; i < sizeof(*batch); ++i)
        ((unsigned char *)batch)[i] = 0u;
    return PL_RAS_PROVIDER_UNSUPPORTED;
}
