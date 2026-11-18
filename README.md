# Pytyliki
### C11 processor-systems workstation

<p align="center"><img src="assets/brand/hero.svg" alt="Pytyliki workstation" width="100%"></p>
<p align="center"><strong>DISCOVER · ANALYZE · OPTIMIZE · VERIFY</strong><br><img src="https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge" alt="C11"> <img src="https://img.shields.io/badge/Windows-Linux-DC2626?style=for-the-badge" alt="Platforms"> <img src="https://img.shields.io/badge/License-MIT-111827?style=for-the-badge" alt="MIT"></p>

**Pytyliki is a modular C11 workstation for read-only processor discovery, bounded local benchmarks, source diagnostics, and PCM16 WAV/DSP utilities.** It provides a Windows/Linux CLI and a native Win32 GUI.

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

> [!IMPORTANT]
> Pytyliki does not reprogram processors, write firmware, change clocks or voltage, or guarantee performance gains.

## Capabilities

| Area | Included |
|---|---|
| Processor | Read-only CPU and ISA inventory |
| Workloads | Bounded processor, memory and matrix tests |
| Analysis | Read-only source scanner and JSON reports |
| Audio / DSP | PCM16 WAV I/O, tone generation, processing and metering |
| Interfaces | Cross-platform CLI and native Windows GUI |
| Planning | 20-capability catalogue and evidence-gated planning |
| HyperFusion | Six bounded analysis modules |
| CI | CMake/CTest on Windows and Linux |

<p align="center"><img src="assets/brand/architecture.svg" alt="Pytyliki architecture" width="100%"></p>

## Quick start

Requirements: Git, CMake and a C11-capable compiler on Windows or Linux.

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows builds also provide `pytyliki-gui.exe`.

```bash
pytyliki --improvement-plan
pytyliki --evolve --evolution-rounds 8
pytyliki --syntax-pie
pytyliki --mtte
```

<p align="center"><img src="assets/brand/terminal-preview.svg" alt="Illustrative CLI workflow" width="100%"></p>
<sub>Command-line presentation mockup; not a captured run or benchmark result.</sub>

## Architecture

| Module | Responsibility |
|---|---|
| `src/core.c/.h` | CPU profile, bounded benchmark, scanner, JSON report |
| `src/processor.c/.h` | Processor and ISA inventory and tests |
| `src/dsp.c/.h` | Bounded WAV and DSP operations |
| `src/improvement.c/.h` | Improvement catalogue and lifecycle |
| `src/hw_rules.c/.h` | Advisory hardware rules |
| `src/evolution.c/.h` | HyperFusion analysis |
| `src/pipeline.c/.h` | C11/C23 reference pipeline plan |
| `src/policy.c/.h` | Local authorization policy |
| `src/cli.c`, `src/win_gui.c` | CLI and Windows GUI |

### Capability status

- **Syntax-pie:** deterministic 20-layer function/syntax catalogue and lookup layer; not a general-purpose parser.
- **MTTE:** 20 semantic/function/mechanical layers and a logical tick counter; it models software scheduling and does not alter hardware timers, CPU frequency, registers, firmware or voltage.
- **Improvement matrix:** 20 engineering, 20 optimization and 20 automation classifications.
- **HyperFusion:** six bounded analysis modules and 36 directed pair interactions per round. Readiness/synergy values are heuristics, not measured speedups.
- **APSIA 2.0:** C11 language-routing planner with target-availability and FFI-cost gates; it proposes routes but does not compile or execute selected modules.
- **Hardware rules:** 12 advisory rules are implemented; other identifiers are reserved. The C11/C23 pipeline is a reference plan, not a production transpiler.

## CI and verified Auto-Merge

Pull requests targeting `main` use CMake/CTest validation on Ubuntu and Windows. The verified Auto-Merge workflow checks that the current PR head matches the validated commit and requires an independent approval tied to that head before requesting GitHub squash Auto-Merge.

Enable **Allow auto-merge**, protect `main`, require relevant checks and independent review, and scope write permissions to the merge job. An Auto-Merge request is not proof that a PR has merged.

## Compatibility and safety

- Validated CI targets: Windows and Linux. Implementation: ISO C11; C23 appears only in the reference pipeline plan.
- Data formats: JSON reports and PCM16 WAV.
- Processor discovery and source scanning are read-only.
- Workloads have explicit limits; benchmark results depend on compiler, OS, hardware and workload.
- No writes to firmware, BIOS/UEFI, microcode, voltage, clocks, power limits, kernels, drivers or device registers.
- Advisory rules and planning scores are not hardware certification or performance guarantees.

## Documentation

See [architecture](docs/ARCHITECTURE.md), [APSIA 2.0](docs/APSIA_2.0.md), [platform validation policy](docs/PLATFORM_VALIDATION_POLICY.md) and [quality audit](docs/QUALITY_AUDIT_0.7.1.md). Workflow results are authoritative for an individual commit.

## License

MIT — see [LICENSE](LICENSE).
