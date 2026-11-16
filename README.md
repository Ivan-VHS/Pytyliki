# 🔵🔴 Pytyliki 0.7.0 — Processor Systems Workstation

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
- Evidence-gated 20-capability improvement engine, 12 implemented advisory hardware rules, deny-by-default policy, C11/C23 reference pipeline and six-module HyperFusion coordinator.

The scanner never executes or rewrites source. ISA flags alone do not establish safe instruction eligibility.

## 🧬 HyperFusion six-module coordinator

Run `pytyliki --evolve --evolution-rounds 8` to evaluate six bounded analysis modules and all 36 directed pair interactions per round. Readiness and synergy are heuristic planning indicators, not measured speedups. The promotion gate stays held until independently verified CI evidence is supplied. HyperFusion does not rewrite source files or merge branches.

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

- `src/core.c/.h` — CPU profile, bounded benchmark, processor source scanner and JSON report.
- `src/processor.c/.h` — architecture/ISA inventory and bounded execution/memory/matrix suite.
- `src/dsp.c/.h` — bounded PCM16 WAV I/O, tone generation, DSP processing and metering.
- `src/improvement.c/.h` — 20×3 capability catalogue and evidence-gated iterative lifecycle.
- `src/hw_rules.c/.h` — 12 implemented advisory hardware rules (remaining catalog IDs reserved).
- `src/evolution.c/.h` — six-module HyperFusion analysis, 36 directed interactions per round.
- `src/pipeline.c/.h` — C11/C23 reference pipeline plan (not an active transpiler).
- `src/policy.c/.h` — deny-by-default local authorization policy.
- `src/processor_catalog.c/.h` — processor workflow metadata and evidence validation.
- `src/win_gui.c` — native Windows workstation UI; `src/cli.c` — cross-platform CLI.
- `tests/` — core/DSP, improvement, hardware-rule, engineering-foundation and HyperFusion regression suites.
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
- [ ] Reproducible optimization/refactoring tools
- [ ] Guarded branch, PR and release automation
- [ ] Cross-architecture transpilation and RTL/FPGA
- [ ] Desktop diagnostics and optimization dashboards

## 🧪 Quality audit and CI

See [the 0.7.1 quality audit](docs/QUALITY_AUDIT_0.7.1.md) for the PR consolidation review, fixes, verification evidence and remaining governance limits. The Windows/Linux CMake + CTest matrix and Recursive Evolution Gate are run on pull requests. Verified auto-merge checks for an independent approval tied to the PR head and fails closed on API errors or stale validation events; configure GitHub branch protection to require the relevant checks and review so manual merges cannot bypass that workflow gate.

## 🛡️ Safety

CPU discovery is read-only. Pytyliki does **not** modify firmware, microcode, voltage, clocks, power limits, kernels or drivers. Benchmarks are advisory. Physical hardware synthesis requires dedicated tools and validation.

**License:** MIT.

---

<p align="center">

### 🔵 ENGINEER WITH PRECISION · 🔴 EVOLVE WITH PURPOSE

**「爱是理解，是陪伴，也是彼此成就。」**

*Love is understanding, companionship, and helping one another grow.*

</p>
