#include "policy.h"
#include <stddef.h>
#include <string.h>

static const char *const role_names[PL_ROLE_COUNT] = {
    "viewer", "analyst", "contributor", "maintainer", "admin"
};

static const char *const action_names[PL_ACTION_COUNT] = {
    "read", "analyze", "benchmark", "propose-patch", "apply-patch",
    "manage-access", "change-policy", "execute-inspected-code",
    "write-firmware", "change-cpu-controls", "export-report",
    "share-report", "enable-telemetry"
};

const char *pl_policy_role_name(pl_role role) {
    return (role >= 0 && role < PL_ROLE_COUNT) ? role_names[role] : "unknown";
}

const char *pl_policy_action_name(pl_action action) {
    return (action >= 0 && action < PL_ACTION_COUNT) ? action_names[action] : "unknown";
}

int pl_policy_parse_role(const char *name, pl_role *out) {
    int i;
    if (name == NULL || out == NULL) return -1;
    for (i = 0; i < PL_ROLE_COUNT; ++i) {
        if (strcmp(name, role_names[i]) == 0) {
            *out = (pl_role)i;
            return 0;
        }
    }
    return -1;
}

int pl_policy_parse_action(const char *name, pl_action *out) {
    int i;
    if (name == NULL || out == NULL) return -1;
    for (i = 0; i < PL_ACTION_COUNT; ++i) {
        if (strcmp(name, action_names[i]) == 0) {
            *out = (pl_action)i;
            return 0;
        }
    }
    return -1;
}

static int pl_policy_role_allows(pl_role role, pl_action action) {
    if (role < 0 || role >= PL_ROLE_COUNT ||
        action < 0 || action >= PL_ACTION_COUNT) return 0;

    if (action == PL_ACTION_EXECUTE_INSPECTED_CODE ||
        action == PL_ACTION_WRITE_FIRMWARE ||
        action == PL_ACTION_CHANGE_CPU_CONTROLS) return 0;

    switch (action) {
        case PL_ACTION_READ:
        case PL_ACTION_ANALYZE:
            return 1;
        case PL_ACTION_BENCHMARK:
        case PL_ACTION_EXPORT_REPORT:
            return role >= PL_ROLE_ANALYST;
        case PL_ACTION_PROPOSE_PATCH:
            return role >= PL_ROLE_CONTRIBUTOR;
        case PL_ACTION_APPLY_PATCH:
        case PL_ACTION_SHARE_REPORT:
            return role >= PL_ROLE_MAINTAINER;
        case PL_ACTION_MANAGE_ACCESS:
        case PL_ACTION_CHANGE_POLICY:
        case PL_ACTION_ENABLE_TELEMETRY:
            return role == PL_ROLE_ADMIN;
        default:
            return 0;
    }
}

int pl_policy_action_requires_consent(pl_action action) {
    return action == PL_ACTION_EXPORT_REPORT ||
        action == PL_ACTION_SHARE_REPORT ||
        action == PL_ACTION_ENABLE_TELEMETRY;
}

int pl_policy_authorize(pl_role role, pl_action action) {
    if (pl_policy_action_requires_consent(action)) return 0;
    return pl_policy_role_allows(role, action);
}

static int pl_bounded_text(const char *value, size_t maximum) {
    const char *end;
    if (value == NULL || value[0] == '\\0') return 0;
    end = (const char *)memchr(value, '\\0', maximum);
    return end != NULL;
}

int pl_consent_grant_validate(const pl_consent_grant *grant,
    const char *subject, const char *purpose, const char *scope,
    uint64_t now_epoch) {
    if (grant == NULL || !pl_bounded_text(grant->subject, sizeof(grant->subject)) ||
        !pl_bounded_text(grant->purpose, sizeof(grant->purpose)) ||
        !pl_bounded_text(grant->scope, sizeof(grant->scope)) ||
        !pl_bounded_text(subject, PL_CONSENT_SUBJECT_MAX) ||
        !pl_bounded_text(purpose, PL_CONSENT_PURPOSE_MAX) ||
        !pl_bounded_text(scope, PL_CONSENT_SCOPE_MAX)) return 0;
    if (grant->approved != 1 || grant->revoked != 0) return 0;
    if (grant->expires_at_epoch <= grant->issued_at_epoch ||
        grant->expires_at_epoch - grant->issued_at_epoch > PL_CONSENT_MAX_TTL_SECONDS ||
        now_epoch < grant->issued_at_epoch || now_epoch >= grant->expires_at_epoch)
        return 0;
    return strcmp(grant->subject, subject) == 0 &&
        strcmp(grant->purpose, purpose) == 0 &&
        strcmp(grant->scope, scope) == 0;
}

int pl_policy_authorize_with_consent(pl_role role, pl_action action,
    const pl_consent_grant *grant, const char *subject, const char *purpose,
    const char *scope, uint64_t now_epoch,
    pl_consent_authenticator authenticate, void *context) {
    if (role < 0 || role >= PL_ROLE_COUNT ||
        action < 0 || action >= PL_ACTION_COUNT) return 0;
    if (!pl_policy_action_requires_consent(action))
        return pl_policy_authorize(role, action);
    if (!pl_policy_role_allows(role, action) ||
        !pl_consent_grant_validate(grant, subject, purpose, scope, now_epoch) ||
        authenticate == NULL) return 0;
    return authenticate(grant, context) == 1;
}
