# External Repository Integration Register

Status date: 2026-11-16  
Target: `Ivan-VHS/Pytyliki`  
Integration branch: `feat/federated-integration-foundation`  
Pre-change snapshot: `backup/pre-integration-2026-11-16`

## Safety and scope

The default branch is not modified by this integration foundation. All work is isolated in the feature branch. The backup branch was created from `main` before the first feature-branch change.

The source repositories listed below are an intake register reconstructed from the project discussion. Their existence, canonical owner, access rights, default branches, licenses, and current contents have **not** been verified. Direct repository metadata lookups under `bboroski/<name>` returned HTTP 404 for every candidate, and the connected GitHub account returned no repositories for owner `bboroski`. A 404 can mean a missing or private repository; it is not proof that the project does not exist.

## Source intake register

| Candidate | Expected role (provisional) | Verification |
|---|---|---|
| SAMBU | Language/runtime and execution substrate | Blocked: source URL/access unverified |
| NUN | OS/platform and service layer | Blocked: source URL/access unverified |
| RippleCripple | Automation / recursive orchestration | Blocked: source URL/access unverified |
| ReaperCripple | REAPER-oriented tooling / automation | Blocked: source URL/access unverified |
| ToolNG | Tooling and developer utilities | Blocked: source URL/access unverified |
| Kloz | Unknown; inspect before classification | Blocked: source URL/access unverified |
| CO | Unknown; inspect before classification | Blocked: source URL/access unverified |
| SaintSI | Unknown; inspect before classification | Blocked: source URL/access unverified |
| Klipot | Unknown; inspect before classification | Blocked: source URL/access unverified |
| NUNLauncher | Launcher / bootstrap | Blocked: source URL/access unverified |
| NUN Desktop | Desktop interface | Blocked: source URL/access unverified |
| SAMBU SDK | SDK / integration interfaces | Blocked: source URL/access unverified |
| Fragile-Baby | Unknown; inspect before classification | Blocked: source URL/access unverified |
| Luci | Unknown; inspect before classification | Blocked: source URL/access unverified |
| Hellduck | Unknown; inspect before classification | Blocked: source URL/access unverified |
| 9circ | Automation / orchestration candidate | Blocked: source URL/access unverified |

## Required source verification

For each source, record:

1. Canonical GitHub URL, owner, repository ID, visibility, default branch, and latest commit.
2. License and notices; do not vendor code until redistribution compatibility is checked.
3. Languages, build systems, dependencies, generated assets, submodules, and external services.
4. Tests, CI workflows, release state, open PRs, and known security advisories.
5. Public APIs, CLI/GUI entry points, data formats, and runtime assumptions.
6. Overlap with existing Pytyliki modules and the proposed destination module.
7. Import decision: adapt, wrap, isolate as a plugin, preserve as a submodule, or reject with rationale.

## Integration policy

- Preserve upstream provenance and license notices for every imported component.
- Prefer small adapters and stable interfaces over copying whole repositories into the core.
- Keep language runtimes, OS services, desktop UI, automation, and developer tools in separate bounded modules.
- Do not execute imported code during discovery. Treat build scripts, workflows, package hooks, and generated binaries as untrusted until reviewed.
- Pin source revisions and dependency versions; record checksums for downloaded release artifacts.
- Require tests for each adapter and regression tests for shared interfaces.
- Do not claim platform support or performance guarantees without dedicated validation.
- Never merge to `main` until CI is green, review requirements are satisfied, and the PR head matches the validated commit.

## Proposed target layout

```text
integrations/
  manifests/       # source identity, pinned revision, license, checksums
  adapters/        # Pytyliki-facing interfaces
  plugins/         # isolated optional integrations
  tests/           # contract and regression tests
docs/
  integrations/    # per-source audit and migration records
```

This is a proposed layout only; source-specific modules should be added after repository inspection, not pre-created as empty placeholders.

## Current outcome

The safety snapshot and isolated integration branch exist. The target repository's existing C11 build/test CI, recursive-evolution policy workflow, and verified auto-merge workflow were inspected. No upstream source code has been copied because none of the candidate source repositories could be fetched or verified through the connected GitHub access.

## Next unblocker

Provide the exact GitHub URLs for the source repositories (or grant the connected GitHub integration access to them). Once accessible, process them one at a time: snapshot metadata, audit license and dependencies, create a source manifest, import into an isolated module, run tests, and open a reviewed PR.
