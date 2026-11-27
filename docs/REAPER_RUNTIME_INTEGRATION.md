# Reaper native runtime integration

Pytyliki can optionally link Reaper's native C17 ABI into the Pytyliki C11 shared ABI. This is the reverse native direction of Reaper's explicit-path `pytyliki.c-abi.v1` adapter.

## Build the reciprocal native bridge

Provide a trusted local Reaper source checkout explicitly:

```sh
cmake -S . -B build -DREAPER_SOURCE_DIR=/absolute/path/to/Reaper
cmake --build build --parallel --target pytyliki-interop-tests
ctest --test-dir build --output-on-failure -R '^interop-abi-tests$'
```

When `REAPER_SOURCE_DIR` is set, CMake adds the supplied Reaper project as an excluded subdirectory, links its `reaper_ffi` C17 target into `pytyliki_ffi`, and defines `PYTYLIKI_WITH_REAPER`. There is no network fetch, PATH search or implicit sibling-directory discovery.

The optional functions in `src/interop.h` are:

- `pytyliki_v1_reaper_available` — returns 1 only when the linked Reaper ABI version is supported.
- `pytyliki_v1_reaper_abi_version` — returns the Reaper ABI version, or 0 when unavailable.
- `pytyliki_v1_reaper_eval_i64` — forwards a bounded signed-i64 expression to Reaper's lexer, expression parser, checked IR and evaluator.
- `pytyliki_v1_reaper_selfhood_lookup` — forwards an exact identity lookup to Reaper.

Reaper status codes are forwarded unchanged for evaluation and lookup. A build without `REAPER_SOURCE_DIR` remains self-contained: availability is 0, the version is 0 and bridge calls return `PYTYLIKI_ABI_BACKEND_UNAVAILABLE`.

## Reciprocal direction

Reaper consumes the Pytyliki C ABI only when a caller supplies an absolute library path and explicitly enables native FFI. See [Reaper's native language integration](REAPER_LANGUAGE_INTEGRATION.md) and the shared [PytyRe architecture contract](PYTYRE_UNIFIED_ARCHITECTURE.md).

## Boundaries

- This is an expression ABI bridge, not full Reaper .rpr program execution inside Pytyliki.
- The vendored `vendor/reaper_frontend` remains a bounded expression-only frontend snapshot; it is not the full Reaper compiler or runtime.
- The bridge does not add filesystem, process, network, plugin, device or hardware privileges.
- Pytyliki's processor discovery, measurements, low-level C11 contracts and DSP/metric ownership remain native to Pytyliki.
- A Reaper API update must update the shared PytyRe contract, both repository integration tests and the pinned cross-repository CI workflow.
