# Robotics Core v1 — Pytyliki Runtime Contract

## Purpose

Provide a deterministic, allocation-free-by-default Embedded C11 foundation for the 13 robotics capabilities defined by Reaper. The library is hardware-neutral; it models contracts and offers adapter boundaries. It must not imply that a host simulation is proof of physical timing, multi-actuator atomicity, electrical safety, or board qualification.

## Ownership split

- Reaper owns the canonical source-level contracts, dimensional model, state-transition rules, and machine-readable robotics semantics.
- Pytyliki owns bounded C11 data structures, deterministic runtime helpers, simulation adapters, and native tests.
- A target HAL owns actual timer reads, interrupt entry/exit, cache maintenance, DMA programming, bus transactions, safe-output activation, and physical actuator writes.
- Board profiles remain advisory until validated against vendor documentation, linker maps, target compiler output, and hardware-in-the-loop evidence.

## The 13 runtime contracts

1. **Timing budget:** represent period, execution ceiling, deadline and jitter separately. Runtime measurement checks observed execution against a declared budget. Static WCET is not inferred from observed time.
2. **Physical quantities:** represent unit and dimension explicitly; reject addition/comparison across incompatible dimensions and require explicit conversions. Integer/fixed-point arithmetic is preferred for control paths.
3. **Isolation quotas:** account CPU ticks and bus units per domain over a fixed window. Accounting is not a scheduler or hardware partition; the HAL/RTOS must enforce preemption and bus arbitration.
4. **Actuator transaction:** stage a complete bounded setpoint frame, validate it, and publish it as one immutable frame at a control-tick boundary. Hardware-wide atomic writes require an adapter that supports a true batch operation.
5. **Failsafe:** bind fault conditions to explicit actions and bounded response policy. A callback can request a safe action; only a qualified HAL can physically enforce it.
6. **Shadow step:** compare model-predicted state with observed state using bounded integer arithmetic and configured tolerances.
7. **Thermal/load budget:** integrate bounded current/load exposure, observe temperature limits and return a deterministic derating factor.
8. **Sensor time alignment:** retain sample timestamp and predict from capture time to control time with a bounded constant-velocity model. Prediction is rejected on stale, future, or over-horizon samples.
9. **DMA lifecycle:** track CPU-owned → device-owned → completed → CPU-owned transitions. Cache operations and DMA programming remain explicit HAL obligations. C API state checks are runtime guards, not a substitute for Reaper's future static borrow checking.
10. **Flight recorder:** fixed-capacity SPSC ring, no allocation, no blocking, no formatting in the producer path; overflow drops the newest event and increments a counter.
11. **Calibration:** validate raw range and maximum step, apply bounded offset/gain, and reject invalid or stuck samples before control use.
12. **Live tuning:** validate a complete versioned parameter set, then publish it at a tick boundary. Readers acquire/release snapshots; a slot cannot be overwritten while readers hold it.
13. **Graceful limp:** map a fault mask to the most capable explicitly allowed mode; never invent a fallback or assume a failed actuator can still produce torque.

## C11 API shape

The implementation is split into focused public headers:

- src/robotics_time.h: timing budgets, monotonic time adapter, per-domain CPU/bus quotas and sensor timestamp prediction.
- src/robotics_motion.h: dimensional quantities, actuator-frame transactions, failsafe policy, shadow-state comparison, thermal derating, calibration and limp-mode selection.
- src/robotics_io.h: DMA ownership, fixed SPSC flight recorder and versioned live-tuning snapshots.
- src/robotics_sim.h: deterministic fake clock, bus/register and actuator-frame adapter for host tests.

All data structures have explicit capacities. Core functions return status codes; they do not allocate, log, sleep, access global hardware, or silently fall back to floating-point operations. Where atomics are used, memory ordering is specified in the API. No network dependency is introduced.

## Safety and evidence boundaries

- No board flashing, bootloader writes, driver installation, or physical MMIO is performed.
- A timing budget check reports observed budget compliance only. It does not certify WCET.
- A software actuator frame is atomically published in memory; it does not make a sequence of physical motor-driver writes atomic.
- DMA ownership state is checked by the API; platform cache and bus coherency must be supplied by the HAL.
- Shadow-model thresholds, thermal limits, sensor horizon, calibration and limp transitions are configuration inputs requiring system-specific validation.
- Simulator tests prove deterministic software behavior only, not real-world robot stability or functional safety.

## Acceptance

- All 13 capability areas have a corresponding public contract and tests.
- All fixed-capacity APIs reject invalid input, overflow and illegal state transitions without partial mutation.
- Producer-path recorder and control-path helpers perform no dynamic allocation.
- Existing Pytyliki public APIs, ABI and C11 baseline remain compatible.
- Linux, Windows and sanitizer CI build/test the new library.
- Reaper and Pytyliki schemas agree on names, dimensions, state transitions and status meanings.
