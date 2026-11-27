#include "processor_ide.h"

#include <string.h>

#define PL_IDE_KNOWN_FLAGS (PL_IDE_FLAG_TARGET_PROVIDER | \
                            PL_IDE_FLAG_DEBUG_PROVIDER | \
                            PL_IDE_FLAG_TARGET_CAPABILITY | \
                            PL_IDE_FLAG_OPERATOR_APPROVED)

static int identifier_copy(char *destination, size_t capacity,
                           const char *source) {
    size_t length = 0u;
    size_t index;

    if (destination == NULL || source == NULL || capacity < 2u) return 0;
    while (length < capacity && source[length] != '\0') ++length;
    if (length == 0u || length >= capacity) return 0;

    for (index = 0u; index < length; ++index) {
        const unsigned char ch = (unsigned char)source[index];
        const int valid =
            (ch >= (unsigned char)'a' && ch <= (unsigned char)'z') ||
            (ch >= (unsigned char)'A' && ch <= (unsigned char)'Z') ||
            (ch >= (unsigned char)'0' && ch <= (unsigned char)'9') ||
            ch == (unsigned char)'-' || ch == (unsigned char)'_' ||
            ch == (unsigned char)'.';
        if (!valid) return 0;
    }

    memcpy(destination, source, length);
    destination[length] = '\0';
    return 1;
}

size_t pl_ide_workspace_catalog(const pl_system_enhancement **out) {
    return pl_system_enhancements(out);
}

const pl_system_enhancement *pl_ide_workspace_find(unsigned id) {
    const pl_system_enhancement *items = NULL;
    size_t count = pl_system_enhancements(&items);
    size_t index;

    if (items == NULL || id == 0u) return NULL;
    for (index = 0u; index < count; ++index) {
        if (items[index].id == id &&
            pl_system_enhancement_validate(&items[index]) == 0) {
            return &items[index];
        }
    }
    return NULL;
}

static int mode_valid(pl_ide_mode mode) {
    return mode >= PL_IDE_MODE_INSPECT &&
           mode <= PL_IDE_MODE_PRIVILEGED;
}

pl_ide_status pl_ide_session_validate(const pl_ide_session *session) {
    const uint32_t target_modes = PL_IDE_MODE_DEBUG |
                                  PL_IDE_MODE_HARDWARE_IN_LOOP |
                                  PL_IDE_MODE_PRIVILEGED;
    if (session == NULL) return PL_IDE_INVALID_ARGUMENT;
    if (session->abi_version != PL_IDE_ABI_VERSION) return PL_IDE_ABI_MISMATCH;
    if (session->project_id[0] == '\0' || session->session_id[0] == '\0')
        return PL_IDE_INVALID_IDENTIFIER;
    if (pl_ide_workspace_find(session->workspace_id) == NULL)
        return PL_IDE_INVALID_WORKSPACE;
    if (!mode_valid(session->mode)) return PL_IDE_INVALID_MODE;
    if ((session->flags & ~PL_IDE_KNOWN_FLAGS) != 0u)
        return PL_IDE_INVALID_FLAGS;
    if (!pl_ll_target_validate(&session->target)) return PL_IDE_INVALID_TARGET;

    if ((unsigned)session->mode >= (unsigned)PL_IDE_MODE_DEBUG) {
        if ((session->flags & PL_IDE_FLAG_TARGET_PROVIDER) == 0u)
            return PL_IDE_PROVIDER_REQUIRED;
        if ((session->flags & PL_IDE_FLAG_TARGET_CAPABILITY) == 0u)
            return PL_IDE_CAPABILITY_REQUIRED;
        if ((session->flags & PL_IDE_FLAG_OPERATOR_APPROVED) == 0u)
            return PL_IDE_OPERATOR_APPROVAL_REQUIRED;
        if (session->mode == PL_IDE_MODE_DEBUG &&
            (session->flags & PL_IDE_FLAG_DEBUG_PROVIDER) == 0u)
            return PL_IDE_PROVIDER_REQUIRED;
    } else if ((target_modes & (uint32_t)session->mode) != 0u) {
        return PL_IDE_INVALID_MODE;
    }

    return PL_IDE_OK;
}

pl_ide_status pl_ide_session_init(
    pl_ide_session *out,
    const char *project_id,
    const char *session_id,
    uint32_t workspace_id,
    pl_ide_mode mode,
    const pl_ll_target *target,
    uint32_t flags) {
    pl_ide_status status;

    if (out == NULL || target == NULL) return PL_IDE_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    out->abi_version = PL_IDE_ABI_VERSION;

    if (!identifier_copy(out->project_id, sizeof(out->project_id), project_id) ||
        !identifier_copy(out->session_id, sizeof(out->session_id), session_id)) {
        memset(out, 0, sizeof(*out));
        return PL_IDE_INVALID_IDENTIFIER;
    }

    out->workspace_id = workspace_id;
    out->mode = mode;
    out->flags = flags;
    out->target = *target;

    status = pl_ide_session_validate(out);
    if (status != PL_IDE_OK) memset(out, 0, sizeof(*out));
    return status;
}

const char *pl_ide_status_string(pl_ide_status status) {
    static const char *const names[] = {
        "ok",
        "invalid-argument",
        "abi-mismatch",
        "invalid-identifier",
        "invalid-workspace",
        "invalid-mode",
        "invalid-target",
        "invalid-flags",
        "provider-required",
        "capability-required",
        "operator-approval-required"
    };
    return (unsigned)status < (sizeof(names) / sizeof(names[0]))
        ? names[(unsigned)status] : "unknown";
}
