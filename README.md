# Pytyliki — C11 Processor Systems Workstation

**TL;DR:** Pytyliki is a free, MIT-licensed C11 processor workstation for Windows and Linux, combining CPU/ISA discovery, bounded benchmarks, source analysis, local PCM16 audio/DSP tools, and a native Windows GUI.

**Language:** English · [简体中文](README.zh-CN.md) · [Русский](README.ru.md) · [Українська](README.uk.md)

<p align="center">

**DISCOVER · ANALYZE · OPTIMIZE · VERIFY**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![Platforms](https://img.shields.io/badge/OS-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

</p>

Pytyliki helps systems programmers, hardware enthusiasts, and C developers inspect processor capabilities and run bounded local experiments without claiming to reprogram physical silicon.

## Why Pytyliki?

Processor information, source-level hints, benchmarks, and local DSP utilities often live in separate tools. Pytyliki brings these workflows together in a native C11 codebase with explicit safety boundaries and testable outputs.

**Unique value:** one auditable workstation for read-only processor discovery, bounded experiments, source diagnostics, and evidence-gated engineering workflows.

## Features

- **CPU and ISA discovery:** collect processor and architecture information; treat feature flags as hints, not proof of OS-enabled instruction state.
- **Bounded benchmarks:** run local processor tests with controlled workloads; results are advisory and hardware-dependent.
- **Source analysis:** scan source code and produce JSON reports without executing or rewriting scanned files.
- **Audio and DSP:** read and write bounded PCM16 WAV data, generate tones, and run local digital signal processing.
- **Desktop and CLI:** native Win32 graphical interface plus command-line workflows on Windows and Linux.
- **Improvement planning:** inspect a 20-capability engineering/optimization/automation catalogue and evidence-gated iteration plans.

## Current architecture

| Area | Implementation |
|---|---|
| Core | CPU profile, bounded benchmark, source scanner, JSON report |
| Processor | Architecture/ISA inventory and bounded execution, memory, and matrix tests |
| DSP | PCM16 WAV I/O, tone generation, processing, and metering |
| Improvement | 20×3 capability catalogue and evidence-gated lifecycle |
| Hardware rules | 12 implemented advisory rules; other catalogue IDs are reserved |
| HyperFusion | Six bounded analysis modules and 36 directed pair interactions per round |
| Pipeline | C11/C23 reference pipeline plan, not a production transpiler |
| Policy | Deny-by-default local authorization policy |
| Interfaces | Cross-platform CLI and native Windows GUI |

HyperFusion readiness and synergy values are heuristic planning indicators, not measured speedups or independent CI certification.

## Quick start

### What do I need?

- CMake and a C11-capable C compiler.
- Windows or Linux.
- Git to clone the repository.

### How do I build and test Pytyliki?

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows builds also provide `pytyliki-gui.exe`.

## CLI workflows

```bash
pytyliki --improvement-plan
pytyliki --evolve --evolution-rounds 8
```

Use the executable's help output to inspect the commands available in the checked-out version. Options can evolve between revisions.

## Before and after

| Separate workflow | Pytyliki workflow |
|---|---|
| Switch between CPU utilities, source scanners, and DSP tools | Use one C11 workstation and shared test/build workflow |
| Treat raw ISA flags as definitive | Keep OS-state and runtime eligibility caveats explicit |
| Rely on informal optimization claims | Record bounded inputs, outputs, and validation evidence |

## Quality and automation

The repository uses CMake and CTest, with Windows/Linux CI. Its improvement catalogue describes a target workflow; it does not mean every transformation is automatically applied.

The guarded integration model is:

1. Create an isolated feature branch.
2. Make a small, traceable change.
3. Build and run tests on supported CI platforms.
4. Obtain independent review and verify the tested commit.
5. Merge only through repository-authorized controls.

Auto-commit, auto-push, and auto-merge are separate permissioned operations. Failed checks must stop promotion. Configure GitHub branch protection to require the relevant CI checks and independent review; a workflow alone cannot prevent an authorized maintainer from manually bypassing repository policy.

## Safety and implementation limits

- CPU discovery is read-only.
- The scanner does not execute or rewrite scanned source.
- Pytyliki does not write firmware, BIOS/UEFI, microcode, voltage, clocks, power limits, kernels, drivers, or device registers.
- Hardware rules are advisory; only 12 rules are currently implemented.
- The C11/C23 pipeline is a reference plan, not a production transpiler.
- Benchmark results depend on hardware, operating system, compiler, and workload; they are not guaranteed performance gains.

## Project layout

- `src/core.c/.h` — CPU profile, benchmark, scanner, JSON report.
- `src/processor.c/.h` — processor and ISA inventory and test suite.
- `src/dsp.c/.h` — WAV and DSP operations.
- `src/improvement.c/.h` — improvement catalogue and lifecycle.
- `src/hw_rules.c/.h` — advisory hardware rules.
- `src/evolution.c/.h` — HyperFusion analysis.
- `src/pipeline.c/.h` — reference pipeline plan.
- `src/policy.c/.h` — local authorization policy.
- `src/cli.c`, `src/win_gui.c` — command-line and Windows desktop interfaces.
- `.github/workflows/` — CI and guarded repository automation.

## Compatibility and formats

- **Operating systems:** Windows and Linux are represented in the project CI matrix.
- **Language:** ISO C11 implementation; C23 appears in the reference pipeline plan.
- **Data:** JSON reports and PCM16 WAV audio.
- **Not claimed:** macOS, ARM64, x86_64-specific performance guarantees, Protobuf, Parquet, Arrow, or production RTL/FPGA synthesis unless separately validated.

## License

Pytyliki is distributed under the [MIT License](LICENSE).

---

<p align="center">

**ENGINEER WITH PRECISION · VERIFY EVERY CHANGE**

</p>
