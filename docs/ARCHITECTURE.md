# Architecture and safety contract

## Layers

1. **Core (core.c)** — portable CPU identity/topology and OS memory query, bounded integer benchmark, read-only heuristic source review, and JSON output.
2. **Processor (processor.c)** — architecture classification, selected ISA capability discovery, and three bounded workloads (integer dependency chain, memory traversal, compact matrix arithmetic).
3. **DSP (dsp.c)** — bounded in-memory audio buffer, PCM16 WAV import/export, tone generation, gain, one-pole low-pass, static-ratio compression, soft saturation, and signal metering.
4. **Presentation** — native Win32 GUI on Windows (editable DSP controls, file dialogs, session log and JSON report export) and a cross-platform CLI.

## Data flow

- The GUI/CLI requests a processor snapshot; the core reads OS topology and CPUID where supported.
- Benchmark operations run fixed workloads with hard upper bounds. The processor suite reports wall-clock duration and an integrity checksum.
- Source review opens only the selected source file in binary-read mode, scans text lines, and records heuristic findings.
- WAV input is checked for RIFF/WAVE, PCM format, 16-bit samples, supported channel count, sample-rate range, alignment and bounded payload size before allocation.
- DSP operators modify only the in-memory float buffer. WAV export clamps samples to the PCM16 output range.

## Concept translation

Medical terms from the source material are intentionally anonymized into engineering vocabulary: etiology becomes root-cause analysis; strain/phenotype becomes CPU family and ISA profile; resistance becomes fault tolerance and validation; remission/recurrence becomes stable operation/regression; pharmacokinetics becomes data-path and resource-flow analysis; auditory modulation becomes the DSP signal chain; prognosis becomes repeatability and regression monitoring.

## Safety boundaries

- No privileged elevation, kernel driver, BIOS/UEFI, firmware, microcode, voltage, frequency, or power-limit writes.
- CPUID capability bits are reported as hardware facts only. The implementation does not dispatch AVX/AVX-512 instructions.
- The processor suite is capped at 100 million integer iterations, 64 MiB memory, and 32 memory passes.
- Audio duration is capped at 30 seconds for tone generation; the in-memory frame ceiling is 4.8 million; file import is capped at 32 MiB.
- Only PCM16 mono/stereo WAV at 8–192 kHz is accepted. Compressed WAV variants and other bit depths are rejected.
- The static scanner is heuristic and does not parse or execute code. Findings are review prompts, not vulnerability verdicts.
- No automatic source rewriting, remote code execution, code downloading, device capture, or background recording is implemented.

## Known limits

- AVX flags do not inspect OSXSAVE/XCR0 vector-state readiness and must not be treated as permission to execute AVX instructions.
- Benchmark numbers are sensitive to the OS scheduler, power policy, thermal state, and competing workloads.
- The current audio chain is a practical PCM16 lab, not a real-time audio engine: there is no ASIO/WASAPI capture, plugin host, multiband processing, or hardware output.
