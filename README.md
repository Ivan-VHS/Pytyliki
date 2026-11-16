![Pytyliki — white / blue / red engineering workstation](assets/pytyliki-banner.svg)

<p align="center">
  <strong>Native C11 engineering workstation · CPU diagnostics · static review · policy checks</strong>
</p>

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg" alt="C11 CI"></a>
  <img src="https://img.shields.io/badge/language-ISO%20C11-245B9A" alt="ISO C11">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux-FFFFFF?labelColor=245B9A" alt="Windows and Linux">
  <img src="https://img.shields.io/badge/license-MIT-D52B3F" alt="MIT license">
</p>

# Pytyliki CPU Lab 0.5.0

Pytyliki is a native C11 workstation utility for local CPU diagnostics, a bounded benchmark, read-only source review, explicit permission checks, and engineering workflow planning.

The application is designed around a strict boundary: inspect and report safely; do not silently change hardware, firmware, source files, or clinical decisions.

## Interface and visual identity

The project uses a crisp **white / blue / red** palette:
- white and cool light-grey surfaces for readable workspace content;
- deep blue for primary structure and navigation;
- restrained red accents for important boundaries and warnings.

The Windows desktop window uses the same visual system. The portable CLI remains available on Windows and Linux.

## Current capabilities

| Module | Status | What it does |
|---|---|---|
| CPU profile | Implemented | CPU vendor, brand string when available, logical processor count, extended family/model decoding |
| Bounded benchmark | Implemented | Fixed integer workload with a hard iteration cap of 500,000,000 |
| Static source review | Implemented, heuristic | Read-only review of a selected C/C++/Rust source file for selected risky API patterns |
| JSON report | Implemented | Escaped JSON metadata for CPU, benchmark, and scan results |
| Access policy | Implemented, local policy layer | Deny-by-default role/action checks; unsafe hardware and source-execution actions are denied |
| C11/C23 pipeline plan | Reference planner | Deterministic stage order and quality gates; **not yet a transpiler** |
| Clinical workflow catalogue | Template catalogue | Infection-control, emergency-response, and oncology-screening workflow shells; no clinical recommendations |
| GitHub automation | CI implemented | Least-privilege workflow permissions and Windows/Linux build-test matrix |

## Build

Requires CMake 3.16+ and a C11 compiler.

<pre><code>cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure</code></pre>

On Windows, the build also produces <code>pytyliki-gui</code>. On Linux, it produces the <code>pytyliki</code> CLI.

## CLI

<pre><code># CPU identity and hardware profile
pytyliki --profile

# Bounded local benchmark
pytyliki --benchmark

# Read-only review of one source file
pytyliki --analyze path/to/source.c

# Export the current diagnostic report
pytyliki --profile --benchmark --json report.json

# Display the reference transpilation pipeline
pytyliki --pipeline --target c11
pytyliki --pipeline --target c23

# Inspect the clinical workflow templates
pytyliki --clinical-catalog

# Check a role/action pair
pytyliki --policy-check analyst benchmark
pytyliki --policy-check viewer apply-patch</code></pre>

Use <code>pytyliki --help</code> for all options. The policy check returns a non-zero exit status when an action is denied.

## Architecture

<pre><code>Pytyliki
├── core/            CPU profile, bounded benchmark, source scanner, JSON
├── policy/          role/action authorization, deny-by-default rules
├── pipeline/        deterministic C11/C23 workflow reference plan
├── clinical/        validated metadata templates with expert sign-off
├── cli/             portable command-line interface
├── win32-gui/       native Windows presentation
├── tests/           core, policy, pipeline, and catalogue tests
└── .github/         Windows/Linux CI</code></pre>

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), [docs/ROADMAP.md](docs/ROADMAP.md), and [docs/THREAD_CONTEXT.md](docs/THREAD_CONTEXT.md).

## Safety model

- Static analysis is read-only. It does not compile, link, load, or execute inspected input.
- The benchmark runs in the current process and is capped at 500,000,000 iterations.
- No overclocking, BIOS/UEFI writes, firmware flashing, microcode writes, voltage/frequency changes, or privileged hardware operations are implemented.
- The local policy layer is an application-level guard, not a replacement for GitHub repository permissions or operating-system security.
- The C11/C23 pipeline is currently a reference plan. No general-purpose parser, IR lowering, or automatic transpiler is claimed.
- Clinical entries are workflow metadata templates only. They contain no diagnostic or treatment logic and require approved source documents and qualified clinical validation before deployment.
- Recommendations and static findings are advisory; a human remains responsible for review and approval.

## Project status

Version 0.5.0 expands the original CPU Lab with policy, workflow-planning, and clinical catalogue foundations. The next milestone is to add parser-backed source analysis and a real, testable intermediate representation before attempting automated source transformation.

**MIT License** — see [LICENSE](LICENSE).
