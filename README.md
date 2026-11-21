# Pytyliki — Processor Systems Workstation

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg"></a>
  <img alt="C11" src="https://img.shields.io/badge/core-ISO%20C11-2563EB?style=flat-square">
  <img alt="Windows and Linux" src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-DC2626?style=flat-square">
  <img alt="MIT" src="https://img.shields.io/badge/license-MIT-334155?style=flat-square">
</p>

**A modular C11 workstation for processor discovery, bounded measurements, source diagnostics and local DSP.** Pytyliki provides a native Windows desktop interface and a portable command-line core. Its default posture is inspect, measure and report — never silently modify hardware.

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

## Interface preview

<p align="center"><img src="assets/pytyliki-workstation-preview.svg" alt="Illustrative Pytyliki Win32 workstation layout" width="100%"></p>
<sub>Illustrative reconstruction based on the checked-in Win32 controls, not a screenshot captured from a live Windows session. A live capture is not yet committed.</sub>

## Workbenches

- **Processor inventory:** CPU identity, architecture, logical topology, memory and ISA capabilities. AVX-family usability includes OS-managed XCR0 state checks.
- **Bounded benchmark suite:** processor, memory and matrix workloads with timing and integrity checks. Results are local comparisons, not hardware certification.
- **Static source review:** inspect supported source files and export findings without compiling, executing or rewriting them. Report export, sharing and telemetry actions have separate consent-aware role gates.
- **Audio / DSP:** PCM16 WAV import/export, test-tone generation, low-pass filtering, compression, saturation, gain and metering.
- **Memory kernel:** typed views, overflow-checked slices, bounded arenas, rewind checkpoints, telemetry and fixed-block pools.
- **TRACE measurement provenance:** bounded C11 records preserve metric, unit, context, source digest, exact sampling window, sample count and declared uncertainty. Comparisons reject incompatible contexts instead of blending them ([design](docs/TRACE_MEASUREMENT_PROVENANCE.md)).
- **Metric engine:** vector norms, distances, orthogonality, weighted quadratic forms, geometry and variance operations.
- **Embedded C workbench:** reference MCU profiles, FLASH/SRAM budgeting, masked simulated MMIO and starter generation. It is not a physical register mapper or firmware flasher.
- **Engineering catalogue:** 66 systems-language capability entries, 14 CPU/MCU/virtual-ISA profiles, APSIA routing and a 33-function processor/blockchain systems lab.
- **Interop:** versioned Pytyliki C ABI v1 for native consumers.
- **QCS frontend:** native C11 lexer, frame-aware parser, compact AST and CLI for domain/frame validation; syntax inspection only, not compilation or hardware control ([spec](docs/QCS_NATIVE.md)).

## Build and test

Requirements: CMake 3.16+, a C11 compiler and Git. Windows and Linux are covered by CI; the Win32 GUI is Windows-only.

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

On Windows, the build also produces `pytyliki-gui.exe`.

Useful commands:

```bash
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --systems
./build/pytyliki --embedded profiles
./build/pytyliki-metrics --demo
./build/pytyliki --json report.json
```

Use `build/Release/pytyliki.exe` on a typical Windows multi-config build.

## Evolution and project scope

The 0.5.0 workstation baseline (16 Nov 2026) centered on processor profiling, source inspection and DSP. Subsequent mainline work expanded the C11 modules and tests, added the memory and metric kernels, native ABI, Embedded C reference tools, architecture and language catalogues, the systems lab, and a versioned project contract. The current source and tests are authoritative for what is implemented.

## Safety and limits

CPU discovery is read-only. Benchmarks are bounded and hardware-dependent. Source review never runs the inspected program. DSP processes local buffers only. Hardware-rule scores and APSIA routes are advisory. No BIOS/UEFI, firmware, microcode, voltage, clock, power-limit, driver or device-register writes are performed. Toolchain discovery checks names on `PATH`; it does not install or run discovered compilers.

## Documentation

[TRACE measurement provenance](docs/TRACE_MEASUREMENT_PROVENANCE.md)

[Human-first governance](docs/HUMAN_FIRST_GOVERNANCE.md) · [UI/UX system](docs/UI_UX_SYSTEM.md) · [Architecture](docs/ARCHITECTURE.md) · [Memory kernel](docs/MEMORY_ARENA.md) · [Embedded C](docs/EMBEDDED_C.md) · [Systems-language atlas](docs/SYSTEMS_LANGUAGE_ARCHITECTURE_ATLAS.md) · [Unified engineering standard](docs/UNIFIED_ENGINEERING_STANDARD.md)


[Doctrine references](docs/HUMAN_FIRST_GOVERNANCE.md) · [Anatidae design system](docs/ANATIDAE_DESKTOP_DESIGN_SYSTEM.md) · [UI/UX architecture](docs/UI_UX_ARCHITECTURE.md) · [12 animation principles](docs/DISNEY_MOTION_SYSTEM.md) · [Clinical safety architecture](docs/clinical-safety-architecture.md) · [Project contract](docs/UNIFIED_PROJECT_CONTRACT.md)
[Windows releases](https://github.com/Ivan-VHS/Pytyliki/releases) · [Desktop build workflow](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml)

## Anatidae engineering doctrine

- **Human-first and consent-aware:** discovery, analysis and export stay distinct; report export, sharing and telemetry use separate consent-aware role gates.
- **Bounded and safe by default:** validate inputs, overflow, slice bounds and workload limits. Processor discovery and source review remain read-only; no BIOS/UEFI, firmware, microcode, driver or device-register writes are performed. Simulation is labelled.
- **Accessible workstation UX:** coherent design tokens, clear hierarchy, keyboard operation, visible focus, sufficient contrast, localization and reduced-motion accommodation.
- **Purposeful motion:** animation supports state and feedback; it never replaces readable status or becomes a requirement for completing a task.
- **Evidence-led claims:** distinguish implemented modules from reference profiles, simulations and advisory scores. Source and tests define the actual capability boundary.

This is the project's design baseline, not a claim that every proposed doctrine item is already implemented.

## Documentation governance

[Documentation control](docs/DOCUMENTATION_CONTROL.md) defines the normative rules for public claims, evidence, privacy and integrations.

## License

[MIT](LICENSE)

UI motion and accessibility: [12 animation principles](docs/DISNEY_MOTION_SYSTEM.md).


## Quality and commercial readiness

The 20 × 3 matrix combines product quality, operational discipline and commercial governance with traceable repository evidence. Run **python tools/quality_commercial.py** or add **--format json**. Its scores do not imply hardware certification, market validation, revenue or profitability. See [the quality-commercial standard](docs/QUALITY_COMMERCIAL_STANDARD.md).
