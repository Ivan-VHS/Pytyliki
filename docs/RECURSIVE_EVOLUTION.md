# Pytyliki Recursive Evolution Engine (PREE)

## Purpose

PREE improves the repository's improvement process. It is a governed, evidence-driven loop, not an unrestricted agent that edits and merges its own code.

## Recursive improvement cycle

1. **Observe** — inventory source, build configuration, tests, documentation, supported platforms and current CI evidence.
2. **Diagnose** — identify a measurable gap and link it to a processor-engineering subsystem.
3. **Propose** — create a small, isolated change with a stated hypothesis, expected benefit, risks and rollback path.
4. **Implement** — work on a dedicated branch; never write directly to `main`.
5. **Verify** — run formatting/static checks where configured, CMake configure/build, CTest, platform CI and targeted regression tests.
6. **Evaluate** — compare before/after correctness, performance and resource use. Reject noisy or non-reproducible performance claims.
7. **Review** — publish a separate PR with evidence, limitations and a human-readable change summary.
8. **Integrate** — merge only after required checks pass, review is complete and repository branch protection permits it.
9. **Learn** — record failures, flaky tests, review feedback and accepted/rejected hypotheses as inputs to the next cycle.
10. **Improve the improver** — change PREE rules only through their own separately tested PR, using historical outcomes to justify the change.

## Improvement-of-improvement metrics

Track these metrics per repository and per release:

- build and test pass rate;
- escaped regression count;
- flaky-test rate;
- median PR lead time;
- review rework rate;
- reproducibility of benchmark results;
- performance change with variance and hardware/software environment recorded;
- rollback frequency;
- proportion of changes with a measurable acceptance criterion.

Do not optimize a single metric at the expense of correctness, security, portability or maintainability.

## Autonomous proposal policy

Automation may read repository metadata, run builds/tests, calculate metrics, create a branch, and open a draft PR when the workflow has explicit write permission. It must not:

- push to `main`, bypass branch protection, approve its own PR, or merge its own changes;
- weaken, delete, skip or rewrite failing tests merely to obtain a green result;
- fabricate benchmark evidence or infer physical CPU properties from source-code heuristics;
- execute untrusted source files or downloaded binaries;
- change firmware, microcode, BIOS/UEFI, voltage, clocks, kernel drivers or privileged system settings;
- broaden permissions, secrets, workflow token scopes or protected-path ownership as part of a routine improvement.

A human maintainer authorizes merge. If required checks are absent, pending, skipped or inconclusive, the PR remains open.

## Repository rollout

Apply PREE one repository at a time:

1. inventory default branch, language, build system, tests, CI, branch protections and ownership;
2. classify applicable PREE modules; do not force processor-specific code into unrelated projects;
3. create a dedicated branch and one scoped PR;
4. run the repository's native checks plus relevant cross-platform checks;
5. review the diff and CI evidence;
6. merge only after explicit acceptance and required checks;
7. verify the resulting default branch before starting the next repository.

## Pytyliki-specific scope

For Pytyliki, proposals must support processor architecture inventory, ISA-safe dispatch, bounded CPU workloads, cache/memory behavior, processor-source review, DSP datapaths, reproducible measurements or desktop engineering UX. Generic automation is allowed only as infrastructure for those processor-engineering tasks.

## Definition of done

A PREE change is complete only when its hypothesis, implementation, tests, CI evidence, limitations and rollback strategy are recorded in the PR. A passing workflow is necessary but not sufficient evidence of hardware correctness.
