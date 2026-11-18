# Pytyliki — C11 Processor Systems Workstation

<p align="center">

**DISCOVER · ANALYZE · PLAN · VERIFY**

![C11](https://img.shields.io/badge/C-ISO_C11-2563EB?style=for-the-badge)
![Platforms](https://img.shields.io/badge/OS-Windows%20%7C%20Linux-DC2626?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-2563EB?style=for-the-badge)

</p>

**TL;DR:** Pytyliki is a native C11 workstation for read-only CPU/ISA inspection, bounded processor benchmarks, source diagnostics, local PCM16 WAV/DSP tasks, and evidence-gated engineering workflows. It provides a CLI on Windows/Linux and a native Win32 desktop GUI. It does not reprogram physical processors.

**Languages:** English · [简体中文](README.zh-CN.md) · [Русский](README.ru.md) · [Українська](README.uk.md)

**Architecture contract:** [66-rule autonomy and modularity requirements](docs/ARCHITECTURE_66.md), the [666-rule Constitution implementation profile](docs/CONSTITUTION_666_PROFILE.md), the [100-proverb engineering crosswalk](docs/CHINESE_PROVERBS_100_ENGINEERING_CROSSWALK.md), and the [MCST Elbrus E2K target profile](docs/ELBRUS_E2K.md). The proverb ledger is guidance, not implementation evidence. **AETHER-ANAS:** [staged integration profile](docs/AETHER_ANAS_INTEGRATION.md) for the C11 core, DSP, capabilities and evidence gates. **Duck Constitution:** [Pytyliki engineering profile](docs/ANATIDAE_ENGINEERING_PROFILE.md) applies the manifesto to the C11 workstation without claiming language-runtime features.

## What Pytyliki does

- **Processor discovery:** collect CPU, architecture, memory and ISA information. Feature flags are hints; OS-enabled instruction state and runtime eligibility must also be considered.
- **Bounded benchmarks:** run controlled local compute, memory and matrix workloads. Results are hardware-, compiler-, OS- and workload-dependent.
- **Source diagnostics:** inspect source-level processor patterns and produce reports without executing or rewriting the inspected source.
- **Audio/DSP:** bounded PCM16 WAV input/output, tone generation, processing and metering.
- **Desktop + CLI:** native Win32 GUI and command-line workflows for Windows and Linux.
- **Improvement planning:** a 20-row engineering/optimization/automation catalogue and a recursive, evidence-gated lifecycle.
- **Hardware guidance:** 12 implemented advisory rules; other catalogue IDs are reserved, not implemented.
- **APSIA 2.0 foundation:** deterministic workload routing and a fail-closed promotion-evidence gate in the C11 core.

## Architecture

| Layer | Current scope |
|---|---|
| Core | CPU profile, bounded benchmarks, source scanner, JSON reporting |
| Processor | Architecture/ISA inventory and bounded processor tests |
| DSP | PCM16 WAV I/O, tone generation, processing and metering |
| Improvement | 20 capability triplets and evidence-gated iteration planning |
| HyperFusion | Six bounded analysis modules; 36 directed pair interactions per round |
| Hardware rules | 12 advisory rules with validated metric ranges |
| APSIA 2.0 | Workload-to-language routing, FFI-cost checks, capability declarations and promotion gate |
| Pipeline | C11/C23 reference pipeline plan; not a production transpiler |
| Policy | Deny-by-default local authorization |
| Interfaces | Windows/Linux CLI and native Windows GUI |

HyperFusion readiness/synergy values are heuristic planning indicators, not measured speedups or independent CI certification. APSIA routing is advisory: it does not compile, load, hot-swap or execute generated modules, and it grants no runtime capabilities.

## Recursive improvement model

The engineering lifecycle is:

`observe → diagnose → propose → implement → verify → evaluate → review → integrate → learn → improve-engine`

A configurable iteration target is a planning/counter boundary, not a promise to perform that many builds or source rewrites. The requested 66,666,666× improvement ambition is aspirational, not a measured multiplier.

The 20×3 catalogue describes 60 engineering, workflow and quality operations. It is a vocabulary and policy matrix, not proof that every operation is autonomous.

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

## APSIA 2.0: guarded polyglot evolution

The implemented C11 foundation evaluates workload type, estimated gain, FFI overhead, host optimization opportunity and platform availability before recommending a language route. Candidate backends include Rust, C++, Zig, CUDA, WebAssembly, Go, Elixir, SQL/DuckDB, Lua and Python.

Promotion is fail-closed. Evidence includes contract validity, differential tests, fuzzing, sanitizers, static analysis, measured benchmark gain, shadow equivalence, independent review, artifact signature, rollback test and matching validated/current head tokens.

**Implemented:** routing policy and promotion-evidence gate.

**Not implemented:** LLM source synthesis, a hermetic multi-language compiler fleet, automatic dependency installation, production eBPF/APM collection, FFI code generation, runtime hot swapping/RCU, unattended deployment or a cross-repository fleet executor. See [APSIA 2.0 design](docs/APSIA_2.0.md).

## Current development boundaries

Recent design work also defines three processor-oriented catalogues:

- **Syntax Pie:** a 20-level function/room/syntax taxonomy, from lexemes and tokens through AST, grammar rules, graphs and hierarchy.
- **MTTE:** a 20-layer semantic/function/mechanical-operation taxonomy with a deterministic software tick-counter design.
- **Berry Core:** a 20-command logical-core request catalogue using dry-run planning semantics.

These catalogues are tracked as staged integration work; they must not be confused with a complete parser, a hardware timer controller, an IPI/affinity controller or a production processor-control interface. The names Concreo, Pyemont, Bollo, JJ-HH, PenWhite and Serious Serious are experimental function-suite concepts, not stable public APIs.

## Integration status of recent catalogues

This change set registers `syntax_pie.c`, `mtte.c`, and `berry_core.c` in `pytyliki_core`, links the default CLI against the core library, and adds focused CTest targets for all three modules. They are bounded catalogues and planning primitives, not a general-purpose parser, hardware timer controller, IPI/affinity controller, or physical processor-control interface. The integration is covered by the Windows/Linux build-and-test matrix and Linux ASan/UBSan job on this branch.

## Build and test

Requirements: CMake 3.16+, a C11 compiler, Git, and Windows or Linux.

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows builds include the native `pytyliki-gui.exe` target.

## CLI examples

```bash
pytyliki --improvement-plan
pytyliki --e2k-profile
pytyliki --evolve --evolution-rounds 8
pytyliki --review-processor source.c
pytyliki --json report.json
```

Use `pytyliki --help` for the options supported by the checked-out build. Audio workflows include `--tone-out`, `--audio-info` and `--process-wav`.

## Quality gates and repository automation

The intended integration sequence is:

1. Create an isolated feature branch.
2. Make a small, traceable change.
3. Build and run CTest on supported CI platforms.
4. Review regressions, evidence and the exact tested commit SHA.
5. Obtain independent human approval.
6. Integrate sequentially through repository-authorized controls.

Auto-commit, auto-push, auto-merge and deployment are separate permissioned operations. Failed, stale or incomplete evidence blocks promotion. Protect `main` with required CI checks and independent review; a workflow alone cannot prevent an authorized maintainer from bypassing repository settings.

README generation is intended to become deterministic and repository-derived (API/AST, metrics, CI, dependencies and history), with Markdown/link checks and idempotent publication. Until that generator is implemented and validated, this README is manually maintained and must not be described as auto-generated.

## Safety and non-goals

- CPU discovery is read-only; the source scanner does not execute or rewrite scanned code.
- No firmware, BIOS/UEFI, microcode, voltage, clock, power-limit, kernel, driver or device-register writes.
- No arbitrary generated-code execution and no automatic grant of plugin capabilities.
- Hardware rules are advisory; benchmark and HyperFusion estimates are not guaranteed performance gains.
- C11 is the implementation baseline. C23 is referenced as a pipeline target, not claimed as a complete transpiler.
- Windows/Linux are the documented CI targets. Do not infer macOS, ARM64 or unverified ISA-specific performance support.

## Project map

- `src/core.c/.h`, `src/processor.c/.h` — core and processor inspection.
- `src/dsp.c/.h` — WAV and DSP operations.
- `src/improvement.c/.h`, `src/evolution.c/.h` — improvement catalogue and evolution analysis.
- `src/hw_rules.c/.h` — advisory processor rules.
- `src/apsia.c/.h`, `src/apsia_cli.c` — APSIA routing and promotion policy.
- `src/pipeline.c/.h`, `src/policy.c/.h` — reference pipeline and authorization policy.
- `src/cli.c`, `src/win_gui.c` — CLI and Windows GUI.
- `.github/workflows/` — CI and guarded repository automation.


## Additional engineering catalogues

- **Forge66:** a deterministic 66-entry catalogue of processor-engineering functions and their metadata; it is a planning/catalogue layer, not autonomous code rewriting or hardware control.
- **CPU Suite Pro 78:** a deterministic, allocation-free catalogue of 78 specialized workstation modules. See [CPU Suite Pro documentation](docs/CPU_SUITE_PRO_78.md).
- **TriMatrix AutoForge 100:** a bounded three-line/three-matrix planning model with validated depth limits. See [TriMatrix documentation](docs/trimatrix-autoforge.md).

## Platform and capability validation

Supported-platform and data-format statements are evidence-gated. See the [platform validation policy](docs/PLATFORM_VALIDATION_POLICY.md) for the current scope and the evidence required before claiming additional compatibility or performance guarantees.


## License

MIT — see [LICENSE](LICENSE).

---

<p align="center">

**ENGINEER WITH PRECISION · VERIFY EVERY CHANGE**

</p>
