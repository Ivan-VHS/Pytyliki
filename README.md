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
- **CPU Suite Pro 78X (feature branch):** 78-entry deterministic module catalogue, six domains, stable IDs, lookups, advisory execution plans and exhaustive CTest coverage. See [CPU Suite Pro 78X](docs/CPU_SUITE_PRO_78.md).

## CPU Suite Pro 78X

CPU Suite Pro is a modular CPU-workstation catalogue inspired by classic integrated software suites. Its 78 modules cover CPU Hub, processor topology, thermal/power analysis, memory/cache, scheduling, diagnostics/security and benchmark workflows.

The C API in `src/cpu_suite.h` provides deterministic enumeration, lookup by ID/name and bounded advisory planning. This feature branch implements the catalogue and testable module contracts; it does not represent 78 completed live hardware-control panels. Vendor sensor adapters, GUI views, OS-specific counters and privileged controls remain separate, independently validated work.

Safety boundary: no voltage, clock, firmware, microcode, fan, interrupt, kernel or processor-register writes are performed. Tuning modules produce plans only. AVX workloads require runtime ISA and OS vector-state validation. No platform-specific performance guarantees are implied.

## 🥧 Syntax-pie function layers

Run `pytyliki --syntax-pie` to inspect the deterministic 20-layer mapping from function verbs and room metaphors to syntax abstractions, from **Зал / Комната / Лексема** through **Залегендарить / Анфилада / Иерархическая грамматика**. The C API in `src/syntax_pie.h` exposes indexed lookup and lookup by function name, room name or syntax level. This is a catalogue and validation layer; it does not claim to parse or execute arbitrary source code.

## ⏱️ Mechanical Tick & Type Engine (MTTE)

Run `pytyliki --mtte` to inspect 20 linked layers. Each row binds a semantic operation (such as **Типизация**), a function designation (**Типизирующий**) and a mechanical tick operation (**Механизирующий**). The C API in `src/mtte.h` provides catalogue enumeration, indexed lookup, name lookup and a deterministic logical tick counter.

The counter uses an explicit nanosecond period, checks integer overflow and supports reset. It is a software model for scheduling and measurement logic: it does **not** change CPU frequency, hardware timers, processor registers, firmware or voltage.

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

The 60 operations define the target engineering policy. They are a capability taxonomy, not a claim that every action is already autonomous.

## APSIA 2.0 polyglot evolution

APSIA extends the existing C11/PREE/HyperFusion workflow with a guarded language-routing planner. The implemented C11 planner maps CPU-bound work to Rust, GPU work to CUDA, analytics to SQL, network I/O to Go, scripting to Python, and sandboxed plugins to WebAssembly. It checks target availability and FFI-adjusted cost before selecting a route; untrusted plugins fail closed if WASM is unavailable.

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

## 🧪 Quality audit and CI

The Windows/Linux CMake + CTest matrix and Recursive Evolution Gate run on pull requests. Verified auto-merge requires independent approval tied to the PR head and fails closed on API errors or stale validation events. Configure GitHub branch protection to require the relevant checks and review.

## Safety and implementation limits

- CPU discovery is read-only.
- The scanner does not execute or rewrite scanned source.
- Pytyliki does not write firmware, BIOS/UEFI, microcode, voltage, clocks, power limits, kernels, drivers, or device registers.
- Hardware rules are advisory; only 12 rules are currently implemented.
- The C11/C23 pipeline is a reference plan, not a production transpiler.
- Benchmark results depend on hardware, operating system, compiler, and workload; they are not guaranteed performance gains.
- CPU Suite Pro 78X currently provides a complete catalogue/API/planning layer; individual hardware backends are not all implemented.

## Compatibility and formats

- **Operating systems:** Windows and Linux are represented in the project CI matrix.
- **Language:** ISO C11 implementation; C23 appears in the reference pipeline plan.
- **Data:** JSON reports and PCM16 WAV audio.
- **Not claimed:** macOS, ARM64, x86_64-specific performance guarantees, Protobuf, Parquet, Arrow, or production RTL/FPGA synthesis unless separately validated.

## License

Pytyliki is distributed under the [MIT License](LICENSE).
