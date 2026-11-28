# Repository Structure Profile — Pytyliki

**Normative reference:** [Ivan-VHS Repository Structure Standard](https://github.com/Ivan-VHS/DAK/blob/main/docs/REPOSITORY_STRUCTURE_STANDARD.md).

This profile maps the shared logical structure to Pytyliki's existing C11/CMake implementation. It does not authorize a wholesale directory rename or a second copy of an existing specification.

## Canonical layout

| Concern | Existing canonical location | Responsibility |
|---|---|---|
| Public project entry | `README*.md`, `CONTRIBUTING.md`, `SECURITY.md`, `LICENSE` | Scope, supported platforms, contribution, security and legal terms |
| Architecture and engineering | `docs/README.md`, `docs/ARCHITECTURE.md`, `docs/ENGINEERING.md`, `docs/UNIFIED_PROJECT_CONTRACT.md` | Navigation, module boundaries, maturity and project contracts |
| Native implementation | `src/`, `include/` | C11 implementation and published native interfaces |
| Build integration | `CMakeLists.txt`, `cmake/` | Targets, configuration and platform-specific build logic |
| Verification | `tests/`, `fixtures/` | CTest/unit/integration checks and deterministic input data |
| Machine contracts | `schemas/`, `contracts/`, root versioned contract JSON | Versioned schemas, ABI and repository-level requirements |
| Reproducible tooling | `scripts/`, `automation/` | Audits, validators and bounded maintenance automation |
| Supporting material | `examples/`, `assets/`, `data/` | Examples, licensed assets and non-authoritative fixtures |
| CI | `.github/workflows/` | Build, tests, contracts, security and documentation gates |

## Domain separation

- C11 source and public headers remain authoritative for native behavior; documentation does not replace implementation evidence.
- The Reaper/Pytyliki reciprocal interface is documented in the existing integration and ABI contracts. Keep both sides versioned and tested.
- Hardware descriptions, simulated MMIO and reference profiles must remain distinguished from physical device access.
- Benchmark observations belong with their measurement method, machine context and provenance; they are not universal performance guarantees.

## Formal evidence

This repository uses executable C/CTest contracts, ABI checks and reproducible fixtures. A dedicated proof-assistant directory is not required until an actual checker and proof sources are selected. Do not create placeholder theorem files or label tests as mathematical proofs.

## Change rule

A path migration must update CMake target sources, public include paths, test discovery, contract evidence paths, documentation links and workflow filters in the same change. The current names above are canonical until such a migration is reviewed.
