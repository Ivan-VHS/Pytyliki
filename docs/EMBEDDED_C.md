# Embedded C native core

Pytyliki exposes a small ISO C11 Embedded C component to its native CLI,
Win32 workstation and C API consumers. The component concentrates on portable
metadata and deterministic host-side simulation.

## MCU catalogue

The initial profiles are:

| Profile | Architecture | Compiler candidate | Clock | FLASH | SRAM | Stack reserve |
|---|---|---|---:|---:|---:|---:|
| cortex-m4-reference | ARM Cortex-M4 | arm-none-eabi-gcc | 84 MHz | 512 KiB | 128 KiB | 2 KiB |
| riscv32imc-reference | RISC-V RV32IMC | riscv64-unknown-elf-gcc | 50 MHz | 256 KiB | 64 KiB | 2 KiB |
| avr-atmega328p | AVR ATmega328P | avr-gcc | 16 MHz | 32 KiB | 2 KiB | 256 B |

The Cortex-M4 and RV32IMC entries are explicitly **reference profiles**. Do not
treat their memory map as a board definition. Even the named AVR profile needs
the board's clock, fuse and peripheral configuration checked.

## CLI

~~~sh
./build/pytyliki --embedded profiles
./build/pytyliki --embedded profile cortex-m4-reference
./build/pytyliki --embedded budget cortex-m4-reference 8192 1024 128 512 2048
./build/pytyliki --embedded template ./main.c
~~~

The budget arguments are \`TEXT RODATA DATA BSS STACK\`. FLASH accounting is
\`TEXT + RODATA + DATA\`; SRAM accounting is \`DATA + BSS + NOINIT + STACK\`.
All additions are overflow-checked. An over-budget result returns exit code 3.

The generated C starter is hardware-neutral and has no board-specific GPIO,
reset vector, clock tree or startup initialization. Add the vendor startup
sequence and confirm the linker memory regions before using an image on hardware.

## Native API

\`src/embedded.h\` exports:

- MCU profile enumeration and lookup.
- overflow-checked FLASH/SRAM budget evaluation;
- a bounded simulated register bank with read/write access and writable masks;
- a C starter-file generator and a compact CLI dispatcher.

The simulated register bank stores values in ordinary process memory. It never
dereferences an MCU address, maps host MMIO or communicates with a physical
device. A write containing bits outside the writable mask is rejected.

## Desktop and Reaper integration

The Win32 desktop has an Embedded C profile inspector. Reaper supplies the
project scaffolding, GNU cross-toolchain invocation, linker-map / ELF section
analysis, CMSIS-SVD import and simulator/telemetry utilities. The two components
share the same reference profile intent, but they do not yet exchange a binary
profile file or provide flash/debug transport.

## Build and tests

~~~sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

The \`embedded-c-core-tests\` target exercises profile lookup, memory budget
boundaries, integer overflow rejection and simulated register permissions.


## Public C ABI v1 bridge

The existing `pytyliki_ffi` shared library also exports the native Embedded C
catalogue and the same overflow-checked memory-budget calculation used by the
CLI. Reaper and other explicit FFI consumers can call:

- `pytyliki_v1_embedded_profile_count(uint64_t *out)`
- `pytyliki_v1_embedded_profile_at(uint64_t index, pytyliki_embedded_profile_v1 *out)`
- `pytyliki_v1_embedded_budget_evaluate(const char *profile_id, const pytyliki_embedded_sections_v1 *sections, pytyliki_embedded_budget_v1 *out)`

The structs use fixed-width fields and fixed-size text arrays. They expose
profile metadata, compiler flags, clock hints and FLASH/SRAM regions. Budget
evaluation is host-side arithmetic; it never maps physical MMIO or writes a
device register. Profile addresses are reference metadata and must be verified
against the exact MCU and board.

Consumers should compile against the installed `include/pytyliki/interop.h`.
The ABI version remains 1; these functions are additive and do not change the
existing vector API.
