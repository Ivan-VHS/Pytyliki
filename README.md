# 🔵🔴 Pytyliki 0.7.0 — Processor Systems Workstation

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

- CPU/ISA discovery, memory information and bounded processor benchmarks.
- Read-only source scanner and JSON reports.
- PCM16 WAV tools, tone generation and local DSP processing.
- Native Win32 GUI; CLI on Windows and Linux.
- Evidence-gated 20-capability improvement engine, 12 implemented advisory hardware rules, deny-by-default policy, C11/C23 reference pipeline and six-module HyperFusion coordinator.

- **CPU and ISA discovery:** collect processor and architecture information; treat feature flags as hints, not proof of OS-enabled instruction state.
- **Bounded benchmarks:** run local processor tests with controlled workloads; results are advisory and hardware-dependent.
- **Source analysis:** scan source code and produce JSON reports without executing or rewriting scanned files.
- **Audio and DSP:** read and write bounded PCM16 WAV data, generate tones, and run local digital signal processing.
- **Desktop and CLI:** native Win32 graphical interface plus command-line workflows on Windows and Linux.
- **Improvement planning:** inspect a 20-capability engineering/optimization/automation catalogue and evidence-gated iteration plans.
- **APSIA 2.0:** C11 polyglot routing planner with FFI cost gates and fail-closed sandbox selection.

## 🥧 Syntax-pie function layers

Run `pytyliki --syntax-pie` to inspect the deterministic 20-layer mapping from function verbs and room metaphors to syntax abstractions, from **Зал / Комната / Лексема** through **Залегендарить / Анфилада / Иерархическая грамматика**. The C API in `src/syntax_pie.h` exposes indexed lookup and lookup by function name, room name or syntax level. This is a catalogue and validation layer; it does not claim to parse or execute arbitrary source code.

## ⏱️ Mechanical Tick & Type Engine (MTTE)

Run `pytyliki --mtte` to inspect 20 linked layers. Each row binds a semantic operation (such as **Типизация**), a function designation (**Типизирующий**) and a mechanical tick operation (**Механизирующий**). The C API in `src/mtte.h` provides catalogue enumeration, indexed lookup, name lookup and a deterministic logical tick counter.

The counter uses an explicit nanosecond period, checks integer overflow and supports reset. It is a software model for scheduling and measurement logic: it does **not** change CPU frequency, hardware timers, processor registers, firmware or voltage.

| # | Semantic layer | Function layer | Mechanical operation |
|---:|---|---|---|
| 01 | Типизация | Типизирующий | Механизирующий |
| 02 | Классификация | Классифицирующий | Тактирующий |
| 03 | Категоризация | Категоризирующий | Тактогенерирующий |
| 04 | Таксономизация | Структурирующий | Тактосинхронизирующий |
| 05 | Спецификация | Классификационный | Тактосчитающий |
| 06 | Структуризация | Параметризующий | Тактоконтролирующий |
| 07 | Формализация | Атрибутирующий | Тактокорректирующий |
| 08 | Систематизация | Маркирующий | Тактокалибрующий |
| 09 | Кодирование | Аннотируемый | Тактопреобразующий |
| 10 | Маркировка | Идентифицирующий | Тактоделящий |
| 11 | Аннотирование | Дифференцирующий | Тактосуммирующий |
| 12 | Идентификация | Специфицирующий | Тактодетектирующий |
| 13 | Дифференциация | Формализующий | Тактопланирующий |
| 14 | Сигнатуризация | Нормализующий | Тактопрофилирующий |
| 15 | Параметризация | Кодирующий | Тактосогласующий |
| 16 | Атрибутизация | Сигнатурирующий | Тактопозиционирующий |
| 17 | Нормализация | Схематизирующий | Тактодискретизирующий |
| 18 | Схематизация | Унифицирующий | Тактопериодизирующий |
| 19 | Унификация | Метатипизирующий | Тактосеквенирующий |
| 20 | Метаклассификация | Автотипизирующий | Тактодиагностирующий |

## 🧬 HyperFusion six-module coordinator

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

## 🧩 Processor improvement capability matrix

The C11 catalogue exposes 20 engineering / optimization / automation triplets through `pytyliki --improvement-plan`.

| # | Engineering | Optimization | Automation |
|---:|---|---|---|
| 01 | Modernizing | Modernizing | Auto-optimizing |
| 02 | Optimizing | Optimizing | Auto-modernizing |
| 03 | Refining | Transpiling | Auto-adapting |
| 04 | Refactoring | Compiling | Auto-correcting |
| 05 | Transforming | Reconfiguring | Auto-compiling |
| 06 | Accelerating | Scaling | Auto-transpiling |
| 07 | Adapting | Accelerating | Auto-diagnosing |
| 08 | Scaling | Synthesizing | Auto-verifying |
| 09 | Automating | Decomposing | Auto-calibrating |
| 10 | Integrating | Integrating | Auto-tuning |
| 11 | Stabilizing | Adapting | Auto-configuring |
| 12 | Unifying | Automating | Auto-automating |
| 13 | Rationalizing | Parallelizing | Auto-integrating |
| 14 | Rebuilding | Verifying | Auto-scaling |
| 15 | Redesigning | Diagnosing | Auto-accelerating |
| 16 | Self-learning | Stabilizing | Auto-recovering |
| 17 | Self-tuning | Reconstructing | Auto-synthesizing |
| 18 | Self-recovering | Initializing | Auto-reconfiguring |
| 19 | Self-optimizing | Crystallizing | Auto-learning |
| 20 | Evolving | Perfecting | Auto-perfecting |

The catalogue is a capability/workflow taxonomy, not a claim that each transformation is autonomously applied. Changes still require tests, review and authorized integration.

## 🚀 20 × 3 Evolution Protocol

## 20 × 3 Evolution Protocol

The 60 operations below define the target engineering policy. They are a capability taxonomy, not a claim that every action is already autonomous.

| # | Core operation | Workflow operation | Quality operation |
|---:|---|---|---|
| 01 | Auto-improving | Auto-committing | Auto-optimizing |
| 02 | Auto-modernizing | Auto-pushing | Auto-refactoring |
| 03 | Auto-optimizing | Auto-merging | Auto-modernizing |
| 04 | Auto-adapting | Auto-integrating | Auto-improving |
| 05 | Auto-correcting | Auto-branching | Auto-simplifying |
| 06 | Auto-perfecting | Auto-cloning | Auto-accelerating |
| 07 | Auto-developing | Auto-forking | Auto-adapting |
| 08 | Automating | Auto-synchronizing | Auto-correcting |
| 09 | Auto-diagnosing | Auto-deploying | Auto-decomposing |
| 10 | Auto-restoring | Auto-building | Automating |
| 11 | Auto-restructuring | Auto-testing | Auto-standardizing |
| 12 | Auto-calibrating | Auto-linting | Auto-perfecting |
| 13 | Auto-configuring | Auto-checking | Auto-rewriting |
| 14 | Auto-testing | Auto-scanning | Auto-transpiling |
| 15 | Auto-validating | Auto-analyzing | Auto-converting |
| 16 | Auto-checking | Auto-fixing | Auto-typing |
| 17 | Auto-rechecking | Auto-updating | Auto-modularizing |
| 18 | Auto-compiling | Auto-generating | Auto-deduplicating |
| 19 | Auto-transpiling | Auto-documenting | Auto-hardening |
| 20 | Auto-evolving | Auto-integrating | Auto-evolving |

## APSIA 2.0 polyglot evolution

APSIA extends the C11/PREE/HyperFusion workflow with a guarded language-routing and promotion-evidence planner. CPU-bound work routes to Rust, GPU workloads to CUDA when explicitly available, concurrent I/O to Go, analytics to DuckDB, embedded scripting to Lua, and untrusted plugins to WebAssembly. It prefers host-language optimization when that captures at least 70% of the opportunity, accounts for FFI cost, grants no capabilities itself, and requires human review.

- `src/core.c/.h` — CPU profile, benchmark, scanner, JSON report.
- `src/syntax_pie.c/.h` — 20-level syntax/function catalogue and lookups.
- `src/processor.c/.h` — architecture/ISA inventory and processor suite.
- `src/dsp.c/.h` — bounded WAV I/O and DSP.
- `src/win_gui.c` — native Windows UI.
- `src/cli.c` — cross-platform CLI.
- `src/apsia.c/.h`, `src/apsia_cli.c` — APSIA routing/promotion planner and CLI.
- `docs/APSIA_2.0.md`, `apsia/evolution-lock.schema.json` — architecture and artifact-evidence schema.
- `tests/test_mtte.c` — MTTE catalogue, lookup, bounds and overflow tests.
- `tests/test_core.c` — core, processor and DSP tests.
- `.github/workflows/ci.yml` — Windows/Linux build and test matrix.

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

Example: `pytyliki-apsia cpu 2.0 0.1 0.2` (advisory route planning only; no generated code is compiled or loaded).

```bash
pytyliki --improvement-plan
pytyliki --evolve --evolution-rounds 8
```

## 🧪 Quality audit and CI

See [the 0.7.1 quality audit](docs/QUALITY_AUDIT_0.7.1.md) for the PR consolidation review, fixes, verification evidence and remaining governance limits. The Windows/Linux CMake + CTest matrix and Recursive Evolution Gate are run on pull requests. Verified auto-merge checks for an independent approval tied to the PR head and fails closed on API errors or stale validation events; configure GitHub branch protection to require the relevant checks and review so manual merges cannot bypass that workflow gate.

`pytyliki-apsia` evaluates a supplied workload and target allow-list; it does not compile or execute the selected language module.

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
