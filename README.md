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
- **Improvement planning:** inspect a 20-capability engineering/optimization/automation catalogue and evidence-gated iteration plans.\n- **APSIA 2.0:** C11 polyglot routing planner with FFI cost gates and fail-closed sandbox selection.

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

APSIA extends the existing C11/PREE/HyperFusion workflow with a guarded language-routing planner. The implemented C11 planner maps CPU-bound work to Rust, GPU work to CUDA, analytics to SQL, network I/O to Go, scripting to Python, and sandboxed plugins to WebAssembly. It checks target availability and FFI-adjusted cost before selecting a route; untrusted plugins fail closed if WASM is unavailable.

The broader APSIA design specifies interface contracts, hermetic builds, SBOM/provenance, differential verification, shadow tests, canary promotion, rollback, and repository-by-repository PR delivery. These are staged capabilities, not claims that automatic code generation, native module loading, production profiling, or hot swapping are already active. See [APSIA 2.0 architecture](docs/APSIA_2.0.md).

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

## Quality audit and CI\n\nSee [the repository quality audit](docs/QUALITY_AUDIT_0.7.1.md) for the historical PR consolidation, defect fixes and CI evidence. APSIA implementation status and limitations are documented separately in [APSIA 2.0](docs/APSIA_2.0.md).\n\n## Safety and implementation limits

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
- `src/policy.c/.h` — local authorization policy.\n- `src/apsia.c/.h` — deterministic polyglot routing planner.\n- `docs/APSIA_2.0.md` — architecture, safety gates and staged delivery plan.
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
