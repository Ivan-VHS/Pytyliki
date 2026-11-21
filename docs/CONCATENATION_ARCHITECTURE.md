# Bounded concatenation and composition architecture

## Purpose

Pytyliki uses concatenation where it is a real processor-workstation primitive: assembling bounded textual reports, metadata fragments and binary evidence buffers. The 666-term taxonomy is an architecture crosswalk, not 666 claims of implemented features.

## Implemented C11 primitive

- `src/concat.h` and `src/concat.c` expose `pl_concat_buffer`.
- Append operations preserve input order and support arbitrary bytes, text and formatted fragments.
- A caller-selected payload ceiling is checked before allocation or copying. Capacity growth is overflow-checked; the trailing NUL byte is reserved separately.
- A rejected append does not change the logical length or previously assembled content.
- The API owns its buffer and releases it with `pl_concat_buffer_free`. Input fragments must not alias the builder's internal storage.
- CTest covers text, formatted appends, binary fragments, invalid arguments and limit rejection.

## Six-domain crosswalk

| Domain | Pytyliki mapping | Status |
|---|---|---|
| 1. Strings, parsing, serialization | Bounded text/byte builder for report and metadata assembly | Implemented |
| 2. Streams and pipelines | Existing deterministic processor pipeline is a reference plan; streaming execution is not implied | Partial |
| 3. Databases and relational joins | No database subsystem exists in this processor workstation | Not applicable |
| 4. Build, compilation, CI | C11 module is linked into `pytyliki_core`; CTest validates the contract | Implemented |
| 5. UI, graphics and multimedia | DSP remains a separate bounded sample-buffer path; no implicit audio-track concatenator is added | Not applicable |
| 6. Algebra and composition | Ordered byte append has an empty identity and associative concatenation semantics | Implemented |

## Algebraic and operational contract

For valid byte sequences, append is associative with respect to the resulting byte sequence and the empty sequence is the identity. The implementation does not reorder, normalize, deduplicate or reinterpret bytes. The configured maximum length is an operational resource constraint, so a mathematically valid concatenation can still be rejected when it exceeds the caller's budget.

## Safety and evidence

The module performs local memory operations only. It does not read files, execute source, access hardware, open network connections or mutate processor settings. It is not a database engine, distributed event bus, compiler linker or audio mixer. Future adapters must specify their input trust boundary, maximum size, ordering semantics, failure behavior and tests before they are described as implemented.
