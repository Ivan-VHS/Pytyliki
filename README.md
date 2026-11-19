# Pytyliki — Processor Systems Workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

**A modular C11 workstation for local processor inspection, bounded benchmarks, source diagnostics and PCM16 audio/DSP.** The project includes a native Win32 desktop interface and a CLI.

## Current capabilities

- **Processor inventory:** CPU identity, logical processors, memory and usable ISA capabilities, including OS vector-state checks.
- **Benchmarks:** bounded processor, memory and matrix workloads for local comparison; results depend on hardware, compiler and OS.
- **Source diagnostics:** inspect supported source files without executing or rewriting them; export JSON reports.
- **Audio/DSP:** PCM16 WAV import/export, tone generation, low-pass filtering, compression, saturation, gain and level metering.
- **Planning catalogues:** Syntax-pie, improvement matrix, HyperFusion and APSIA routing plans. These are advisory models, not autonomous optimizers or compilers.
- **Desktop:** native Windows GUI for processor scans, source review and audio controls.

## Screenshots

No verified capture from a running application is currently checked into this repository. The previous illustrative UI mockup is intentionally not presented as a real screenshot. Add captures from the built Win32 application here when available.

## Build and test

Requirements: CMake 3.16+, a C11 compiler, Git, Windows or Linux.

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows builds also produce `pytyliki-gui.exe`. Example CLI commands:

```bash
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --improvement-plan
./build/pytyliki --tone-out test-tone.wav --frequency 440 --seconds 3
./build/pytyliki --json report.json
```

Use `build/Release/pytyliki.exe` on a typical Windows multi-config build.

## Architecture and safety

The project is implemented in ISO C11 and built with CMake/CTest. GitHub Actions validates Windows and Linux builds; a Linux sanitizer job runs AddressSanitizer and UndefinedBehaviorSanitizer.

Processor discovery is read-only. The application does not write firmware, BIOS/UEFI, microcode, clocks, voltages, power limits, kernel settings, drivers or device registers. Source review does not execute scanned files. Benchmark scores are not certification or guaranteed speedups. Advisory catalogues and readiness values are planning aids, not proof of optimization.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [APSIA routing](docs/APSIA_2.0.md)
- [Hardware rule engine](docs/HARDWARE_RULE_ENGINE.md)
- [Processor taxonomy](docs/PROCESSOR_TAXONOMY.md)
- [Platform validation](docs/PLATFORM_VALIDATION_POLICY.md)

## License

[MIT](LICENSE).

> 千里之行，始于足下。 — A journey of a thousand miles begins with a single step.
