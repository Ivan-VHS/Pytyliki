# Low-level C generation and cross-ISA validation

## Scope

Pytyliki uses ISO C11 as the portable semantic baseline. CI validates the
application natively on Linux and Windows and cross-compiles it for AArch64 and
RISC-V 64. The target assembly is retained as a review artifact and a QEMU CLI
smoke test checks that the cross-built executable starts. This does not prove
microarchitectural performance or implement a general-purpose transpiler.

## Optimization contract

Compiler optimization is enabled with explicit flags (-O3 for cross-target
compilation). Alignment, vector intrinsics, and architecture-specific
implementations may be introduced only behind a portable C interface and
guarded by compile-time feature checks. The scalar implementation remains the
reference semantics and is covered by tests. Benchmarks must record target,
compiler, flags, input size, and result; performance claims require comparable
native-hardware runs.

## Hardware safety boundary

The application and generated code must not perform arbitrary MMIO, write CPU
control registers, modify MSRs, flash firmware, alter BIOS/UEFI, set voltage or
clock controls, or access physical memory. volatile is not a synchronization
primitive and does not make an arbitrary address safe. Memory barriers and
architecture intrinsics are permitted only for ordinary process memory and
documented, application-owned synchronization.

The default application remains hosted C11. -ffreestanding, -nostdlib,
privileged instructions, inline assembly, and device-register access are not
enabled in the product build. Any future bare-metal experiment must live in a
separate target, be opt-in, be reviewed independently, and never be called by
the desktop utility.

## Cross-ISA synthesis stages

1. Preserve a portable scalar C implementation as the semantic oracle.
2. Add a target-specific implementation in a dedicated translation unit.
3. Compile it with a pinned toolchain for the requested target.
4. Run native tests and, where available, an emulator smoke test. Emulation is
   not a substitute for physical performance measurement.
5. Compare outputs against the scalar oracle over deterministic boundary and
   randomized inputs.
6. Publish generated assembly and compiler metadata as review artifacts.
7. Accept the optimization only through a reviewed pull request.

No pipeline may autonomously weaken tests, rewrite security policy, or merge a
generated optimization without human review. SMT-based translation validation,
LLVM/MLIR lifting, and real-hardware runner benchmarks remain future extension
points rather than capabilities that the current build claims to provide.
