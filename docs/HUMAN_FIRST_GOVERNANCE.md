# Human-first governance for Pytyliki

Status: consent-aware action policy implemented in the C11 core. Pytyliki remains a processor workstation and local engineering utility. Its separate clinical-safety modules are research/engineering prototypes only, not a certified clinical product or medical device.

## Implemented behavior

- The role policy now includes report export, report sharing and telemetry activation as consent-sensitive actions.
- The legacy role-only authorization API denies these actions by default. Callers must use the new consent-aware authorization API.
- A grant is bound to an exact subject, purpose and scope. It must be explicitly approved, not revoked, already issued, not expired, and have a finite TTL no longer than 365 days.
- A caller-supplied authenticator callback is mandatory for consent-sensitive actions. The callback must verify the grant issuer/signature using the deploying application's trusted cryptographic provider. A missing callback or a result other than exactly 1 denies the action.
- The role threshold still applies after consent validation: analyst for export, maintainer for sharing, administrator for telemetry. Consent never overrides a role restriction.
- Existing deny rules for executing inspected source, writing firmware and changing CPU controls remain absolute.

## API sketch

    pl_consent_grant grant = {
        "local-operator", "diagnostic-export", "summary-json",
        issued_epoch, expires_epoch, 1, 0
    };
    int allowed = pl_policy_authorize_with_consent(
        PL_ROLE_ANALYST, PL_ACTION_EXPORT_REPORT, &grant,
        "local-operator", "diagnostic-export", "summary-json",
        now_epoch, trusted_signature_verifier, verifier_context
    );

The metadata validator is not itself a digital-signature implementation. Production callers must authenticate the issuer, validate the signature and protect keys outside this C library. Never treat the approved field as proof of a person's identity or legal capacity.

## Scope crosswalk

| Medical-rights concept | Pytyliki implementation / boundary |
|---|---|
| Informed consent and withdrawal | Exact subject/purpose/scope grant; finite expiry; explicit revoked state |
| Confidentiality / minimization | Role policy and narrow report/telemetry scopes; no universal PHI store |
| Access audit | Existing CLI evidence and caller-owned audit provider; no WORM promise |
| Clinical workflow quality | Existing clinical-safety utilities remain bounded calculations and checklist summaries only |
| Patient access, erasure, cross-border transfer | No patient record service or remote data plane is implemented |
| Research, prescribing, pharmacovigilance | Not applicable to processor profiling and engineering tools |
| SaMD / clinical AI | No certification or clinical decision authority is claimed |
| Human oversight | Dangerous hardware and inspected-code actions remain denied even for administrators |

## Clinical-safety boundary

The functions described in the clinical-safety architecture remain prototype interfaces over caller-supplied data. They do not diagnose, prescribe, control devices, or authorize clinical care. Clinical use requires institutional governance, privacy/security controls, external device validation, usability testing and jurisdiction-specific regulatory assessment.
