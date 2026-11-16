# 🔵🔴 Pytyliki — Processor Systems Workstation

<p align="center">

**DISCOVER · ANALYZE · OPTIMIZE · VERIFY**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![OS](https://img.shields.io/badge/OS-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

**One C11 workstation. Clear evidence. Safer experiments.**

</p>

**Languages:** English · [简体中文](README.zh-CN.md) · [Русский](README.ru.md) · [Українська](README.uk.md)

Pytyliki is a modular C11 workstation for processor discovery, bounded performance experiments, source inspection and local DSP workflows. It combines CLI tools, a native Windows GUI and testable engineering components.

## At a glance

| Area | Capabilities |
|---|---|
| CPU & ISA | Processor and instruction-set discovery; runtime checks must also validate OS-enabled state |
| Benchmarks | Bounded local workloads; results are advisory and hardware-dependent |
| Source analysis | Read-only scanning and structured JSON reports |
| DSP | PCM16 WAV handling, tone generation and local signal processing |
| Interfaces | CLI for Windows/Linux; native Win32 GUI |
| Engineering | CMake/CTest, evidence-gated planning and deny-by-default policy |
| CPU Suite Pro 78X | 78-module catalogue, stable IDs, six domains, lookup API and advisory plans |

## CPU Suite Pro 78X

Inspired by classic integrated software suites, CPU Suite Pro organizes 78 specialist tools around a shared CPU Hub.

| Domain | Module IDs |
|---|---:|
| Hub & workstation tools | 1–14 |
| Architecture intelligence | 15–26 |
| Thermal & power | 27–38 |
| Memory & cache | 39–50 |
| Scheduling & concurrency | 51–60 |
| Diagnostics & benchmark lab | 61–78 |

**Implemented:** deterministic 78-entry registry; unique numeric IDs; six domain labels; allocation-free enumeration; lookup by index, ID and name; advisory plan generation; CTest coverage.

**Scope boundary:** 78 catalogue entries are not 78 finished hardware-control applications. Live sensor adapters, vendor-specific tuning, OS performance-counter integrations and individual GUI panels require separate implementation and validation. The current planner does not write CPU registers or alter voltage, clocks, firmware, microcode, fans, interrupts or kernel settings.

See the [CPU Suite Pro 78X technical specification](docs/CPU_SUITE_PRO_78.md).

## Architecture

- **Language:** ISO C11; C23 appears only in a reference pipeline plan.
- **Build and tests:** CMake and CTest.
- **Interfaces:** cross-platform CLI and native Windows GUI.
- **Core workflows:** processor inventory, bounded benchmarks, source scanner, JSON reporting and PCM16/DSP processing.
- **Change control:** evidence-gated changes, reviewable integration and deny-by-default authorization.

## Build and test

Requirements: CMake, a C11-capable compiler, Windows or Linux.

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows builds also define the native `pytyliki-gui` target.

## Quality, compatibility & limits

- Check current workflow runs for CI status; this README does not assume tests passed.
- Results vary with CPU, firmware, OS, compiler, cooling and workload; no speedup is guaranteed.
- ISA flags alone do not prove safe execution; validate CPU features and OS-enabled vector state at runtime.
- Stress workloads need explicit bounds and stop conditions.
- No macOS, ARM64 or x86_64-specific performance guarantee is claimed.
- Protobuf, Parquet, Arrow and production RTL/FPGA synthesis are outside validated scope unless separately tested.
- CPU Suite Pro does not automatically tune voltage/clocks, flash firmware, disable microcode mitigations or write privileged registers.

## Project map

- `src/cpu_suite.h/.c` — 78-module catalogue and advisory planning API.
- `tests/test_cpu_suite.c` — catalogue, uniqueness, lookup and invalid-input tests.
- `docs/CPU_SUITE_PRO_78.md` — domains, API and safety contract.
- `src/` — core, processor, DSP, policy, improvement, CLI and GUI components.
- `CMakeLists.txt` — library, applications and CTest targets.

## License

MIT — see [LICENSE](LICENSE).

<p align="center">

**愿你所爱，皆有回响。**  
*May everything you love find its echo.*

🔵 **Build with clarity. Improve with evidence.** 🔴

</p>
