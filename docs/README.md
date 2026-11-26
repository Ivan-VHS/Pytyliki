# Pytyliki documentation

**Documentation baseline:** 2026-11-26 · **Project status:** development / bounded C11 workstation.

Start with the [root README](../README.md) for scope, build commands, first checks and read-only boundaries. The [Information Architecture](INFORMATION_ARCHITECTURE.md) page catalogs the complete library.

## Key technical references

- [Feature matrix](FEATURES.md) — capability scope and limits.
- [Architecture](ARCHITECTURE.md) — C11 core and data flow.
- [Engineering baseline](ENGINEERING_BASELINE.md) — project-level quality gates.
- [Benchmark statistics](BENCHMARK_STATISTICS.md) — measurement methodology and provenance.
- [Memory arena](MEMORY_ARENA.md) — checked views, arenas and pools.
- [Embedded C](EMBEDDED_C.md) — reference MCU workbench and simulated MMIO.
- [QCS native](QCS_NATIVE.md) — syntax/structure validation.
- [Report envelope](REPORT_ENVELOPE.md) — versioned report metadata.
- [Reaper integration](REAPER_INTEGRATION.md) — explicit ABI integration boundary.
- [Unified project contract](UNIFIED_PROJECT_CONTRACT.md) — machine-readable capabilities and interfaces.

## Browse by topic

- [Architecture and system contracts](clusters/architecture.md)
- [Processor and computational labs](clusters/processor-labs.md)
- [Quality and evidence](clusters/quality-evidence.md)
- [UI and visual documentation](clusters/ui-visual.md)
- [Federation and integrations](clusters/federation.md)
- [Governance and planning](clusters/governance-planning.md)

## Shared standards

- [Documentation publication standard](UNIFIED_DOCUMENTATION_STANDARD.md)
- [Documentation control](DOCUMENTATION_CONTROL.md)
- [Engineering evolution framework](ENGINEERING_EVOLUTION_FRAMEWORK.md)
- [Repository evolution directives](REPOSITORY_EVOLUTION_DIRECTIVES.md)
- [Contributing](../CONTRIBUTING.md)
- [Security](../SECURITY.md)

## Evidence rule

Inventory counts and capability catalogues describe registered material, not certification. Hardware discovery is read-only; embedded MMIO is reference-only; source review is static; FFI is explicit and trusted-library-only. Benchmark output is machine-dependent.
