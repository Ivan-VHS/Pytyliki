# Pytyliki — C11 Processor Systems Workstation

<p align="center">
  <strong>DISCOVER · ANALYZE · OPTIMIZE · VERIFY</strong><br>
  <img alt="C11" src="https://img.shields.io/badge/C-ISO_C11-2563EB?style=flat-square">
  <img alt="Windows and Linux" src="https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux-DC2626?style=flat-square">
  <img alt="MIT License" src="https://img.shields.io/badge/License-MIT-FFFFFF?style=flat-square&labelColor=1F2937">
</p>

**Pytyliki is a modular C11 workstation for read-only processor discovery, bounded local benchmarks, source diagnostics, and PCM16 audio/DSP utilities.** It provides a CLI on Windows and Linux and a native Win32 GUI. It does not reprogram physical silicon.

**Documentation:** [English](README.md) · [Русский](README.ru.md) · [Українська](README.uk.md) · [简体中文](README.zh-CN.md)

---

## At a glance

| Area | What is included |
|---|---|
| Processor | CPU and ISA inventory; bounded processor, memory, and matrix tests |
| Analysis | Read-only source scanner and JSON reports |
| Audio/DSP | PCM16 WAV I/O, tone generation, processing, and metering |
| Interfaces | Cross-platform CLI and native Windows GUI |
| Improvement | 20-capability catalogue and evidence-gated planning |
| Coordination | Six bounded HyperFusion analysis modules |
| Policy | Deny-by-default local authorization model |
| CI | CMake/CTest validation on Windows and Linux |

> [!NOTE]
> Capability catalogues and evolution plans describe available classifications and intended workflows. They do not mean that every transformation is autonomous, applied to source code, or guaranteed to improve performance.

## Quick start

### Requirements

- CMake and a C11-capable compiler.
- Windows or Linux.
- Git.

### Build and test

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows builds also provide `pytyliki-gui.exe`.

### CLI examples

```bash
pytyliki --improvement-plan
pytyliki --evolve --evolution-rounds 8
pytyliki --syntax-pie
pytyliki --mtte
```

## Architecture

| Module | Responsibility |
|---|---|
| `src/core.c/.h` | CPU profile, bounded benchmark, scanner, JSON report |
| `src/processor.c/.h` | Processor and ISA inventory and test suite |
| `src/dsp.c/.h` | Bounded WAV and DSP operations |
| `src/improvement.c/.h` | Improvement catalogue and lifecycle |
| `src/hw_rules.c/.h` | Advisory hardware rules |
| `src/evolution.c/.h` | HyperFusion analysis |
| `src/pipeline.c/.h` | C11/C23 reference pipeline plan |
| `src/policy.c/.h` | Local authorization policy |
| `src/cli.c`, `src/win_gui.c` | CLI and Windows desktop interface |
| `.github/workflows/` | CI and guarded repository automation |

### Capability catalogues

- **Syntax-pie:** deterministic 20-layer mapping from function verbs and room metaphors to syntax abstractions. It is a catalogue and lookup layer, not a general-purpose parser.
- **MTTE:** 20 linked semantic/function/mechanical layers and a deterministic logical tick counter. The counter models software scheduling; it does not change hardware timers, CPU frequency, registers, firmware, or voltage.
- **Improvement matrix:** 20 engineering, 20 optimization, and 20 automation classifications exposed through `pytyliki --improvement-plan`.
- **HyperFusion:** six bounded analysis modules with 36 directed pair interactions per round. Readiness and synergy values are heuristic planning indicators, not measured speedups.
- **APSIA 2.0:** C11 language-routing planner with target-availability and FFI-cost gates. It proposes routes; it does not compile or execute the selected language module.

### Evidence and implementation status

The hardware-rule catalogue currently has **12 implemented advisory rules**; other catalogue identifiers are reserved. The C11/C23 pipeline is a reference plan, not a production transpiler. Any source change or integration requires explicit authorization, tests, and review.

## Verified GitHub Auto-Merge

The repository includes `.github/workflows/verified-auto-merge.yml` and GitHub native Auto-Merge (squash).

1. Pull requests targeting `main` are built and tested on `ubuntu-latest` and `windows-latest`.
2. CMake configures and builds the project; CTest runs the registered suite.
3. Before enabling Auto-Merge, the workflow verifies that the current PR head SHA is the exact commit that passed validation.
4. It requires an independent `APPROVED` review tied to that exact head commit. An approval for an older commit does not qualify.
5. It requests `gh pr merge --auto --squash --delete-branch`. GitHub performs the merge only after all repository rules are satisfied.

Repository requirements:

- Enable **Allow auto-merge**.
- Protect `main`; require the applicable CI checks and independent review.
- Grant write permissions only to the merge job.
- Fail closed on missing approval, changed head SHA, or API errors. Never bypass branch protection.

> [!IMPORTANT]
> Auto-Merge being enabled does not mean that a pull request has merged. Missing approvals or required checks keep it open. The workflow cannot create approval or override repository protections.

## Quality and documentation policy

The documentation system follows a restrained blue / white / red identity:

| Token | Value | Use |
|---|---|---|
| Blue | `#2563EB` | Primary links, structure, neutral technical emphasis |
| White | `#FFFFFF` | Canvas and high-contrast surfaces |
| Red | `#DC2626` | Errors, blockers, destructive or critical states |
| Ink | `#111827` | Body text and headings |
| Muted | `#6B7280` | Secondary metadata |

Layout rules:

- Use one H1, followed by a gap-free heading hierarchy.
- Put purpose, platform scope, and the first useful action before deep technical detail.
- Keep paragraphs short and each section independently understandable.
- Use consistent 8 px rhythm as the design reference: 8 / 16 / 24 / 32 px. GitHub controls actual Markdown rendering, so these are content/layout guidelines rather than CSS guarantees.
- Prefer concise tables for comparable facts and lists for sequential actions.
- Keep code examples executable, language-tagged, and narrow enough to read on small screens.
- Use descriptive link text, stable section anchors, GitHub Alerts, and text-based diagrams where they clarify system relationships.
- Avoid decorative badge walls, emoji-heavy headings, redundant summaries, and unsupported performance claims.
- Preserve contrast and meaning without relying on color alone. GitHub's Markdown renderer controls responsive layout and typography; this repository cannot enforce a custom font, exact line width, or pixel-perfect grid inside README pages.

## CI and verification

The Windows/Linux CMake + CTest matrix and Recursive Evolution Gate are used on pull requests. See [the quality audit](docs/QUALITY_AUDIT_0.7.1.md) for recorded review findings and verification evidence. Workflow results, not README statements, are the source of truth for the current status of a commit.

## Compatibility and limits

- **Validated CI targets:** Windows and Linux.
- **Language:** ISO C11 implementation; C23 appears in the reference pipeline plan.
- **Data formats:** JSON reports and PCM16 WAV audio.
- **Not claimed:** macOS, ARM64, x86_64-specific performance guarantees, Protobuf, Parquet, Arrow, or production RTL/FPGA synthesis unless separately validated.

## Safety boundaries

- Processor discovery is read-only.
- The source scanner does not execute or rewrite scanned files.
- No writes to firmware, BIOS/UEFI, microcode, voltage, clocks, power limits, kernels, drivers, or device registers.
- Benchmarks are workload-, compiler-, OS-, and hardware-dependent; no speedup is guaranteed.
- Advisory rules and planning scores are not hardware certification.

## License

Distributed under the [MIT License](LICENSE).
