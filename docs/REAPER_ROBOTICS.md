# Reaper Robotics Core

**Status:** initial implemented core; simulator-only.  
**Reaper contract:** `robotics/robotics_core.rpr`  
**Machine-readable contract:** `robotics/contracts.v1.json`  
**Compiler contract:** `robotics/compiler_contract.v1.json`

Reaper Robotics Core gives Pytyliki a deterministic robotics subsystem while keeping the language-level contracts in Reaper. The native implementation is C11 and reuses Pytyliki's bounded Embedded C register bank. The Reaper source is the executable fixed-point reference contract used by CI.

## The three implementation stages

### 1. Reaper Robotics Core — implemented

The native API in `src/robotics.h` and `src/robotics.c` exposes thirteen operation groups:

1. Sensor sampling and range validation.
2. Two-point sensor calibration.
3. Bounded moving-average filtering.
4. 2D coordinate transforms.
5. Two-link forward kinematics.
6. Two-link inverse kinematics with elbow-branch selection.
7. Time-bounded trajectory interpolation.
8. Actuator position and slew limiting.
9. PID control with integral and output clamps.
10. Coordinated multi-axis stepping.
11. Safety supervision for axis bounds, temperature and watchdog heartbeat.
12. Versioned packet encoding/decoding with CRC16-CCITT.
13. A deterministic six-axis simulator.

The supporting runtime primitives are an allocation-free 256-byte ring buffer, an explicit logical timer, a permission/mask-checked in-memory register bank, optimistic generation-checked state transactions, rollback, and structured diagnostics.

The native C interface uses `double` for geometry and control calculations, caller-selected length units, radians for inverse-kinematics angles, milliseconds for trajectory time, and seconds for PID delta time. The Reaper contract uses signed integer Q15 values (`32768 == 1.0`) for coordinates and trigonometric components. Conversion boundaries must be explicit; values from the two representations must not be mixed without conversion.

### 2. Reaper Robotics Compiler — contracts integrated; full native AOT remains open

The Reaper module implements the fixed-point reference algorithms and is checked/executed by Reaper's existing language toolchain in CI. `robotics/compiler_contract.v1.json` defines the typed surface, HIR operation names, required MIR invariants, effects and backend acceptance gates for all thirteen operations.

The current Reaper native compiler is still a scalar C17 subset. It does not yet lower the robotics module's aggregate results, collections, loops and hardware-effect model into native objects. Therefore this integration does **not** claim full robotics AOT parity, a self-hosted compiler, or a complete robotics IR/MIR pipeline. Those are explicit compiler milestones, not inferred from a passing reference-runtime check.

### 3. Robotics Specification — implemented

`robotics/contracts.v1.json` records public operation names, inputs/outputs, mathematical models, units, limits, effects, safety invariants, simulator boundaries and required verification evidence. The compiler contract is separate so language semantics are not confused with what the current backend can emit.

## Build and verify

Native C11 core test:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --target pytyliki-robotics-tests
ctest --test-dir build --output-on-failure -R reaper-robotics-core-contract-tests
```

Reaper reference contract (run from a checkout of the pinned Reaper toolchain):

```sh
PYTHONPATH=../Reaper python -m reaper --check robotics/robotics_core.rpr
PYTHONPATH=../Reaper python -m reaper robotics/robotics_core.rpr
```

The second command must print the exact line `PYTYLIKI_REAPER_ROBOTICS:PASS`. Python here is only the current host entry point for Reaper's existing reference toolchain; the robotics implementation and native test suite are C11/Reaper, not a new Python subsystem.

## Hardware and safety boundary

This implementation is deliberately hardware-neutral:

- Register operations use an in-memory simulator bank and never dereference physical addresses.
- No board drivers are installed, no firmware is flashed, and no physical IRQ or DMA operation is performed.
- No actuator command is sent to a real device.
- The simulator uses a deterministic logical clock and bounded integration, not wall-clock sleeps.
- Safety checks report violations; a future host adapter must enforce a fail-safe output policy before any physical actuator can be enabled.

Real MMIO, IRQ and DMA support requires a separately reviewed OS/HAL adapter, capability grants, target-specific memory and cache rules, rollback, and hardware-in-the-loop qualification. An emulator pass is not evidence of physical-board support.
