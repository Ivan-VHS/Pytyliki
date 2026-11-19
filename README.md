# Pytyliki — Processor Systems Workstation

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="Windows and Linux CI" src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg"></a>
  <img alt="ISO C11" src="https://img.shields.io/badge/C-ISO%20C11-2563EB?style=flat-square">
  <img alt="Windows and Linux" src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-DC2626?style=flat-square">
  <img alt="MIT License" src="https://img.shields.io/badge/license-MIT-111827?style=flat-square">
</p>

<p align="center"><strong>DISCOVER · ANALYZE · PROCESS · VERIFY</strong></p>

<p align="center"><img src="assets/pytyliki-hero.svg" alt="Pytyliki — processor diagnostics, source analysis and local DSP workstation" width="100%"></p>

**A practical C11 workstation for understanding a machine and inspecting local workloads.** Pytyliki combines read-only processor discovery, bounded benchmarks, source-code diagnostics, PCM16 WAV/DSP tools and a native Windows desktop interface in one modular project.

It is built for developers and engineers who want useful local measurements, inspectable reports and explicit safety boundaries — not a black box that silently changes hardware.

**Read this in:** [English](README.md) · [Русский](README.ru.md) · [Українська](README.uk.md) · [简体中文](README.zh-CN.md)

## The workstation at a glance

| Workbench | What you can do |
|---|---|
| **Processor lab** | Inspect CPU identity, logical processors, memory and usable ISA capabilities |
| **Benchmark suite** | Run bounded processor, memory and matrix workloads for local comparison |
| **Source review** | Scan supported source files without executing or rewriting them; export JSON reports |
| **Audio / DSP** | Read and write PCM16 WAV, generate test tones, process audio and inspect levels |
| **Windows desktop** | Use the native Win32 GUI for processor scans, source review and audio controls |
| **Engineering catalogue** | Explore improvement classifications, advisory hardware rules and reference plans |
| **Verification** | Build with CMake and run CTest on Windows and Linux |

## See the interface

<p align="center"><img src="assets/pytyliki-workstation-preview.svg" alt="Illustrative preview of the current Pytyliki Win32 workstation layout" width="100%"></p>

<sub>Illustrative interface preview reconstructed from the checked-in Win32 controls and labels; it is not a screenshot captured from a live Windows session.</sub>

The desktop application exposes CPU capability scans, processor suites, source review, PCM WAV import/export, tone generation, low-pass filtering, compression, saturation, gain, metering and JSON report export.

## How the pieces fit

<p align="center"><img src="assets/pytyliki-architecture.svg" alt="Pytyliki architecture: inspect, analyze, process, verify" width="100%"></p>

## Quick start

### Requirements

- Windows or Linux
- CMake 3.16 or newer
- A C11-capable compiler
- Git

### Build and test

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

On Windows, the build also produces **`pytyliki-gui.exe`**.

### Useful commands

```bash
# Read-only processor profile
./build/pytyliki --profile

# Bounded integer, memory and matrix suite
./build/pytyliki --suite

# Inspect the improvement catalogue
./build/pytyliki --improvement-plan

# Generate a local 48 kHz PCM16 WAV test tone
./build/pytyliki --tone-out test-tone.wav --frequency 440 --seconds 3

# Write a machine-readable local report
./build/pytyliki --json report.json
```

Use the platform-specific executable path on Windows (for example, `build/Release/pytyliki.exe`).

## Engineering capabilities

- **Processor inventory:** CPU vendor/model, architecture, logical processor count, memory and ISA flags. AVX-family usability includes operating-system vector-state checks.
- **Bounded measurements:** repeatable local workloads for processor, memory and matrix paths. Scores are workload- and platform-dependent, not certification.
- **Static source diagnostics:** review supported source files and produce findings without running or modifying the files.
- **PCM16 audio tools:** WAV import/export, test-tone generation, low-pass, compressor, soft saturation, gain, aggregate metering and independent per-channel peak/RMS/DC/crest/clipping metrics.
- **Syntax-pie:** a deterministic 20-layer catalogue linking function verbs and syntax abstractions; it is a lookup catalogue, not a general-purpose parser.
- **Improvement matrix:** 20 engineering, 20 optimization and 20 automation classifications, displayed by `--improvement-plan`.
- **HyperFusion:** six bounded analysis modules and 36 directed pair interactions per round. Readiness and synergy are heuristic planning indicators, not measured speedups.
- **APSIA 2.0:** a language-routing planner with target-availability and FFI-cost gates. It proposes routes; it does not compile or execute the selected language module.
- **Hardware rules:** 12 implemented advisory rules; other catalogue identifiers are reserved. Findings are prompts for engineering review, not hardware certification.
- **Reference pipeline:** a C11/C23 planning model, not a production transpiler.

## CI, quality and safety

The GitHub Actions workflow builds and tests on **Ubuntu and Windows**. A separate Linux job runs the test suite with AddressSanitizer and UndefinedBehaviorSanitizer. CMake/CTest results are the source of truth for a given commit.

Pytyliki follows explicit local-operation boundaries:

- Processor discovery is read-only. No firmware, BIOS/UEFI, microcode, clock, voltage, power-limit, kernel, driver or device-register writes are performed.
- Source review does not execute or rewrite scanned files.
- Benchmarks are bounded and depend on hardware, operating system, compiler and workload; no speedup is promised.
- Advisory scores and planning catalogues are not hardware certification or proof of autonomous optimization.
- Local audio processing operates on bounded PCM16 WAV buffers.

## Compatibility

| Item | Current scope |
|---|---|
| Implementation | ISO C11 |
| Validated CI platforms | Windows and Linux |
| Desktop interface | Native Win32 GUI on Windows |
| Data formats | JSON reports; PCM16 WAV audio |
| Build and tests | CMake + CTest |
| Not claimed | macOS support, guaranteed ARM64/x86_64 performance, production RTL/FPGA synthesis or an autonomous source transformer |

## Documentation

- [HMGA v2.0 engineering contract](docs/HMGA_V2.0.md)
- [HMGA v3.0 bounded operator matrix](docs/HMGA_V3.md)
- [Engineering standard](docs/ENGINEERING_STANDARD.md)


- [Architecture](docs/ARCHITECTURE.md)
- [APSIA 2.0 routing design](docs/APSIA_2.0.md)
- [Hardware rule engine](docs/HARDWARE_RULE_ENGINE.md)
- [Processor taxonomy](docs/PROCESSOR_TAXONOMY.md)
- [Platform validation policy](docs/PLATFORM_VALIDATION_POLICY.md)
- [Recursive evolution model](docs/RECURSIVE_EVOLUTION.md)
- [Quality audit](docs/QUALITY_AUDIT_0.7.1.md)

## Updated interface previews

<p align="center"><img src="assets/pytyliki-workstation-preview.svg" alt="Illustrative Win32 workstation preview using the white-red-blue theme" width="100%"></p>
<p align="center"><img src="assets/pytyliki-audio-meters-preview.svg" alt="Illustrative stereo audio metering preview with independent channel measurements" width="100%"></p>

<sub>These are source-aligned vector previews, not screenshots captured from a running Windows session. A genuine application capture requires running the Win32 build on Windows.</sub>

## Consolidated engineering updates

The current integration adds RIFF-container boundary validation for PCM16 WAV input, independent per-channel peak/RMS/DC/crest/clipping measurements, the bounded 20 × 20 HMGA v3 planning catalogue, a read-only HMGA repository audit, and the white-red-blue Win32 control theme. The HMGA matrix enumerates 400 descriptors with a maximum of 64 planning rounds; it does not generate or execute code.

## License

Released under the [MIT License](LICENSE).
