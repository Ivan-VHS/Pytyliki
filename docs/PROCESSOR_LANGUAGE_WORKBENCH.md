# Processor Language Workbench

## Purpose

The C11 workbench provides one stable processor-oriented entry point for a curated catalogue of exactly 66 distinct source/IR/HDL profiles. Every entry has a unique ID, display name and file extension. Assembly dialects are separate entries only where the actual assembler/frontend differs; target families, bytecode, compiler IR and hardware-description languages are labelled separately rather than presented as native assemblers.

This is a profile and tooling integration layer. It does not claim that 66 compilers are bundled or installed.

## Five core functions

1. **Registry** — pl_language_catalog enumerates the immutable 66-entry table; pl_language_find resolves an ID or case-insensitive display name.
2. **Detection** — pl_language_detect identifies one profile from a unique file extension, then checks a small number of unmistakable text signatures. Ambiguous and unknown inputs are reported separately.
3. **Inspection** — pl_language_inspect performs a bounded structural scan: byte/line counts, tokens, comments, quoted strings, delimiter balance and maximum delimiter nesting. Input is capped at 1 MiB and nesting at 128 delimiters.
4. **Target validation** — pl_language_check_target checks whether a profile's declared architecture/VM target mask includes the requested target alias. It is a catalogue compatibility check, not a compiler-feature or ABI proof.
5. **Build planning** — pl_language_build_plan_create returns an executable name and a bounded argv array for a small set of known toolchain routes. The result is data only: this library never launches the executable, concatenates a shell command, installs a compiler, links an image or writes hardware.

## CLI

~~~sh
# List all 66 unique profiles
pytyliki --language catalog

# Inspect one profile
pytyliki --language show asm-riscv-gas

# Detect a language from a source file
pytyliki --language detect firmware.S

# Check basic source structure (not full syntax)
pytyliki --language inspect sys-c src/main.c

# Check profile/target compatibility
pytyliki --language target asm-riscv-gas riscv64

# Print a compiler/assembler argument plan without execution
pytyliki --language plan sys-c x86_64 src/main.c build/main.o
~~~

On Windows, use pytyliki.exe in the selected CMake configuration directory.

## Catalogue (66 unique profiles)

| # | Profile | Classification | Target |
|---:|---|---|---|
| 1 | x86 MASM (Intel syntax) | Assembler / dialect | x86 |
| 2 | x86 GNU as (AT&T syntax) | Assembler / dialect | x86 |
| 3 | x86-64 NASM | Assembler | x86-64 |
| 4 | ARM32 ARMASM | Assembler | ARM32 |
| 5 | AArch64 GNU as | Assembler | AArch64 |
| 6 | MIPS GNU as | Assembler | MIPS |
| 7 | RISC-V GNU as | Assembler | RISC-V |
| 8 | PowerPC GNU as | Assembler | PowerPC |
| 9 | SPARC GNU as | Assembler | SPARC |
| 10 | IA-64 GNU as | Assembler | Itanium |
| 11 | AVR GNU as | Assembler | AVR |
| 12 | PIC MPASM | Assembler | PIC |
| 13 | 8051 ASEM-51 | Assembler | 8051 |
| 14 | Z80 assembly | Assembler | Z80 |
| 15 | 6502 ca65 | Assembler | 6502 |
| 16 | Motorola 68000 vasm | Assembler | 68000 |
| 17 | MSP430 GNU as | Assembler | MSP430 |
| 18 | HC11/HCS12 assembly | Assembler | HC11/HCS12 |
| 19 | STM8 SDAS | Assembler | STM8 |
| 20 | ESP32 Xtensa assembly | Assembler | Xtensa |
| 21 | Analog Devices SHARC assembly | DSP assembler | SHARC |
| 22 | Blackfin GNU as | DSP assembler | Blackfin |
| 23 | TI C6000 assembly | DSP/VLIW assembler | TMS320 |
| 24 | Qualcomm Hexagon assembly | DSP assembler | Hexagon |
| 25 | VLIW assembly format | Instruction-format family | Several VLIW ISAs |
| 26 | NVIDIA PTX | GPU virtual ISA | NVIDIA |
| 27 | AMD GCN assembly | GPU ISA | AMD |
| 28 | Cell BE SPU assembly | Coprocessor assembler | Cell SPU |
| 29 | eBPF assembly | Verified virtual ISA | eBPF |
| 30 | C | Systems language | CPU / WebAssembly targets |
| 31 | C++ | Systems language | CPU / WebAssembly targets |
| 32 | Rust | Systems language | CPU / WebAssembly targets |
| 33 | D (BetterC) | Systems language subset | CPU |
| 34 | Zig | Systems language | CPU / WebAssembly targets |
| 35 | Ada / SPARK | Systems / verified language | CPU |
| 36 | Modula-2 | Systems language | CPU |
| 37 | Oberon | Systems language | CPU |
| 38 | Embedded Pascal | Embedded language | CPU / MCU |
| 39 | Forth | Embedded language | CPU / MCU |
| 40 | BCPL | Historical systems language | CPU |
| 41 | ALGOL 68 | Historical systems language | CPU |
| 42 | Bell Labs B | Historical systems language | CPU |
| 43 | PL/S | Historical systems language | IBM systems |
| 44 | BLISS | Systems language | DEC / CPU |
| 45 | HAL/S | Aerospace systems language | Historical systems |
| 46 | JOVIAL | Real-time systems language | Historical systems |
| 47 | CMS-2 | Embedded systems language | Historical systems |
| 48 | PL/M | Microprocessor systems language | Intel MCU |
| 49 | nesC | Embedded systems language | TinyOS targets |
| 50 | Limbo | Systems language | Inferno VM |
| 51 | Newsqueak | Concurrent systems language | Research VM |
| 52 | occam | Parallel systems language | Transputer lineage |
| 53 | Cyclone | Memory-safe systems language | CPU |
| 54 | WebAssembly text (WAT) | Virtual ISA text | WebAssembly |
| 55 | LLVM IR | Compiler intermediate representation | LLVM backends |
| 56 | SPIR-V assembly | GPU intermediate representation | Vulkan GPU |
| 57 | QBE IL | Compiler intermediate representation | QBE backends |
| 58 | JVM bytecode (Jasmin) | VM assembly | JVM |
| 59 | .NET CIL (ILAsm) | VM assembly | CLR |
| 60 | OpenCL C | Heterogeneous systems language | CPU / GPU |
| 61 | Verilog | Hardware description language | FPGA / ASIC |
| 62 | VHDL | Hardware description language | FPGA / ASIC |
| 63 | SystemVerilog | Hardware description language | FPGA / ASIC |
| 64 | Chisel | Hardware construction language | FPGA / ASIC |
| 65 | Processor microcode description | Vendor-specific microcode reference | Vendor-specific CPU |
| 66 | Open Firmware FCode | Firmware bytecode | Open Firmware VM |

The original brainstorming list also included aliases or items that are not distinct native processor toolchains (for example, Thompson's B versus B, and the two Clay descriptions). The catalogue avoids duplicate records. It also makes room for additional processor-facing formats such as eBPF, QBE IL and FCode. Generic VLIW and microcode entries are explicitly families/references, not universal assembler implementations.

## Implemented build-plan routes

The current argument planner contains route adapters for MASM, GNU as (32-bit x86), NASM ELF64, Clang, Clang++, Rustc, Zig, LDC BetterC (with an explicit target triple), LLVM MC, WAT-to-Wasm, LLC, SPIR-V assembler, QBE IL (x86-64 SysV only) and Icarus Verilog. Other profiles are reference-only until an exact toolchain, target tuple and output contract are implemented.

A route being present does not mean the executable is installed or that a generated object has been verified. A future explicit execution adapter must add bounded process lifetime, captured stdout/stderr, exit-code handling, artifact type/size checks, timeout/cancellation and reproducible tool-version capture before the feature can be described as compilation.

## Safety and correctness boundaries

- No source is executed, rewritten or uploaded.
- No external tool is started by the current API or CLI.
- Toolchain names are labels; PATH discovery remains separate and read-only.
- Paths are passed as separate bounded arguments. Shell parsing is not used.
- The structural scanner is not a grammar parser, assembler, disassembler, verifier or formal proof engine. Syntax errors that do not affect strings/comments/delimiters can pass.
- A target match does not validate ABI, ISA extensions, endianness, device memory maps, linker scripts, runtime libraries or silicon revision.
- Hardware access, MMIO/DMA, interrupt control, firmware flashing, microcode updates, voltage/frequency changes and driver installation are out of scope.

## Native API

See src/processor_languages.h. The registry is static and allocation-free. Source scanning and build-plan output use caller-owned buffers and deterministic return codes. The module is part of pytyliki_core and its public header is installed with the other Pytyliki headers.
