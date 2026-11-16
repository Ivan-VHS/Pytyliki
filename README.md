# 🔵🔴 Pytyliki — Processor Systems Workstation

<p align="center">

**⚡ ANALYZE · OPTIMIZE · TRANSFORM · VERIFY · EVOLVE**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![Interface](https://img.shields.io/badge/Desktop-Native-1D4ED8?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

</p>

> **A native C11 workstation for processor discovery, bounded benchmarking, source review and local audio/DSP experiments.**

🔵 **BLUE** — precision, architecture and analysis.  
🔴 **RED** — transformation, performance and execution.  
⚪ **WHITE** — clarity, safety and engineering discipline.

---

## 💎 What Pytyliki does today

- **CPU discovery:** architecture, CPU identity, logical processors, memory information and selected x86 ISA flags.
- **Processor suite:** bounded integer, memory-touch and matrix workloads with elapsed-time measurements and integrity checksums.
- **Source review:** read-only heuristic scanning for selected risky source patterns.
- **DSP laboratory:** PCM16 WAV import/export, tone generation, filtering, compression, saturation, gain and audio metering.
- **Native desktop UI:** Win32 workstation interface on Windows; CLI on Windows and Linux.
- **Reports:** user-triggered JSON CPU and benchmark export.

The source scanner does not compile, execute or rewrite scanned files. ISA flags are informational and do not alone establish safe instruction eligibility.

## 🚀 The 20 × 3 Evolution Protocol

These 20 triplets define the **target automation policy** for Pytyliki. They are engineering objectives, not a claim that every automation is already implemented.

| # | Core operation | Workflow automation | Quality objective |
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

### 🔐 Safe automation contract

Automation must be **observable, bounded, reversible and test-gated**:

1. Work on isolated feature branches; never rewrite `main` directly.
2. Commit only reviewed, relevant changes with traceable messages.
3. Push automation output to a branch and open a pull request.
4. Run formatting, lint, static checks, builds and tests before promotion.
5. Merge only after required checks and repository review rules pass.
6. Never force-push, bypass branch protection, or auto-approve its own PR.
7. Deploy only explicitly configured artifacts/environments, after validation.
8. Keep backups or recovery paths for generated and transformed files.
9. Require human approval for destructive changes, release publication and hardware-affecting actions.
10. Record logs, tool versions, inputs, outputs and benchmark baselines.

**Auto-commit, auto-push and auto-merge are separate permissioned stages.** A failed check stops the pipeline; no automation may silently promote an unverified result.

## 🧬 Engineering architecture

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

**Current source map**

- `src/core.c/.h` — CPU profile, benchmark, source scanner and JSON report.
- `src/processor.c/.h` — architecture/ISA inventory and processor suite.
- `src/dsp.c/.h` — bounded WAV I/O and DSP pipeline.
- `src/win_gui.c` — native Windows GUI.
- `src/cli.c` — cross-platform CLI.
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

- [x] Native C11 foundation, CLI and Windows desktop UI
- [x] CPU capability discovery and bounded benchmarks
- [x] Read-only source review and local PCM16 DSP
- [x] Windows/Linux CI build-and-test matrix
- [ ] Expanded processor profiling and benchmark history
- [ ] Reproducible optimization and refactoring tools
- [ ] Guarded branch/PR automation and release workflows
- [ ] Cross-architecture transpilation and RTL/FPGA integrations
- [ ] Desktop diagnostics, rule manager and benchmark dashboards

## 🛡️ Safety boundaries

Pytyliki performs read-only CPU discovery. It does **not** modify BIOS/UEFI, firmware, microcode, voltage, clocks, power limits, kernels or drivers. Benchmarks are bounded and advisory. Software optimization does not directly alter physical processor fabrication. Hardware synthesis requires dedicated toolchains and validation.

**License:** MIT.

---

<p align="center">

### 🔵 ENGINEER WITH PRECISION · 🔴 EVOLVE WITH PURPOSE

**「爱是理解，是陪伴，也是彼此成就。」**

*Love is understanding, companionship, and helping one another grow.*

</p>
