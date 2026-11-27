# Repository Security Assurance v2

## Coverage

The federated assurance workflow checks the project identity and local security controls of **Pytyliki, Reaper, OPlague and VentuSample**, then applies one centrally maintained scanner and dependency inventory to all four checked-out source trees. Each product keeps its own project contract and implementation-specific security boundaries. The shared Beelzebul rule catalogue and `SECURITY.md` are checked for parity. Static scanning does not execute inspected source. Findings contain rule ID, severity, repository-relative path and line number only; secret values and source snippets are never included.

### Automated controls

- **Source audit:** high-confidence private-key and token patterns, hard-coded credential assignments, developer-machine paths, sensitive-value logging and selected risky native/Python APIs.
- **CodeQL:** C/C++ and GitHub Actions source analysis using the extended query suite.
- **Dependency vulnerabilities:** OSV-Scanner compares dependency advisories on Pull Requests and performs a scheduled full scan; GitHub Dependency Review gates pull-request dependency changes, while Dependabot updates for GitHub Actions and an offline inventory of CMake/Python/package manifests, lockfiles and exact version pins.
- **Fuzz smoke:** the production pl_audio_load_wav routine receives a deterministic corpus of malformed and RIFF-shaped byte streams; each result buffer is released after parsing.
- **Privacy regression:** synthetic-token detection, placeholder handling, no-secret-output assertions and policy prohibitions are tested.
- **Federation:** validate each repository's `project-contract.json`, `SECURITY.md` and CodeQL workflow; confirm the shared Beelzebul rule catalogue and security policy are identical; then run the central source and dependency audits against each repository's tracked files. Peer repositories do not need to copy Pytyliki's scanner scripts or CI workflow.

## Run locally

1. Run scripts/security_audit.py with --output security-audit-report.json.
2. Run scripts/dependency_audit.py with --output dependency-audit-report.json.
3. Run scripts/test_privacy_regressions.py.
4. Configure and build: cmake -S . -B build -DBUILD_TESTING=ON; cmake --build build --parallel.
5. Run scripts/fuzz_smoke.py.

The fuzz wrapper runs the CTest target wav-container-boundary-tests. The full CTest suite and existing ASan/UBSan CI job remain separate regression controls.

## Shared score

Every repository uses the same 100-point weights:

| Category | Weight |
|---|---:|
| Secret hygiene | 20 |
| Privacy leak prevention | 15 |
| Source-risk review | 15 |
| Fuzz robustness | 15 |
| Dependency pin hygiene | 5 |
| Known dependency vulnerabilities (OSV) | 10 |
| Privacy regressions | 10 |
| Federation consistency | 10 |

The federated report also calculates a cross-repository score from project-identity and security-contract coverage, shared policy parity, aggregate source findings and dependency pin hygiene. Scores are engineering trend indicators, not certifications. A critical credential finding, an OSV scan failure or known vulnerable dependency, failed fuzz/privacy/federation gate or score below 70 fails the assurance gate.

## Boundaries

The source scanner is rule-based and can produce false positives or miss novel patterns. CodeQL adds language-aware analysis. Dependency inventory and pin checks are offline; GitHub Dependency Review checks supported dependency changes but does not establish that every external system package is vulnerability-free. The fuzz corpus is deterministic smoke fuzzing, not continuous coverage-guided fuzzing. Review alerts, run deeper fuzz campaigns, and retain sanitizer testing before release.
