#include "interop.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

_Static_assert(sizeof(pytyliki_diagnostic_v1) == 136u,
               "diagnostics v1 layout must remain stable");
_Static_assert(offsetof(pytyliki_diagnostic_v1, source_length) == 24u,
               "source_length offset must remain stable");
_Static_assert(offsetof(pytyliki_diagnostic_v1, message_size) == 32u,
               "message_size offset must remain stable");

int main(void) {
    pytyliki_diagnostic_v1 record;
    struct {
        pytyliki_diagnostic_v1 v1;
        uint8_t future_extension[8];
    } extended;
    pytyliki_diagnostic_v1 unchanged;
    size_t index;

    assert(pytyliki_diagnostics_abi_version() == PYTYLIKI_DIAGNOSTICS_ABI_VERSION);

    memset(&record, 0, sizeof(record));
    assert(pytyliki_v1_diagnostic_describe(
        PYTYLIKI_DIAG_INVALID_ARGUMENT, PYTYLIKI_DIAGNOSTICS_ABI_VERSION,
        (uint32_t)sizeof(record), &record) == PYTYLIKI_ABI_OK);
    assert(record.struct_size == PYTYLIKI_DIAGNOSTIC_V1_SIZE);
    assert(record.abi_version == PYTYLIKI_DIAGNOSTICS_ABI_VERSION);
    assert(record.code == PYTYLIKI_DIAG_INVALID_ARGUMENT);
    assert(record.severity == PYTYLIKI_DIAG_SEVERITY_ERROR);
    assert(record.source_offset == 0u && record.source_length == 0u);
    assert(record.message_size == (uint32_t)strlen("Invalid argument") + 1u);
    assert(strcmp(record.message, "Invalid argument") == 0);
    assert(record.reserved == 0u);

    memset(&extended, 0xa5, sizeof(extended));
    assert(pytyliki_v1_diagnostic_describe(
        PYTYLIKI_DIAG_NOT_FOUND, PYTYLIKI_DIAGNOSTICS_ABI_VERSION,
        (uint32_t)sizeof(extended), &extended.v1) == PYTYLIKI_ABI_OK);
    for (index = 0u; index < sizeof(extended.future_extension); ++index) {
        assert(extended.future_extension[index] == 0xa5u);
    }

    memset(&unchanged, 0x5a, sizeof(unchanged));
    record = unchanged;
    assert(pytyliki_v1_diagnostic_describe(
        PYTYLIKI_DIAG_INVALID_ARGUMENT, PYTYLIKI_DIAGNOSTICS_ABI_VERSION,
        (uint32_t)sizeof(record) - 1u, &record) == PYTYLIKI_ABI_BUFFER_TOO_SMALL);
    assert(memcmp(&record, &unchanged, sizeof(record)) == 0);

    assert(pytyliki_v1_diagnostic_describe(
        PYTYLIKI_DIAG_INVALID_ARGUMENT, 2u,
        (uint32_t)sizeof(record), &record) == PYTYLIKI_ABI_UNSUPPORTED_ABI);
    assert(memcmp(&record, &unchanged, sizeof(record)) == 0);

    assert(pytyliki_v1_diagnostic_describe(
        9999, PYTYLIKI_DIAGNOSTICS_ABI_VERSION,
        (uint32_t)sizeof(record), &record) == PYTYLIKI_ABI_NOT_FOUND);
    assert(memcmp(&record, &unchanged, sizeof(record)) == 0);

    assert(pytyliki_v1_diagnostic_describe(
        PYTYLIKI_DIAG_INTERNAL, PYTYLIKI_DIAGNOSTICS_ABI_VERSION,
        (uint32_t)sizeof(record), NULL) == PYTYLIKI_ABI_INVALID_ARGUMENT);
    return 0;
}
