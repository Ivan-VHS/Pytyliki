# Embedded C integration

Status: implemented as a bounded target catalogue, read-only source inspection API and an additive C ABI v1 extension. It does not flash devices, execute firmware, write processor registers or perform covert runtime injection.

## Target profiles

Pytyliki exposes seven reference profiles through `pl_embedded_target_count`, `pl_embedded_target_at` and `pl_embedded_target_find`:

| Profile | Target | Toolchain hint |
|---|---|---|
| `native-c11` | Host C11 | `cc` |
| `avr-atmega328p` | 8-bit AVR / ATmega328P | `avr-gcc` |
| `arm-cortex-m0` | 32-bit ARM Cortex-M0 | `arm-none-eabi-gcc` |
| `riscv32-imac` | 32-bit RISC-V IMAC | `riscv64-unknown-elf-gcc` |
| `msp430-g2553` | 16-bit MSP430G2553 | `msp430-elf-gcc` |
| `xtensa-esp32` | Xtensa LX6 / ESP32 | `xtensa-esp32-elf-gcc` |
| `wasm32-wasi` | WebAssembly32 / WASI | `clang` |

Profiles are reference metadata, not a claim that the toolchains or board SDKs are installed. The project does not guess MCU clock rates, peripheral register maps, pin assignments or board-specific linker scripts.

## Static source inspection

`pl_embedded_inspect_source` accepts at most 1 MiB and reports source bytes, lines, include directives, likely heap calls, blocking calls, register-oriented markers and host-I/O calls. The scanner is lexical and advisory; it is not a C parser, MISRA checker, timing proof or safety certification. Comments and string literals can contribute to counts.

Warning categories: dynamic allocation (`malloc`, `calloc`, `realloc`, `free`); blocking delays (`sleep`, `usleep`, `HAL_Delay`, `vTaskDelay`); register-oriented code (`volatile`, `GPIO`, `PORT`, `REG_`, `_MMIO`); host I/O (`printf`, `fprintf`, `fopen`).

## Additive C ABI v1 extension

The existing `pytyliki_abi_version() == 1` contract remains unchanged. New functions expose target metadata and source reports: `pytyliki_v1_embedded_target_count`, `pytyliki_v1_embedded_target_at` and `pytyliki_v1_embedded_inspect_source`. Consumers should compile against the current `interop.h`. Calls are synchronous; the library does not retain source pointers or output buffers.

## CLI

```sh
pytyliki --embedded-c
pytyliki --embedded-inspect path/to/firmware.c
```

Inspection does not compile or execute source. Cross-compilation and deployment remain explicit engineering operations outside this interface.
