# Pytyliki — Processor Systems Workstation

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg"></a>
  <img alt="C11" src="https://img.shields.io/badge/core-ISO%20C11-0039A6?style=flat-square">
  <img alt="platforms" src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-0A1E46?style=flat-square">
  <img alt="license" src="https://img.shields.io/badge/license-MIT-D52B1E?style=flat-square">
</p>

**Pytyliki is a native C11 engineering workstation for processor discovery, bounded measurements, source diagnostics, local DSP and systems analysis.** The Windows desktop is a native Win32 application; the portable core and command-line tools are available on Windows and Linux.

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

## Workbench map

| Area | Purpose |
|---|---|
| Processor inventory | Reads CPU identity, architecture, logical topology, memory and ISA capabilities. AVX-family usability includes OS-managed XCR0 checks. |
| Bounded benchmark suite | Runs CPU, memory and matrix workloads with timing and integrity checks. Results are local comparisons, not hardware certification. |
| Static source review | Inspects supported source files and exports findings without compiling, executing or rewriting the inspected program. |
| Audio / DSP | Imports and exports PCM16 WAV, generates test tones, applies local filtering, compression, saturation and gain, and measures output. |
| Memory kernel | Provides typed views, checked slices, bounded arenas, rewind checkpoints, telemetry and fixed-block pools. |
| Metric engine | Supplies vector norms, distances, orthogonality, weighted forms, geometry and variance operations. |
| Embedded C workbench | Provides reference MCU profiles, FLASH/SRAM budgets, masked simulated MMIO and starter generation. It does not flash hardware. |
| Engineering catalogues | Includes systems-language and CPU/MCU/virtual-ISA profiles, APSIA routing and the processor/blockchain systems lab. |
| TRACE and QCS | Preserves measurement provenance and provides a native syntax/domain frontend; neither is a hardware-control surface. |
| Interoperability | Publishes the versioned Pytyliki C ABI v1 for explicitly configured native consumers. |

## Desktop design: Duck Sovereign Canon

The Win32/GDI workstation follows a strict vertical hierarchy: system and processor actions first, task-specific controls next, then measurements, report and journal. The approved white/blue/red palette is paired with a local serif heading face and a readable local sans-serif. Panels use compact rectangular geometry; keyboard focus, DPI awareness and reduced-motion behavior remain explicit.

The complete standard, including exact tokens, contrast rules, accessibility and offline/security requirements, is in [docs/DUCK_SOVEREIGN_CANON.md](docs/DUCK_SOVEREIGN_CANON.md). Static palette verification is part of CI.

## Build and test

Requirements: CMake 3.16+, a C11 compiler and Git. Windows and Linux are covered by CI; the native Win32 desktop target is Windows-only.

~~~sh
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

On Windows, the build also produces pytyliki-gui.exe.

## Command line

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --systems
./build/pytyliki --embedded profiles
./build/pytyliki-metrics --demo
./build/pytyliki --json report.json
~~~

For a typical Windows multi-config build, use build/Release/pytyliki.exe.

## Safety and implementation boundaries

- Processor discovery is read-only.
- Benchmarks have bounded workloads and are hardware-dependent.
- Source review never runs the inspected program.
- DSP processes local buffers.
- APSIA routes and hardware-rule scores are advisory.
- No BIOS/UEFI, firmware, microcode, voltage, clock, power-limit, driver or device-register writes are performed.
- Toolchain discovery checks names on PATH; it does not install or execute discovered compilers.
- Embedded profiles and masked MMIO are reference/simulation facilities, not physical register access.

The source and tests define the implemented capability boundary. A reference profile, advisory score or future design item is not hardware certification.

## Engineering references

- [Architecture](docs/ARCHITECTURE.md)
- [TRACE measurement provenance](docs/TRACE_MEASUREMENT_PROVENANCE.md)
- [UI/UX system](docs/UI_UX_SYSTEM.md)
- [Unified engineering standard](docs/UNIFIED_ENGINEERING_STANDARD.md)
- [Quality and commercial readiness](docs/QUALITY_COMMERCIAL_STANDARD.md)
- [Four-project federation](https://github.com/Ivan-VHS/Reaper/blob/main/docs/PROJECT_FEDERATION_MATRIX.md)

## License

[MIT](LICENSE)