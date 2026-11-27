# Universal Division & Decomposition

Pytyliki exposes a bounded C11 arithmetic core in `src/division.h`.

## Contracts

- Euclidean integer division returns quotient and remainder satisfying
  `a = q*b + r`, with `0 <= r < abs(b)` for every representable result.
- Division by zero and the signed overflow case `INT64_MIN / -1` are rejected.
- Even distribution conserves the exact signed total; earlier buckets receive
  the indivisible remainder.
- APIs validate pointers and output capacity and perform no allocation.
- The engine is deterministic and side-effect-free.

## Extension points

Future work can add exact rational arithmetic, weighted largest-remainder
allocation, factorization, bounded matrix tiling, dependency-graph partitioning,
and resource-aware workload scheduling. Such planners must distinguish
measured cost from estimates, preserve stable ordering where required, and
verify reconstruction and conservation invariants.

## Auto-improvement policy

The engine may propose alternative partitions and benchmark them against a
baseline. It must not silently rewrite source, change hardware settings, or
apply an optimization whose correctness and rollback path have not been
verified. An optimization is accepted only when its tests pass and measured
results justify the change.
