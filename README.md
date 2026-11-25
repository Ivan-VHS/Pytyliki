# Pytyliki — Processor Systems Workstation

**A local processor-systems workstation built around a bounded C11 core, a Windows desktop interface and portable command-line tools.**

<p align="center">
  <img src="assets/readme/dashboard.svg" alt="Pytyliki architecture and capability overview" width="100%">
</p>

<p align="center">
  <a href="assets/screenshots/pytyliki-workstation.png">
    <img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki running on Windows" width="100%">
  </a>
</p>

<p align="center">
  [![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml)
  [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml)
  [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)
</p>

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

**Scope:** processor and file inspection, bounded local analysis and advisory reports. Hardware discovery is read-only; Pytyliki does not change OS scheduling, affinity, voltage, frequency or firmware.

## Data and analysis flow

```mermaid
flowchart TD
  HOST["Local host and selected files"] --> CORE["Bounded C11 core"]
  CORE --> CPU["CPU topology, ISA and memory inventory"]
  CORE --> PROF["Profile import and stack analysis"]
  CORE --> SRC["Source diagnostics and QCS inspection"]
  CPU --> REPORT["Structured metrics and advisory reports"]
  PROF --> FLAME["Folded stacks and SVG flamegraphs"]
  SRC --> REPORT
  CORE --> CLI["Portable command-line tools"]
  CORE --> GUI["Windows desktop interface"]
```

## Capabilities

| Work area | What it provides |
|---|---|
| Processor inventory | CPU topology, ISA and memory inventory |
| Bounded measurements | Local CPU/memory measurements and workload-partition examples |
| Profiling | Profile import, stack analysis, folded stacks and SVG flamegraphs |
| Source inspection | Source diagnostics and QCS syntax inspection |
| Embedded reference | MCU FLASH/SRAM reference estimates |
| Reports and ABI | Versioned C ABI, structured reports, provenance and explicit limitations |

Read more in [Features](docs/FEATURES.md), [Architecture](docs/ARCHITECTURE.md), [Benchmark statistics](docs/BENCHMARK_STATISTICS.md), [Embedded C](docs/EMBEDDED_C.md) and [QCS](docs/QCS_NATIVE.md).

## Build and test

Requirements: CMake 3.16+, a C11 compiler (MSVC, GCC or Clang) and Git. The Win32 GUI is Windows-only; the core and CLI are portable. Python is optional for extra repository-contract checks, not for the C11 runtime.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

## First CLI checks

```sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
```

## Safety and integration boundaries

- Hardware discovery is read-only.
- Reports are advisory and retain provenance and limitations.
- Workload partition examples do not alter the host scheduler or process affinity.
- Embedded FLASH/SRAM values are reference estimates, not a device programming operation.
- [Reaper policy integration](docs/REAPER_INTEGRATION.md) is an optional CI validation example, not an application runtime dependency.

Documentation and integration: [Documentation index](docs/README.md) · [Release process](docs/RELEASE.md) · [Report envelope](docs/REPORT_ENVELOPE.md) · [Shared ecosystem standards](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/SHARED_STANDARDS.md).

[Contributing](CONTRIBUTING.md) · [Security policy](SECURITY.md) · [MIT License](LICENSE)
