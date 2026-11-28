# Pytyliki documentation

> **Documentation review:** 2026-11-27 UTC · **CMake project:** 0.8.0 · **Core:** C11

[English README](../README.md) · [Українська](../README.uk.md) · [Русский](../README.ru.md) · [简体中文](../README.zh-CN.md) · [Source](https://github.com/Ivan-VHS/Pytyliki) · [Issues](https://github.com/Ivan-VHS/Pytyliki/issues)

Start with a language-specific project README for scope, supported platforms, build commands and maturity boundaries. The C11 core and CLI are built for Windows/Linux; the Win32 GUI is Windows-only.

## Key technical references

- [Feature matrix](FEATURES.md) — implemented behavior and explicit limits.
- [Architecture](ARCHITECTURE.md) — component ownership and data flow.
- [Engineering gates](ENGINEERING.md) — build, quality and safety checks.
- [Benchmark statistics](BENCHMARK_STATISTICS.md) — measurement method and provenance.
- [Memory arena](MEMORY_ARENA.md) — bounded allocation primitives.
- [Embedded C](EMBEDDED_C.md) — reference MCU profiles and simulated hardware descriptions.
- [Hardware syntax](PYTYLIKI_HARDWARE_SYNTAX.md) — twenty structural construct families.
- [MOS switch-level model](PHYSICAL_SWITCH_MODEL_V1.md) — bounded N/P topology resolver.
- [Processor Fabric](PROCESSOR_FABRIC_V1.md) — processor graph and advisory planning.
- [SystemArray sigil operators](SYSTEM_SIGIL_OPERATORS.md) — metadata-only typed operator registry.
- [Iron Core](IRON_CORE.md) — structural grammar and hardware evidence gates.
- [100 low-level rules](PYTYLIKI_LOW_LEVEL_100_RULES.md) — implementation mapping, target contracts, maturity and verification boundaries.
- [QCS native inspection](QCS_NATIVE.md) — static syntax checks.
- [System-array syntax](SYSTEM_ARRAY_SYNTAX.md) — normalized `system(array)` aliases and framed syntax; syntax-only, no hardware access.
- [Report envelope](REPORT_ENVELOPE.md) — versioned output and evidence metadata.
- [Reaper integration](REAPER_INTEGRATION.md)
- [IRON CORE v1](IRON_CORE.md) — 100 hardware-engineering rules and evidence gates.
- [Omega-Null](OMEGA_NULL.md) — experimental physics-inspired profile and physical capability boundaries. — opt-in policy and C ABI boundaries.
- [Reaper neural capacity evidence v1](REAPER_NEURAL_CAPACITY_EVIDENCE_V1.md) — bounded read-only resource evidence and deterministic ALLOW/DEFER/REJECT recommendations.

## Browse by topic

- [Architecture](clusters/architecture.md) · [Processor labs](clusters/processor-labs.md)
- [Quality evidence](clusters/quality-evidence.md) · [UI and visual system](clusters/ui-visual.md)
- [Federation](clusters/federation.md) · [Governance and planning](clusters/governance-planning.md)

## Shared engineering rules

- [Unified documentation standard](UNIFIED_DOCUMENTATION_STANDARD.md)
- [Engineering evolution framework](ENGINEERING_EVOLUTION_FRAMEWORK.md)
- [Repository evolution directives](REPOSITORY_EVOLUTION_DIRECTIVES.md)

## Evidence and maturity

Describe behavior from source, build configuration, schemas, tests and CI evidence. Hardware discovery is read-only; benchmark numbers are local measurements. The embedded workbench is reference-only and does not imply physical MMIO mapping or firmware flashing. The native C ABI is opt-in and requires an explicitly trusted library.

**The index is navigation, not proof of implementation.** Mark partial or planned capabilities accordingly.

- [Processor domain and physical models](PYTYLIKI_PROCESSOR_DOMAIN.md) — processor lifecycle, DMA ownership, pipelines and unit-aware physical models.
