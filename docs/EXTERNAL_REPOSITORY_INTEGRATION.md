# External Integration Policy

**Status:** normative  
**Applies to:** third-party source, binary, service and repository integrations.

## Admission criteria

An integration MUST be reviewed before its code, binaries or services are adopted. The review records:

1. Canonical upstream identity and the exact revision under consideration.
2. License, notices and redistribution obligations.
3. Supported platforms, build system, dependency graph and runtime requirements.
4. Public interfaces, data formats, lifecycle behavior and failure modes.
5. Security posture, permissions, network behavior and handling of untrusted input.
6. Test coverage, maintenance state and compatibility risks.
7. The receiving module, adapter boundary and rollback procedure.

An unavailable repository or unverified artifact MUST be classified as **unverified**. Its identity, license, contents and compatibility MUST NOT be inferred from a name or description.

## Implementation rules

- Prefer narrow adapters and versioned interfaces over copying whole projects into the core.
- Pin revisions and dependency versions; retain required license notices.
- Treat build scripts, workflows, package hooks and generated binaries as untrusted until reviewed.
- Do not execute imported code during discovery or metadata inspection.
- Keep language runtimes, operating-system services, desktop UI, automation and developer tools in separate bounded modules.
- Require contract and regression tests for every adopted adapter.
- Do not claim compatibility, security or performance without dedicated validation.

## Release gate

An integration is eligible for release only after review, tests, documentation and CI pass against the exact revision being released. A proposal or intake record does not constitute approval or adoption.
