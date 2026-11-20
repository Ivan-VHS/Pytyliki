#include "architecture_atlas.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const pl_architecture_profile profiles[PL_ARCH_ATLAS_COUNT] = {
    {1u, "x86_64", "x86", "desktop/server", 64u, "little", "out-of-order CISC", "GCC, Clang/LLVM, MSVC, Intel oneAPI", "CPUID, OS-managed vector state, cache topology, ABI and calling convention"},
    {2u, "x86", "x86", "legacy desktop/embedded", 32u, "little", "out-of-order or in-order CISC", "GCC, Clang/LLVM, MSVC", "CPUID availability, x87/SSE state, 32-bit ABI and address-space limits"},
    {3u, "aarch64", "Arm", "mobile/desktop/server", 64u, "little (common profile)", "load/store RISC, optional SIMD", "GCC, Clang/LLVM, Apple Clang, Arm Compiler", "NEON/SVE feature discovery, cache topology, pointer authentication where present"},
    {4u, "armv7-m", "Arm", "microcontroller", 32u, "little (common profile)", "Thumb load/store RISC, interrupt-driven", "Armclang, GCC, LLVM embedded", "NVIC priorities, interrupt latency, SRAM/flash map, FPU presence"},
    {5u, "riscv64", "RISC-V", "open ISA / embedded-server", 64u, "target-defined", "modular load/store RISC", "GCC, LLVM, vendor toolchains", "ISA extension set, privilege modes, vector extension and ABI string"},
    {6u, "riscv32", "RISC-V", "microcontroller/embedded", 32u, "target-defined", "modular load/store RISC", "GCC, LLVM, vendor toolchains", "XLEN, extension set, interrupt controller and memory map"},
    {7u, "loongarch64", "LoongArch", "desktop/server", 64u, "little (common profile)", "load/store RISC", "GCC, LLVM and platform toolchains", "ISA feature flags, ABI variant, kernel/userspace boundary"},
    {8u, "e2k", "Elbrus", "VLIW desktop/server", 64u, "target-defined", "explicitly scheduled VLIW", "MCST LCC / platform compiler", "compiler version, VLIW scheduling, ABI and platform-specific counters"},
    {9u, "ppc64le", "Power ISA", "server/accelerator", 64u, "little", "load/store RISC", "GCC, LLVM, IBM XL where available", "VSX/AltiVec capabilities, endian ABI, cache and NUMA topology"},
    {10u, "s390x", "IBM Z", "mainframe", 64u, "big", "register-rich CISC", "GCC, LLVM, IBM toolchains", "big-endian data layout, vector facility, ABI and processor facilities"},
    {11u, "avr", "AVR", "8-bit microcontroller", 8u, "little", "Harvard-style MCU", "avr-gcc, LLVM-based toolchains where supported", "program/data address spaces, flash/RAM limits, interrupt vectors"},
    {12u, "pic8", "Microchip PIC", "8-bit microcontroller", 8u, "target-defined", "banked Harvard MCU", "MPLAB XC8, device-specific assemblers", "register banks, page boundaries, peripheral map and device revision"},
    {13u, "xmos-xcore", "XMOS", "deterministic multicore MCU", 32u, "little", "hardware threads and channel communication", "XMOS XTC / XC toolchain", "tile/thread allocation, channel topology and cycle-budget assumptions"},
    {14u, "wasm32", "WebAssembly", "portable virtual ISA", 32u, "little", "sandboxed stack machine", "Clang/LLVM, wasi-sdk, language-specific toolchains", "WASI capability set, linear-memory bounds and host ABI"}
};

static int equal_folded(const char *left, const char *right) {
    if (!left || !right) return 0;
    while (*left && *right) {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) return 0;
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

size_t pl_architecture_atlas_catalog(const pl_architecture_profile **out) {
    if (!out) return 0u;
    *out = profiles;
    return PL_ARCH_ATLAS_COUNT;
}

int pl_architecture_atlas_find(const char *name, pl_architecture_profile *out) {
    size_t i;
    if (!name || !out || !name[0]) return -1;
    for (i = 0u; i < PL_ARCH_ATLAS_COUNT; ++i) {
        if (equal_folded(name, profiles[i].name) || equal_folded(name, profiles[i].family)) {
            *out = profiles[i];
            return 0;
        }
    }
    return -2;
}

int pl_architecture_atlas_current(char *out, size_t capacity) {
    const char *name = "unknown";
    if (!out || capacity == 0u) return -1;
#if defined(__x86_64__) || defined(_M_X64)
    name = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    name = "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
    name = "aarch64";
#elif defined(__arm__) || defined(_M_ARM)
    name = "armv7-m";
#elif defined(__riscv) && (__riscv_xlen == 64)
    name = "riscv64";
#elif defined(__riscv) && (__riscv_xlen == 32)
    name = "riscv32";
#elif defined(__loongarch64)
    name = "loongarch64";
#elif defined(__powerpc64__) && defined(__LITTLE_ENDIAN__)
    name = "ppc64le";
#elif defined(__s390x__)
    name = "s390x";
#elif defined(__AVR__)
    name = "avr";
#elif defined(__XC8)
    name = "pic8";
#elif defined(__xcore__)
    name = "xmos-xcore";
#elif defined(__wasm32__)
    name = "wasm32";
#endif
    if (snprintf(out, capacity, "%s", name) < 0 || strlen(name) >= capacity) {
        out[0] = '\0';
        return -1;
    }
    return 0;
}
