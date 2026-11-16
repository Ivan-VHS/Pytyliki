#ifndef PYTYLIKI_POLICY_H
#define PYTYLIKI_POLICY_H

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
    PL_ACTION_COUNT
} pl_action;

/* Deny by default. Dangerous hardware/source-execution actions are never
 * granted by this local policy layer, including to the administrator role. */
int pl_policy_authorize(pl_role role, pl_action action);
const char *pl_policy_role_name(pl_role role);
const char *pl_policy_action_name(pl_action action);
int pl_policy_parse_role(const char *name, pl_role *out);
int pl_policy_parse_action(const char *name, pl_action *out);

#endif
