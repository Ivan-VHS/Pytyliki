# Reaper Processor Silicon Runtime

Status: native C11 implementation, API version 1  
Scope: explicit hardware primitives for embedded and native processor targets

## Design boundary

The public API is exposed by processor_silicon.h. The existing processor.c remains responsible for host CPU discovery and feature inventory; embedded.c remains responsible for MCU profiles and FLASH/SRAM budgeting. Reaper programs may describe and validate these operations, while native C remains the only layer that performs explicit register access.

This module does not install drivers, change firmware, clocks, voltage, frequency, CPU affinity or interrupt vectors. It does not start OS threads, execute arbitrary code, or claim that a synthetic trace is live hardware telemetry.

## Public functions

| Area | Functions | Contract |
|---|---|---|
| Target identity | pl_silicon_architecture_current, pl_silicon_architecture_name | Build-target identity, not a complete runtime CPU inventory |
| Barriers | pl_silicon_barrier_execute | Reports unsupported semantics instead of silently substituting a weaker fence |
| Cycle sources | pl_silicon_cycle_source_native, pl_silicon_cycle_source_dwt, pl_silicon_dwt_enable, pl_silicon_cycle_read | Explicit counter selection; DWT register writes require a direct call |
| Cycle accounting | pl_silicon_cycle_accumulator_init, pl_silicon_cycle_accumulator_sample | Handles 32-bit counter rollover and saturates long-running totals |
| MMIO | pl_silicon_mmio_region_init, pl_silicon_mmio_read, pl_silicon_mmio_write | Requires a mapped base, allowlisted bus range, width mask and explicit permissions |
| Code layout | pl_silicon_isa_image_validate | Validates target ISA alignment and bounds only; does not decode or execute instructions |
| Fault evidence | pl_silicon_fault_snapshot_validate | Validates a caller-supplied read-only exception snapshot |
| Trace | pl_silicon_trace_ring_init, pl_silicon_trace_ring_is_lock_free, pl_silicon_trace_ring_push, pl_silicon_trace_ring_pop, pl_silicon_trace_ring_dropped | Bounded single-producer/single-consumer ring using C11 atomics |

## Architecture rules

### Cycle counters

- x86 uses an ordered RDTSC read and reports TSC ticks, not guaranteed core cycles. TSC frequency is left unknown unless the platform supplies a separately measured or trusted frequency.
- Cortex-M uses an explicitly supplied DWT->CYCCNT pointer. DWT setup is accepted only when the compiler identifies the M profile through __ARM_ARCH_PROFILE == 'M' or the board explicitly defines PYTYLIKI_ENABLE_CORTEX_M_DWT; it then checks DWT_CTRL.NOCYCCNT, sets DEMCR.TRCENA and DWT_CTRL.CYCCNTENA, and optionally resets the counter. The call assumes privileged execution and valid CMSIS/device register mappings. The explicit build flag is a board/toolchain assertion, not runtime silicon discovery. Check the exact MCU reference manual because DWT can be absent or reduced; bind the cycle source after successful setup.
- RISC-V cycle CSR access is not enabled automatically. A build may opt in with PYTYLIKI_ENABLE_RISCV_CYCLE_CSR only after the execution environment confirms that counter access is permitted. RV32 reads use a high/low/high retry sequence.
- Counter deltas are modulo the source width. Do not compare timestamps across cores unless the platform documents a shared, synchronized counter.

### Memory barriers

- ARM maps data, completion and instruction barriers to DMB, DSB and ISB respectively.
- RISC-V maps data ordering to FENCE iorw,iorw so memory and device-I/O accesses are ordered, and instruction-stream synchronization to FENCE.I only when the build declares Zifencei. A generic FENCE is not reported as a DSB-equivalent completion operation.
- x86 maps data ordering to MFENCE (only on targets with SSE2) and instruction serialization to CPUID. A DSB-like device completion operation is reported unsupported by this generic layer.
- A compiler memory clobber is not a hardware fence. Architecture barriers and compiler ordering are handled separately. An instruction barrier is not a universal cache-maintenance routine: self-modifying/JIT code may additionally require data-cache clean, instruction-cache invalidate, completion barriers and cross-hart coordination according to the target architecture. This module does not patch or execute code.

### MMIO

A region must be initialized with the hardware bus base, a pointer already mapped by the platform, byte size, permitted widths, permissions, and barrier policy. Every access validates width, alignment, address overflow and region bounds before touching memory. volatile ensures the access is emitted; it does not provide inter-thread synchronization or make an unmapped address safe.

The caller owns mapping lifetime, MPU/PMA attributes, privilege, cacheability, bus faults, endian policy, and register-specific side effects. Do not pass guessed peripheral addresses. Derive addresses and access widths from the exact SoC reference manual, CMSIS/SVD or vendor header. Respect write-one-to-clear, read-to-clear, reserved-bit and side-effect semantics in the device-specific layer. This generic API cannot infer those semantics.

### Machine code and traps

pl_silicon_isa_image_validate validates only the image's declared ISA, entry alignment and bounded layout. It deliberately reports instruction_decode_performed = false. Thumb-2 and RISC-V compressed encodings have mixed instruction widths; x86 has variable-length instructions. A fixed 16- or 32-bit word stream is not a portable representation of these ISAs.

The validator does not claim that the ISA matches the currently running CPU and does not validate instruction semantics, relocations, permissions or executable-memory policy. The module records a caller-supplied fault snapshot but does not replace vector tables or install exception handlers. Vector ownership, stacking rules and fault-register interpretation remain platform-specific. fault_address_valid is authoritative: address zero is not treated as an invalid sentinel.

### Trace ring

The ring supports exactly one producer and one consumer at a time. Call pl_silicon_trace_ring_init before querying lock-free status or using it. pl_silicon_trace_ring_is_lock_free must return true before considering it for interrupt-context use; C11 atomics are not guaranteed to be lock-free on every MCU. A full ring drops the incoming record and increments the drop counter.

## Example: Cortex-M DWT

    #include "processor_silicon.h"

    static pl_silicon_cycle_source cycles;

    void board_cycle_counter_init(void) {
        const pl_silicon_dwt_registers dwt = {
            &CoreDebug->DEMCR,
            &DWT->CTRL,
            &DWT->CYCCNT
        };

        if (pl_silicon_dwt_enable(&dwt, true) != PL_SILICON_OK) {
            /* Report board-specific startup failure. */
            return;
        }
        (void)pl_silicon_cycle_source_dwt(&cycles, &DWT->CYCCNT, SystemCoreClock);
    }

The example requires the target's CMSIS device header and a Cortex-M implementation with DWT cycle counting. It is not a portable startup routine for every Cortex-M part.

## Example: explicitly mapped MMIO window

    pl_silicon_mmio_region gpio;
    uint32_t output;

    /* gpio_window must already be mapped by the board/OS integration. */
    if (pl_silicon_mmio_region_init(
            &gpio, GPIOA_BASE, gpio_window, GPIO_WINDOW_BYTES,
            PL_SILICON_WIDTH_32,
            PL_SILICON_MMIO_READ | PL_SILICON_MMIO_WRITE,
            PL_SILICON_BARRIER_DATA,
            PL_SILICON_BARRIER_COMPLETION) == PL_SILICON_OK) {
        (void)pl_silicon_mmio_read(&gpio, GPIOA_ODR, 32u, &output);
    }

On architectures where the selected barrier is unsupported, the access fails before touching the mapped region. Choose a barrier policy supported by the target and peripheral memory attributes.

## Build and tests

The module is part of pytyliki_core; its public header is installed under include/pytyliki. Run the normal CMake/CTest suite. The focused tests use RAM-backed fake registers and counters: they prove bounds checks and API behavior, not access to a physical board.

A real board validation still requires the exact MCU/SoC, compiler, board memory map, linker script, startup code, CMSIS/vendor headers and hardware test evidence. These host-side tests do not validate register side effects, clock-tree setup, interrupt entry, cache coherency, electrical behavior or timing guarantees on silicon.
