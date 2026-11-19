# MCST Elbrus E2K target profile

Status: **compile-time target detection and CMake adapter implemented; native AETHER-ANAS backend and hardware validation pending**.

## Verified platform facts

- MCST documents __e2k__ as the C target macro and __LCC__ as the MCST compiler major-version macro.
- MCST classifies Elbrus as VLIW: the compiler analyses dependencies, schedules operations and emits wide instructions. Software pipelining is a documented loop optimisation. No universal issue width or register/ALU count is hard-coded here.
- The MCST SDK includes LCC, assembler/linker, debugger and profiling tools. The LCC source code is not included; toolchain availability and compatibility depend on processor and OS release.

References:
- [MCST effective programming guide — architecture and VLIW](https://dev.mcst.ru/book/chapter4.html)
- [MCST effective programming guide — software pipelining](https://dev.mcst.ru/book/chapter6.html)
- [MCST effective programming guide — compiler target macros](https://dev.mcst.ru/book/chapter2.html)
- [MCST Elbrus SDK](https://www.mcst.ru/SDK)
- [MCST LCC compiler](https://www.mcst.ru/LCC)

## Build

Install the licensed MCST SDK for the exact target processor and Linux release.

~~~sh
# Find lcc on PATH, or provide MCST_LCC
MCST_LCC=/opt/mcst/bin/lcc cmake -S . -B build-e2k \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/e2k-lcc.cmake \
  -DBUILD_TESTING=ON

# For a cross-build, pass the SDK's matching sysroot
MCST_LCC=/opt/mcst/bin/lcc cmake -S . -B build-e2k \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/e2k-lcc.cmake \
  -DCMAKE_SYSROOT=/path/to/target-sysroot \
  -DBUILD_TESTING=ON

cmake --build build-e2k --parallel
~~~

On native E2K Linux, run CTest and the profile:

~~~sh
ctest --test-dir build-e2k --output-on-failure
./build-e2k/pytyliki --e2k-profile
~~~

Cross-compiled tests cannot run on the build host without a validated emulator. The repository's current Windows/Linux CI does not validate E2K.

## Implemented

- Processor inventory identifies an E2K build through __e2k__ and reports LCC major version only when __LCC__ is defined for that E2K build.
- The x86 CPUID path is not selected for E2K compilation.
- The --e2k-profile CLI reports the compile target and states that native code generation is not implemented.
- Existing CPU suite tests cover E2K and non-E2K compile-time branches.

## AETHER-ANAS interpretation

| Proposal | Status in Pytyliki |
|---|---|
| 66 laws and 12 mathematical models | Engineering heuristics only; every performance hypothesis needs measured evidence. |
| Dependency graph, VLIW scheduling, software pipelining | Future compiler work. Use MCST LCC diagnostics, generated assembly and profiling as reference evidence. |
| Typed semantic hypergraph | Future IR. Keep source inspectable and define a versioned, deterministic schema before storing compiled graph artifacts. |
| RAII, ownership and memory regions | Future language/runtime semantics; no new memory-safety guarantee is claimed for the C11 workstation. |
| FBIP and SoA layout | Workload-specific optimisation candidates; require correctness, aliasing checks and comparative benchmarks. |
| Bounded channels and backpressure | Future stream/concurrency design, not a current runtime feature. |
| Native E2K code generator | Not implemented. It needs a language spec, typed IR, ABI, lowering, register allocation, linker/debug support and target conformance tests. |

No fixed wide-command width, ALU count, register-file size, memory-channel count or zero-vulnerability guarantee is asserted. Existing repository licensing is unchanged; MCST SDK components retain their own terms.
