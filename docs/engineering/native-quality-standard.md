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
- CI must never force-push, bypass branch protection, approve its own PR, or merge to `main` without required checks and independent human review.
- Startup integration is opt-in, visible, reversible, and never silently installs persistence.
- Secrets use least privilege; workflow permissions are read-only unless a job has a documented need.

## Native desktop quality

- Keep the command-line interface and core library usable without the GUI.
- GUI actions call the same tested core APIs as the CLI.
- Long-running work is cancellable and reports progress without blocking the UI thread.
- User settings are validated, versioned, and stored in an OS-appropriate per-user location.
- Installer, autostart, and update behavior must be explicit and reversible.

## HMGA v3: deterministic, bounded evolution

The iteration hierarchy is an orchestration model, not a request to execute astronomical or unbounded loops. Every automated pass has explicit limits for runtime, memory, generated diff size, test workload, and retry count.

1. **Inventory:** capture the default-branch SHA, open PR heads and bases, active branches, CI evidence, public capability IDs, and supported platform matrix before proposing changes.
2. **Dependency graph:** represent stacked PRs and module dependencies as a directed acyclic graph. Promote prerequisites before dependent work; never infer that similarly named branches are interchangeable.
3. **Capability registry:** give each public capability a stable ID, contract, owner/module, dependencies, compatibility scope, and acceptance tests. Deduplicate equivalent functionality before adding a new API.
4. **Cross-application matrix:** test every declared compatible interaction edge between capabilities. Keep the matrix sparse and contract-driven: incompatible pairs receive an explicit not-applicable reason rather than artificial wrappers or cloned functions.
5. **Bounded retries:** retry only identified transient infrastructure failures, with a small configured attempt budget. A deterministic test or build failure requires a code fix; repeating it is not a remedy.
6. **Evidence record:** attach exact source/base SHAs, commands, platform results, test outcomes, risks, and rollback instructions to each change.

## Deterministic promotion to main

- Work on an isolated branch and submit one coherent PR per dependency layer.
- Run validation against the proposed merge result and record the exact PR head and base SHAs.
- Before promotion, verify that the target is `main`, neither SHA changed during validation, all required checks passed on the current candidate, and at least one approval exists from someone other than the PR author.
- If `main` advances, update the feature branch through the repository's normal mechanism and rerun the complete required matrix.
- GitHub native auto-merge may be enabled only after the evidence and independent approval gates pass. Branch protection and required checks remain authoritative.
- Never force-push, skip checks, fabricate approvals, approve a PR from its own automation, or write directly to `main`. A failed or inconclusive gate leaves the PR open.

## Improvement loop

For each proposal: inspect current behavior → define contract → implement the smallest coherent change → add tests → run CI and sanitizers → review security and UX → document limitations → submit a pull request. Apply shared improvements across related functions only when their semantics genuinely match; do not multiply duplicate APIs.

## Acceptance criteria

A change is ready when its behavior is documented, tests cover its contract, supported platforms build, diagnostics are actionable, exact-SHA evidence is attached, independent review is complete, and rollback is straightforward.
