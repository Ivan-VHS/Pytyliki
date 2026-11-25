# Ecosystem Cloud Adapter — Pytyliki

**Adapter:** `pytyliki` · **Shared contract:** `ivan-vhs.ecosystem-cloud.v1` · **Status:** contract-only

Pytyliki remains a local processor/device inspection and profiling tool. Cloud synchronization is not part of startup, enumeration, analysis or report generation.

## Eligible data

Only an aggregate profile, sanitized performance summary or user-selected device-class label may be published. A user must review the preview and explicitly confirm publication.

## Prohibited data

Never export serial numbers, PnP instance IDs, MAC addresses, motherboard identifiers, raw hardware IDs, complete device inventories, local filesystem paths or credentials. Device fingerprints must not be derived from hardware identifiers. Telemetry is disabled unless a future, separately reviewed feature defines a privacy and sampling budget.

## Runtime rules

- Device inspection and profiling remain read-only and local.
- No HTTP request or account prompt occurs during startup or device enumeration.
- Data is minimized before it reaches the adapter boundary; raw evidence stays local.
- Client instance IDs are random UUIDs, never hashes of device or host properties.
- Retries are bounded and idempotent; a cloud outage cannot interrupt C11 analysis.

## Implementation boundary

This PR adds a policy manifest, documentation and focused regression tests only. It introduces no Supabase SDK, HTTP library, driver action, device scan change or background telemetry. A native transport adapter requires a separate ABI/API contract and local-stack integration tests.
