# Pytyliki — Processor Engineering Workstation 0.6.0

Pytyliki is a native C11 desktop workstation for processor architecture discovery, instruction-set inventory, bounded micro-workload benchmarking, processor-source review, and digital signal-processing datapath experiments. Windows builds include a native Win32 GUI; Windows and Linux builds include the CLI.

## Processor engineering modules

- **Architecture inventory:** CPU vendor, family/model, brand string, architecture, logical topology, OS-reported memory and selected ISA capability flags.
- **Execution workload lab:** deterministic integer dependency chains, bounded memory traversal and matrix arithmetic, with timing and integrity checks.
- **Processor implementation review:** read-only source inspection for CPUID use, timestamp counters, SIMD intrinsics, vector widths, cache prefetch, atomics, memory fences, volatile accesses, non-temporal stores, alignment and inline assembly.
- **DSP datapath laboratory:** bounded sample-vector workloads for filter, compressor, saturation, gain and numerical metering. PCM16 WAV is retained strictly as a portable test-vector container; this is not an audio-production or recording application.
- **Engineering evidence:** structured JSON reports, deterministic tests, CMake builds and Windows/Linux CI.

## CLI

    pytyliki --profile --suite
    pytyliki --capabilities
    pytyliki --analyze path/to/processor_kernel.c
    pytyliki --json processor-report.json
    pytyliki --tone-out datapath-vector.wav --frequency 440 --seconds 3
    pytyliki --audio-info datapath-vector.wav
    pytyliki --process-wav input-vector.wav output-vector.wav --lowpass 5000 --compress -12 4 --saturate 1.8 --gain -1

The legacy WAV option names are retained for command-line compatibility. Their data is treated as bounded DSP test vectors, not as a general audio workflow. Input is uncompressed PCM16 mono/stereo, 8–192 kHz; unsupported formats are rejected.

## Build and test

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

On Windows, CMake also builds `pytyliki-gui.exe`.

## Processor-domain translation

All formerly general-purpose or non-processor concepts are assigned a concrete processor-engineering role:

| General concept | Processor engineering function |
|---|---|
| Root cause / etiology | Fault-origin tracing across front end, execution, cache, memory, interconnect and power/thermal domains |
| Population / cluster / epicenter | CPU fleet topology, socket/core/thread grouping and affected-silicon scope |
| Mutation / phenotype / variant | Stepping, family/model, microarchitecture, ISA profile and build-target variants |
| Infection / contamination / invasion | Corrupted inputs, compromised build dependencies, invalid instruction paths and fault propagation |
| Resistance / immunity | Fault containment, ECC awareness, bounds checks, reset recovery and integrity verification |
| Proliferation / hypertrophy | Thread scaling, queue growth, cache pressure, memory bandwidth saturation and execution-resource occupancy |
| Ischemia / hypoxia / thrombosis | Pipeline starvation, cache misses, memory stalls, port contention and interconnect backpressure |
| Pharmacokinetics / metabolism | Operand movement, register allocation, cache residency, data-path throughput and resource lifetime |
| Auditory modulation / stimulation | DSP execution kernels, vector arithmetic, filter state, saturation and numerical signal metering |
| Remission / recurrence / exacerbation | Stable operating point, repeatability drift, performance regression and thermal throttling |
| Screening / diagnosis / audit | Capability enumeration, processor-source review, benchmark validation and reproducible CI evidence |

These are engineering translations only. No clinical terminology or human-health interpretation is used by the application.

## Safety and evidence contract

- CPU discovery uses documented read-only OS queries and CPUID.
- ISA flags are informational. AVX-family flags do not establish OS vector-state readiness and are not permission to execute those instructions.
- Benchmarks have fixed workload and allocation limits and run without privileged access.
- Source review reads text only. It does not compile, link, execute, load or rewrite inspected code. Findings are review prompts, not vulnerability verdicts.
- DSP vectors are bounded local test data. There is no microphone capture, background recording, device control or network transfer.
- No BIOS/UEFI, firmware, microcode, voltage, clock, power-limit, kernel or driver writes are performed.
- Results are advisory and are not silicon certification or guarantees of performance.

## Project layout

- `src/core.c`, `src/core.h` — CPU identity, bounded baseline benchmark, processor-source review and JSON output.
- `src/processor.c`, `src/processor.h` — architecture/ISA inventory and execution workload suite.
- `src/dsp.c`, `src/dsp.h` — bounded DSP datapath test-vector operations.
- `src/win_gui.c` — native Windows engineering workstation UI.
- `src/cli.c` — cross-platform command-line interface.
- `tests/test_core.c` — processor, DSP datapath and core tests.
- `.github/workflows/ci.yml` — Windows/Linux build and test matrix.

License: MIT.
