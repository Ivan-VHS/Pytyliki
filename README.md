# Pytyliki — Processor Systems Workstation

[CI](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) · [Windows desktop build](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) · C11 · Windows/Linux · MIT

Pytyliki is a modular native workstation for processor discovery, bounded workload planning, source diagnostics, numerical tools and local DSP. The Windows GUI and portable command-line tools share one C11 core.

## Live Windows workstation

<p align="center"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki workstation running on Windows" width="100%"></p>
<sub>Actual Windows desktop capture produced by the desktop-build workflow (1920×1080).</sub>

## Capability matrix

| Module | Current scope | Safety boundary |
|---|---|---|
| CPU | identity, topology, memory, ISA/XCR0 and processor profiles | read-only discovery |
| Benchmarks | CPU, memory, matrix and checked workload partitioning | bounded local measurements |
| Source review | static diagnostics and structured reports | does not build or execute inspected source |
| DSP | PCM16 WAV, filtering, compression, saturation and metering | local buffers; validated RIFF chunk bounds |
| Memory | typed views, checked slices, arenas and pools | fixed limits and checked ranges |
| Embedded C | MCU profiles, FLASH/SRAM estimates and MMIO model | simulation/reference data only |
| Rules / APSIA / TRACE | advisory routing, workload plans and measurement provenance | evidence, not certification |
| QCS | lexer, parser, AST and diagnostics | syntax analysis only |
| Interop | versioned C ABI v1 | explicit trusted library selection |

## Build and test

Requirements: CMake 3.16+, a C11 compiler and Git.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

The Windows desktop workflow builds the GUI and CLI tools, runs CTest, and publishes a Windows x64 archive. The current archive contains `pytyliki-gui.exe`, `pytyliki.exe`, `pytyliki-apsia.exe` and `pytyliki-metrics.exe`.

## Command-line examples

```sh
pytyliki --profile
pytyliki --suite
pytyliki --systems
pytyliki --embedded profiles
pytyliki-metrics --demo

pytyliki --divide -17 5
pytyliki --partition 10000 8 256
pytyliki --partition-auto 10000 256
```

Partition plans are deterministic, bounded and advisory. They describe contiguous work ranges; they do not launch threads, change CPU affinity or claim an unmeasured speedup.

## Catalogues

| Catalogue | Entries |
|---|---:|
| Systems-language capabilities | 66 |
| CPU / MCU / virtual-ISA profiles | 14 |
| Systems-lab functions | 33 |

[Features](docs/FEATURES.md) · [Architecture](docs/ARCHITECTURE.md) · [Division and partitioning](docs/DIVISION_ENGINE.md) · [TRACE provenance](docs/TRACE_MEASUREMENT_PROVENANCE.md) · [Documentation index](docs/README.md)

[Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md) · [Releases](https://github.com/Ivan-VHS/Pytyliki/releases) · [MIT](LICENSE)
