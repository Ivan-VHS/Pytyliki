# Pytyliki C Diagnostics ABI v1

**Status:** additive ABI proposal; implementation is under review  
**Owner:** Pytyliki C11 core  
**Consumer:** OPlague native integration  
**Existing ABI:** `pytyliki.c-abi.v1` remains unchanged

## Scope

This extension adds a caller-owned diagnostic envelope and an error-reporting form of the Euclidean-distance operation. It does not replace existing ABI v1 entry points, introduce a second allocator, load plugins, or grant hardware access.

The diagnostic extension has its own version constant, `PYTYLIKI_DIAGNOSTICS_ABI_VERSION`. A consumer must check both `pytyliki_abi_version()` and `pytyliki_diagnostics_abi_version()` before using the extension.

## Binary layout

`pytyliki_diagnostic_v1` is a fixed-width, 8-byte-aligned C structure.

| Field | Type | Offset | Meaning |
|---|---|---:|---|
| `struct_size` | `uint32_t` | 0 | Caller-provided writable capacity |
| `abi_version` | `uint32_t` | 4 | Diagnostics schema version |
| `code` | `int32_t` | 8 | Stable diagnostic code |
| `severity` | `uint32_t` | 12 | Stable severity value |
| `source_offset` | `uint64_t` | 16 | Source byte offset or no-source sentinel |
| `source_length` | `uint64_t` | 24 | Source byte length |
| `detail` | `uint64_t` | 32 | Code-specific numeric context |
| `message_size` | `uint32_t` | 40 | Message size including the terminating NUL |
| `reserved` | `uint32_t` | 44 | Must be zero |
| `message` | `char[96]` | 48 | Bounded UTF-8/ASCII diagnostic text |

The required layout is **144 bytes**, alignment **8 bytes**. The public header contains C11/C++ compile-time assertions for size, alignment and offsets. Native tests repeat the assertions so a consumer build also verifies the contract.

`PYTYLIKI_DIAGNOSTIC_NO_SOURCE` (`UINT64_MAX`) means the operation has no source location. The distance operation uses that sentinel and a zero source length.

## Calling convention and ownership

- Exported functions use `PYTYLIKI_FFI_CALL`, defined as `__cdecl` on MSVC and 32-bit Windows GCC/Clang; other supported targets use the platform C convention.
- All arguments are borrowed for the duration of a synchronous call.
- The caller owns input vectors, output scalars and the diagnostic buffer.
- The library never retains a pointer, returns an allocated pointer, frees caller memory, or transfers ownership.
- The library performs no allocation and uses no caller-provided allocator.
- The diagnostic catalogue and metric implementation use no mutable process-global state.
- The ABI does not promise that callers may concurrently mutate the same output buffer. Distinct call-local buffers are independent.

## Initialization and size negotiation

1. Allocate a `pytyliki_diagnostic_v1` or a larger append-only structure whose first bytes match v1.
2. Call `pytyliki_v1_diagnostic_init(sizeof(buffer), &buffer)`.
3. Pass the same capacity as `diagnostic_buffer_size` to diagnostic-producing functions.
4. The library rejects a buffer smaller than 144 bytes, a mismatched declared capacity, or an unsupported diagnostics version without writing the buffer.
5. For a larger buffer, the library writes only the 144-byte v1 prefix. Bytes after that prefix remain caller-owned and untouched.

A caller must not lie about the actual allocation size. The explicit size parameter and the structure's `struct_size` field must match.

## Error model

The C return value is the machine-readable operation status. The diagnostic `code` is the stable diagnostic category. Human-readable message text is explanatory and must not be parsed to determine behavior.

| C status | Diagnostic code | Meaning |
|---:|---:|---|
| 0 | 0 | Success |
| -1 | 1001 | Invalid argument |
| -2 | 1003 | Unsupported diagnostics ABI |
| -3 | 1002 | Diagnostic buffer too small |
| -5 | 1004 | Vector dimension exceeds 4096 |
| -6 | 1005 | Input contains NaN or infinity |
| -7 | 1006 | Result is outside finite float64 range |

For `PYTYLIKI_DIAG_LIMIT_EXCEEDED`, `detail` is the supported dimension limit. For `PYTYLIKI_DIAG_NON_FINITE_INPUT`, it is the zero-based offending element index. For numeric overflow, it is the requested dimension. For success, it is the number of processed elements.

On failure, the distance function does not modify the caller's output scalar. If the diagnostic buffer itself is invalid, the function returns the corresponding ABI status and leaves that buffer untouched.

## API

```c
uint32_t pytyliki_diagnostics_abi_version(void);

int32_t pytyliki_v1_diagnostic_init(
    uint32_t out_size,
    pytyliki_diagnostic_v1 *out);

int32_t pytyliki_v1_distance_f64_ex(
    const double *left,
    const double *right,
    uint64_t count,
    double *out,
    uint32_t diagnostic_buffer_size,
    pytyliki_diagnostic_v1 *diagnostic);
```

The diagnostic pointer is optional only when `diagnostic_buffer_size == 0`. The operation accepts dimensions from 1 through 4096.

## Compatibility policy

- Existing `pytyliki.c-abi.v1` symbols and structure layouts remain unchanged.
- New symbols are additive and gated by the separate diagnostics ABI version.
- A v1 implementation accepts a buffer at least 144 bytes long and writes only the v1 prefix.
- Future append-only fields require a new structure version and new size/layout assertions.
- Any incompatible field reorder, width, alignment, calling-convention or ownership change requires a new major ABI version.
- Consumers must reject unsupported versions rather than guessing layouts.

## Verification

The native C11 test target verifies:

- exact size, alignment and member offsets;
- initialization and version negotiation;
- successful distance evaluation;
- invalid arguments, zero and oversized dimensions;
- non-finite input and numeric overflow;
- no output mutation on failed calculations;
- too-small, mismatched and unsupported diagnostic buffers;
- preservation of extension bytes after the v1 prefix;
- null diagnostic behavior.

The downstream OPlague integration must compile its own layout assertions and exercise the real shared library using its native toolchain. Mock-only FFI tests are not evidence of binary compatibility.
