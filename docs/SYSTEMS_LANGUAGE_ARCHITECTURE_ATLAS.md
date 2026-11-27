# Systems Language and Architecture Atlas

This atlas adds a hardware-facing reference layer to Pytyliki. It separates **processor discovery** from **architecture knowledge**: the live scanner reports what the current build and operating system can actually observe, while the atlas describes common target families and the questions an engineer should validate.

## Runtime interface

- `pytyliki --architectures` lists the immutable target catalogue.
- `pytyliki --language catalog` lists the separate 66-entry processor-language workbench.
- `pytyliki --language show ID` and `target ID ARCH` inspect a source profile and its declared target mask.
- The full registry, detection, structural inspection and build-plan contract is documented in [Processor Language Workbench](PROCESSOR_LANGUAGE_WORKBENCH.md).
- `pytyliki --architecture aarch64` resolves one target profile.
- The library API in `architecture_atlas.h` exposes enumeration, name lookup and a compile-time build-target label.

The registry is allocation-free, deterministic and read-only. A profile is not proof that a compiler, emulator, ABI, or device is installed on the current machine.

## Architecture profiles

| Target | Class | Width | Execution model | Primary diagnostic focus |
|---|---|---:|---|---|
| x86_64 / x86 | Desktop, server, legacy embedded | 64 / 32 | CISC, out-of-order or legacy in-order | CPUID, OS vector state, ABI, cache topology |
| aarch64 / armv7-m | Mobile, server, MCU | 64 / 32 | Load/store RISC | NEON/SVE, NVIC, memory map, interrupt latency |
| riscv64 / riscv32 | Open ISA, embedded | 64 / 32 | Modular RISC | ISA extension string, XLEN, privilege modes |
| loongarch64 | Desktop, server | 64 | Load/store RISC | ABI variant and feature discovery |
| e2k | Server | 64 | VLIW | MCST compiler, scheduling and ABI |
| ppc64le | Server, accelerator | 64 | Load/store RISC | VSX, endian ABI and NUMA |
| s390x | Mainframe | 64 | Register-rich CISC | Big-endian data layout and vector facilities |
| avr / pic8 | Microcontroller | 8 | Harvard / banked Harvard | Address spaces, register banks, flash/RAM |
| xmos-xcore | Deterministic MCU | 32 | Hardware threads and channels | Tile topology and cycle budgets |
| wasm32 | Portable virtual ISA | 32 | Sandboxed stack machine | WASI capabilities and linear-memory bounds |

The list is deliberately not a universal CPU detector. Processor-specific facts must come from platform APIs, target compiler macros, vendor documentation or validated runtime probing.

## Language families to map into engineering workflows

The separate systems-language matrix in `tools/systems_languages.json` contains 66 unique candidate profiles. The C11 processor workbench has its own 66-entry registry that classifies assembly, systems languages, VM bytecode, compiler IR and HDL independently. This separation avoids presenting research languages or hardware-description formats as interchangeable assemblers:

- **Explicit native control:** Odin, Jai, C3, Hare, Myrddin, V, Vale, Austral, Cone, Kit, Carp, Dale, Zed, Biscuit.
- **C++ and LLVM evolution:** Carbon, Cpp2, Lobster, Nim, Crystal, D BetterC, Terra, Clay, Felix, Volt.
- **Verification and safety:** ATS, F*, Cogent, BitC, Habit, Ivory, ParaSail, SPARK.
- **Embedded and constrained targets:** nesC, eC, XC, Forth, Factor, JAL, Plankalkül, C--.
- **Historical systems lineages:** BLISS, BCPL, B, PL/M, PL/S, ESPOL, Mesa, Modula-2, Oberon, Active Oberon, Component Pascal, Sather, Alef, Limbo, Newsqueak.
- **Research and specialist systems:** Cyclone, Vault, Sing#, Midori C#, Pre-Scheme, Linear Lisp, Slang, Mojo, HLA.

These names are a research index, not a claim that Pytyliki compiles or executes them. Several are historical, experimental, closed, or highly target-specific.

## Engineering rules

1. Preserve architecture identity, ABI, endianness and pointer width as separate fields.
2. Do not equate ISA capability with OS-enabled usability (for example, AVX requires OS-managed vector state).
3. Treat toolchain availability as an explicit probe, not an inference from architecture name.
4. Mark unknown target facts as unknown; do not fill them with vendor-neutral guesses.
5. Keep all discovery read-only. The atlas never writes CPU registers, firmware, clocks, power limits or device configuration.
6. Keep benchmark comparisons workload-specific and record compiler, flags, OS, target and measurement conditions.

## Boundaries

This module is a reference catalogue and build-target label. It is not a disassembler, compiler, emulator, binary translator, device driver, formal verifier or hardware certification engine.
