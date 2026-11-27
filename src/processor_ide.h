#ifndef PYTYLIKI_PROCESSOR_IDE_H
#define PYTYLIKI_PROCESSOR_IDE_H

#include "ironclad_systems.h"
#include "low_level_contract.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_IDE_ABI_VERSION UINT32_C(0x00010000)
#define PL_IDE_PROJECT_ID_MAX 48u
#define PL_IDE_SESSION_ID_MAX 48u

typedef enum {
    PL_IDE_MODE_INSPECT = 1,
    PL_IDE_MODE_PLAN = 2,
    PL_IDE_MODE_STATIC = 3,
    PL_IDE_MODE_SIMULATE = 4,
    PL_IDE_MODE_DEBUG = 5,
    PL_IDE_MODE_HARDWARE_IN_LOOP = 6,
    PL_IDE_MODE_PRIVILEGED = 7
} pl_ide_mode;

enum {
    PL_IDE_FLAG_TARGET_PROVIDER = UINT32_C(1) << 0,
    PL_IDE_FLAG_DEBUG_PROVIDER = UINT32_C(1) << 1,
    PL_IDE_FLAG_TARGET_CAPABILITY = UINT32_C(1) << 2,
    PL_IDE_FLAG_OPERATOR_APPROVED = UINT32_C(1) << 3
};

typedef enum {
    PL_IDE_OK = 0,
    PL_IDE_INVALID_ARGUMENT,
    PL_IDE_ABI_MISMATCH,
    PL_IDE_INVALID_IDENTIFIER,
    PL_IDE_INVALID_WORKSPACE,
    PL_IDE_INVALID_MODE,
    PL_IDE_INVALID_TARGET,
    PL_IDE_INVALID_FLAGS,
    PL_IDE_PROVIDER_REQUIRED,
    PL_IDE_CAPABILITY_REQUIRED,
    PL_IDE_OPERATOR_APPROVAL_REQUIRED
} pl_ide_status;

typedef struct {
    uint32_t abi_version;
    char project_id[PL_IDE_PROJECT_ID_MAX];
    char session_id[PL_IDE_SESSION_ID_MAX];
    uint32_t workspace_id;
    pl_ide_mode mode;
    uint32_t flags;
    pl_ll_target target;
} pl_ide_session;

/*
 * Expose the existing twenty-domain registry without creating a second
 * workspace catalogue. The returned entries remain owned by the registry.
 */
size_t pl_ide_workspace_catalog(const pl_system_enhancement **out);
const pl_system_enhancement *pl_ide_workspace_find(unsigned id);

/*
 * Build a target-bound session descriptor. This function validates metadata
 * only; it never opens a device, writes MMIO, starts a debugger or invokes a
 * tool. The default safe mode is PL_IDE_MODE_INSPECT.
 */
pl_ide_status pl_ide_session_init(
    pl_ide_session *out,
    const char *project_id,
    const char *session_id,
    uint32_t workspace_id,
    pl_ide_mode mode,
    const pl_ll_target *target,
    uint32_t flags);

pl_ide_status pl_ide_session_validate(const pl_ide_session *session);
const char *pl_ide_status_string(pl_ide_status status);

#ifdef __cplusplus
}
#endif

#endif
