# Pytyliki — Processor Engineering Workstation 0.7.0

Pytyliki is a native C11 desktop workstation for processor architecture discovery, instruction-set inventory, bounded micro-workload benchmarking, processor-source review, DSP datapath experiments and evidence-gated improvement planning. Windows builds include a native Win32 GUI; Windows and Linux builds include the CLI.

## Processor engineering modules

- **Architecture inventory:** CPU vendor, family/model, brand string, architecture, logical topology, OS-reported memory and selected ISA capability flags.
- **Execution workload lab:** deterministic integer dependency chains, bounded memory traversal and matrix arithmetic, with timing and integrity checks.
- **Processor implementation review:** read-only source inspection for CPUID use, timestamp counters, SIMD intrinsics, vector widths, cache prefetch, atomics, memory fences, volatile accesses, non-temporal stores, alignment and inline assembly.
- **DSP datapath laboratory:** bounded sample-vector workloads for filter, compressor, saturation, gain and numerical metering. PCM16 WAV is retained strictly as a portable test-vector container.
- **Recursive improvement engine:** a deterministic 20-row capability catalogue and a gated lifecycle from observation through verification, human review, integration and meta-improvement.

## Recursive improvement engine

Run `pytyliki --improvement-plan` to print the 20 capability rows. Each row combines an engineering transformation, an optimization activity and an automation direction.

The C11 API in `src/improvement.h` exposes the catalogue and an evidence-gated state machine:

`observe -> diagnose -> propose -> implement -> verify -> evaluate -> review -> integrate -> learn -> improve-engine`.

The lifecycle requires evidence for each phase, prohibits skipping phases and requires explicit human approval before integration. The library records workflow state; it does not silently rewrite source code or merge changes. See [the engine specification](docs/recursive-processor-improvement.md).

## CLI

    pytyliki --profile --suite
    pytyliki --capabilities
    pytyliki --improvement-plan
    pytyliki --review-processor path/to/processor_kernel.c
    pytyliki --json processor-report.json
    pytyliki --tone-out datapath-vector.wav --frequency 440 --seconds 3
    pytyliki --audio-info input-vector.wav
    pytyliki --process-wav input-vector.wav output-vector.wav --lowpass 5000 --compress -12 4 --saturate 1.8 --gain -1

The legacy WAV option names are retained for command-line compatibility. Input is uncompressed PCM16 mono/stereo, 8–192 kHz; unsupported formats are rejected.

## Build and test

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

On Windows, CMake also builds `pytyliki-gui.exe`.

## Processor-domain translation

| General concept | Processor engineering function |
|---|---|
| Root cause / origin | Fault-origin tracing across front end, execution, cache, memory, interconnect and power/thermal domains |
| Topology / cluster | CPU fleet topology, socket/core/thread grouping and affected-silicon scope |
| Variant / phenotype | Stepping, family/model, microarchitecture, ISA profile and build-target variants |
| Contamination / propagation | Corrupted inputs, compromised build dependencies, invalid instruction paths and fault propagation |
| Resistance / recovery | Fault containment, bounds checks, reset recovery and integrity verification |
| Scaling / saturation | Thread scaling, queue growth, cache pressure, bandwidth saturation and execution-resource occupancy |
| Bottlenecks | Pipeline starvation, cache misses, memory stalls, port contention and interconnect backpressure |
| Data-path efficiency | Operand movement, register allocation, cache residency, throughput and resource lifetime |
| DSP execution | Vector arithmetic, filter state, saturation and numerical signal metering |
| Regression / stability | Stable operating point, repeatability drift, performance regression and thermal throttling |
| Audit / verification | Capability enumeration, source review, benchmark validation and reproducible CI evidence |

## Safety and evidence contract

- CPU discovery uses documented read-only OS queries and CPUID.
- ISA flags are informational. AVX-family flags do not establish OS vector-state readiness.
- Benchmarks have fixed workload and allocation limits and run without privileged access.
- Source review reads text only. It does not compile, link, execute, load or rewrite inspected code.
- DSP vectors are bounded local test data; there is no microphone capture, device control or network transfer.
- No BIOS/UEFI, firmware, microcode, voltage, clock, power-limit, kernel or driver writes are performed.
- Improvement proposals require deterministic tests, review and explicit human authorization. The engine cannot approve or merge its own changes, weaken tests or bypass branch protection.
- Results are advisory, not silicon certification or performance guarantees.

## Project layout

- `src/core.c`, `src/core.h` — CPU identity, bounded baseline benchmark, processor-source review and JSON output.
- `src/processor.c`, `src/processor.h` — architecture/ISA inventory and execution workload suite.
- `src/dsp.c`, `src/dsp.h` — bounded DSP datapath test-vector operations.
- `src/improvement.c`, `src/improvement.h` — 20-capability catalogue and evidence-gated evolution lifecycle.
- `src/win_gui.c` — native Windows engineering workstation UI.
- `src/cli.c` — cross-platform command-line interface.
- `tests/test_core.c`, `tests/test_improvement.c` — core and improvement-engine tests.
- `.github/workflows/ci.yml` — Windows/Linux build and test matrix.

License: MIT.
