# Pytyliki — Processor Systems Workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

> A local C11 workstation for processor discovery, bounded measurements, source diagnostics and versioned native interoperability.

**Project record:** Development · CMake project version 0.8.0 · C11 core · Windows desktop GUI · Windows/Linux core and CLI · MIT · **Documentation reviewed:** 2026-11-27 UTC

<p align="center"><img src="assets/readme/dashboard.svg" alt="Pytyliki project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki workstation on Windows" width="100%"></a></p>

## Purpose and scope

Pytyliki collects and analyses processor-system evidence locally. Its C11 core and command-line tools provide read-only host discovery, bounded CPU and memory workloads, static source inspection and numerical metrics. The Win32 graphical application is Windows-only; the core and CLI are built for Windows and Linux.

Hardware discovery does not change CPU scheduling, affinity, voltage, frequency, firmware, drivers or device configuration. Benchmarks are local observations, not universal performance rankings.

## Capabilities

- **Processor inventory:** CPU topology, usable instruction-set features and memory information, including operating-system vector-state checks.
- **Bounded measurements:** repeatable CPU, memory and matrix workloads with explicit run limits and structured results.
- **Source inspection:** static diagnostics and QCS syntax validation; inspected source is not executed or rewritten.
- **Numerical metrics:** C11 vector, distance, geometry and quadratic-form operations through a stable native API.
- **Memory utilities:** typed views, overflow-checked slices, bounded arenas, rewind checkpoints and fixed-block pools.
- **Native interoperability:** versioned C ABI v1 with caller-owned buffers; consumers must explicitly select a trusted library.
- **Embedded references:** MCU profiles, memory estimates, starter generation and simulated MMIO masks. These are reference tools, not physical memory mapping, device access or firmware flashing.
- **Toolchain inventory:** read-only discovery of available tools; discovered programs are not installed or launched.

- **Reaper Robotics:** thirteen deterministic operation groups for sensor processing, geometry, kinematics, trajectories, actuator limits, PID control, axis synchronization, safety supervision, packet framing and simulation. Its register bank is an in-memory simulator and never accesses physical MMIO.

PCM16 WAV/DSP processing is not part of the current delivered Pytyliki core. See the project contract and feature documentation before depending on partial or planned capabilities.

## Build and verify

Requirements: CMake 3.16+, a C11 compiler (MSVC, GCC or Clang) and Git. The Win32 GUI is built on Windows. Python is optional for supplementary repository-contract tests; it is not the C11 runtime.

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

Useful first runs on Linux:

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
~~~

On Windows, use the corresponding executable under the selected CMake configuration directory, for example `build/Release/pytyliki.exe`.

For GCC/Clang builds, AddressSanitizer and UndefinedBehaviorSanitizer can be enabled with `-DPYTYLIKI_ENABLE_SANITIZERS=ON`.

## Interfaces and ecosystem

| Interface | Role | Boundary |
|---|---|---|
| `pytyliki.c-abi.v1` | Native metrics API | Explicit library path; caller-owned buffers |
| `octa.pyramid.peer-mesh.v1` | Peer contract | Versioned metadata and explicit requests |
| Reaper | Optional metrics consumer | Native FFI is opt-in and requires a trusted ABI-compatible library |
| OPlague | Optional metrics consumer | Explicit trusted-library selection; no implicit loading |

Integrations do not grant hardware privileges or broaden the workstation's read-only discovery scope.

## Documentation

- [Feature overview](docs/FEATURES.md) · [Architecture](docs/ARCHITECTURE.md)
- [Benchmark statistics](docs/BENCHMARK_STATISTICS.md) · [Embedded C reference workbench](docs/EMBEDDED_C.md)
- [QCS native inspection](docs/QCS_NATIVE.md) · [Documentation index](docs/README.md) · [Reaper Robotics](docs/REAPER_ROBOTICS.md)
- [Release process](docs/RELEASE.md) · [Report envelope](docs/REPORT_ENVELOPE.md)
- [Reaper integration](docs/REAPER_INTEGRATION.md) · [Shared ecosystem standards](https://github.com/Ivan-VHS/DAK/blob/main/docs/UNIFIED_DOCUMENTATION_STANDARD.md)

## Security and contribution

Treat imported profiles and source files as untrusted input. Keep analysis bounded and deterministic. Do not add hidden network activity, privileged hardware changes, automatic tool installation or firmware/device writes.

Read [CONTRIBUTING.md](CONTRIBUTING.md) before proposing changes. Report security issues according to [SECURITY.md](SECURITY.md). This repository is licensed under [MIT](LICENSE).

---
<p align="center"><sub>Pytyliki · README review 2026-11-27 UTC</sub></p>
