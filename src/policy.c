#include "policy.h"
#include <stddef.h>
#include <string.h>

static const char *const role_names[PL_ROLE_COUNT] = {
    "viewer", "analyst", "contributor", "maintainer", "admin"
};

static const char *const action_names[PL_ACTION_COUNT] = {
    "read", "analyze", "benchmark", "propose-patch", "apply-patch",
    "manage-access", "change-policy", "execute-inspected-code",
    "write-firmware", "change-cpu-controls"
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

int pl_policy_authorize(pl_role role, pl_action action) {
    if (role < 0 || role >= PL_ROLE_COUNT ||
        action < 0 || action >= PL_ACTION_COUNT) return 0;

    /* These operations are outside the product's safety boundary. */
    if (action == PL_ACTION_EXECUTE_INSPECTED_CODE ||
        action == PL_ACTION_WRITE_FIRMWARE ||
        action == PL_ACTION_CHANGE_CPU_CONTROLS) return 0;

    switch (action) {
        case PL_ACTION_READ:
        case PL_ACTION_ANALYZE:
            return 1;
        case PL_ACTION_BENCHMARK:
            return role >= PL_ROLE_ANALYST;
        case PL_ACTION_PROPOSE_PATCH:
            return role >= PL_ROLE_CONTRIBUTOR;
        case PL_ACTION_APPLY_PATCH:
            return role >= PL_ROLE_MAINTAINER;
        case PL_ACTION_MANAGE_ACCESS:
        case PL_ACTION_CHANGE_POLICY:
            return role == PL_ROLE_ADMIN;
        default:
            return 0;
    }
}
