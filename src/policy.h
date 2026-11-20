#ifndef PYTYLIKI_POLICY_H
#define PYTYLIKI_POLICY_H
#include <stdint.h>

typedef enum {
    PL_ROLE_VIEWER = 0,
    PL_ROLE_ANALYST,
    PL_ROLE_CONTRIBUTOR,
    PL_ROLE_MAINTAINER,
    PL_ROLE_ADMIN,
    PL_ROLE_COUNT
} pl_role;

typedef enum {
    PL_ACTION_READ = 0,
    PL_ACTION_ANALYZE,
    PL_ACTION_BENCHMARK,
    PL_ACTION_PROPOSE_PATCH,
    PL_ACTION_APPLY_PATCH,
    PL_ACTION_MANAGE_ACCESS,
    PL_ACTION_CHANGE_POLICY,
    PL_ACTION_EXECUTE_INSPECTED_CODE,
    PL_ACTION_WRITE_FIRMWARE,
    PL_ACTION_CHANGE_CPU_CONTROLS,
    PL_ACTION_EXPORT_REPORT,
    PL_ACTION_SHARE_REPORT,
    PL_ACTION_ENABLE_TELEMETRY,
    PL_ACTION_COUNT
} pl_action;

/* Deny by default. Dangerous hardware/source-execution actions are never
 * granted by this local policy layer, including to the administrator role. */
#define PL_CONSENT_SUBJECT_MAX 128
#define PL_CONSENT_PURPOSE_MAX 64
#define PL_CONSENT_SCOPE_MAX 128
#define PL_CONSENT_MAX_TTL_SECONDS UINT64_C(31536000)

typedef struct {
    char subject[PL_CONSENT_SUBJECT_MAX];
    char purpose[PL_CONSENT_PURPOSE_MAX];
    char scope[PL_CONSENT_SCOPE_MAX];
    uint64_t issued_at_epoch;
    uint64_t expires_at_epoch;
    int approved;
    int revoked;
} pl_consent_grant;

/* External verifier authenticates issuer/signature; this library checks scope,
 * purpose, subject, TTL and revocation. Returning anything except 1 denies. */
typedef int (*pl_consent_authenticator)(const pl_consent_grant *grant, void *context);

int pl_policy_authorize(pl_role role, pl_action action);
int pl_policy_action_requires_consent(pl_action action);
int pl_consent_grant_validate(const pl_consent_grant *grant,
    const char *subject, const char *purpose, const char *scope,
    uint64_t now_epoch);
int pl_policy_authorize_with_consent(pl_role role, pl_action action,
    const pl_consent_grant *grant, const char *subject, const char *purpose,
    const char *scope, uint64_t now_epoch,
    pl_consent_authenticator authenticate, void *context);
const char *pl_policy_role_name(pl_role role);
const char *pl_policy_action_name(pl_action action);
int pl_policy_parse_role(const char *name, pl_role *out);
int pl_policy_parse_action(const char *name, pl_action *out);

#endif
