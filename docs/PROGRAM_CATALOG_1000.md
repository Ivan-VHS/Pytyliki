# 1,000-program adoption map: Pytyliki

This file assigns the Pytyliki portion of the supplied 1,000-item catalogue to a realistic implementation boundary. It is a feature-adoption map, not a claim that 250 external products have been cloned.

## Ownership within the catalogue

| Catalogue IDs | Original subject area | Pytyliki treatment |
|---|---|---|
| 1–60 | CPU, motherboard, memory, thermal and host inventory | Native/local read-only profile sources and evidence-qualified telemetry |
| 61–120 | Benchmarks and profilers | Bounded benchmark workloads, repeatability, statistical summaries and imported profiler evidence |
| 121–170 | DSP and audio analysis | **Transferred to VentuSample.** No audio-processing source, CLI, or GUI belongs here |
| 171–220 | Static analysis, formal checks, fuzzing and coverage | QCS/static review, source-quality contracts, bounded parser fuzzing and CI reports |
| 221–250 | Embedded toolchains and quantum simulation | MCU reference profiles, memory estimates, simulation and conformance fixtures |

## First implementation wave

- **Benchmark Statistics** — src/benchmark_stats.c/.h, pytyliki-benchstats, and CTest. The function analyzes caller-supplied timing samples using nearest-rank p90/p95/p99, sample deviation, coefficient of variation and MAD.
- The existing CPU inventory, bounded suite, TRACE provenance and source-analysis modules remain the shared base. The new statistics tool does not run workloads or claim universal benchmark scores.
- **Profile Import** — `tools/profile_import.py` reads collapsed stacks and Speedscope sampled profiles, aggregates frame weights, and exports folded stacks or an escaped SVG flamegraph. It is bounded and never launches a profiler.
- **Sensor Snapshot** — `tools/sensor_snapshot.py` reads available Linux sysfs/procfs CPU-frequency, governor, thermal and energy counters with source paths and quality labels. Missing sensors remain missing; unsupported platforms are reported explicitly.

## Adoption rules

1. Reimplement ideas through native C11 APIs and local OS adapters; do not copy upstream implementation code.
2. CPU, cache, NUMA, thermal and energy values must carry source and quality labels. Unknown and unsupported sensors stay unknown; do not synthesize telemetry.
3. Benchmark runs require explicit bounds, warm-up/repeat metadata and reproducible result context. Comparisons across different machines or measurement windows must not be presented as equivalent.
4. QCS stays a static analysis path unless a separately reviewed, sandboxed execution engine is introduced.
5. Embedded MMIO and device-control ideas remain reference or dry-run features. No register, frequency, voltage or firmware writes.
6. Audio and media processing belong only in VentuSample.

## Remaining backlog themes

The 1–60 cluster prioritizes CPUID evidence, NUMA/cache topology, PDH/ETW or Linux counter adapters, RAPL/energy provenance, and SMART/NVMe inventory. The 61–120 cluster prioritizes statistical suites, flamegraph/profile import, memory/heap evidence, and benchmark archive comparisons. The 171–220 cluster prioritizes fuzz harnesses for QCS parsers, mutation testing, coverage reports, CodeQL/Semgrep rule packs and dependency audit summaries. The 221–250 cluster prioritizes target metadata, toolchain/map-file readers and deterministic simulator fixtures.

Each item stays **proposed** until its source adapter, limits, tests and evidence semantics are implemented.
