# Pytyliki — Processor Systems Workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Pytyliki project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki running on Windows" width="100%"></a></p>

Pytyliki is a local processor-systems workstation built around a bounded C11 core, a Windows desktop interface and portable command-line tools.

**Scope:** processor and file inspection, bounded local analysis and advisory reports. Hardware discovery is read-only; the application does not change OS scheduling, affinity, voltage, frequency or firmware.

## Capabilities

- CPU topology, ISA and memory inventory; bounded CPU/memory measurements and workload-partition examples.
- Profile import and analysis, folded stacks and SVG flamegraphs.
- Source diagnostics, QCS syntax inspection and embedded-MCU FLASH/SRAM reference estimates.
- Versioned C ABI and structured reports with provenance and explicit limitations.

See [Features](docs/FEATURES.md), [Architecture](docs/ARCHITECTURE.md), [Benchmark statistics](docs/BENCHMARK_STATISTICS.md), [Embedded C](docs/EMBEDDED_C.md) and [QCS](docs/QCS_NATIVE.md).

## Build and test

Requirements: CMake 3.16+, a C11 compiler (MSVC, GCC or Clang) and Git. The Win32 GUI is Windows-only; the core and CLI are portable. Python is optional for extra repository-contract checks, not for the C11 runtime.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

First checks:

```sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
```

## Specifications and integration

- [Documentation index](docs/README.md) · [Release process](docs/RELEASE.md) · [Report envelope](docs/REPORT_ENVELOPE.md)
- [Reaper policy integration](docs/REAPER_INTEGRATION.md) is an optional CI validation example, not an application runtime dependency.
- [Shared ecosystem standards](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/SHARED_STANDARDS.md)

[Contributing](CONTRIBUTING.md) · [Security policy](SECURITY.md) · [MIT License](LICENSE)
