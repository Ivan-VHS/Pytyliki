# Pytyliki — CPU Lab 0.4.0

A safe, native C11 workstation utility for inspecting CPU capabilities, running a bounded local benchmark, and statically reviewing source code. It does **not** overclock, flash firmware, alter BIOS settings, write CPU microcode, or change voltage/frequency controls.

## Features
- Native C11 core; Windows GUI (Win32) and cross-platform CLI (Windows/Linux).
- CPU vendor/family/model and logical processor count where available.
- Bounded deterministic integer benchmark; results are informational, not a hardware certification.
- Read-only source scanner for C, C++, and Rust. It flags selected risky API patterns; it never compiles or executes inspected input.
- JSON report output with escaped strings.
- CMake build and CTest; GitHub Actions build/test workflow.

## Build
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

On Windows, the build also produces the native `pytyliki-gui` application. On Linux, it produces `pytyliki`.

## CLI
```sh
pytyliki --profile
pytyliki --benchmark
pytyliki --analyze path/to/source.c
pytyliki --json report.json
```
Options can be combined. With no options, a short profile and benchmark summary is printed. Use `--help` for usage.

## Safety model
Analysis is static and read-only. Files are opened for reading only. Benchmark work is capped and runs in the current process. No privileged operations, kernel interfaces, device writes, firmware operations, or automatic source modifications are implemented. Recommendations are advisory and must be reviewed by a human.

## Project layout
- `src/core.c`: CPU profile, bounded benchmark, static scanner, JSON report.
- `src/cli.c`: portable command-line interface.
- `src/win_gui.c`: optional native Win32 GUI.
- `tests/test_core.c`: deterministic unit tests.
- `.github/workflows/ci.yml`: Windows/Linux build and test matrix.

License: MIT.
