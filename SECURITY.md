# Security policy

## Scope

Pytyliki currently performs local CPU identification, a bounded benchmark, read-only heuristic source review, local permission checks, and workflow metadata display.

The application does not execute inspected source, apply patches, change CPU controls, write firmware, or access privileged hardware interfaces.

## Reporting a vulnerability

Please report suspected vulnerabilities privately to the repository maintainer rather than publishing exploit details in an issue. Include:
- affected version and operating system;
- exact command or UI path;
- a minimal reproducible example;
- observed behavior and expected behavior;
- whether user data or local files may be affected.

Do not include secrets, personal data, or live credentials in reports.

## Security design

- Deny unknown actions by default.
- Keep inspected source read-only.
- Keep benchmarks bounded.
- Keep CI permissions to the minimum required for checkout and build.
- Require review and regression tests before source transformation is introduced.

The heuristic scanner is not a complete security auditor. Its findings must be reviewed by a qualified engineer.
