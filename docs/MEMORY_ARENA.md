# C11 Arena Allocator

Pytyliki includes a bounded bump allocator for short-lived compiler, analysis and
workbench data. The allocator is intentionally small: it does not replace
general-purpose allocation, ownership discipline or synchronization.

## API

Header: `src/memory.h`; implementation: `src/memory.c`.

- `pl_arena_init`: allocate an owned arena buffer.
- `pl_arena_init_buffer`: use caller-owned storage without taking ownership.
- `pl_arena_alloc`: allocate a non-zero byte range aligned to a power of two.
- `pl_arena_calloc`: overflow-checked count × element-size allocation, zero-filled.
- `pl_arena_mark` / `pl_arena_rewind`: checkpoint and release later allocations.
- `pl_arena_reset`: release the current logical contents while retaining capacity.
- `pl_arena_get_stats`: inspect capacity, usage, free space, peak use and allocation count.
- `pl_arena_destroy`: free owned storage and clear the descriptor; external buffers remain caller-owned.

## Safety contract

1. The arena is single-owner and not thread-safe. Concurrent access requires external synchronization.
2. Alignment must be a non-zero power of two. Zero-byte requests, invalid alignment,
   multiplication overflow and requests beyond remaining capacity are rejected.
3. The allocator checks address/size arithmetic before moving its bump pointer.
4. A mark is an offset into the current arena. Rewinding beyond the current
   position is rejected. A rewind invalidates all allocations made after that mark.
5. Reset and destroy invalidate every pointer previously returned by the arena.
6. Optional clearing is a normal memory clear, not a guaranteed secure-erasure primitive.
7. Reinitializing an arena that already owns storage requires destroying it first.

The allocator deliberately has no hidden growth, global state, fallback heap
allocation or cross-thread ownership. Callers remain responsible for object
lifetimes, destructors for non-trivial resources, and not retaining pointers
past rewind/reset/destroy.

## Verification

The `memory-arena-tests` CTest target covers owned and external storage,
alignment, zero-size and invalid-alignment rejection, zero-fill, statistics,
rewind/reset, capacity exhaustion and multiplication overflow.

Run the complete project test suite:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
