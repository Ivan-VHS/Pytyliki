# Elbrus E2K support

## Support status

Pytyliki has a portable C11 core and already recognizes the MCST compiler target macro
`__e2k__`. This integration makes the target visible in the processor report, locks that
behavior with tests, and adds a reproducible native build path.

| Layer | E2K status |
|---|---|
| C11 core / CLI / shared C ABI | Buildable with a matching native or cross LCC toolchain; validate on target |
| Processor identity | Reports the generic `e2k` architecture and MCST/LCC provenance; does not guess an exact CPU model |
| Memory inventory / monotonic timer | Uses the existing POSIX path where the target OS exposes it |
| x86 CPUID / SSE / AVX fields | Not applicable to E2K; these flags remain clear |
| Thermal, voltage, PMU, cache and scheduler telemetry | Not claimed unless a separately implemented OS interface returns measured evidence |
| Windows GUI | Windows-only; not part of the E2K Linux build |

The architecture catalogue records the E2K family independently of runtime detection. A catalogue
entry is descriptive data, not proof that a compiler, emulator or physical device is available.

## Supported compiler and OS path

Use the MCST System Programming Kit (SDK), which supplies LCC, assembler/linker tools, GDB,
profiling and coverage utilities. The upstream MCST download portal currently lists LCC 1.31.05
cross-toolchain packages for Elbrus Linux 9/10 and Linux sources with E2K support.

- [MCST SDK and compiler matrix](https://www.mcst.ru/SDK)
- [MCST LCC compiler](https://www.mcst.ru/LCC)
- [MCST developer downloads](https://dev.mcst.ru/download/)
- [Elbrus Linux supported processor generations](https://www.mcst.ru/elbrus_linux)

The platform uses the compiler-defined `__e2k__` macro. Do not infer generation from that macro:
select the target package/sysroot that matches the actual processor and OS release.

## Native build and tests

On an Elbrus Linux machine with the native LCC driver exposed as `gcc`:

```sh
CC=gcc CXX=g++ bash scripts/elbrus/build-e2k.sh
```

Or set `E2K_CC` to the exact compiler command/path provided by the installed SDK. The script
checks that the compiler actually defines `__e2k__` before configuring CMake, then builds and
runs CTest on the native host.

## Cross compilation

The repository provides `cmake/toolchains/elbrus-e2k.cmake`. Supply the cross-compiler and,
when required by the vendor package, its matching sysroot:

```sh
E2K_CC=/opt/mcst/bin/e2k-gcc E2K_SYSROOT=/opt/mcst/sysroot \
  cmake -S . -B build-e2k-cross \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/elbrus-e2k.cmake \
  -DBUILD_TESTING=ON
cmake --build build-e2k-cross --parallel
```

The example compiler path is a placeholder: use the executable and sysroot shipped with your
licensed/native or cross LCC package. No generic architecture flags are injected. Cross-built
binaries must be run on matching E2K hardware or a separately validated `qemu-e2k` environment;
a successful link is not a runtime test.

## Optimization and evidence

Start with the compiler defaults and release build, then profile on the target system. Keep
portable C11 as the baseline. Add E2K-specific code only behind `__e2k__`, document its ABI and
compiler-version assumptions, and retain a portable fallback. Do not reuse x86 CPUID, XCR0,
SSE or AVX logic for E2K.

Benchmark outputs are measurements, not cross-machine performance promises. Record the processor
model, E2K generation, OS release, LCC version, build flags, input size and repeated-run spread
with each performance comparison.

## CI

`.github/workflows/elbrus-e2k.yml` is a manual workflow for a self-hosted runner labelled
`self-hosted, linux, e2k`. It is deliberately opt-in until such a runner is configured.
The standard hosted x86/Windows jobs remain independent and unchanged.
