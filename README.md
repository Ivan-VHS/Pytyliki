# 🔵🔴 Pytyliki — Processor Systems Workstation

<p align="center">

**⚡ ANALYZE · OPTIMIZE · TRANSFORM · VERIFY · EVOLVE**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

</p>

> Native C11 workstation for processor discovery, bounded benchmarking, read-only source review and local audio/DSP experiments.

## 💎 Current capabilities

- CPU/ISA discovery, memory information and bounded processor benchmarks.
- Read-only source scanner and JSON reports.
- PCM16 WAV tools, tone generation and local DSP processing.
- Native Win32 GUI; CLI on Windows and Linux.
- Evidence-gated 20-capability improvement engine, 12 advisory hardware rules, deny-by-default policy and C11/C23 pipeline plan.
- Mechanical Tick & Type Engine: a deterministic 20-layer semantic/function/mechanical-operation catalogue with a bounded software-only tick counter.

The scanner never executes or rewrites source. ISA flags alone do not establish safe instruction eligibility.

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

Run `pytyliki --evolve --evolution-rounds 8` to evaluate six bounded analysis modules and all 36 directed pair interactions per round. Readiness and synergy are heuristic planning indicators, not measured speedups. The promotion gate stays held until independently verified CI evidence is supplied. HyperFusion does not rewrite source files or merge branches.

## 🚀 20 × 3 Evolution Protocol

These triplets define the **target policy**, not implemented-feature claims.

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

### 🔐 Guarded automation contract

1. Use isolated branches; protect `main`.
2. Commit traceable, relevant changes.
3. Push feature branches and open PRs.
4. Run lint, static checks, builds and tests.
5. Merge only after checks and review approval.
6. Never force-push, bypass protections or self-approve.
7. Deploy only configured, validated artifacts.
8. Keep recovery paths and operation logs.
9. Require human approval for destructive actions/releases.
10. Record inputs, outputs, versions and benchmark baselines.

**Auto-commit, auto-push and auto-merge are separate permissioned stages.** A failed check stops promotion. Automation must be observable, bounded, reversible and test-gated.

## 🧬 Architecture

```text
          PYTYLIKI
              |
      +-------+--------+
      |       |        |
   DISCOVER TRANSFORM VERIFY
      |       |        |
   CPU/ISA  C11/DSP  TESTS/CI
   BENCH    ANALYSIS STATIC
      |       |        |
      +-------+--------+
              |
        REVIEWED PR
              |
             MAIN
```

### Source map

- `src/core.c/.h` — CPU profile, benchmark, scanner, JSON report.
- `src/mtte.c/.h` — 20-layer semantic/function/tick catalogue and logical counter.
- `src/processor.c/.h` — architecture/ISA inventory and processor suite.
- `src/dsp.c/.h` — bounded WAV I/O and DSP.
- `src/win_gui.c` — native Windows UI.
- `src/cli.c` — cross-platform CLI.
- `tests/test_mtte.c` — MTTE catalogue, lookup, bounds and overflow tests.
- `tests/test_core.c` — core, processor and DSP tests.
- `.github/workflows/ci.yml` — Windows/Linux build and test matrix.

## ⚙️ Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows builds also provide `pytyliki-gui.exe`.

## 🗺️ Roadmap

- [x] Native C11 core, CLI and Windows desktop UI
- [x] CPU discovery, bounded benchmarks and source review
- [x] Local PCM16 DSP and Windows/Linux CI
- [x] MTTE 20-layer catalogue, logical tick counter and CTest coverage
- [ ] Expanded profiling and benchmark history
- [ ] Reproducible optimization/refactoring tools
- [ ] Guarded branch, PR and release automation
- [ ] Cross-architecture transpilation and RTL/FPGA
- [ ] Desktop diagnostics and optimization dashboards

## 🛡️ Safety

CPU discovery is read-only. Pytyliki does **not** modify firmware, microcode, voltage, clocks, power limits, kernels or drivers. The MTTE tick counter is software-only. Benchmarks are advisory. Physical hardware synthesis requires dedicated tools and validation.

**License:** MIT.

---

<p align="center">

### 🔵 ENGINEER WITH PRECISION · 🔴 EVOLVE WITH PURPOSE

**「爱是理解，是陪伴，也是彼此成就。」**

*Love is understanding, companionship, and helping one another grow.*

</p>
