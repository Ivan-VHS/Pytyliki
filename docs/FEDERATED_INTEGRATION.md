# Federated integration: Pytyliki, OPlague and Reaper

Pytyliki stays a standalone C11 numerical core. OPlague owns Windows PnP
inventory and driver-package inspection. Reaper owns .rpr orchestration and
reports. The projects exchange versioned data and a small C ABI, not source
files or internal object layouts.

## Pytyliki C ABI v1

CMake builds the optional shared library target pytyliki_ffi.

- Windows: pytyliki_ffi.dll (multi-config builds commonly use build/Release/)
- Linux: libpytyliki_ffi.so
- macOS: libpytyliki_ffi.dylib

The public header is src/interop.h. Consumers must check
pytyliki_abi_version() == 1 before calling a metric function. ABI v1 exposes
contiguous double vectors, fixed-width uint64_t counts, output pointers and
integer status codes. Calls are synchronous and never retain caller buffers.

| Exported symbol | Purpose |
|---|---|
| pytyliki_v1_dot_f64 | dot product |
| pytyliki_v1_norm_f64 | Euclidean norm |
| pytyliki_v1_distance_f64 | Euclidean distance |
| pytyliki_v1_orthogonal_f64 | tolerance-based orthogonality |

Status 0 means success; -1 means invalid input, dimension, non-finite value or
unrepresentable result. The core limit of 4096 vector elements is preserved.
The shared library is opt-in: consumers explicitly name a trusted library path.

## Data ownership and safety

OPlague publishes the oplage.device-snapshot.v1 JSON contract. Device IDs
are HMAC-SHA-256 pseudonyms over PnP instance IDs with domain separation; raw
instance IDs are not exported. Each default snapshot uses an ephemeral key, so
identities cannot be correlated across exports by default. A caller-managed
protected key deliberately enables pseudonymous cross-snapshot matching.
Pseudonymization is not anonymization. Metric profiles use [0, 1], where higher
is healthier/better. Missing telemetry is never fabricated.

Reaper reads snapshots and writes analysis reports. It may load Pytyliki ABI v1
only when a script explicitly supplies a library path. Native loading executes
trusted native code, so the path must identify a trusted build.

This integration does not grant driver-management capabilities. Inventory and
metric calculations are read-only. Installation, removal, rollback, service
changes and privileged operations remain behind OPlague's dry-run and explicit
confirmation gates.

## Build and verify

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The interop-abi-tests target checks versioning, numeric results, invalid
arguments and vector limits.


## Cross-repository references

- **Shared 3 × 3 map:** [Project federation matrix](https://github.com/Ivan-VHS/Reaper/blob/main/docs/PROJECT_FEDERATION_MATRIX.md).
- **Reaper:** [repository](https://github.com/Ivan-VHS/Reaper), [federation contract](https://github.com/Ivan-VHS/Reaper/blob/main/docs/FEDERATED_INTEGRATION.md), [native FFI consumer](https://github.com/Ivan-VHS/Reaper/blob/main/reaper/ffi.py), [project manifest](https://github.com/Ivan-VHS/Reaper/blob/main/project-contract.json).
- **OPlague:** [repository](https://github.com/Ivan-VHS/OPlague), [federation contract](https://github.com/Ivan-VHS/OPlague/blob/main/docs/FEDERATED_INTEGRATION.md), [native metric adapter](https://github.com/Ivan-VHS/OPlague/blob/main/tools/pytyliki_ffi.py), [project manifest](https://github.com/Ivan-VHS/OPlague/blob/main/project-contract.json).
- **Pytyliki evidence:** [public ABI header](../src/interop.h), [ABI implementation](../src/interop.c), [project manifest](../project-contract.json).


## OCTA-8 horizontal peer product and octagonal pyramid

The shared [OCTA-8 topology contract](OCTA_PYRAMID_FEDERATION.md) defines the directed Reaper↔Pytyliki↔OPlague peer product: six directed links, three local self-loops and eight protocol facets. Pytyliki remains the portable C11 numerical provider. Calls are synchronous and return results over the same explicitly selected ABI; Pytyliki does not launch peer runtimes or load peer code.
