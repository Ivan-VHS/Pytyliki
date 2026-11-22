# Repeated benchmark statistics

Pytyliki now exposes a small C11 statistics API for timing samples collected by a caller. It complements the bounded CPU suite; it does not start a workload, alter scheduling, or make a universal speed claim.

## API

`pl_benchmark_stats_analyze(samples, count, &result)` accepts 1–100,000 finite, non-negative sample values. The input array is not modified. Invalid values, an invalid count, or allocation failure return `-1`.

The result contains minimum, maximum, arithmetic mean, median, nearest-rank p90/p95/p99, sample standard deviation, coefficient of variation and median absolute deviation (MAD). Percentiles use the nearest-rank definition: rank = ceil(p × N), one-based.

## CLI

```sh
pytyliki-benchstats 1.8,1.7,1.9,1.8,2.4,1.8
```

The CLI accepts at most 256 comma-separated millisecond samples and emits `pytyliki.benchmark-statistics.v1` JSON. It summarizes existing data only; it does not execute or repeat the benchmark. Keep the original machine, workload, compiler, warm-up and measurement context alongside exported results.

## Catalog links

This first implementation wave adopts the statistical reporting patterns of Google Benchmark, Criterion, hyperfine and pyperf (catalog items 73–76) without importing their code or dependencies. The API is also useful for comparing profiler and trace outputs. It is descriptive instrumentation, not processor certification.
