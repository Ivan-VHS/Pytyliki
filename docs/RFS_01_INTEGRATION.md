# RFS-01 integration — Pytyliki

Pytyliki implements the consumer-side C11 validation and planning surface for
RFS-01. Reaper remains the canonical owner of grammar, phase semantics,
operator semantics, and the reference execution model.

## Public ABI

Header: `src/interop.h`  
Shared library: `pytyliki_ffi`  
Contract version: `PYTYLIKI_RFS01_ABI_VERSION == 1`

The adapter exposes fixed-size, caller-owned records for nodes, links, and
signal frames. All calls are synchronous. No pointer is retained and no
caller-owned memory is allocated.

- `pytyliki_rfs01_validate_node_v1`, `validate_link_v1`, and
  `validate_signal_v1` validate record structure, versions, finite numeric
  domains, enums, bounded NUL-terminated fields, and strict UTF-8 validity (including rejection of overlong encodings, surrogate encodings, truncated sequences, and code points above U+10FFFF). The signal payload accepts exactly 4,096 UTF-8 bytes plus its required NUL terminator.
- `pytyliki_rfs01_pressure_reserve_v1` computes
  `integrity * (1 - min(pressure / load_limit, 1))`.
- `pytyliki_rfs01_plan_pressure_forward_v1` returns a proposed target state.
  It rejects blocked links and load-limit violations; the output is unchanged
  on failure.
- `pytyliki_rfs01_plan_signal_forward_v1` accepts only trusted, non-manufactured
  frames and applies the RFS-01 noise attenuation to a proposed target record.
- `pytyliki_rfs01_root_restore_v1` restores the complete supplied root
  snapshot; `pytyliki_rfs01_core_restore_v1` restores only the core seed.

The single-frame ABI validates each sequence as a non-negative uint64 and validates provenance; it does not retain history or enforce per-origin monotonic ordering across calls. Reaper is the canonical sequence-order validator.\n\nThe functions are policy and validation helpers, not an OS scheduler, runtime,
kernel, hardware controller, or privileged service. They do not write process
state, CPU affinity, device registers, firmware, clocks, or voltage controls.

## Build and verification

The adapter is compiled into the existing `pytyliki_ffi` library and covered
by the CTest target `rfs-01-c11-contract`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure -R rfs-01-c11-contract
```

JSON interchange records are described by
[the versioned schema](../schemas/rfs-01-pytyliki.v1.schema.json). The C ABI
uses numeric enums and fixed-size strings rather than parsing JSON in-process.

## Ecosystem boundary

- Reaper: canonical RFS-01 grammar, semantics, typed program, and runtime.
- Duck OS: Reaper-native system adapter and bounded system-policy consumer.
- Pytyliki: independent C11 record validation, diagnostics, and proposal
  calculation. It neither forks the language nor executes RFS source.

Native loading remains explicit and must use a trusted ABI-compatible library.
