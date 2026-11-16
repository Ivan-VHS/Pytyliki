# Processor engineering taxonomy

This taxonomy defines the vocabulary used throughout Pytyliki. General, clinical or non-processor source concepts are translated into concrete processor engineering domains; the original terminology is not used in application labels, logs or reports.

## I. Fault origin and processor topology

| General source concept | Processor-domain interpretation | Engineering surface |
|---|---|---|
| Root cause, pathogenesis, early warning | Fault origin, failure path, pre-fault telemetry | Diagnostic event and subsystem mapping |
| Population, cluster, epicenter | CPU fleet, socket/core topology, affected-node scope | Processor inventory |
| Mutation, strain, phenotype | Silicon stepping, family/model, microarchitecture and ISA profile | CPUID and build-target matrix |
| Contamination, invasion | Corrupted inputs, compromised toolchain, invalid execution path | Input validation and source review |
| Resistance, immunity | Fault containment, ECC, bounds safety and recovery | Integrity gates and safe failure paths |
| Hypertrophy, proliferation | Queue growth, thread fan-out, cache pressure and bandwidth saturation | Scaling workloads |

## II. Execution and microarchitecture

- **Front end:** instruction fetch/decode supply, branch prediction and instruction-cache pressure.
- **Execution:** dependency chains, issue width, execution-port utilization and retirement throughput.
- **Control flow:** branch misprediction, speculation boundaries and synchronization overhead.
- **Memory hierarchy:** register pressure, L1/L2/LLC locality, TLB behavior, memory-level parallelism and NUMA placement.
- **Interconnect:** cache-coherence traffic, socket links, fabric contention and backpressure.
- **Power/thermal:** package power, operating points, thermal headroom and throttling telemetry.

## III. Processor data-path and DSP kernels

| General source concept | Processor interpretation | Implemented/targeted operation |
|---|---|---|
| Kinetics, metabolism, clearance | Operand flow, data movement, allocation lifetime | Bounded memory and vector workloads |
| Tolerance, saturation, inhibition | Numeric range, clipping, nonlinear transfer response | Saturation and compression kernels |
| Receptor / agonist / antagonist / synergy | Filter coefficients and stage interaction | Sequential datapath operators |
| Acoustic modulation | DSP vector execution and sample-domain transforms | PCM16 test-vector import/export, filter and metering |
| Analysis, synthesis, prediction | Measurement, transform and comparative modeling | Timing, checksums and signal statistics |

PCM/WAV is a test-data transport format only. It does not define the product domain.

## IV. Contention, faults and isolation

- **Contention:** core, cache, memory, execution-port and interconnect competition.
- **Fault propagation:** a local stall or corrupted operand affecting downstream execution stages.
- **Isolation:** workload bounds, validated input sizes and non-executing source inspection.
- **Recovery:** deterministic error returns and integrity checks; no automatic hardware mutation.
- **Evidence:** profiler counters, simulator traces, compiler output, formal checks and physical-design reports.

## V. Stability and verification

- **Stable state:** valid inputs, completed workload and passing integrity checksum.
- **Regression:** a measured change against the same CPU, build, workload and operating conditions.
- **Reliability:** repeatability, thermal behavior, aging and platform-specific error telemetry.
- **Verification:** CMake, CTest, CI, compiler diagnostics and, for RTL work, lint/formal/timing/power/area evidence.
- **Persistence:** reports are generated only on explicit user request; no silent telemetry collection.

## Product modules

1. Processor inventory and ISA capability matrix.
2. Integer, memory and matrix execution workload suite.
3. Read-only processor implementation source reviewer.
4. Bounded DSP datapath test-vector laboratory.
5. CLI, native desktop GUI and structured engineering reports.

## Non-goals

Pytyliki does not diagnose human conditions, process general media projects, record from hardware devices, overclock, change voltage/frequency, write firmware, flash microcode, load drivers, execute inspected source or claim silicon-level optimization without the required design artifacts and verification evidence.
