# Pytyliki: 20-system engineering programme

This registry maps the twenty named improvement tracks to enforceable engineering objectives. It is deliberately a **roadmap and contract catalogue**, not a claim that all twenty capabilities are already implemented.

| Track | Engineering objective |
|---|---|
| Chrome Ironclad | Reliability invariants and fail-safe behavior |
| Velocity Tempest | Reproducible performance benchmarks |
| Midnight Monolith | Explicit modular boundaries |
| Raven Vortex | Evidence-backed diagnostics |
| Circuit Cobalt | Capability-gated hardware paths |
| Thunder Phantom | Bounded asynchronous work |
| Phantom Radiance | Stable CLI and library contracts |
| Velvet Gravitas | Consistent, accessible reporting |
| Obsidian Inferno | Safe error handling and recovery |
| Horizon Spectral | Windows/Linux portability |
| Crimson Overdrive | Strict compiler diagnostics |
| Voltage Citadel | Least-privilege security |
| Shadow Nemesis | Boundary and regression tests |
| Mirage Aether | Explicit distributed-system contracts |
| Titanium Riptide | Resource ownership and bounds |
| Eclipse Outlaw | Versioned extension points |
| Neutron Zenith | Measurement provenance |
| Scarlet Avalanche | Reproducible CI automation |
| Quantum Paradox | Isolated opt-in experiments |
| Reactor Afterglow | Health and progress observability |

## Implementation contract

- Registry entries are immutable static data and have stable numeric IDs.
- Validation rejects missing names, objectives, acceptance criteria, and invalid domains.
- A catalogue entry does not activate hardware writes, network services, autonomous code changes, or experimental execution.
- Performance claims require recorded baselines and repeatable measurements.
- New runtime behavior must be implemented in its owning subsystem and covered by tests before its status is described as implemented.

## Verification

The CTest target `system-enhancement-registry-tests` checks the exact count, stable sequential IDs, non-empty contracts, unique names, and invalid-input handling.
