# Pytyliki documentation

The root [README](../README.md) is the entry point for product scope, supported targets, build commands and current evidence. This directory contains the detailed engineering contracts.

## Topic hubs

| Hub | Covers |
|---|---|
| [Architecture and system contracts](clusters/architecture.md) | Runtime layers, data flow, bounded execution, memory and processor models |
| [Processor and computational labs](clusters/processor-labs.md) | CPU capabilities, numerical tools, embedded reference models and lab workflows |
| [Quality, safety and evidence](clusters/quality-evidence.md) | Validation, repeatability, provenance, reporting and safety boundaries |
| [UI, motion and visual documentation](clusters/ui-visual.md) | Native desktop interaction, accessibility, motion and documentation assets |
| [Integrations and project federation](clusters/federation.md) | Plugins, external interfaces and cross-project contracts |
| [Governance and planning](clusters/governance-planning.md) | Ownership, decisions, roadmaps and reference material |

## Core references

- [Architecture](ARCHITECTURE.md) — implemented runtime and data flow.
- [Features](FEATURES.md) — module inventory and capability boundaries.
- [Unified documentation standard](UNIFIED_DOCUMENTATION_STANDARD.md) — maturity labels, evidence rules and terminology.
- [Engineering](ENGINEERING.md) — validation and safety gates.
- [Information architecture](INFORMATION_ARCHITECTURE.md) — complete topic map.

## Repository boundaries

Source code, tests and schemas define executable behavior. A proposal, registry entry or architecture contract does not by itself prove implementation. The root README distinguishes local host observations from example data and states which operations are read-only.

For changes, update the owning specification, relevant topic hub and this index when navigation or ownership changes. Run the native build and CTest suite; see [CONTRIBUTING.md](../CONTRIBUTING.md) for the repository workflow.
