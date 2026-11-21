# Division and Work Partitioning Engine

## Purpose

Pytyliki provides checked integer division, exact-divisibility queries,
balanced partition planning, a bounded worker-count recommendation, and exact
integer apportionment. These primitives can be used by source analysis,
processor workload planning, local report generation, and other CPU-bound
workloads without coupling their correctness to audio or hardware discovery.

## Arithmetic contract

For signed 64-bit inputs, Euclidean division returns quotient q and remainder
r that satisfy:

    dividend = quotient * divisor + remainder
    0 <= remainder < abs(divisor)

Zero divisors and the signed overflow case INT64_MIN / -1 are rejected.
pl_division_is_exact reports whether the Euclidean remainder is zero.

## Partition planning

pl_partition_plan_create creates contiguous, disjoint ranges. Their union
covers the entire input range, each item appears once, and range lengths differ
by at most one. The earliest ranges receive any extra items.

pl_division_recommend_workers produces a bounded advisory recommendation
from work-item count, available logical workers, and a minimum work grain.
--partition-auto reads the logical processor count and prints the resulting
plan. This is a planner only: it does not start threads, change affinity, or
modify hardware.

CLI examples:

    pytyliki --divide -17 5
    pytyliki --partition 10000 8 256
    pytyliki --partition-auto 10000 256

## Proportional allocation

pl_division_weighted_split implements deterministic largest-remainder
apportionment. It preserves the requested integer total exactly, uses input
order to break equal-remainder ties, rejects empty or all-zero weights, and
bounds intermediate values to avoid overflow.

## Boundaries

These operations are pure calculations and planning. They do not access files,
execute source, install software, change processor settings, or claim an
unmeasured performance improvement. Downstream parallel execution must remain
separately bounded, observable, and explicitly validated.

## Validation

CTest covers positive and negative operand combinations, exactness, zero
division, signed overflow rejection, partition coverage, empty partitions,
worker recommendation limits, and weighted-allocation conservation. CLI
smoke tests cover division and partition output.
