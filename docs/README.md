> **Navigation:** Start with the [information architecture map](INFORMATION_ARCHITECTURE.md), then open the relevant [topic hub](clusters/architecture.md). The map catalogs every Markdown page and keeps the repository's topic clusters discoverable.

## Topic hubs

- [Architecture and system contracts](clusters/architecture.md) — System structure, data flow, processor models and bounded execution contracts.
- [Processor, hardware and computational labs](clusters/processor-labs.md) — CPU capabilities, hardware reference models, numerical engines and bounded lab workflows.
- [Quality, safety and evidence](clusters/quality-evidence.md) — Validation gates, repeatability, traceability, reporting and safety boundaries.
- [UI, motion and visual documentation](clusters/ui-visual.md) — Desktop interaction, accessibility, motion behavior and visual asset standards.
- [Integrations and project federation](clusters/federation.md) — External interfaces, plugins, shared contracts and cross-project integration boundaries.
- [Governance, planning and reference material](clusters/governance-planning.md) — Project stewardship, decision standards, roadmaps and supporting references.

---

# Documentation

## Normative documentation standard

[Unified documentation and semantics standard](UNIFIED_DOCUMENTATION_STANDARD.md) defines the shared terminology, capability maturity, runtime states, verification outcomes, evidence rules, localization requirements and documentation acceptance criteria. It is normative for this repository; project-specific architecture and implementation evidence remain authoritative for the product itself.

| File | Scope |
|---|---|
| [FEATURES.md](FEATURES.md) | module inventory and capability boundaries |
| [BENCHMARK_STATISTICS.md](BENCHMARK_STATISTICS.md) | bounded statistical analysis of timing samples |
| [PROGRAM_CATALOG_1000.md](PROGRAM_CATALOG_1000.md) | adoption map and project ownership for the 1,000-item catalogue |
| [ENGINEERING.md](ENGINEERING.md) | quality and safety gates |
| [ARCHITECTURE.md](ARCHITECTURE.md) | runtime and data flow |
| [DIVISION_ENGINE.md](DIVISION_ENGINE.md) | checked division, balanced partitioning and weighted allocation |
| [COMPUTE_KERNEL.md](COMPUTE_KERNEL.md) | memory and metrics core |
| [ORTHOGONAL_METRIC_ENGINE.md](ORTHOGONAL_METRIC_ENGINE.md) | numerical models |
| [CPU_SUITE_PRO_78.md](CPU_SUITE_PRO_78.md) | CPU module registry |
| [BLOCKCHAIN_CPU_SYSTEMS_LAB.md](BLOCKCHAIN_CPU_SYSTEMS_LAB.md) | 33 systems-lab functions |
| [APSIA_2.0.md](APSIA_2.0.md) | advisory workload routing and promotion |
| [QUALITY_COHESION_CADENCE.md](QUALITY_COHESION_CADENCE.md) | measurement cadence |
| [TRACE_MEASUREMENT_PROVENANCE.md](TRACE_MEASUREMENT_PROVENANCE.md) | measurement context and evidence provenance |
| [MEMORY_ARENA.md](MEMORY_ARENA.md) | bounded allocation |
| [PROCESSOR_TAXONOMY.md](PROCESSOR_TAXONOMY.md) | processor profiles |
| [EMBEDDED_C.md](EMBEDDED_C.md) | MCU reference tools |
| [HARDWARE_RULE_ENGINE.md](HARDWARE_RULE_ENGINE.md) | advisory rules |
| [QCS_NATIVE.md](QCS_NATIVE.md) | syntax-only frontend |
| [MORA_AMBER_DESIGN_SYSTEM.md](MORA_AMBER_DESIGN_SYSTEM.md) | native Win32 Amber palette, persistence and contrast contract |
| [UI_UX_SYSTEM.md](UI_UX_SYSTEM.md) | desktop interaction |
| [QUALITY_REPEATABILITY_NATIVE.md](QUALITY_REPEATABILITY_NATIVE.md) | reproducibility gates |

The root README includes the live Windows desktop capture and current build/test commands. Legacy contract paths remain concise for validators. Machine-readable JSON, source and tests are authoritative for implemented behavior.

| [UI_MOTION.md](UI_MOTION.md) | native Windows motion, accessibility preferences and reduced-motion behavior |
| [VISUAL_DOCUMENTATION.md](VISUAL_DOCUMENTATION.md) | README graphics, SVG/raster formats, typography, Markdown/MDX, diagrams and rendering rules |


## Motion and embedded graphics

[Motion Constitution v2026](MOTION_CONSTITUTION_V2026.md) is the shared source for timing/easing tokens, adaptive motion, reduced-motion precedence, themes, the 2×2/focus-scene lab layout, SVG/Canvas telemetry rules and the four-domain capability matrix. [UI_MOTION.md](UI_MOTION.md) records the native behavior implemented in this repository; planned showcase features are not reported as shipped.


## Bounded 500 UAH allocation reference

- [Multilingual specification](RESOURCE_ALLOCATION_500_UAH.md) — bounded budget model, scenarios and operating rules.
- [Offline interactive simulator](resource-allocation-500-uah.html) — adjust category amounts and see the remainder or overrun.
- [Example JSON](../examples/resource-allocation-500-uah.json) and [JSON Schema](../schemas/resource-allocation.v1.schema.json).

This is an illustrative personal-budgeting reference, not a claim that the host project manages payments or live retail prices.


## Project-specific engineering profile

[IVAN_VHS_ENGINEERING_PROFILE.md](IVAN_VHS_ENGINEERING_PROFILE.md) translates the project's direction into concrete architecture boundaries, implementation priorities, human-readable code conventions and acceptance tests. It is a development contract; it does not mark planned work as implemented.
