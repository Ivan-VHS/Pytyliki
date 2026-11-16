# Processor workstation architecture and safety contract

## Product boundary

Every module must inspect, measure, validate or simulate a CPU, microarchitecture, processor implementation, execution path, memory subsystem, or DSP datapath. General-purpose functionality is accepted only when it directly supports those tasks.

## Layers

1. **Core** — processor identity, platform topology, bounded baseline execution benchmark, processor-source review and structured report output.
2. **Processor inventory** — architecture classification, vendor/family/model, selected ISA flags, OS memory inventory and logical processor topology.
3. **Execution lab** — bounded integer dependency, memory traversal and matrix workloads; timing and integrity checksum.
4. **Implementation review** — read-only inspection of CPUID, timestamp counters, SIMD intrinsics, vector types, prefetch hints, atomics, fences, volatile accesses, non-temporal stores, alignment and inline assembly.
5. **DSP datapath lab** — bounded numerical sample vectors processed by filter, compressor, saturation and gain kernels. PCM16 WAV is only an import/export test-vector envelope.
6. **Presentation** — native Win32 GUI and cross-platform CLI, both exposing the same local processor-engineering operations.
7. **APSIA routing planner** — deterministic workload-to-language selection, sandbox precedence, target-availability checks and FFI-adjusted cost gates. See [APSIA 2.0](APSIA_2.0.md).

## Data flow

- The presentation layer requests a processor snapshot; the core obtains OS topology and CPUID data where supported.
- The execution lab runs bounded workloads and reports elapsed time and checksums.
- The implementation reviewer reads selected source files as text and emits heuristic prompts for architecture-specific review.
- The DSP lab validates vector dimensions, sample rate, channel count and payload bounds before allocating test data. Kernels operate only on in-memory vectors.
- The APSIA planner accepts explicit workload and benchmark estimates, chooses an available target language, and retains the host implementation when sandbox, FFI or performance policy fails.
- Reports contain measurements and explicit evidence limitations; missing telemetry is never invented.

## HyperFusion semantics

- Concreo scores architecture/source-review readiness; Pyemont plans transformations without rewriting files; Bollo evaluates benchmark/memory evidence; JJ-HH cross-checks test and scan evidence; PenWhite records trace events; Serious Serious controls promotion.
- The 6×6 interaction matrix evaluates 36 directed pairs per round, including self-checks. A 1–64 round limit bounds work; diminishing returns permit early convergence.
- Scores are heuristic coordination metrics, not benchmark speedups, correctness proofs or physical processor modifications. The CLI cannot certify CI success and keeps promotion held.

## APSIA implementation boundary

The current APSIA module is a planning library only. It does not generate source code, invoke compilers, execute inspected code, load native libraries, start production profilers, or perform hot swapping. WIT/IDL generation, hermetic builds, SBOM/signing, differential fuzzing, shadow execution, runtime canaries and artifact persistence remain staged capabilities described in [APSIA 2.0](APSIA_2.0.md). The evolution-lock schema records evidence metadata; it is not an execution authorization.

## Safety boundaries

- No privileged elevation, kernel driver, BIOS/UEFI, firmware, microcode, voltage, frequency or power-limit writes.
- CPUID feature bits do not establish OSXSAVE/XCR0 readiness. AVX-family instructions are not dispatched by capability flags alone.
- Workloads retain hard iteration, memory, pass-count and sample-frame ceilings.
- Source review never compiles, loads, executes or modifies inspected files.
- DSP vectors are local test data. No hardware audio capture, device playback, background monitoring or network activity is performed.
- Findings are advisory and must be corroborated by trusted profiler, simulator, compiler, formal, timing or physical-design evidence.

## Extension rules

New modules must declare: processor subsystem, required telemetry, supported architectures, workload limits, failure behavior, evidence quality, and whether the operation is read-only or simulated. Hardware-affecting actions remain out of scope unless a separately reviewed, vendor-documented and explicitly approved interface is designed.
