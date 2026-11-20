# Pytyliki compute kernel: memory, metrics and operation registry

This document records the first implementation increment for the compute-kernel roadmap. The public interface remains ISO C11 and all pre-existing metric entry points remain available.

## Implemented in this increment

### Memory and views
- memory.h / memory.c add stable status values, typed strided views, bounded zero-copy slicing, f64 view access, chunk metadata, an arena allocator, a fixed-block pool, alignment inspection and aligned allocation helpers.
- Views never free their backing storage. PL_VIEW_OWNS_DATA is descriptive metadata for the caller's lifecycle policy; callers remain responsible for releasing owned allocations with their matching allocator.
- extent_bytes is the accessible byte extent beginning at data. For a non-empty view, the implementation checks (count - 1) * stride_bytes + elem_size <= extent_bytes with overflow checks.
- A zero stride is rejected. Empty views are allowed but must have a zero extent.
- Arena allocations are monotonic and reset as a unit. pl_arena_create accepts caller-supplied allocate/deallocate callbacks; pl_arena_init uses caller-owned storage.
- Pool storage and the allocation bitmap are caller-owned. Pool operations do not allocate internally. Free rejects foreign pointers and double frees.

### Metric and statistical kernel
The existing dot product, Euclidean norm/distance, orthogonality check and quadratic form are retained. New operations include L1/L-infinity norms, weighted L2 norm and dot product, squared/L1/L-infinity/weighted distances, vector projection, orthogonal remainder, cosine similarity, mean/variance, RMS, covariance and Pythagorean/triangle checks.

Statistical routines use Welford-style online updates. sample != 0 selects the sample denominator (n - 1); otherwise the population denominator (n) is used. The double API rejects non-finite input and rejects results that cannot be represented as a finite double.

### Operation registry and CLI
The static operation registry exposes numeric IDs, canonical names, categories, search terms, descriptions, complexity notes, supported type masks and capability flags. Search is case-insensitive substring matching across the name, category, terms and description. Callers may pass a category and required capability mask.

Examples:

    pytyliki-metrics --demo
    pytyliki-metrics --list
    pytyliki-metrics --search orthogonal
    pytyliki-metrics --category statistics
    pytyliki-metrics --describe 1040

## API contracts and limits
- Public headers are memory.h, metric_geometry.h and metric_registry.h.
- Existing metric functions continue returning 0 on success and -1 on invalid, incompatible or non-finite input. The new memory functions use pl_status_t.
- The matrix quadratic form remains dense and bounded by PL_METRIC_MAX_MATRIX_DIMENSION.
- The chunk type is metadata over an already-read buffer. It does not open files, map files or perform asynchronous I/O.
- The pool currently performs a deterministic linear scan for a free block. This favors compact caller-owned metadata over a hidden free-list allocation.
- Current public metrics operate on double arrays. Typed pl_view_t is the zero-copy foundation; generic typed numeric dispatch and direct view-to-metric adapters can be added without changing existing metric signatures.

## Validation
CTest adds dedicated memory and expanded metric/registry targets, plus CLI smoke coverage for listing and search. Run:

    cmake -S . -B build -DBUILD_TESTING=ON
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure

## Next integration slice
The next increment should add view-backed metric adapters, streaming-statistics state objects, multi-point nearest/farthest queries, JSON registry output, ABI version query functions and a Win32 Metric Lab panel that calls only the C API. These are not claimed as implemented by this change.
