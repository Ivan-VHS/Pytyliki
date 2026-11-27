#include "processor_languages.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define TARGET_X86      (1ULL << 0)
#define TARGET_X64      (1ULL << 1)
#define TARGET_ARM32    (1ULL << 2)
#define TARGET_ARM64    (1ULL << 3)
#define TARGET_MIPS     (1ULL << 4)
#define TARGET_RISCV    (1ULL << 5)
#define TARGET_PPC      (1ULL << 6)
#define TARGET_SPARC    (1ULL << 7)
#define TARGET_IA64     (1ULL << 8)
#define TARGET_AVR      (1ULL << 9)
#define TARGET_PIC      (1ULL << 10)
#define TARGET_8051     (1ULL << 11)
#define TARGET_Z80      (1ULL << 12)
#define TARGET_6502     (1ULL << 13)
#define TARGET_M68K     (1ULL << 14)
#define TARGET_MSP430   (1ULL << 15)
#define TARGET_HCS12    (1ULL << 16)
#define TARGET_STM8     (1ULL << 17)
#define TARGET_XTENSA   (1ULL << 18)
#define TARGET_SHARC    (1ULL << 19)
#define TARGET_BLACKFIN (1ULL << 20)
#define TARGET_TMS320   (1ULL << 21)
#define TARGET_HEXAGON  (1ULL << 22)
#define TARGET_CELL     (1ULL << 23)
#define TARGET_NVIDIA   (1ULL << 24)
#define TARGET_AMD      (1ULL << 25)
#define TARGET_BPF      (1ULL << 26)
#define TARGET_WASM     (1ULL << 27)
#define TARGET_JVM      (1ULL << 28)
#define TARGET_CLR      (1ULL << 29)
#define TARGET_HDL      (1ULL << 30)
#define TARGET_CPU ((1ULL << 24) - 1ULL)
#define TARGET_PORTABLE (TARGET_CPU | TARGET_WASM)
#define TARGET_GPU (TARGET_NVIDIA | TARGET_AMD)
#define TARGET_VLIW (TARGET_IA64 | TARGET_PPC | TARGET_SHARC | TARGET_BLACKFIN | TARGET_TMS320 | TARGET_HEXAGON | TARGET_CELL)

#define C_HASH PL_LANGUAGE_COMMENT_HASH
#define C_SEMI PL_LANGUAGE_COMMENT_SEMICOLON
#define C_SLASH PL_LANGUAGE_COMMENT_SLASH
#define C_BLOCK PL_LANGUAGE_COMMENT_BLOCK
#define C_DASH PL_LANGUAGE_COMMENT_DASH_DASH
#define C_BACKSLASH PL_LANGUAGE_COMMENT_BACKSLASH

#define PROFILE(id_, name_, family_, kind_, extension_, toolchain_, target_, comments_, route_, note_) \
    { id_, name_, family_, extension_, toolchain_, #target_, note_, kind_, route_, target_, comments_ }

static const pl_language_profile profiles[PL_LANGUAGE_COUNT] = {
    PROFILE("asm-x86-masm", "x86 MASM (Intel syntax)", "x86 assembly", PL_LANGUAGE_ASSEMBLY, ".masm", "ml", TARGET_X86, C_SEMI, PL_LANGUAGE_ROUTE_MASM, "Assembler profile; the Microsoft toolchain is an external dependency."),
    PROFILE("asm-x86-gas", "x86 GNU as (AT&T syntax)", "x86 assembly", PL_LANGUAGE_ASSEMBLY, ".gas", "as", TARGET_X86, C_HASH, PL_LANGUAGE_ROUTE_GAS, "Assembler profile; the GNU assembler is an external dependency."),
    PROFILE("asm-x86_64-nasm", "x86-64 NASM", "x86-64 assembly", PL_LANGUAGE_ASSEMBLY, ".nasm", "nasm", TARGET_X64, C_SEMI, PL_LANGUAGE_ROUTE_NASM64, "NASM ELF64 object-format planning is available; NASM itself is not bundled."),
    PROFILE("asm-arm32-armasm", "ARM32 ARMASM", "ARM assembly", PL_LANGUAGE_ASSEMBLY, ".armasm", "armasm", TARGET_ARM32, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "Vendor assembler profile; syntax and ABI depend on the selected ARM toolchain."),
    PROFILE("asm-aarch64-gas", "AArch64 GNU as", "AArch64 assembly", PL_LANGUAGE_ASSEMBLY, ".a64", "aarch64-linux-gnu-as", TARGET_ARM64, C_SLASH, PL_LANGUAGE_ROUTE_NONE, "GNU cross-assembler profile; no cross-toolchain is bundled."),
    PROFILE("asm-mips-gas", "MIPS GNU as", "MIPS assembly", PL_LANGUAGE_ASSEMBLY, ".mipsasm", "mips-linux-gnu-as", TARGET_MIPS, C_HASH, PL_LANGUAGE_ROUTE_NONE, "GNU cross-assembler profile; ABI and endianness must be selected externally."),
    PROFILE("asm-riscv-gas", "RISC-V GNU as", "RISC-V assembly", PL_LANGUAGE_ASSEMBLY, ".rvasm", "riscv64-unknown-elf-as", TARGET_RISCV, C_HASH, PL_LANGUAGE_ROUTE_NONE, "GNU cross-assembler profile; ISA extensions remain an explicit build choice."),
    PROFILE("asm-powerpc-gas", "PowerPC GNU as", "PowerPC assembly", PL_LANGUAGE_ASSEMBLY, ".ppcasm", "powerpc-linux-gnu-as", TARGET_PPC, C_HASH, PL_LANGUAGE_ROUTE_NONE, "GNU cross-assembler profile; ABI and endianness are target-specific."),
    PROFILE("asm-sparc-gas", "SPARC GNU as", "SPARC assembly", PL_LANGUAGE_ASSEMBLY, ".sparcasm", "sparc64-linux-gnu-as", TARGET_SPARC, C_HASH, PL_LANGUAGE_ROUTE_NONE, "GNU cross-assembler profile; target width and ABI are external settings."),
    PROFILE("asm-ia64-gas", "IA-64 GNU as", "Itanium assembly", PL_LANGUAGE_ASSEMBLY, ".ia64asm", "ia64-linux-gnu-as", TARGET_IA64, C_HASH, PL_LANGUAGE_ROUTE_NONE, "Historical target profile; toolchain availability is uncommon."),
    PROFILE("asm-avr-gas", "AVR GNU as", "AVR assembly", PL_LANGUAGE_ASSEMBLY, ".avrasm", "avr-as", TARGET_AVR, C_HASH, PL_LANGUAGE_ROUTE_NONE, "MCU assembler profile; device selection and linker script are external."),
    PROFILE("asm-pic-mpasm", "PIC MPASM", "PIC assembly", PL_LANGUAGE_ASSEMBLY, ".picasm", "gpasm", TARGET_PIC, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "PIC family and memory model must be selected for the exact device."),
    PROFILE("asm-8051-asem", "8051 ASEM-51", "8051 assembly", PL_LANGUAGE_ASSEMBLY, ".a51", "asem", TARGET_8051, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "8051 assembler profile; memory-space rules are device-specific."),
    PROFILE("asm-z80", "Z80 assembly", "Z80 assembly", PL_LANGUAGE_ASSEMBLY, ".z80", "z80asm", TARGET_Z80, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "Assembler dialects vary; this entry is a neutral Z80 source profile."),
    PROFILE("asm-6502-ca65", "6502 ca65", "6502 assembly", PL_LANGUAGE_ASSEMBLY, ".6502", "ca65", TARGET_6502, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "cc65/ca65 syntax profile; linker configuration is not inferred."),
    PROFILE("asm-m68k-vasm", "Motorola 68000 vasm", "68000 assembly", PL_LANGUAGE_ASSEMBLY, ".m68k", "vasmm68k_mot", TARGET_M68K, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "vasm Motorola syntax profile; CPU generation is a separate choice."),
    PROFILE("asm-msp430-gas", "MSP430 GNU as", "MSP430 assembly", PL_LANGUAGE_ASSEMBLY, ".msp430", "msp430-elf-as", TARGET_MSP430, C_HASH, PL_LANGUAGE_ROUTE_NONE, "GNU MCU assembler profile; device and linker script are external."),
    PROFILE("asm-hc11-hcs12", "HC11/HCS12 assembly", "HC11/HCS12 assembly", PL_LANGUAGE_ASSEMBLY, ".hcs12", "m68hc11-as", TARGET_HCS12, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "Motorola MCU assembler profile; instruction availability varies by core."),
    PROFILE("asm-stm8-sdas", "STM8 SDAS", "STM8 assembly", PL_LANGUAGE_ASSEMBLY, ".stm8", "sdasstm8", TARGET_STM8, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "SDCC assembler profile; the selected STM8 device is not inferred."),
    PROFILE("asm-xtensa-esp32", "ESP32 Xtensa assembly", "Xtensa assembly", PL_LANGUAGE_ASSEMBLY, ".xtensa", "xtensa-esp32-elf-as", TARGET_XTENSA, C_HASH, PL_LANGUAGE_ROUTE_NONE, "ESP32-specific assembler profile; chip variant and ABI are external."),
    PROFILE("asm-sharc", "Analog Devices SHARC assembly", "SHARC DSP", PL_LANGUAGE_ASSEMBLY, ".sharc", "cc21k", TARGET_SHARC, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "Vendor DSP assembler profile; proprietary toolchain may be required."),
    PROFILE("asm-blackfin-gas", "Blackfin GNU as", "Blackfin DSP", PL_LANGUAGE_ASSEMBLY, ".blackfin", "bfin-elf-as", TARGET_BLACKFIN, C_HASH, PL_LANGUAGE_ROUTE_NONE, "GNU DSP assembler profile; hardware revision is not inferred."),
    PROFILE("asm-ti-c6000", "TI C6000 assembly", "Texas Instruments DSP", PL_LANGUAGE_ASSEMBLY, ".c6x", "cl6x", TARGET_TMS320, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "VLIW DSP assembler profile; scheduling and silicon revision are external."),
    PROFILE("asm-hexagon", "Qualcomm Hexagon assembly", "Hexagon DSP", PL_LANGUAGE_ASSEMBLY, ".hexagon", "hexagon-as", TARGET_HEXAGON, C_HASH, PL_LANGUAGE_ROUTE_NONE, "Hexagon assembler profile; packet constraints need the vendor assembler."),
    PROFILE("asm-vliw-generic", "VLIW assembly format", "VLIW assembly", PL_LANGUAGE_ASSEMBLY, ".vliw", "vendor-specific", TARGET_VLIW, C_HASH, PL_LANGUAGE_ROUTE_NONE, "VLIW is an instruction-format family, not one universal ISA or assembler."),
    PROFILE("asm-nvidia-ptx", "NVIDIA PTX", "GPU virtual ISA", PL_LANGUAGE_ASSEMBLY, ".ptx", "ptxas", TARGET_NVIDIA, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "PTX is a virtual ISA; an SM target must be selected explicitly before a safe build route is available."),
    PROFILE("asm-amd-gcn", "AMD GCN assembly", "AMD GPU ISA", PL_LANGUAGE_ASSEMBLY, ".amdgpu", "llvm-mc", TARGET_AMD, C_HASH, PL_LANGUAGE_ROUTE_LLVM_MC, "AMDGPU machine assembly route is planned through LLVM MC."),
    PROFILE("asm-cell-spu", "Cell BE SPU assembly", "Cell Broadband Engine", PL_LANGUAGE_ASSEMBLY, ".spu", "spu-as", TARGET_CELL, C_HASH, PL_LANGUAGE_ROUTE_NONE, "Historical SPU assembler profile; toolchain availability is uncommon."),
    PROFILE("asm-ebpf", "eBPF assembly", "Linux virtual ISA", PL_LANGUAGE_ASSEMBLY, ".bpf", "llvm-mc", TARGET_BPF, C_HASH, PL_LANGUAGE_ROUTE_LLVM_MC, "eBPF assembly route is planned through LLVM MC; verifier approval is separate."),
    PROFILE("sys-c", "C", "systems programming", PL_LANGUAGE_SYSTEMS, ".c", "clang", TARGET_PORTABLE, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_CLANG, "Clang object-build plan is available; sysroot and linker remain external."),
    PROFILE("sys-cpp", "C++", "systems programming", PL_LANGUAGE_SYSTEMS, ".cpp", "clang++", TARGET_PORTABLE, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_CLANGXX, "Clang++ object-build plan is available; standard library and sysroot remain external."),
    PROFILE("sys-rust", "Rust", "systems programming", PL_LANGUAGE_SYSTEMS, ".rs", "rustc", TARGET_PORTABLE, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_RUSTC, "Rust object-build plan is available; target standard library must be installed separately."),
    PROFILE("sys-d-betterc", "D (BetterC)", "systems programming", PL_LANGUAGE_SYSTEMS, ".d", "ldc2", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_LDC, "LDC object-build plan passes -betterC; source-level API restrictions still require compiler diagnostics and review."),
    PROFILE("sys-zig", "Zig", "systems programming", PL_LANGUAGE_SYSTEMS, ".zig", "zig", TARGET_PORTABLE, C_SLASH, PL_LANGUAGE_ROUTE_ZIG, "Zig object-build plan is available; target support depends on the installed compiler."),
    PROFILE("sys-ada-spark", "Ada / SPARK", "verified systems programming", PL_LANGUAGE_SYSTEMS, ".adb", "gnatmake", TARGET_CPU, C_DASH, PL_LANGUAGE_ROUTE_NONE, "Ada/SPARK profile; proof and compiler checks require a configured GNAT/SPARK toolchain."),
    PROFILE("sys-modula-2", "Modula-2", "systems programming", PL_LANGUAGE_SYSTEMS, ".mod2", "m2c", TARGET_CPU, C_SLASH, PL_LANGUAGE_ROUTE_NONE, "Historical systems language profile; dialect and compiler are implementation-specific."),
    PROFILE("sys-oberon", "Oberon", "systems programming", PL_LANGUAGE_SYSTEMS, ".ob", "obc", TARGET_CPU, C_SLASH, PL_LANGUAGE_ROUTE_NONE, "Oberon source profile; compiler dialects are not interchangeable."),
    PROFILE("sys-embedded-pascal", "Embedded Pascal", "embedded systems", PL_LANGUAGE_SYSTEMS, ".epas", "fpc", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Embedded Pascal profile; target runtime and device libraries are external."),
    PROFILE("sys-forth", "Forth", "embedded systems", PL_LANGUAGE_SYSTEMS, ".fs", "gforth", TARGET_CPU, C_BACKSLASH, PL_LANGUAGE_ROUTE_NONE, "Forth source profile; dictionary and target-cell width are implementation-specific."),
    PROFILE("sys-bcpl", "BCPL", "historical systems programming", PL_LANGUAGE_SYSTEMS, ".bcp", "cintsys", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Historical systems language profile; runtime model varies by implementation."),
    PROFILE("sys-algol68", "ALGOL 68", "historical systems programming", PL_LANGUAGE_SYSTEMS, ".a68", "a68g", TARGET_CPU, C_DASH, PL_LANGUAGE_ROUTE_NONE, "Historical language profile; implementation availability is limited."),
    PROFILE("sys-b", "Bell Labs B", "historical systems programming", PL_LANGUAGE_SYSTEMS, ".b", "historical", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Historical language profile; no modern portable compiler is assumed."),
    PROFILE("sys-pl-s", "PL/S", "IBM systems programming", PL_LANGUAGE_SYSTEMS, ".pls", "historical", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "IBM internal systems language lineage; no public portable toolchain is assumed."),
    PROFILE("sys-bliss", "BLISS", "DEC systems programming", PL_LANGUAGE_SYSTEMS, ".bliss", "bliss", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "BLISS dialects and target backends are implementation-specific."),
    PROFILE("sys-hal-s", "HAL/S", "aerospace systems programming", PL_LANGUAGE_SYSTEMS, ".hals", "historical", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Historical aerospace language profile; no compiler is bundled."),
    PROFILE("sys-jovial", "JOVIAL", "real-time systems programming", PL_LANGUAGE_SYSTEMS, ".jov", "historical", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Historical real-time language profile; dialect is project-specific."),
    PROFILE("sys-cms-2", "CMS-2", "military embedded programming", PL_LANGUAGE_SYSTEMS, ".cms2", "historical", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Historical Navy language family; no portable modern toolchain is assumed."),
    PROFILE("sys-pl-m", "PL/M", "Intel microprocessor systems programming", PL_LANGUAGE_SYSTEMS, ".plm", "historical", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Historical Intel systems language profile; toolchain availability is limited."),
    PROFILE("sys-nesc", "nesC", "embedded systems", PL_LANGUAGE_SYSTEMS, ".nc", "nescc", TARGET_AVR, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "TinyOS-oriented language profile; target runtime is external."),
    PROFILE("sys-limbo", "Limbo", "Inferno systems programming", PL_LANGUAGE_SYSTEMS, ".limbo", "limbo", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Limbo source profile; Inferno runtime and compiler are not bundled."),
    PROFILE("sys-newsqueak", "Newsqueak", "concurrent systems programming", PL_LANGUAGE_SYSTEMS, ".nsq", "historical", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Historical concurrent language profile; compiler availability is limited."),
    PROFILE("sys-occam", "occam", "parallel embedded programming", PL_LANGUAGE_SYSTEMS, ".occ", "kroc", TARGET_CPU, C_DASH, PL_LANGUAGE_ROUTE_NONE, "occam source profile; transputer/runtime assumptions are external."),
    PROFILE("sys-cyclone", "Cyclone", "memory-safe systems programming", PL_LANGUAGE_SYSTEMS, ".cyc", "cyclone", TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Research systems language profile; no compiler is bundled."),
    PROFILE("vm-wasm-text", "WebAssembly text (WAT)", "virtual instruction set", PL_LANGUAGE_BYTECODE, ".wat", "wat2wasm", TARGET_WASM, C_SEMI, PL_LANGUAGE_ROUTE_WAT2WASM, "WAT-to-Wasm planning is available; runtime and WASI policy are separate."),
    PROFILE("ir-llvm", "LLVM IR", "compiler intermediate representation", PL_LANGUAGE_INTERMEDIATE, ".ll", "llc", TARGET_PORTABLE, C_SEMI, PL_LANGUAGE_ROUTE_LLC, "LLVM object-build plan is available; backend target support is external."),
    PROFILE("ir-spirv", "SPIR-V assembly", "GPU intermediate representation", PL_LANGUAGE_INTERMEDIATE, ".spvasm", "spirv-as", TARGET_GPU, C_SEMI, PL_LANGUAGE_ROUTE_SPIRV_AS, "SPIR-V assembly planning targets Vulkan 1.2; validation is a separate step."),
    PROFILE("ir-qbe", "QBE IL", "compiler intermediate representation", PL_LANGUAGE_INTERMEDIATE, ".ssa", "qbe", TARGET_X64, C_SEMI, PL_LANGUAGE_ROUTE_QBE, "QBE object-build plan is available for supported host backends."),
    PROFILE("vm-jasmin", "JVM bytecode (Jasmin)", "virtual machine bytecode", PL_LANGUAGE_BYTECODE, ".j", "jasmin", TARGET_JVM, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "Assembler profile for JVM class files; verifier and runtime are separate."),
    PROFILE("vm-dotnet-il", ".NET CIL (ILAsm)", "virtual machine bytecode", PL_LANGUAGE_BYTECODE, ".il", "ilasm", TARGET_CLR, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "Assembler profile for CLR assemblies; verification and runtime are separate."),
    PROFILE("sys-opencl-c", "OpenCL C", "heterogeneous systems programming", PL_LANGUAGE_SYSTEMS, ".cl", "clang", TARGET_GPU | TARGET_CPU, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Kernel source profile; device compilation options are vendor/runtime-specific."),
    PROFILE("hdl-verilog", "Verilog", "hardware description", PL_LANGUAGE_HARDWARE, ".v", "iverilog", TARGET_HDL, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_IVERILOG, "Icarus Verilog simulation-build plan is available; synthesis is not implied."),
    PROFILE("hdl-vhdl", "VHDL", "hardware description", PL_LANGUAGE_HARDWARE, ".vhd", "ghdl", TARGET_HDL, C_DASH, PL_LANGUAGE_ROUTE_NONE, "VHDL analysis is toolchain-specific; the generic object recipe is intentionally absent."),
    PROFILE("hdl-systemverilog", "SystemVerilog", "hardware description", PL_LANGUAGE_HARDWARE, ".sv", "verilator", TARGET_HDL, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "SystemVerilog lint/simulation options depend on the selected Verilator flow."),
    PROFILE("hdl-chisel", "Chisel", "hardware construction language", PL_LANGUAGE_HARDWARE, ".chisel", "sbt", TARGET_HDL, C_SLASH | C_BLOCK, PL_LANGUAGE_ROUTE_NONE, "Chisel is a Scala-based hardware construction language, not an assembler."),
    PROFILE("microcode-vendor", "Processor microcode description", "vendor-specific microcode", PL_LANGUAGE_MICROCODE, ".ucode", "vendor-specific", TARGET_X86 | TARGET_X64, C_HASH, PL_LANGUAGE_ROUTE_NONE, "Microcode formats are vendor- and generation-specific; no universal assembler exists."),
    PROFILE("vm-openfirmware-fcode", "Open Firmware FCode", "firmware virtual machine", PL_LANGUAGE_BYTECODE, ".fcode", "fcode", TARGET_CPU, C_SEMI, PL_LANGUAGE_ROUTE_NONE, "FCode is firmware bytecode; execution requires a compatible Open Firmware environment.")
};

static int text_equal(const char *left, const char *right) {
    if (left == NULL || right == NULL) return 0;
    while (*left != '\0' && *right != '\0') {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) return 0;
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

static int path_has_extension(const char *path, const char *extension) {
    const char *dot;
    const char *separator_a;
    const char *separator_b;
    if (path == NULL || extension == NULL) return 0;
    dot = strrchr(path, '.');
    separator_a = strrchr(path, '/');
    separator_b = strrchr(path, '\\');
    if (dot == NULL || (separator_a != NULL && dot < separator_a) ||
        (separator_b != NULL && dot < separator_b)) return 0;
    return text_equal(dot, extension);
}

size_t pl_language_catalog(const pl_language_profile **out) {
    if (out == NULL) return 0u;
    *out = profiles;
    return PL_LANGUAGE_COUNT;
}

const pl_language_profile *pl_language_find(const char *id_or_name) {
    if (id_or_name == NULL || id_or_name[0] == '\0') return NULL;
    for (size_t i = 0u; i < PL_LANGUAGE_COUNT; ++i) {
        if (text_equal(id_or_name, profiles[i].id) ||
            text_equal(id_or_name, profiles[i].name)) return &profiles[i];
    }
    return NULL;
}

const char *pl_language_kind_name(pl_language_kind kind) {
    switch (kind) {
        case PL_LANGUAGE_ASSEMBLY: return "assembly";
        case PL_LANGUAGE_SYSTEMS: return "systems";
        case PL_LANGUAGE_BYTECODE: return "bytecode";
        case PL_LANGUAGE_INTERMEDIATE: return "intermediate";
        case PL_LANGUAGE_HARDWARE: return "hardware-description";
        case PL_LANGUAGE_MICROCODE: return "microcode";
        default: return "unknown";
    }
}

static int starts_with(const char *source, size_t length, const char *prefix) {
    const size_t prefix_length = strlen(prefix);
    if (length < prefix_length) return 0;
    return memcmp(source, prefix, prefix_length) == 0;
}

int pl_language_detect(const char *path, const char *source, size_t length,
                       const pl_language_profile **out) {
    const pl_language_profile *match = NULL;
    size_t matches = 0u;
    if (out == NULL || length > PL_LANGUAGE_SOURCE_MAX || (length > 0u && source == NULL))
        return -1;
    *out = NULL;
    if (path != NULL) {
        for (size_t i = 0u; i < PL_LANGUAGE_COUNT; ++i) {
            if (path_has_extension(path, profiles[i].extension)) {
                match = &profiles[i];
                ++matches;
            }
        }
    }
    if (matches == 1u) {
        *out = match;
        return 0;
    }
    if (matches > 1u) return 2;
    if (path != NULL && (path_has_extension(path, ".asm") ||
                         path_has_extension(path, ".s"))) return 2;
    if (source == NULL || length == 0u) return 1;

    while (length > 0u && isspace((unsigned char)*source)) {
        ++source;
        --length;
    }
    if (starts_with(source, length, "(module")) match = pl_language_find("vm-wasm-text");
    else if (starts_with(source, length, ".version")) match = pl_language_find("asm-nvidia-ptx");
    else if (starts_with(source, length, "; SPIR-V")) match = pl_language_find("ir-spirv");
    else if (starts_with(source, length, "; ModuleID")) match = pl_language_find("ir-llvm");
    if (match == NULL) return 1;
    *out = match;
    return 0;
}

static int is_open_delimiter(char value) {
    return value == '(' || value == '[' || value == '{';
}

static int is_close_delimiter(char value) {
    return value == ')' || value == ']' || value == '}';
}

static int delimiters_match(char opening, char closing) {
    return (opening == '(' && closing == ')') ||
           (opening == '[' && closing == ']') ||
           (opening == '{' && closing == '}');
}

int pl_language_inspect(const char *id_or_name, const char *source, size_t length,
                        pl_language_report *out) {
    const pl_language_profile *profile = pl_language_find(id_or_name);
    char delimiter_stack[128];
    size_t delimiter_offsets[128];
    size_t depth = 0u;
    size_t i = 0u;
    char quote = '\0';
    int escaped = 0;
    int line_comment = 0;
    int block_comment = 0;
    int token_active = 0;

    if (out == NULL || profile == NULL || length > PL_LANGUAGE_SOURCE_MAX ||
        (length > 0u && source == NULL)) return -1;
    memset(out, 0, sizeof(*out));
    out->source_bytes = length;
    out->line_count = length == 0u ? 0u : 1u;
    out->structurally_valid = 1;

    while (i < length) {
        const char current = source[i];
        const char next = i + 1u < length ? source[i + 1u] : '\0';

        if (current == '\n') ++out->line_count;

        if (line_comment) {
            if (current == '\n') line_comment = 0;
            ++i;
            continue;
        }
        if (block_comment) {
            if ((profile->comment_styles & C_BLOCK) != 0u && current == '*' && next == '/') {
                block_comment = 0;
                i += 2u;
            } else {
                ++i;
            }
            continue;
        }
        if (quote != '\0') {
            if (escaped) escaped = 0;
            else if (current == '\\') escaped = 1;
            else if (current == quote) quote = '\0';
            ++i;
            continue;
        }

        if ((profile->comment_styles & C_BLOCK) != 0u && current == '/' && next == '*') {
            block_comment = 1;
            ++out->comment_count;
            token_active = 0;
            i += 2u;
            continue;
        }
        if ((profile->comment_styles & C_SLASH) != 0u && current == '/' && next == '/') {
            line_comment = 1;
            ++out->comment_count;
            token_active = 0;
            i += 2u;
            continue;
        }
        if (((profile->comment_styles & C_HASH) != 0u && current == '#') ||
            ((profile->comment_styles & C_SEMI) != 0u && current == ';') ||
            ((profile->comment_styles & C_BACKSLASH) != 0u && current == '\\') ||
            ((profile->comment_styles & C_DASH) != 0u && current == '-' && next == '-')) {
            line_comment = 1;
            ++out->comment_count;
            token_active = 0;
            i += ((profile->comment_styles & C_DASH) != 0u && current == '-' && next == '-') ? 2u : 1u;
            continue;
        }
        if (current == '"' || current == '\'') {
            quote = current;
            escaped = 0;
            ++out->string_count;
            ++out->token_count;
            token_active = 0;
            ++i;
            continue;
        }
        if (is_open_delimiter(current)) {
            if (depth >= sizeof(delimiter_stack)) {
                out->structurally_valid = 0;
                out->error_offset = i;
                return -2;
            }
            delimiter_stack[depth] = current;
            delimiter_offsets[depth] = i;
            ++depth;
            if (depth > out->max_delimiter_depth) out->max_delimiter_depth = depth;
        } else if (is_close_delimiter(current)) {
            if (depth == 0u || !delimiters_match(delimiter_stack[depth - 1u], current)) {
                out->structurally_valid = 0;
                out->error_offset = i;
                return -3;
            }
            --depth;
        }

        if (isspace((unsigned char)current)) token_active = 0;
        else if (!token_active) {
            ++out->token_count;
            token_active = 1;
        }
        ++i;
    }

    if (quote != '\0') {
        out->unterminated_string = 1;
        out->structurally_valid = 0;
        out->error_offset = length;
        return -4;
    }
    if (block_comment) {
        out->unterminated_comment = 1;
        out->structurally_valid = 0;
        out->error_offset = length;
        return -5;
    }
    if (depth != 0u) {
        out->structurally_valid = 0;
        out->error_offset = delimiter_offsets[depth - 1u];
        return -3;
    }
    return 0;
}

static unsigned long long target_bit(const char *target) {
    if (text_equal(target, "x86") || text_equal(target, "i386") ||
        text_equal(target, "i686") || text_equal(target, "x86_32")) return TARGET_X86;
    if (text_equal(target, "x86_64") || text_equal(target, "x64") ||
        text_equal(target, "amd64")) return TARGET_X64;
    if (text_equal(target, "arm") || text_equal(target, "arm32") ||
        text_equal(target, "armv7") || text_equal(target, "armv7-m")) return TARGET_ARM32;
    if (text_equal(target, "aarch64") || text_equal(target, "arm64")) return TARGET_ARM64;
    if (text_equal(target, "mips") || text_equal(target, "mips32") ||
        text_equal(target, "mips64")) return TARGET_MIPS;
    if (text_equal(target, "riscv") || text_equal(target, "riscv32") ||
        text_equal(target, "riscv64")) return TARGET_RISCV;
    if (text_equal(target, "powerpc") || text_equal(target, "ppc") ||
        text_equal(target, "ppc64le")) return TARGET_PPC;
    if (text_equal(target, "sparc") || text_equal(target, "sparc64")) return TARGET_SPARC;
    if (text_equal(target, "ia64") || text_equal(target, "itanium")) return TARGET_IA64;
    if (text_equal(target, "avr")) return TARGET_AVR;
    if (text_equal(target, "pic") || text_equal(target, "pic8")) return TARGET_PIC;
    if (text_equal(target, "8051") || text_equal(target, "mcs51")) return TARGET_8051;
    if (text_equal(target, "z80")) return TARGET_Z80;
    if (text_equal(target, "6502") || text_equal(target, "mos6502")) return TARGET_6502;
    if (text_equal(target, "68000") || text_equal(target, "m68k")) return TARGET_M68K;
    if (text_equal(target, "msp430")) return TARGET_MSP430;
    if (text_equal(target, "hc11") || text_equal(target, "hcs12")) return TARGET_HCS12;
    if (text_equal(target, "stm8")) return TARGET_STM8;
    if (text_equal(target, "xtensa") || text_equal(target, "esp32")) return TARGET_XTENSA;
    if (text_equal(target, "sharc")) return TARGET_SHARC;
    if (text_equal(target, "blackfin")) return TARGET_BLACKFIN;
    if (text_equal(target, "c6000") || text_equal(target, "tms320")) return TARGET_TMS320;
    if (text_equal(target, "hexagon")) return TARGET_HEXAGON;
    if (text_equal(target, "cell") || text_equal(target, "spu")) return TARGET_CELL;
    if (text_equal(target, "nvidia") || text_equal(target, "nvptx")) return TARGET_NVIDIA;
    if (text_equal(target, "amd") || text_equal(target, "amdgpu") ||
        text_equal(target, "gcn")) return TARGET_AMD;
    if (text_equal(target, "bpf") || text_equal(target, "ebpf")) return TARGET_BPF;
    if (text_equal(target, "wasm") || text_equal(target, "wasm32")) return TARGET_WASM;
    if (text_equal(target, "jvm")) return TARGET_JVM;
    if (text_equal(target, "clr") || text_equal(target, "cil") ||
        text_equal(target, "dotnet")) return TARGET_CLR;
    if (text_equal(target, "hdl") || text_equal(target, "fpga") ||
        text_equal(target, "hardware")) return TARGET_HDL;
    return 0u;
}

int pl_language_check_target(const char *id_or_name, const char *target,
                             int *compatible) {
    const pl_language_profile *profile = pl_language_find(id_or_name);
    const unsigned long long bit = target_bit(target);
    if (profile == NULL || target == NULL || compatible == NULL || bit == 0u) return -1;
    *compatible = (profile->target_mask & bit) != 0u;
    return 0;
}

static int safe_path_argument(const char *path) {
    if (path == NULL || path[0] == '\0' || path[0] == '-') return 0;
    if (strlen(path) >= PL_LANGUAGE_ARGUMENT_SIZE) return 0;
    for (const unsigned char *cursor = (const unsigned char *)path; *cursor != 0u; ++cursor)
        if (*cursor < 0x20u || *cursor == 0x7fu) return 0;
    return 1;
}

static int copy_bounded_text(char *destination, size_t capacity, const char *source) {
    size_t length;
    if (destination == NULL || source == NULL || capacity == 0u) return -1;
    length = strlen(source);
    if (length >= capacity) return -1;
    memcpy(destination, source, length + 1u);
    return 0;
}

static int add_argument(pl_language_build_plan *plan, const char *value) {
    if (plan == NULL || plan->argument_count >= PL_LANGUAGE_ARGUMENT_COUNT ||
        copy_bounded_text(plan->arguments[plan->argument_count],
                          PL_LANGUAGE_ARGUMENT_SIZE, value) != 0) return -1;
    ++plan->argument_count;
    return 0;
}

static const char *target_triple(const char *target) {
    if (text_equal(target, "x86") || text_equal(target, "i386") ||
        text_equal(target, "i686") || text_equal(target, "x86_32")) return "i686-unknown-linux-gnu";
    if (text_equal(target, "x86_64") || text_equal(target, "x64") ||
        text_equal(target, "amd64")) return "x86_64-unknown-linux-gnu";
    if (text_equal(target, "arm") || text_equal(target, "arm32") ||
        text_equal(target, "armv7") || text_equal(target, "armv7-m")) return "armv7-none-eabi";
    if (text_equal(target, "aarch64") || text_equal(target, "arm64")) return "aarch64-linux-gnu";
    if (text_equal(target, "mips") || text_equal(target, "mips32")) return "mips-unknown-elf";
    if (text_equal(target, "mips64")) return "mips64-unknown-elf";
    if (text_equal(target, "riscv") || text_equal(target, "riscv64")) return "riscv64-unknown-elf";
    if (text_equal(target, "riscv32")) return "riscv32-unknown-elf";
    if (text_equal(target, "powerpc") || text_equal(target, "ppc")) return "powerpc-unknown-linux-gnu";
    if (text_equal(target, "ppc64le")) return "powerpc64le-unknown-linux-gnu";
    if (text_equal(target, "sparc") || text_equal(target, "sparc64")) return "sparc64-unknown-linux-gnu";
    if (text_equal(target, "wasm") || text_equal(target, "wasm32")) return "wasm32-wasi";
    return NULL;
}

static int add_formatted_argument(pl_language_build_plan *plan,
                                  const char *prefix, const char *value) {
    char buffer[PL_LANGUAGE_ARGUMENT_SIZE];
    const int length = snprintf(buffer, sizeof(buffer), "%s%s", prefix, value);
    if (length < 0 || (size_t)length >= sizeof(buffer)) return -1;
    return add_argument(plan, buffer);
}

int pl_language_build_plan_create(const char *id_or_name, const char *target,
                                  const char *source_path, const char *output_path,
                                  pl_language_build_plan *out) {
    const pl_language_profile *profile = pl_language_find(id_or_name);
    const char *triple;
    int compatible = 0;
    if (out == NULL || profile == NULL || !safe_path_argument(source_path) ||
        !safe_path_argument(output_path)) return -1;
    memset(out, 0, sizeof(*out));
    if (pl_language_check_target(id_or_name, target, &compatible) != 0) return -2;
    if (!compatible) return -3;
    if (profile->route == PL_LANGUAGE_ROUTE_NONE) return -4;
    if (copy_bounded_text(out->executable, sizeof(out->executable),
                          profile->toolchain) != 0) return -1;
    out->plan_only = 1;
    out->external_toolchain_required = 1;
    triple = target_triple(target);

    switch (profile->route) {
        case PL_LANGUAGE_ROUTE_MASM:
            if (add_argument(out, "/c") != 0 ||
                add_formatted_argument(out, "/Fo", output_path) != 0 ||
                add_argument(out, source_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_GAS:
            if (add_argument(out, "--32") != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0 ||
                add_argument(out, source_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_NASM64:
            if (add_argument(out, "-f") != 0 ||
                add_argument(out, "elf64") != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_CLANG:
        case PL_LANGUAGE_ROUTE_CLANGXX:
            if (triple == NULL ||
                add_formatted_argument(out, "--target=", triple) != 0 ||
                add_argument(out, "-c") != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_RUSTC:
            if (triple == NULL ||
                add_argument(out, "--emit=obj") != 0 ||
                add_argument(out, "--target") != 0 ||
                add_argument(out, triple) != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_ZIG:
            if (triple == NULL ||
                add_argument(out, "build-obj") != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-target") != 0 ||
                add_argument(out, triple) != 0 ||
                add_formatted_argument(out, "-femit-bin=", output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_LDC:
            if (triple == NULL ||
                add_argument(out, "-betterC") != 0 ||
                add_argument(out, "-c") != 0 ||
                add_formatted_argument(out, "-mtriple=", triple) != 0 ||
                add_argument(out, source_path) != 0 ||
                add_formatted_argument(out, "-of=", output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_PTXAS:
            if (add_argument(out, "-arch=sm_70") != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_LLVM_MC:
            if (add_formatted_argument(out, "-triple=",
                    profile->target_mask == TARGET_BPF ? "bpf" : "amdgcn-amd-amdhsa") != 0 ||
                add_argument(out, "-filetype=obj") != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_WAT2WASM:
            if (add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_LLC:
            if (triple == NULL ||
                add_argument(out, "-filetype=obj") != 0 ||
                add_formatted_argument(out, "-mtriple=", triple) != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_SPIRV_AS:
            if (add_argument(out, "--target-env") != 0 ||
                add_argument(out, "vulkan1.2") != 0 ||
                add_argument(out, source_path) != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_QBE:
            if (add_argument(out, "-t") != 0 ||
                add_argument(out, "amd64_sysv") != 0 ||
                add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0 ||
                add_argument(out, source_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_IVERILOG:
            if (add_argument(out, "-o") != 0 ||
                add_argument(out, output_path) != 0 ||
                add_argument(out, source_path) != 0) return -1;
            break;
        case PL_LANGUAGE_ROUTE_OPENCL:
        case PL_LANGUAGE_ROUTE_NONE:
        default:
            return -4;
    }
    return 0;
}
