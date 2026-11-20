# Memory Kernel: Views, Arenas and Fixed-Block Pools

Pytyliki exposes a small ISO C11 memory layer for compute, metric and streaming
workloads. It is designed around caller-visible ownership and bounded storage.

## Typed views

`pl_view_t` describes an existing memory region through its data pointer,
element count, type, stride, accessible extent and read-only flags. View
initialization validates the minimum byte extent using overflow-checked
arithmetic. Slicing is zero-copy: it returns another view into the same
backing storage. Views do not implicitly allocate or free that storage.

## Arena allocator

`pl_arena_t` supports external caller-owned storage through
`pl_arena_init`, or owned storage through `pl_arena_create`. Custom allocator
callbacks can be supplied to the owned form. Allocations are monotonic and
bounded; reset releases all allocations as a unit.

The current ABI also exposes:
- actual-address alignment for every allocation, including an intentionally
  misaligned external buffer;
- `pl_arena_mark` and `pl_arena_rewind` checkpoints that restore both the
  used offset and allocation count;
- optional clearing of the released byte range on rewind;
- `pl_arena_get_stats` with capacity, used bytes, remaining bytes, high-water
  usage and allocation count;
- explicit reset and destroy operations.

The ABI version is exposed as `PL_MEMORY_ABI_VERSION`. The current value is
2 because the arena descriptor now includes allocation telemetry. Code that
embeds `pl_arena_t` across a binary boundary must rebuild against the matching
header and ABI version.

## Fixed-block pool

`pl_pool_t` uses caller-owned storage and bitmap memory. The pool computes an
aligned stride, allocates fixed-size blocks without hidden heap activity and
rejects foreign pointers, misaligned pointers and double frees. The current
free-block search is a deterministic linear scan.

## Safety and lifetime rules

1. An arena and pool are not internally synchronized. Use external locking
   when sharing them between threads.
2. Arena alignment must be a non-zero power of two. Allocation returns null
   for zero-sized requests, invalid alignment, overflow or exhausted capacity.
3. A checkpoint belongs to the arena state at which it was taken. Rewinding
   invalidates allocations after that checkpoint; rewinding forward is rejected.
4. Reset, rewind and destroy invalidate pointers into the released region.
5. External storage and pool bitmaps remain caller-owned. Destroy frees arena
   memory only when it was created through `pl_arena_create`.
6. A custom allocate/deallocate callback pair must be supplied together.
7. Optional clearing is ordinary memory clearing; it is not a guaranteed
   secure-erasure primitive.

## Verification

CTest's `memory-view-arena-pool-tests` covers typed view validation and
slicing, read-only enforcement, arena alignment against a misaligned external
buffer, checkpoints, rewind clearing, telemetry, owned allocation, block-pool
allocation/free and aligned allocation helpers.

Run the full suite:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
