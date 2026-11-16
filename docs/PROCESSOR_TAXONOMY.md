# Processor taxonomy — translated engineering vocabulary

This document translates the five source groups into neutral software/processor engineering domains. The original clinical wording is not retained in the application UI, identifiers, logs, or generated reports.

## I. Root cause, system topology and fault epicenter

| Original conceptual family | Engineering vocabulary | Desktop representation |
|---|---|---|
| Etiology, pathogenesis, prodrome | Root cause, failure path, early warning | Diagnostic event source and symptom-to-subsystem notes |
| Epidemiology, population, cluster, epicenter | Fleet topology, host group, affected-node scope | Host/CPU inventory and future fleet aggregation point |
| Mutation, strain, phenotype | CPU stepping, family/model, ISA profile, build variant | CPUID snapshot, architecture label and feature matrix |
| Contamination, infection, invasion | Input corruption, dependency compromise, unexpected execution path | Static review flags and strict file-format validation |
| Resistance, immunosuppression, hypervirulence | Fault tolerance, weakened isolation, high-impact defect | Input bounds, validation paths and integrity checks |
| Hypertrophy, proliferation, apoptosis, necrosis | Resource growth, process fan-out, lifecycle termination, memory corruption | Workload scaling and memory-allocation boundaries |
| Ischemia, hypoxia, thrombosis, embolism | Starvation, stalled pipeline, lock contention, queued-work blockage | Performance bottleneck vocabulary for future telemetry adapters |
| Screening, diagnostics, verification, classification | Inventory, static analysis, validation, taxonomy | CPU scan, source review and test matrix |

## II. Processor profiles and execution-path vectors

- **Execution phenotype:** architecture, vendor, family/model, logical processor topology and selected ISA feature flags.
- **Instruction compatibility:** baseline ISA requirements and feature-gated implementation choices. Hardware flags are informational; AVX-family state readiness is not established by this version.
- **Scheduler and execution resources:** logical processor count, bounded CPU workloads and repeatable measurement windows.
- **Memory subsystem:** total/available memory inventory and a bounded cache-line-stride memory traversal benchmark.
- **Control-flow integrity:** static source review prompts, input validation and deterministic unit tests.
- **System presentation:** native Win32 desktop controls, local session log, file dialogs and CLI equivalents.

## III. Resource kinetics and signal-processing chain

| Original conceptual family | Engineering vocabulary | Implemented operation |
|---|---|---|
| Kinetics, metabolism, clearance | Data flow, resource lifetime, allocation and release | Bounded allocation/deallocation in benchmark and DSP paths |
| Tolerance, saturation, inhibition | Threshold response, dynamic range and signal shaping | DSP compressor, soft saturation and bounded gain |
| Receptor / agonist / antagonist / synergy | Filter coefficients, gain stages, operator interaction | One-pole low-pass and sequential DSP chain |
| Acoustic analyzer and modulation | Sample buffer, sample rate, amplitude and frequency domain | PCM16 WAV import/export, sine test-tone generator and signal metering |
| Analysis, synthesis, prediction | Measurement, transformation, comparative interpretation | Peak/RMS/DC/crest/clipping metrics and benchmark timing |

## IV. Contention cascades, subsystem mutation and isolation

- **Competition:** CPU and memory workload contention; future extension point for scheduler/NUMA telemetry.
- **Mutation:** a change in build flags, implementation variant, firmware-reported CPU identity or runtime profile; only software-observable inventory is read here.
- **Cascade:** a local delay or resource bottleneck that propagates through a pipeline; the current suite measures independent workload classes rather than claiming to diagnose a complete cascade.
- **Isolation:** each benchmark has fixed limits; the source scanner reads text without executing it; the DSP chain operates on a bounded in-memory buffer.
- **Recovery:** a failed import or benchmark returns an error without attempting hardware changes or automatically rewriting user files.

## V. Stability, regression history and lifecycle

- **Stable state:** a successful run with valid inputs and a passing integrity check.
- **Regression / recurrence:** a future comparison of the same workload profile over saved reports or CI runs.
- **Prognosis:** trend analysis over repeatable measurements, not a prediction of device lifetime.
- **Audit and certification vocabulary:** versioned CMake build, CTest, CI matrix and JSON report metadata.
- **Persistence:** the current version exports a report when requested; it does not run background monitoring or silently collect telemetry.

## Implemented desktop surfaces

1. **Processor inventory:** CPU brand string where available, vendor, family/model, architecture, logical processors, OS-reported physical/available memory and selected ISA flags.
2. **Processor workload lab:** integer dependency-chain benchmark, memory traversal and compact matrix workload, each with timing and checksum.
3. **Source review:** heuristic checks for selected process-execution APIs, unbounded string APIs, manual memory mapping and inline assembly.
4. **DSP workbench:** PCM16 mono/stereo import, sine generation, low-pass, compression, saturation, gain, metering and PCM16 export.
5. **CLI and automation path:** command-line equivalents for CPU inventory, benchmark suite, source review, WAV generation, inspection and processing.

## Non-goals

The taxonomy is an engineering metaphor. It does not model medical conditions or recommend treatment. Pytyliki does not overclock, set voltage/frequency, write BIOS/UEFI or firmware, flash microcode, load drivers, run arbitrary inspected source, or capture audio from hardware devices.
