# Documentation control

**Baseline date:** 2026-11-26 · **Scope:** README locales, C11 core, hardware-reference workbench, reports, ABI, benchmark methods and federation.

## Local publication gates

| Gate | Acceptance |
|---|---|
| Product identity | Exact name, technical purpose, metadata and version declaration state. |
| Platform | Windows/Linux CLI/core; Windows-only Win32 desktop. |
| Architecture | CLI/UI → input validation → bounded C11 modules → structured report. |
| Read-only boundary | Hardware discovery and source review do not mutate host/source state. |
| Numeric bounds | Inputs, workloads, memory sizes, parser limits and reports state limits. |
| ABI | C ABI v1 layout and caller-owned buffer behavior are versioned and tested. |
| FFI | Only explicit trusted library paths; native code executes in process. |
| Embedded | MCU memory/MMIO references are simulated; no physical mapping or flashing. |
| Benchmarks | Host/compiler/build/workload/sample/statistics recorded; no universal score. |
| Maturity | Separate implemented, partial, specified, planned and verified capabilities. |
| Release | Version and support statements must come from tags/manifests/artifacts. |
| Localization | EN/RU/UK/ZH-CN sections retain the same technical claims and structure. |
| Navigation | Full catalog, contextual cluster links and valid local paths. |
| Legal | MIT terms and third-party notices preserved. |
| Maintenance | Source, tests, schemas, README and docs updated together on contract changes. |

## README review mapped to the 50-point standard

### Identity, scope and first screen (1–10)
1. Exact project name and one-line technical task.
2. Metadata for status, version, platforms, core language, stack and license.
3. Record material name/API/architecture evolution with evidence.
4. Put an actual architecture visual or screenshot near the top with accurate caption.
5. State purpose, guarantees and exclusions in the first paragraphs.
6. Use only reproducible quantitative claims.
7. Link peer repositories with exact interface boundaries.
8. Add a contents list to long/multi-topic README files.
9. Keep release chronology out of the mission summary.
10. Maintain direct technical language.

### Chronology and release records (11–20)
11. Organize supported milestones by dates and artifacts.
12. Preserve verified periods of freeze, stack change or rewrite.
13. Cite Git tags and revisions for release milestones.
14. Separate stable, experimental and archived artifacts only when present.
15. Document compatible and replaced contracts.
16. State branch maintenance status only when supported.
17. Separate version history from API descriptions.
18. Use version-labelled matrices where useful.
19. Remove stale completed/cancelled plans from current summaries.
20. Link architecture milestones to tags, specs or migration artifacts.

### Architecture and contracts (21–30)
21. Give cross-cutting principles a dedicated architecture section.
22. Explain trade-offs and alternative choices where material.
23. Keep paragraphs short and precise.
24. Balance prose, diagrams, tables and self-contained code.
25. Put detailed protocol and schema content under docs/.
26. Separate normative contract from implementation and experiment.
27. Keep language variants structurally and terminologically aligned.
28. Distinguish components, packages, services, processes and interfaces.
29. Answer a concrete engineering question in each section.
30. Balance deep technical content with a fast onboarding path.

### Breaking changes and limitations (31–40)
31. Disclose known issues, limits and breaking changes.
32. Highlight privilege, compatibility and data-loss risk.
33. Explain why a public contract changed.
34. Separate source limitations from OS/compiler/dependency limits.
35. Compare approaches with measured or architectural properties.
36. List unsupported platforms and workloads.
37. Give verified workarounds and migration instructions.
38. Retire fixed limitations only after tests verify the fix.
39. Keep the README out of issue-tracker detail.
40. Be explicit about memory, concurrency, privilege and effects.

### Sources, verification and maintenance (41–50)
41. Supply reproducible performance methods and host configuration.
42. Navigate separately to API, architecture, examples, RFC and standards.
43. Check all local/external links before release.
44. Give exact versions for referenced standards and algorithms.
45. State audits only with issuer, date, scope and evidence.
46. Link issue, contribution and security disclosure routes.
47. Link actual LICENSE, CONTRIBUTING and code-of-conduct files; state absent files.
48. Preserve attribution and third-party licenses.
49. Update docs in the same change as CLI/API/config/build changes.
50. Test onboarding commands from a clean environment.

## Status vocabulary and evidence

- **Specified:** contract exists; runtime behavior is not claimed.
- **Planned:** accepted future work is not implemented.
- **Partial:** bounded implementation exists with stated omissions.
- **Implemented:** source behavior is connected to an executable workflow.
- **Verified:** acceptance passed in a named environment and revision.

A C ABI declaration does not prove consumer compatibility. A simulated MMIO mask is not physical I/O. A measurement catalogue is not a benchmark result. Static source analysis is not a compiler or execution sandbox.

## Before publication

- [ ] Verify commands using the current CMake/CI configuration.
- [ ] Verify every changed Markdown link and asset path.
- [ ] Align all four README locales.
- [ ] Update the index and cluster hub for every added/moved document.
- [ ] Preserve MIT and third-party license notices.
- [ ] Record actual CI/benchmark evidence; never invent a pass.
