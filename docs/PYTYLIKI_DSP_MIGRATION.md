# Pytyliki DSP ownership and migration boundary

**Status:** migration completed in the federated VentuSample repository  
**Former source:** Pytyliki C11 audio-processing modules  
**Current owner:** VentuSample  
**Scope:** repository ownership and interface traceability; this page does not provide a duplicate DSP implementation.

## Ownership decision

Pytyliki now focuses on processor/ISA discovery, read-only hardware evidence, bounded measurements, memory utilities, metrics, QCS and its explicit native ABI. Its current feature matrix does not claim an audio-device host, real-time audio engine or retained audio-processing subsystem.

The portable C11 audio-processing implementation was transferred to VentuSample. The migrated code is owned and built there under `src/compat/pytyliki-dsp/`, with the C11 API and tests compiled into the VentuSample build. This avoids two independently maintained implementations of the same audio algorithms.

## Transferred scope

The VentuSample migration record identifies these source groups:

| Former module | Current responsibility |
|---|---|
| `dsp.c` / `dsp.h` | bounded PCM16 WAV import/export, tone generation, gain, filtering, compression, saturation and measurements |
| `dsp_engine.c` / `dsp_engine.h` | scalar block processing, biquad/FIR filters, limiter and block statistics |

The original boundary and engine tests moved with the implementation and are registered in VentuSample CTest. Pytyliki does not maintain an unbuilt duplicate or expose the migrated code as a processor-discovery feature.

## Integration contract

- **Ownership:** VentuSample owns audio generation and offline DSP; Pytyliki owns processor/system discovery, measured evidence and generic compute/metrics.
- **Execution:** migrated audio code remains offline and buffer-based. This migration does not create real-time audio callbacks, device access, plugin hosting or network processing.
- **Interface:** cross-project reuse is through explicit versioned interfaces and manifests. Do not link private source directories or silently copy the same C implementation into both repositories.
- **Build:** VentuSample compiles the imported module as C11 alongside its C++20 engine; its native tests run in VentuSample CTest.
- **Diagnostics:** audio-specific VNT error IDs and pack-manifest audit are owned and documented in VentuSample.

## Verification and source of truth

The implementation, build graph, tests and detailed migration record are maintained in:

- [VentuSample DSP migration contract](https://github.com/Ivan-VHS/VentuSample/blob/main/docs/PYTYLIKI_DSP_MIGRATION.md)
- [VentuSample source compatibility module](https://github.com/Ivan-VHS/VentuSample/tree/main/src/compat/pytyliki-dsp)
- [Pytyliki feature matrix](FEATURES.md)
- [Pytyliki federation contract](../federation/octa-pyramid.v1.json)

A passing evidence-path check verifies that the ownership record is present; it does not independently certify audio fidelity or real-time behavior. DSP correctness remains governed by VentuSample's CTest and numerical regression evidence.

## Maintenance rule

Changes to the migrated audio implementation are made in VentuSample only. Pytyliki may update this boundary record and the federation contract when a public interface or ownership decision changes, but must not reintroduce a second implementation.
