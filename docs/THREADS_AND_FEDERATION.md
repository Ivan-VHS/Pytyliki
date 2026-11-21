# Pytyliki — project-thread integration ledger

This ledger consolidates processor systems, native DSP, memory arena, metric engine, systems atlas, consent, TRACE and desktop-design requirements from prior project threads. It records what is backed by current source.

## Implemented and source-backed

| Requirement | Current implementation | Evidence |
|---|---|---|
| Processor discovery | Read-only CPU identity, memory, ISA and OS-managed vector-state checks | `src/processor.c`, `src/architecture_atlas.c` |
| Bounded benchmark suite | Processor, memory and matrix workloads with bounds and integrity checks | `src/cpu_suite.c`, `src/cli.c` |
| Native DSP | PCM16 WAV I/O, tone generation, filtering, gain, dynamics, saturation and metering | `src/dsp.c`, `src/dsp_engine.c` |
| Memory kernel | Typed views, checked slices, bounded arenas, rewind checkpoints, pools and telemetry | `src/memory.c`, `docs/MEMORY_ARENA.md` |
| Metric Lab | C11 vector, distance, geometry, variance and weighted metric operations | `src/metric_geometry.c`, `src/metric_registry.c` |
| TRACE provenance | Context, unit, source digest, measurement window, sample count and declared uncertainty | `src/trace.c`, `docs/TRACE_MEASUREMENT_PROVENANCE.md` |
| Embedded C / QCS | MCU reference profiles, budget checks, simulated MMIO masks and syntax validation | `src/embedded.c`, `src/qcs.c` |
| Systems atlas / self-product | Versioned language and architecture catalogues and bounded matrix composition | `src/architecture_atlas.c`, `src/self_product.c` |
| Native GUI | Windows Win32 workstation with read-only controls and light/dark theme | `src/win_gui.c` |

## Federation

Pytyliki provides ABI v1 metrics to Reaper and OPlague through explicit caller-selected native library paths. Its VentuSample connection remains planned; no audio-generator adapter is shipped. See [OCTA-8](OCTA_PYRAMID_FEDERATION.md).

## Deliberate boundaries

- CPU discovery is read-only. Benchmarks are bounded, machine-dependent measurements, not hardware certification.
- Source diagnostics do not compile, execute or rewrite inspected source.
- Hardware catalogues, APSIA routes and scores are advisory metadata.
- Embedded MMIO is simulated/reference data; no physical firmware or peripheral writes occur.
- ABI loading is explicit trusted-library loading.
- Realtime audio-device playback and DAW plugin hosting are not claimed by the current offline DSP API.

## Repositories

[Reaper](https://github.com/Ivan-VHS/Reaper) · [Pytyliki](https://github.com/Ivan-VHS/Pytyliki) · [OPlague](https://github.com/Ivan-VHS/OPlague) · [VentuSample](https://github.com/Ivan-VHS/VentuSample)
