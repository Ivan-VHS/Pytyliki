# Processor systems lab — 33 functions

Local C11 API for bounded native algorithms, caller-owned simulation and explicit adapters. It is not a blockchain node, EVM, hypervisor, JIT, FPGA compiler or privileged tuner.

| Mode | Meaning |
|---|---|
| Native | bounded local algorithm |
| Simulated | caller-owned state only |
| Adapter | caller supplies verifier/crypto/vendor API |
| Planning | advisory plan; no application |

Discover: `pytyliki --systems-lab`. API: `src/systems_accel.h`; implementation: `src/systems_accel.c`; tests: `tests/test_systems_accel.c`.

The 33-entry registry describes available function contracts, not 33 full external integrations. RPC endpoint selection obeys provider quotas; proof/relay helpers do not broadcast or bypass controls.
