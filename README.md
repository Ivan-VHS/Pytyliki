# Pytyliki — CPU Lab 0.5.0

A safe, native C11 workstation utility for CPU profiling, bounded local benchmarking,
read-only source review, and advisory hardware-optimization analysis.

## Features
- Native C11 core; Windows GUI (Win32) and cross-platform CLI (Windows/Linux).
- CPU vendor/family/model and logical processor count where available.
- Bounded deterministic integer benchmark; results are informational, not certification.
- Read-only source scanner for C, C++, and Rust; inspected source is never executed.
- Hardware Optimization Rule Engine: 12 executable advisory rules and a reserved
  1,000-ID catalog organized into 20 engineering domains.
- Analysis of supplied IPC, cache miss, branch prediction, memory stall, power,
  temperature, frequency and utilization measurements.
- CMake build, CTest coverage, and Windows/Linux CI.

## Build
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

On Windows, the build also produces `pytyliki-gui`. On Linux, it produces `pytyliki`.

## CLI
```sh
pytyliki --profile
pytyliki --benchmark
pytyliki --analyze path/to/source.c
pytyliki --json report.json
```

## Hardware rule engine
The C API is in `src/hw_rules.h`; implementation is in `src/hw_rules.c`.
See [docs/HARDWARE_RULE_ENGINE.md](docs/HARDWARE_RULE_ENGINE.md) for the 20-domain
catalog, rule lifecycle, evidence contract and safety boundaries.

The current 12 rules are executable; the remaining catalog IDs are reserved and are
not represented as implemented. Metrics must be supplied by a trusted profiler or
simulator. Missing measurements remain unknown. Thresholds are advisory and require
platform/workload calibration.

## Safety model
All analysis is read-only. No BIOS/UEFI writes, firmware flashing, microcode writes,
kernel/driver mutation, privileged device-register access, automatic voltage/clock
changes, or automatic source/RTL modification are performed. Hardware recommendations
are advisory. Physical design changes require HDL/RTL access, process libraries,
formal verification, timing closure, power/area analysis and human approval.

## Project layout
- `src/core.c`: CPU profile, bounded benchmark, static scanner, JSON report.
- `src/hw_rules.h`, `src/hw_rules.c`: hardware optimization rule API and evaluator.
- `src/cli.c`: portable command-line interface.
- `src/win_gui.c`: optional native Win32 GUI.
- `tests/`: core and hardware-rule unit tests.
- `.github/workflows/ci.yml`: Windows/Linux build and test matrix.

License: MIT.
