# Embedded C workbench

The Embedded C workbench now has two explicit layers:

- **Board planning:** reference MCU profiles, FLASH/SRAM budgets, compiler flags, linker-script generation and project templates. Profiles remain advisory until checked against the exact board and vendor documentation.
- **Kernel-control model:** the fixed-capacity C11 model in `src/embedded_kernel.h` and `src/embedded_kernel.c`. It exercises core-state transitions, wrap-safe u32 timer sampling, jitter measurement, saturating pressure accounting, deterministic pressure decisions, cooperative quotas, a bounded trace ring and overflow-safe memory-span checks.

The model uses caller-owned/static storage, fixed-width integer types and no dynamic allocation. It is a single-writer reference component: callers must serialize access. It does not perform a hardware context switch, install an interrupt handler, set CPU affinity, change a clock, map physical memory, program a bus, allocate DMA memory or patch executable code.

## State and failure policy

The lifecycle is `boot → idle → active`, with explicit `diagnostic`, `degraded` and terminal `halted` states. Entry to active work requires the declared CPU, memory, clock and interrupt health flags. Faults move the model to degraded; recovery requires all health flags and an explicit fault-clear step. A halted model cannot be restarted through this API.

Timer samples use unsigned subtraction, which is defined modulo (2^{32}). Intervals over (2^{31}-1) ticks are rejected as ambiguous. The pressure governor throttles diagnostic work before it requests degraded operation or a safe stop; it never deliberately slows a hard real-time path.

## Build and verification

The C11 module is part of `pytyliki_core`. CTest target `embedded-kernel-control-model-tests` covers invalid transitions, fault/recovery rules, wrap-around and ambiguous timer intervals, pressure saturation, quota yield/breath cycles, trace overflow and memory-span bounds.

MMIO remains simulated by the existing register bank. Physical register access, DMA synchronization, cache maintenance, IRQ entry, atomics, context switching and hardware qualification belong to target-specific trusted backends. A CRC is not an authenticity check, and a live executable patch is not exposed by this model.
