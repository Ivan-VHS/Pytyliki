#include "interop.h"

#include <string.h>

typedef struct {
    int32_t code;
    uint32_t severity;
    const char *message;
} diagnostic_entry;

static const diagnostic_entry DIAGNOSTICS[] = {
    {PYTYLIKI_DIAG_INVALID_ARGUMENT, PYTYLIKI_DIAG_SEVERITY_ERROR,
     "Invalid argument"},
    {PYTYLIKI_DIAG_UNSUPPORTED_ABI, PYTYLIKI_DIAG_SEVERITY_ERROR,
     "Unsupported ABI version"},
    {PYTYLIKI_DIAG_BUFFER_TOO_SMALL, PYTYLIKI_DIAG_SEVERITY_ERROR,
     "Output buffer is too small"},
    {PYTYLIKI_DIAG_NOT_FOUND, PYTYLIKI_DIAG_SEVERITY_WARNING,
     "Requested item was not found"},
    {PYTYLIKI_DIAG_INTERNAL, PYTYLIKI_DIAG_SEVERITY_FATAL,
     "Internal diagnostic"}
};

static const diagnostic_entry *find_diagnostic(int32_t code) {
    size_t index;
    for (index = 0u; index < sizeof(DIAGNOSTICS) / sizeof(DIAGNOSTICS[0]); ++index) {
        if (DIAGNOSTICS[index].code == code) {
            return &DIAGNOSTICS[index];
        }
    }
    return NULL;
}

uint32_t PYTYLIKI_FFI_CALL pytyliki_diagnostics_abi_version(void) {
    return PYTYLIKI_DIAGNOSTICS_ABI_VERSION;
}

int32_t PYTYLIKI_FFI_CALL pytyliki_v1_diagnostic_describe(
    int32_t diagnostic_code,
    uint32_t requested_abi_version,
    uint32_t out_size,
    pytyliki_diagnostic_v1 *out
) {
    const diagnostic_entry *entry;
    size_t message_length;

    if (out == NULL) {
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    }
    if (requested_abi_version != PYTYLIKI_DIAGNOSTICS_ABI_VERSION) {
        return PYTYLIKI_ABI_UNSUPPORTED_ABI;
    }
    if (out_size < PYTYLIKI_DIAGNOSTIC_V1_SIZE) {
        return PYTYLIKI_ABI_BUFFER_TOO_SMALL;
    }

    entry = find_diagnostic(diagnostic_code);
    if (entry == NULL) {
        return PYTYLIKI_ABI_NOT_FOUND;
    }

    message_length = strlen(entry->message) + 1u;
    if (message_length > PYTYLIKI_DIAGNOSTIC_MESSAGE_CAP) {
        return PYTYLIKI_ABI_NOT_FOUND;
    }

    memset(out, 0, sizeof(*out));
    out->struct_size = PYTYLIKI_DIAGNOSTIC_V1_SIZE;
    out->abi_version = PYTYLIKI_DIAGNOSTICS_ABI_VERSION;
    out->code = entry->code;
    out->severity = entry->severity;
    out->source_offset = 0u;
    out->source_length = 0u;
    out->message_size = (uint32_t)message_length;
    memcpy(out->message, entry->message, message_length);
    return PYTYLIKI_ABI_OK;
}
