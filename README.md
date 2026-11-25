# 🧮 Pytyliki — Processor Systems Workstation

<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

> **Processor insight with hard limits. Native measurements. Evidence over guesswork.**

**State:** Development · bounded C11 core · **Version:** 0.8.0 · CMake project · **License:** [MIT](LICENSE) · **README standard:** Psychoporter 666 v1.0 · **Last reviewed:** 2026-11-25 · UTC · **Source:** [Ivan-VHS/Pytyliki](https://github.com/Ivan-VHS/Pytyliki)

<p align="center"><img src="assets/pytyliki-hero.svg" alt="Pytyliki — Processor Systems Workstation — project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki — Processor Systems Workstation — application screenshot" width="100%"></a></p>

## Quick navigation

[🚀 Demo: workstation screenshot](assets/screenshots/pytyliki-workstation.png) · [Documentation](docs/README.md) · [Source repository](https://github.com/Ivan-VHS/Pytyliki) · [Issues](https://github.com/Ivan-VHS/Pytyliki/issues)

## Mission & scope

A local processor-systems workstation built around a bounded C11 core, a Windows-only Win32 desktop interface and portable command-line tools. Hardware discovery is read-only. Static source review does not compile, execute or rewrite inspected files.

## Capabilities

- Read-only CPU topology, ISA and memory inventory.
- Bounded CPU, memory and matrix workloads with timing statistics; results depend on the host machine.
- Profile import, folded stacks and SVG flamegraphs.
- Static source diagnostics, QCS syntax inspection and reference-only MCU FLASH/SRAM estimates.
- Versioned C ABI v1 with caller-owned buffers and structured reports with provenance.

## Architecture & data flow

~~~mermaid
flowchart LR
  U["User / CLI / Win32 UI"] --> C["C11 core"]
  C --> D["Read-only discovery"]
  C --> B["Bounded measurements"]
  C --> S["Static source and QCS checks"]
  D --> R["Structured evidence report"]
  B --> R
  S --> R
  C --> A["Versioned C ABI v1"]
~~~

Source code, schemas, build files, tests and CI are the evidence for implemented behavior. Roadmap entries and catalogue records describe plans or inventories, not shipped functionality.

## Requirements

CMake 3.16+, a C11 compiler (MSVC, GCC or Clang) and Git. Core and CLI targets are portable across the declared Windows/Linux runtime set; the Win32 GUI is Windows-only. Python is optional test tooling, not a runtime requirement.

## Build

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

The Win32 GUI target is Windows-only. The C11 core and CLI are the portable entry points.

## First checks

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
~~~

Commands print local, machine-dependent results; they do not tune the operating system.

## Run

Use the portable CLI for local checks; the Win32 graphical interface is available in Windows builds.

## Interfaces

| Surface | Contract / entry point | Boundary |
|---|---|---|
| CLI | `pytyliki / pytyliki-metrics / pytyliki-qcs` | Local commands; see --help and feature docs. |
| Native FFI | `pytyliki_ffi / C ABI v1` | Explicitly selected library; caller-owned buffers. |
| Reports | `Versioned JSON/text` | Provenance and limitations are part of the report contract. |

REST/GraphQL, WebSocket, OAuth, cloud deployment and cookie mechanisms are not advertised without a versioned implementation and tests.

## Security & trust boundaries

No firmware, BIOS/UEFI, microcode, clock, voltage, driver or device-register writes are performed. Embedded MMIO descriptions are simulated/reference-only. Native FFI must use an explicitly trusted library path.

Security reporting: [SECURITY.md](SECURITY.md). Do not publish secrets, device identifiers or private logs in public issues.

## Verification

The configured CTest suite is run by the command in the Build section; no additional verification command is required.

The checked-in CTest/unittest targets and workflows define the test scope. No coverage percentage, fuzz result or load-test report is claimed without a published artifact.

## Performance evidence

Benchmark outputs are host-dependent and intended for local comparison. The repository documents statistical methods and archived samples; no universal latency, memory-footprint or cross-machine score is promised.

## Compatibility

Windows and Linux runtime targets are declared; the Win32 GUI is Windows-only. The C11 core and CLI are the portable entry points.

## Documentation & ecosystem

- [Feature matrix](docs/FEATURES.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Benchmark statistics](docs/BENCHMARK_STATISTICS.md)
- [Report envelope](docs/REPORT_ENVELOPE.md)
- [Embedded C](docs/EMBEDDED_C.md)
- [QCS](docs/QCS_NATIVE.md)
- [Documentation index](docs/README.md)
- [Reaper integration](docs/REAPER_INTEGRATION.md)

Feature matrix, architecture, benchmark statistics and the report envelope are the primary technical references.

Cross-project integrations use documented versioned interfaces and explicit adapters. An optional adapter does not grant another project runtime or hardware privileges.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before changing code or contracts. Keep changes reviewable, add deterministic tests and use Conventional Commits where required by the repository guide.

## License & legal

Distributed under [MIT](LICENSE). The software is provided “AS IS” under the license terms. Third-party project names and trademarks remain with their respective owners.

This repository documents local software interfaces. Hosted-service terms, cookie consent, OAuth and CORS are not applicable to the runtime surfaces described above unless a separate hosted component is introduced and documented.

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 · UTC</sub></p>
