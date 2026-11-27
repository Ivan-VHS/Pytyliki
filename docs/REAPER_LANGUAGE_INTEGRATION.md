# Reaper language integration

Pytyliki remains a C11 processor/DSP workstation. Reaper 0.2 is added as an optional, host-side orchestration and analysis language; it does not replace real-time DSP, CPU discovery, C ABI primitives or bounded native processing.

## Reaper program

`automation/reaper/processor_metrics.rpr` is an executable integration program. It opens the explicitly built Pytyliki C ABI v1 library, calls norm/dot/distance/orthogonality operations through Reaper's typed bridge, discovers the 18-entry native plugin catalogue without loading plugins, resolves one metadata descriptor, checks the returned values, and writes a versioned JSON report.

Build and run from the repository root:

```sh
cmake -S . -B build -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target pytyliki_ffi
PYTHONPATH=/path/to/Reaper python -m reaper --check automation/reaper/processor_metrics.rpr
PYTHONPATH=/path/to/Reaper python -m reaper --engine vm --allow-native-ffi automation/reaper/processor_metrics.rpr
```

The example expects the Linux shared-library path `build/libpytyliki_ffi.so`. On Windows, use the CMake-generated DLL path in the same explicit way; do not add a PATH or implicit search fallback.

## Integration contract

- Reaper owns source-level orchestration, structured reporting and bounded collection transforms.
- Pytyliki owns the native numerical implementation and C ABI v1.
- The script passes only finite, small vectors and scalar tolerances to the native ABI.
- The native library is loaded only with the explicit `--allow-native-ffi` opt-in and a path resolved relative to the script.
- `pytyliki_plugin_catalog` and `pytyliki_plugin_info` expose bounded metadata only; plugin code is not loaded or invoked.
- There is no automatic network access, executable lookup, plugin loading or audio-callback dependency.
- Reaper's Python reference runtime and bytecode VM are both test targets; C17 Reaper remains a separate partial compiler track.

The integration workflow builds the real C library and executes the `.rpr` program in the Reaper VM.
