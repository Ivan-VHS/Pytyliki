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

The scanner never executes or rewrites source. ISA flags alone do not establish safe instruction eligibility. The HyperFusion engine coordinates six bounded analysis modules; its readiness score is heuristic, not a measured speedup.

## 🧬 HyperFusion Six-Module Engine

`Concreo`, `Pyemont`, `Bollo`, `JJ-HH`, `PenWhite` and `Serious Serious` operate as one C11 evaluation pipeline. Each round invokes all six modules and evaluates all 36 directed module pairs (including self-checks). Every pair multiplies the two module scores and applies a compatibility weight; the products accumulate into a cross-module synergy score. Repeated rounds use diminishing-return scoring and stop at convergence or the configured cap.

- **Concreo:** architecture and source-review readiness.
- **Pyemont:** bounded transformation planning (no automatic source rewrite).
- **Bollo:** benchmark and memory evidence assessment.
- **JJ-HH:** cross-validation against test and scan evidence.
- **PenWhite:** per-round trace accounting.
- **Serious Serious:** final promotion gate; test success and known architecture are required.

Run `pytyliki --evolve --evolution-rounds 8`. The CLI deliberately keeps the promotion gate closed because it cannot independently certify that CI passed. The reported readiness/synergy values are planning indicators, not proof of faster hardware or modified processor silicon.

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
- `src/processor.c/.h` — architecture/ISA inventory and processor suite.
- `src/evolution.c/.h` — six-module HyperFusion coordinator and 36-edge interaction matrix.
- `src/dsp.c/.h` — bounded WAV I/O and DSP.
- `src/win_gui.c` — native Windows UI.
- `src/cli.c` — cross-platform CLI.
- `tests/test_core.c` — core, processor and DSP tests.
- `tests/test_evolution.c` — module interaction, convergence bounds and promotion-gate tests.
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
- [ ] Expanded profiling and benchmark history
- [x] Bounded six-module HyperFusion orchestration and pairwise interaction engine
- [ ] Reproducible source optimization/refactoring tools
- [ ] Guarded branch, PR and release automation
- [ ] Cross-architecture transpilation and RTL/FPGA
- [ ] Desktop diagnostics and optimization dashboards

## 🛡️ Safety

CPU discovery is read-only. Pytyliki does **not** modify firmware, microcode, voltage, clocks, power limits, kernels or drivers. Benchmarks are advisory. Physical hardware synthesis requires dedicated tools and validation.

**License:** MIT.

---

<p align="center">

### 🔵 ENGINEER WITH PRECISION · 🔴 EVOLVE WITH PURPOSE

**「爱是理解，是陪伴，也是彼此成就。」**

*Love is understanding, companionship, and helping one another grow.*

</p>
