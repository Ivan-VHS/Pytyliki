# Pytyliki — Processor Systems Workstation

[CI](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) · C11 · Windows/Linux · MIT

Modular native workstation for processor discovery, bounded workloads, source diagnostics, numerical tools and local DSP. The Windows GUI and portable CLI share one C11 core.

| Module | Scope | Safety boundary |
|---|---|---|
| CPU | identity, topology, memory, ISA/XCR0 | read-only |
| Benchmarks | CPU, memory, matrix, checked partitioning | bounded local comparison |
| Source review | static diagnostics and reports | no build or execution |
| DSP | PCM16 WAV, filter, compression, saturation, metering | local buffers |
| Memory | typed views, checked slices, arenas, pools | fixed limits |
| Embedded C | MCU profiles, FLASH/SRAM, MMIO model | simulation only |
| Rules/TRACE | advisory routing and measurement provenance | evidence, not certification |
| QCS | lexer, parser, AST, diagnostics | syntax-only |
| Interop | versioned C ABI v1 | explicit library selection |

Build: CMake 3.16+, C11 compiler, Git.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Useful commands: `pytyliki --profile`, `--suite`, `--systems`, `--embedded profiles`, `pytyliki-metrics --demo`.

| Catalogue | Count |
|---|---:|
| Systems-language capabilities | 66 |
| CPU/MCU/virtual-ISA profiles | 14 |
| Systems-lab functions | 33 |

Division CLI: `--divide A B`, `--partition ITEMS WORKERS GRAIN`, `--partition-auto ITEMS GRAIN`. Partition plans are bounded and advisory; they do not launch threads or set CPU affinity.

[Features](docs/FEATURES.md) · [Architecture](docs/ARCHITECTURE.md) · [Division](docs/DIVISION_ENGINE.md) · [TRACE](docs/TRACE_MEASUREMENT_PROVENANCE.md) · [Docs index](docs/README.md)

[Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md) · [Releases](https://github.com/Ivan-VHS/Pytyliki/releases) · [MIT](LICENSE)
