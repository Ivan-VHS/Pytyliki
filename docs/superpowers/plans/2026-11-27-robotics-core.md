# Reaper Robotics Core — Pytyliki Implementation Plan

> For agentic workers: implement task-by-task, use test-driven development, and verify every task before marking it complete.

**Goal:** Add a bounded Embedded C11 robotics runtime for all 13 Reaper Robotics Core contracts.
**Architecture:** Four cohesive C11 modules provide timing/quotas/sensor prediction, motion and safety logic, DMA/recorder/tuning primitives, and deterministic host simulation. APIs are fixed-capacity, hardware-neutral and adapter-driven.
**Tech Stack:** C11, CMake, CTest, stdatomic.h, existing Pytyliki test conventions.
**Spec:** docs/ROBOTICS_CORE_V1.md

## Global Constraints

- Keep C_STANDARD 11, C_EXTENSIONS OFF and the existing public ABI stable.
- No allocation, blocking, formatting, network, physical MMIO, firmware flashing or hidden device access in control-path functions.
- Use only bounded fixed-capacity state in new APIs.
- Return explicit status codes; reject invalid input and integer overflow.
- Never claim measured timing proves static WCET or host simulation proves hardware safety.
- Preserve existing MCU profile and MMIO simulation boundaries.
- Use a C11 host simulator for tests; physical hardware requires an explicit qualified HAL.

## Review Focus

- Counter arithmetic near UINT64_MAX and signed conversion overflow.
- Flight-recorder producer/consumer wraparound and full-ring behavior.
- Live-tuning readers overlapping two consecutive publications.
- Sensor timestamps in the future, too old, or beyond the prediction horizon.
- Failure during actuator publication and safe-state callback handling.

## File Map

- Create src/robotics_time.h/.c: timing budget checks, timebase conversion, CPU/bus quotas and sensor prediction.
- Create src/robotics_motion.h/.c: dimensions and quantities, actuator transaction, failsafe, shadow state, thermal/load budget, calibration and limp policy.
- Create src/robotics_io.h/.c: DMA lifecycle, SPSC recorder and reader-pinned tuning snapshots.
- Create src/robotics_sim.h/.c: deterministic fake clock and fake actuator/bus callbacks.
- Create tests/test_robotics_time.c, tests/test_robotics_motion.c, tests/test_robotics_io.c and tests/test_robotics_sim.c.
- Modify CMakeLists.txt to compile/install the four modules and register tests.

---

### Task 1: Timing budgets, isolation quotas and sensor time alignment

**Files:** create robotics_time.h/.c and tests/test_robotics_time.c; modify CMakeLists.txt.
**Produces:** pl_rb_timing_budget, pl_rb_timing_result, pl_rb_quota, pl_rb_sensor_sample, pl_rb_sensor_prediction; check, quota-consume and predict functions.
**Checks:** exact deadline boundary, execution overrun, positive/invalid periods, quota exhaustion and window rollover, future/stale sample rejection, prediction overflow and maximum horizon.

### Task 2: Quantities, calibration, actuator frames and failsafe policy

**Files:** create robotics_motion.h/.c and tests/test_robotics_motion.c; modify CMakeLists.txt.
**Produces:** typed dimension/unit descriptors, checked add/convert/multiply; fixed-size actuator staging and publication callback; explicit failsafe action table; calibration validation/apply.
**Checks:** mm↔m and mA↔A conversion, incompatible dimension rejection, arithmetic overflow, no partial frame publication, callback failure, invalid calibration range, max-step and stuck sample rejection.

### Task 3: Shadow model, thermal derating and graceful limp

**Files:** extend robotics_motion.h/.c and tests/test_robotics_motion.c.
**Consumes:** the quantity, calibration and status types from Task 2.
**Produces:** fixed-point shadow prediction/residual comparison, saturating load integration and derating result, fault-mask-to-approved-mode selection.
**Checks:** exact tolerance boundary, residual overflow, thermal saturation, temperature trip, monotonic derating, no configured fallback and deterministic mode selection.

### Task 4: DMA ownership and live tuning snapshots

**Files:** create robotics_io.h/.c and tests/test_robotics_io.c; modify CMakeLists.txt.
**Produces:** aligned DMA descriptor/state transitions; bounded tuning slots with atomic active index and reader pins.
**Checks:** invalid alignment/direction, double-submit, reclaim-before-complete, stale token, writer refusal while a slot is pinned, generation conflict and acquire/release balance.

### Task 5: Flight recorder

**Files:** extend robotics_io.h/.c and tests/test_robotics_io.c.
**Produces:** fixed-size SPSC recorder with constant-size records, monotonic sequence numbers, drop-newest overflow policy and dropped counter.
**Checks:** FIFO order, empty/full behavior, drop accounting, wraparound and no allocation in the API.

### Task 6: Deterministic hardware simulator and repository integration

**Files:** create robotics_sim.h/.c and tests/test_robotics_sim.c; update CMakeLists.txt and docs/ROBOTICS_CORE_V1.md.
**Consumes:** all three runtime modules.
**Produces:** fake monotonic clock and fake batch actuator/bus adapter; end-to-end fault, timing, DMA, tuning and recorder scenarios.
**Checks:** exact deterministic replay for identical input streams; simulator is explicitly labelled host-only and cannot report physical qualification.

## Commit sequence

1. Add failing C tests and CTest targets.
2. Implement timing and sensor contracts.
3. Implement motion/safety contracts.
4. Implement DMA and live-tuning lifecycle.
5. Implement flight recorder.
6. Implement simulator and run complete CTest plus sanitizer build.
7. Review the entire diff, confirm public headers are installed, and open a draft PR; do not merge.
