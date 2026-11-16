# Pytyliki — Processor Systems Workstation 0.5.0

Pytyliki is a native C11 desktop workstation for read-only CPU capability discovery, bounded processor benchmarking, heuristic source review, and local PCM audio/DSP experiments. Windows builds include a native Win32 GUI; Windows and Linux builds include the CLI.

## Desktop workstation

- **CPU capability scan:** identifies the target architecture, CPU marketing string where CPUID exposes it, vendor/family/model, logical processor count and OS-reported physical/available memory, plus selected x86 ISA feature flags (SSE2, SSE4.2, AVX, AVX2, AVX-512F, AES, FMA, virtualization bit).
- **Processor suite:** bounded integer, memory-touch, and matrix workloads with elapsed-time measurements and an integrity checksum.
- **Source review:** read-only heuristic scan for selected process-execution, unbounded-string, manual-memory-mapping, and inline-assembly patterns.
- **DSP laboratory:** import PCM 16-bit mono/stereo WAV, generate a 440 Hz test tone, configure low-pass cutoff/compressor threshold and ratio/saturation drive/gain in the GUI, measure peak/RMS/DC/crest/clipping, and export PCM16 WAV.
- **Session log and report export:** visible results/error feedback plus a user-triggered JSON CPU/benchmark report.

The GUI presents actions as native Windows controls and does not require a browser or runtime framework.

## CLI

    pytyliki --profile --suite
    pytyliki --capabilities
    pytyliki --analyze path/to/source.c
    pytyliki --json report.json
    pytyliki --tone-out test-tone.wav --frequency 440 --seconds 3
    pytyliki --audio-info input.wav
    pytyliki --process-wav input.wav output.wav --lowpass 5000 --compress -12 4 --saturate 1.8 --gain -1

The process command accepts uncompressed PCM16 WAV (mono or stereo, 8–192 kHz). Unsupported formats are rejected instead of guessed or silently converted.

## Build and test

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

On Windows, CMake also builds pytyliki-gui.exe. The GUI links the system common-dialog library for file selection.

## Processor terminology mapping

| Source concept | Processor/workstation interpretation |
|---|---|
| Etiology / root cause | Failure-origin analysis, subsystem classification, diagnostic event source |
| Mutation / strain / phenotype | CPU family/model, ISA feature profile, microarchitecture and build variants |
| Resistance / immunity | Fault containment, bounds checks, input validation, integrity verification |
| Proliferation / dissemination | Workload scaling, thread topology, memory pressure and task fan-out |
| Remission / recurrence / exacerbation | Stable run, regression, repeatability drift and performance degradation |
| Receptor / antagonist / synergy | Instruction-feature compatibility, competing resource use and optimization trade-offs |
| Pharmacokinetics / metabolism | Data-path throughput, memory movement, cache locality and resource lifecycle |
| Auditory stimulation / modulation | DSP processing chain: filter, compressor, saturation, gain and metering |
| Chronicity / survival / prognosis | Long-run reliability metrics, trend reports, benchmark history and regression monitoring |
| Classification / coding / audit | Capability taxonomy, structured reports, deterministic tests and CI validation |

These mappings are software engineering metaphors, not medical models. Pytyliki is not a medical tool and does not diagnose people or devices.

## Safety contract

- CPU inspection uses documented read-only operating-system and CPUID queries.
- No BIOS/UEFI, firmware, microcode, voltage, clock, power-limit, kernel, or driver changes are performed.
- ISA flags are informational; AVX-family hardware flags do not establish OS vector-state support or safe execution eligibility.
- Benchmarks have hard workload and allocation bounds and run in the current process.
- The source scanner reads source as text. It does not compile, link, load, execute, or rewrite scanned files.
- Audio is processed in a bounded local memory buffer. WAV import supports PCM16 only; no network, device capture, or background recording is performed.
- Findings and benchmark comparisons are advisory. They are not certification, proof of vulnerabilities, or guarantees of performance.

## Project layout

- src/core.c, src/core.h — CPU profile, bounded benchmark, static scanner, JSON report.
- src/processor.c, src/processor.h — architecture/ISA inventory and bounded processor suite.
- src/dsp.c, src/dsp.h — bounded WAV I/O and local DSP pipeline.
- src/win_gui.c — native Win32 workstation UI.
- src/cli.c — cross-platform command-line interface.
- tests/test_core.c — core, processor, and DSP tests.
- .github/workflows/ci.yml — Windows/Linux build and test matrix.

License: MIT.
