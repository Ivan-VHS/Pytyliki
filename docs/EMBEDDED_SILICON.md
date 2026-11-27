# Reaper-style silicon runtime for Embedded C

> Implementation status: **C11 silicon-runtime v1**. Hardware paths are opt-in and board-specific. Host tests use a fake HAL; they are not evidence of operation on a physical MCU.

[Russian implementation guide](EMBEDDED_SILICON_RU.md) · [Embedded C workbench](EMBEDDED_C.md)

## Implemented surface

`src/silicon_runtime.h` and `src/silicon_runtime.c` add a versioned C11 runtime for processor portraits, hardware abstraction, exact MMIO allowlists, cycle accounting, guarded state transitions, fault tracing, thread admission budgets, deterministic route locking, CRC-checked patch staging, table export, fixed-point operations and machine-stream verification.

| Target | v1 support | Important limit |
|---|---|---|
| Cortex-M | DMB/DSB/ISB, WFI, optional DWT CYCCNT | DWT is compiled in only with `PL_SILICON_ENABLE_CORTEX_M_DWT` |
| RISC-V | `fence`, optional `fence.i`, `rdcycle` | privilege controls may deny cycle CSR access |
| x86/x86_64 | `mfence`, CPUID serialization, LFENCE/RDTSC | TSC ticks are not guaranteed core cycles |
| MMIO | volatile 8/16/32-bit HAL callbacks, exact-address allowlist, masks and barriers | board/SoC-specific map required |
| Machine code | Thumb-16 and RV32I arithmetic whitelist verifier; bounded RPR32 interpreter | x86 decoder, native execution and JIT are not implemented |
| Faults | atomic bounded SPSC event ring | multiple concurrent producers need a different queue/IRQ port |
| Patching | in-memory staging and CRC-16 verification | no Flash writes or execution of staged payloads |

## Build and inspect

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
./build/pytyliki --embedded silicon
```

The CLI command reports the binary's compile-time architecture and enabled backend options. It does not probe a connected board, read peripheral registers or modify hardware.

## Hardware integration contract

A board support package must supply the exact MCU/SoC revision, startup and vector table, linker script, clock/power setup, verified memory map, fault handlers, watchdog policy and a `pl_silicon_hal`. The caller registers every MMIO register by exact name/address/width/access/writable mask/barrier. No universal GPIO/RCC/SCB/PLIC address map is embedded in the portable library.

`volatile` preserves observable compiler accesses; it does not provide atomicity or replace memory barriers. ARM uses `dmb sy`, `dsb sy`, `isb sy`; RISC-V uses `fence rw,rw`, `fence iorw,iorw`, and `fence.i` only when the compiler target advertises Zifencei; x86 uses `mfence` and a CPUID serialization sequence for the ISB-like boundary. Unsupported barriers fail explicitly.

Cycle backends are DWT CYCCNT (opt-in Cortex-M), `rdcycle` (RISC-V), and serialized RDTSC (x86). TSC must be calibrated and treated as TSC ticks; account for invariant-TSC capability, virtualization and migration. RISC-V cycle CSR may be privilege-disabled. A 16/32/64-bit modular counter can account for one wrap between samples, not multiple unobserved wraps.

`PL_SILICON_ENABLE_DIRECT_MMIO` compiles native volatile accessors but does not authorize arbitrary addresses: application access still passes through the exact allowlist. Keep this macro out of ordinary workstation builds. Registers with W1C, read-to-clear, FIFO or key-sequence semantics require device-specific methods rather than generic read-modify-write.

## State, fault, concurrency and execution rules

State changes are policy decisions only. `pl_silicon_wait()` invokes the HAL wait callback only after an explicit transition to `STANDBY_WFI`; `SAFE_HALT` requires an unrecoverable condition. No x86 `HLT` path is provided.

The event ring is single-producer/single-consumer. Check `atomic_is_lock_free()` before using its atomics from an ISR, and do not use it with nested/multiple producers. The thread-budget API is admission control for an existing scheduler, not an OS scheduler or affinity controller.

The Thumb-16 and RV32I verifiers accept only a small arithmetic whitelist and reject control-flow/system/memory instructions. Thumb-2 32-bit instructions are outside v1. x86/x86_64 validation returns unsupported until a reviewed full decoder is integrated. A successful whitelist check is not a proof of ABI, memory or control-flow safety.

RPR32 is a separate fixed 32-bit virtual instruction format. `VERIFY_ONLY` has no peripheral side effects; `RPR32_INTERPRET` updates shadow registers and uses allowlisted MMIO. `NATIVE_ASM` and `JIT_TRAMPOLINE` are intentionally rejected. Patch slots are in-memory data staging, not executable live patching.

For the full board bring-up checklist, counter caveats, SPSC rules, MMIO semantics and fixed-point contract, see the [Russian implementation guide](EMBEDDED_SILICON_RU.md).
