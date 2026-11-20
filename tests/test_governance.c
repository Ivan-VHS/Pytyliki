#include "policy.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

static int verifier_accepts(const pl_consent_grant *grant, void *context) {
    const int *accepted = (const int *)context;
    return grant != NULL && accepted != NULL && *accepted == 1 ? 1 : 0;
}

int main(void) {
    pl_consent_grant grant = {
        "local-operator", "diagnostic-export", "summary-json",
        UINT64_C(1000), UINT64_C(1900), 1, 0
    };
    int signature_ok = 1;
    assert(pl_policy_action_requires_consent(PL_ACTION_EXPORT_REPORT));
    assert(pl_policy_action_requires_consent(PL_ACTION_SHARE_REPORT));
    assert(pl_policy_action_requires_consent(PL_ACTION_ENABLE_TELEMETRY));
    assert(!pl_policy_action_requires_consent(PL_ACTION_READ));

    /* A role alone is not enough for sensitive export/share/telemetry. */
    assert(!pl_policy_authorize(PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT));
    assert(!pl_policy_authorize(PL_ROLE_MAINTAINER, PL_ACTION_SHARE_REPORT));
    assert(!pl_policy_authorize(PL_ROLE_ADMIN, PL_ACTION_ENABLE_TELEMETRY));

    assert(pl_consent_grant_validate(
        &grant, "local-operator", "diagnostic-export", "summary-json", 1001));
    assert(pl_policy_authorize_with_consent(
        PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT, &grant,
        "local-operator", "diagnostic-export", "summary-json", 1001,
        verifier_accepts, &signature_ok));

    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_VIEWER, PL_ACTION_EXPORT_REPORT, &grant,
        "local-operator", "diagnostic-export", "summary-json", 1001,
        verifier_accepts, &signature_ok));
    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT, &grant,
        "other-operator", "diagnostic-export", "summary-json", 1001,
        verifier_accepts, &signature_ok));
    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT, &grant,
        "local-operator", "marketing", "summary-json", 1001,
        verifier_accepts, &signature_ok));
    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT, &grant,
        "local-operator", "diagnostic-export", "all-data", 1001,
        verifier_accepts, &signature_ok));

    assert(!pl_consent_grant_validate(
        &grant, "local-operator", "diagnostic-export", "summary-json", 1900));
    assert(!pl_consent_grant_validate(
        &grant, "local-operator", "diagnostic-export", "summary-json", 999));
    grant.revoked = 1;
    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT, &grant,
        "local-operator", "diagnostic-export", "summary-json", 1001,
        verifier_accepts, &signature_ok));
    grant.revoked = 0;
    grant.expires_at_epoch = grant.issued_at_epoch + PL_CONSENT_MAX_TTL_SECONDS + 1;
    assert(!pl_consent_grant_validate(
        &grant, "local-operator", "diagnostic-export", "summary-json", 1001));

    grant.expires_at_epoch = 1900;
    signature_ok = 0;
    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT, &grant,
        "local-operator", "diagnostic-export", "summary-json", 1001,
        verifier_accepts, &signature_ok));
    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_ADMIN, PL_ACTION_ENABLE_TELEMETRY, &grant,
        "local-operator", "diagnostic-export", "summary-json", 1001,
        NULL, NULL));
    assert(!pl_policy_authorize_with_consent(
        PL_ROLE_ADMIN, PL_ACTION_ENABLE_TELEMETRY, &grant,
        "local-operator", "diagnostic-export", "summary-json", 1001,
        verifier_accepts, &signature_ok));
    strcpy(grant.purpose, "telemetry-enable");
    strcpy(grant.scope, "metrics");
    assert(pl_policy_authorize_with_consent(
        PL_ROLE_ADMIN, PL_ACTION_ENABLE_TELEMETRY, &grant,
        "local-operator", "telemetry-enable", "metrics", 1001,
        verifier_accepts, &(int){1}));

    assert(strcmp(pl_policy_action_name(PL_ACTION_EXPORT_REPORT), "export-report") == 0);
    return 0;
}
