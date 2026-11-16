# Hardware Optimization Rule Engine

## Purpose

Pytyliki's Hardware Optimization Rule Engine converts read-only profiler evidence into
traceable engineering recommendations. It does not claim to redesign a fabricated CPU.
Physical changes require RTL/HDL sources, target-cell libraries, process constraints,
verification, timing closure and a fabrication flow.

## Rule identity and lifecycle

The catalog reserves 1,000 stable identifiers across 20 domains (50 identifiers each).
Version 1 implements 12 executable rules; the remaining identifiers are reserved for
future rules and are **not** counted as evaluated. A rule must move through:

1. Proposed — documented hypothesis and target block.
2. Instrumented — required metrics and provenance defined.
3. Implemented — deterministic evaluator and unit tests exist.
4. Validated — thresholds justified against a workload and platform.
5. RTL candidate — optional proposed hardware change represented in HDL.
6. Verified — lint, simulation, formal equivalence and CDC/RDC as applicable.
7. PPA reviewed — power, performance, area and timing results recorded.
8. Accepted — human approval; never auto-flashed or auto-programmed.

## Twenty domains (50 IDs each)

| Domain | ID range |
|---|---:|
| Core architecture and pipeline | 001–050 |
| Execution units and datapaths | 051–100 |
| Cache hierarchy and memory | 101–150 |
| Power delivery and energy | 151–200 |
| Thermal behavior | 201–250 |
| Physical design and routing | 251–300 |
| Transistor and standard-cell design | 301–350 |
| Semiconductor process constraints | 351–400 |
| Multicore and coherency | 401–450 |
| Buses and interconnect | 451–500 |
| Branch prediction and front-end | 501–550 |
| Hardware security | 551–600 |
| Reliability and fault tolerance | 601–650 |
| RTL and formal verification | 651–700 |
| HDL synthesis and optimization | 701–750 |
| Floorplanning and topology | 751–800 |
| Performance instrumentation | 801–850 |
| Adaptive resource management | 851–900 |
| Cross-ISA and compiler interface | 901–950 |
| Optimization governance and release | 951–1000 |

## Rule record contract

Each rule must define: `id`, `domain`, `target_block`, `required_metrics`,
`detection`, `recommendation`, `expected_effect`, `risk`, `verification`,
`evidence_source`, `status`, and `rollback_plan`.

Thresholds are advisory defaults until calibrated. Missing metrics are unknown, never
silently treated as zero. A recommendation must not claim a performance gain until
measured against a reproducible baseline.

## Safety boundaries

- Read-only hardware telemetry only.
- No BIOS/UEFI writes, firmware flashing, microcode writes or kernel/driver mutation.
- No automatic voltage, clock, power-limit or fan-control changes.
- No raw device-register writes or privileged hardware access.
- No automatic RTL deployment or fabrication release.
- Every hardware candidate requires human review and reproducible verification.

## Current executable rules

The C11 evaluator currently contains 12 rules covering IPC, L1/L2 miss rates, branch
misprediction, memory stalls, power/temperature observations, utilization, missing
profiler evidence, and missing physical-design/reliability evidence. The latter are
process gates, not claims that the processor is defective.
