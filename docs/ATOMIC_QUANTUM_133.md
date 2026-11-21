# Atomic-Quantum 133 — Pytyliki C11 kernel

The shared catalogue registers 133 stable operation IDs across six families. Pytyliki exposes the bounded numerical simulation core to the systems workstation; it is a classical state-vector model, not a quantum processor or hardware-control driver.

## Families

| Ordinals | Family | Scope |
|---|---|---|
| 001–022 | State | Qubit/atom initialization, normalization, rotations and level models |
| 023–044 | Entanglement | Bell/GHZ/W construction and correlation measurements |
| 045–066 | Measurement | Projective sampling, readout, feedback and telemetry |
| 067–088 | Noise / ECC | Channels, syndrome primitives and code helpers |
| 089–110 | Atomic dynamics | Lattice, potential and transport models |
| 111–133 | Algorithms / telemetry | Algorithmic kernels and health metrics |

## Contracts and limits

- Canonical manifest: `schemas/atomic-quantum-functions.v1.json`; schema: `schemas/atomic-quantum-functions.v1.schema.json`.
- State-vector simulation is bounded by the compiled maximum qubit count; all numeric inputs must be finite and normalized state is checked within tolerance.
- Measurement randomness is seedable for repeatable tests.
- Catalogue membership is not a claim that every advanced research model is a validated physical solver. Unsupported operations remain explicitly catalogued until their numerical model and tests are implemented.
- No implicit device access, hardware control, calibration, or quantum-hardware certification is performed.

See also [Pytyliki architecture](ARCHITECTURE.md) and [TRACE measurement provenance](TRACE_MEASUREMENT_PROVENANCE.md).
