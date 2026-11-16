# Engineering roadmap

The roadmap separates implemented foundations from future work. A feature is not considered complete because a directory, label, or interface has been created.

## Milestone 0.5.0 — foundations

- [x] Native C11 core, portable CLI, and Win32 GUI.
- [x] Bounded local CPU benchmark.
- [x] CPU vendor, brand string where available, and extended family/model decoding.
- [x] Read-only heuristic source scan.
- [x] JSON diagnostic output.
- [x] Local deny-by-default role/action policy.
- [x] C11/C23 deterministic workflow reference plan.
- [x] Infection-control, emergency-response, and oncology workflow metadata templates.
- [x] Windows/Linux CI and least-privilege workflow permissions.
- [x] White / blue / red README and desktop presentation.

## Milestone 0.6 — source analysis

- [ ] Tokenizer and parser-backed analysis for a deliberately limited C subset.
- [ ] Structured findings with file, line, rule ID, severity, and rationale.
- [ ] Directory traversal with explicit allowlists and file-size limits.
- [ ] Regression fixtures for false positives and false negatives.
- [ ] Optional JSON schema for machine-readable analysis reports.

## Milestone 0.7 — intermediate representation

- [ ] Typed IR with source locations and explicit ownership rules.
- [ ] Deterministic serialization and stable identifiers.
- [ ] Validation passes for type, control-flow, and data-flow invariants.
- [ ] C11 backend for a narrowly defined source subset.
- [ ] Differential tests against reference implementations.
- [ ] C23 output only after compiler support and compatibility tests.

## Milestone 0.8 — repository integration

- [ ] Read-only repository inventory.
- [ ] GitHub App installation and permission diagnostics.
- [ ] Branch and Pull Request proposal workflow.
- [ ] Secret scanning and dependency review.
- [ ] No direct merge or protected-branch bypass.
- [ ] Audit log with commit, diff, tests, and reviewer identity.

## Clinical workflow foundation

- [ ] Source registry with issuing body, title, publication date, jurisdiction, version, and retrieval date.
- [ ] Clinical expert approval for every rule set.
- [ ] Versioned decision tables with traceability to source paragraphs.
- [ ] Positive, negative, boundary, and regression test scenarios.
- [ ] Clear separation between workflow reminders and clinical recommendations.
- [ ] Privacy, data-protection, regulatory, and safety review before real-world deployment.

## Acceptance gate

A milestone is accepted only when its documented scope is implemented, builds on supported platforms, passes automated tests, has traceable requirements, and does not claim capabilities that are still planned.
