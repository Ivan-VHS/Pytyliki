# Documentation information architecture

[Project README](../README.md) · [Documentation index](README.md)

This page is the canonical navigation map for Pytyliki. The documentation is organized as a topic cluster graph: the project README is the entry point, this document is the pillar map, and each cluster hub routes readers to its implementation contracts and supporting references.

## Navigation model

- **Entry point:** the root README describes the product scope and links to this documentation index.
- **Pillar:** this map explains the topic structure and links to every documentation page in the repository.
- **Topic hubs:** the cluster pages below group related material and provide contextual paths between documents.
- **Source of truth:** code, tests, schemas and the specific contract document remain authoritative for implementation and status claims.

## Topic hubs

- [Architecture and system contracts](clusters/architecture.md) — System structure, data flow, processor models and bounded execution contracts.
- [Processor, hardware and computational labs](clusters/processor-labs.md) — CPU capabilities, hardware reference models, numerical engines and bounded lab workflows.
- [Quality, safety and evidence](clusters/quality-evidence.md) — Validation gates, repeatability, traceability, reporting and safety boundaries.
- [UI, motion and visual documentation](clusters/ui-visual.md) — Desktop interaction, accessibility, motion behavior and visual asset standards.
- [Integrations and project federation](clusters/federation.md) — External interfaces, plugins, shared contracts and cross-project integration boundaries.
- [Governance, planning and reference material](clusters/governance-planning.md) — Project stewardship, decision standards, roadmaps and supporting references.

## Cross-repository boundaries

These projects share selected engineering conventions, not a single runtime or interchangeable implementation. Follow the project-specific contract before treating an interface as compatible.

- [Reaper documentation map](https://github.com/Ivan-VHS/Reaper/blob/main/docs/INFORMATION_ARCHITECTURE.md) — language, compiler and runtime contracts.
- [OPlague documentation map](https://github.com/Ivan-VHS/OPlague/blob/main/docs/INFORMATION_ARCHITECTURE.md) — device and driver inspection contracts.
- [VentuSample documentation map](https://github.com/Ivan-VHS/VentuSample/blob/main/docs/INFORMATION_ARCHITECTURE.md) — audio and DSP contracts.
- [DAK documentation map](https://github.com/Ivan-VHS/DAK/blob/main/docs/INFORMATION_ARCHITECTURE.md) — repository entry-point map.

## Full document catalog

The catalog gives each Markdown document a direct inbound path in addition to its topic hub. The surrounding hub supplies the semantic context; the link label names the actual target.

### Architecture and system contracts

- [DAK MOTION CONSTITUTION](DAK_MOTION_CONSTITUTION.md)
- [ARCHITECTURE 66](ARCHITECTURE_66.md)
- [ARCHITECTURE](ARCHITECTURE.md)
- [Clinical Safety Architecture](clinical-safety-architecture.md)
- [COMPUTE KERNEL](COMPUTE_KERNEL.md)
- [ENGINEERING CONSTITUTION PROFILE](ENGINEERING_CONSTITUTION_PROFILE.md)
- [DISTRIBUTED SYSTEMS ENGINEERING STANDARD](DISTRIBUTED_SYSTEMS_ENGINEERING_STANDARD.md)
- [DIVISION ENGINE](DIVISION_ENGINE.md)
- [MEMORY ARENA](MEMORY_ARENA.md)
- [MOTION CONSTITUTION V2026](MOTION_CONSTITUTION_V2026.md)
- [PROCESSOR TAXONOMY](PROCESSOR_TAXONOMY.md)
- [SYSTEMS LANGUAGE ARCHITECTURE ATLAS](SYSTEMS_LANGUAGE_ARCHITECTURE_ATLAS.md)
- [TRANSACTIONAL RUNTIME](TRANSACTIONAL_RUNTIME.md)
- [UNIFIED PROJECT CONTRACT](UNIFIED_PROJECT_CONTRACT.md)

### Processor, hardware and computational labs

- [APSIA 2.0](APSIA_2.0.md)
- [BENCHMARK STATISTICS](BENCHMARK_STATISTICS.md)
- [BLOCKCHAIN CPU SYSTEMS LAB](BLOCKCHAIN_CPU_SYSTEMS_LAB.md)
- [CPU SUITE PRO 78](CPU_SUITE_PRO_78.md)
- [EMBEDDED C](EMBEDDED_C.md)
- [HARDWARE RULE ENGINE](HARDWARE_RULE_ENGINE.md)
- [MICROBUDGET PLANNER](MICROBUDGET_PLANNER.md)
- [ORTHOGONAL METRIC ENGINE](ORTHOGONAL_METRIC_ENGINE.md)
- [PROGRAM CATALOG 1000](PROGRAM_CATALOG_1000.md)
- [QCS NATIVE](QCS_NATIVE.md)
- [RESOURCE ALLOCATION 500 UAH](RESOURCE_ALLOCATION_500_UAH.md)
- [TRACE MEASUREMENT PROVENANCE](TRACE_MEASUREMENT_PROVENANCE.md)

### Quality, safety and evidence

- [CHINESE PROVERBS 100 ENGINEERING CROSSWALK](CHINESE_PROVERBS_100_ENGINEERING_CROSSWALK.md)
- [DOCUMENTATION CONTROL](DOCUMENTATION_CONTROL.md)
- [ENGINEERING BASELINE](ENGINEERING_BASELINE.md)
- [ENGINEERING EVOLUTION FRAMEWORK](ENGINEERING_EVOLUTION_FRAMEWORK.md)
- [ENGINEERING](ENGINEERING.md)
- [IMPROVEMENT CONFLICT STANDARD](IMPROVEMENT_CONFLICT_STANDARD.md)
- [IVAN VHS ENGINEERING PROFILE](IVAN_VHS_ENGINEERING_PROFILE.md)
- [PLATFORM VALIDATION POLICY](PLATFORM_VALIDATION_POLICY.md)
- [QUALITY AUDIT 0.7.1](QUALITY_AUDIT_0.7.1.md)
- [QUALITY COHESION CADENCE](QUALITY_COHESION_CADENCE.md)
- [QUALITY COMMERCIAL STANDARD](QUALITY_COMMERCIAL_STANDARD.md)
- [QUALITY REMEDIATION STANDARD](QUALITY_REMEDIATION_STANDARD.md)
- [QUALITY REPEATABILITY NATIVE](QUALITY_REPEATABILITY_NATIVE.md)
- [ROADMAP GOVERNANCE AND CI SCALE](ROADMAP_GOVERNANCE_AND_CI_SCALE.md)
- [REPORT ENVELOPE](REPORT_ENVELOPE.md)

### UI, motion and visual documentation

- [UI MOTION](UI_MOTION.md)
- [UI UX 2026 CONVERGENCE](UI_UX_2026_CONVERGENCE.md)
- [UI UX CODEX 40026](UI_UX_CODEX_40026.md)
- [UI UX SYSTEM](UI_UX_SYSTEM.md)
- [VISUAL DOCUMENTATION](VISUAL_DOCUMENTATION.md)

### Integrations and project federation

- [AUTOMATION ROADMAP](AUTOMATION_ROADMAP.md)
- [EXTERNAL REPOSITORY INTEGRATION](EXTERNAL_REPOSITORY_INTEGRATION.md)
- [FEDERATED INTEGRATION](FEDERATED_INTEGRATION.md)
- [OCTA PYRAMID FEDERATION](OCTA_PYRAMID_FEDERATION.md)
- [PLUGIN FEDERATION](PLUGIN_FEDERATION.md)
- [RECURSIVE EVOLUTION](RECURSIVE_EVOLUTION.md)
- [REPLACEMENT TOOLS](REPLACEMENT_TOOLS.md)
- [SELF PRODUCT COMPOSITION](SELF_PRODUCT_COMPOSITION.md)

### Governance, planning and reference material

- [CONTRIBUTING](../CONTRIBUTING.md)
- [ATOMIC QUANTUM 133](ATOMIC_QUANTUM_133.md)
- [DUCKSTORY](DAKSTORY.md)
- [FEATURES](FEATURES.md)
- [HUMAN FIRST GOVERNANCE](HUMAN_FIRST_GOVERNANCE.md)
- [MARKET DEVELOPMENT STANDARD](MARKET_DEVELOPMENT_STANDARD.md)
- [UNIFIED DOCUMENTATION STANDARD](UNIFIED_DOCUMENTATION_STANDARD.md)
- [README](../README.md)
- [README.Ru](../README.ru.md)
- [README.Uk](../README.uk.md)
- [README.Zh CN](../README.zh-CN.md)
- [SECURITY](../SECURITY.md)

## Maintenance contract

A new document is not complete until its owner has updated its topic hub and this catalog, added at least two meaningful incoming paths, and linked it from its parent topic where relevant. Keep important pages within three to four navigation steps from the root README. Prefer direct relative links inside this repository and canonical HTTPS links across repositories. Do not use vague anchors, duplicate competing targets for the same concept, or add links that are not useful in context.
