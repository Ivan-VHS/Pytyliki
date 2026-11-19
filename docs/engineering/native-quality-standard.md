# Native Quality and Automation Standard

## Purpose

This repository evolves as a practical, inspectable processor workstation. Improvements must remain useful, measurable, reversible, and compatible with the supported C11, Windows, and Linux targets.

## Engineering principles

1. **Natural use:** commands, labels, errors, and workflows use direct language; defaults are unsurprising and every destructive or persistent action is explicit.
2. **Native first:** keep the core portable C11. Use native Win32 controls and platform APIs where they improve integration; isolate platform-specific code behind narrow adapters.
3. **Composable capabilities:** extend shared primitives and public interfaces instead of cloning near-identical functions. A new capability must have a distinct contract, bounded inputs, documented failure modes, and a measurable purpose.
4. **Read-only hardware boundary:** discovery and diagnostics may inspect hardware. No BIOS, firmware, microcode, voltage, clock, or persistent hardware changes are performed automatically.
5. **White / red / blue design system:** use white or neutral surfaces, restrained red for warnings/destructive actions, and blue for primary actions, focus, and informational states. Maintain contrast, keyboard focus, DPI awareness, and system accessibility settings.
6. **Small, reviewable changes:** each change solves one coherent problem and includes migration notes where needed.

## Verification contract

- Every public function has focused tests for normal, boundary, invalid, and failure inputs where applicable.
- C code is built with strict warnings; CTest runs on Windows and Linux.
- Sanitizer builds run on supported Linux CI runners.
- Parsers, file readers, DSP boundaries, and externally supplied data receive fuzz/property testing where practical.
- Benchmarks are bounded, reproducible, and report methodology; performance claims require before/after measurements.
- Tests must not depend on network access, real hardware mutation, or nondeterministic timing.

## Automation and release safety

- CI may build, test, lint, scan, and publish artifacts.
- Automation may create or update isolated feature branches and open pull requests.
- CI must never force-push, bypass branch protection, or merge to `main` without required checks and human review.
- Startup integration is opt-in, visible, reversible, and never silently installs persistence.
- Secrets use least privilege; workflow permissions are read-only unless a job has a documented need.

## Native desktop quality

- Keep the command-line interface and core library usable without the GUI.
- GUI actions call the same tested core APIs as the CLI.
- Long-running work is cancellable and reports progress without blocking the UI thread.
- User settings are validated, versioned, and stored in an OS-appropriate per-user location.
- Installer, autostart, and update behavior must be explicit and reversible.

## Improvement loop

For each proposal: inspect current behavior → define contract → implement the smallest coherent change → add tests → run CI and sanitizers → review security and UX → document limitations → submit a pull request. Apply shared improvements across related functions only when their semantics genuinely match; do not multiply duplicate APIs.

## Acceptance criteria

A change is ready when its behavior is documented, tests cover its contract, supported platforms build, diagnostics are actionable, and rollback is straightforward.