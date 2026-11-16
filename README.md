# Pytyliki — CPU Lab 0.4.0

A safe, native C11 workstation utility for inspecting CPU capabilities, running a bounded local benchmark, and statically reviewing source code. It does **not** overclock, flash firmware, alter BIOS settings, write CPU microcode, or change voltage/frequency controls.

## Features
- Native C11 core; Windows GUI (Win32) and cross-platform CLI (Windows/Linux).
- CPU vendor/family/model and logical processor count where available.
- Bounded deterministic integer benchmark; results are informational, not a hardware certification.
- Read-only source scanner for C, C++ and Rust. It flags selected risky API patterns; it never compiles or executes inspected input.
- Clinical-safety engineering prototype library: pseudonymous contact-graph review, sterilization-profile checks and chain-digest interface, CPR measurement flags, RSI checklist review, model-label disagreement and biomarker trend math.
- JSON report output with escaped strings.
- CMake build and CTest; GitHub Actions build/test workflow.

## Build
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    ctest --test-dir build --output-on-failure

On Windows, the build also produces the native pytyliki-gui application. On Linux, it produces pytyliki.

## CLI
    pytyliki --profile
    pytyliki --benchmark
    pytyliki --analyze path/to/source.c
    pytyliki --json report.json

Options can be combined. With no options, a short profile and benchmark summary is printed. Use --help for usage.

## Clinical module safety boundary

The clinical-safety interfaces are engineering prototypes, not approved clinical decision support or medical-device software. They do not diagnose, prescribe, order biopsy, determine sterility, command defibrillators, integrate live sensors, or operate hospital access controls. Parameters and workflow checklists must come from a locally approved and versioned source. Clinical decisions remain with qualified professionals and institutional governance.

The contact-graph API expects caller-pseudonymised identifiers. The sterilization digest API requires a caller-supplied reviewed SHA-256 provider and does not itself provide immutable storage or signatures. The biomarker function estimates concentration trend only, not tumour-volume doubling time or a biopsy trigger. Full limits and integration boundaries are documented in docs/clinical-safety-architecture.md.

## Existing CPU Lab safety model

CPU analysis is static and read-only. Files are opened for reading only. Benchmark work is capped and runs in the current process. No privileged operations, kernel interfaces, device writes, firmware operations, or automatic source modifications are implemented. Recommendations are advisory and must be reviewed by a human.

## Project layout
- src/core.c and src/core.h: CPU profile, bounded benchmark, static scanner, JSON report.
- src/clinical_safety.c and src/clinical_safety.h: clinical-safety engineering utilities.
- src/cli.c: portable command-line interface.
- src/win_gui.c: optional native Win32 GUI.
- tests/: deterministic unit tests, including clinical module fixtures.
- docs/clinical-safety-architecture.md: module scope, data flow and production-readiness gates.
- .github/workflows/ci.yml: Windows/Linux build and test matrix.

License: MIT.
