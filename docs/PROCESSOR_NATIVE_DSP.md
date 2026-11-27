# Processor-Native DSP Architecture

## Scope
This extension adds a portable C11 streaming reference engine alongside the existing bounded PCM16 WAV utilities. It does not modify CPU microcode, firmware, registers, clocking, voltage, or silicon.

## Processing model
- Control plane validates sample rate, block limits and coefficients before audio processing starts.
- Data plane processes caller-owned float buffers. Processing calls allocate no memory, acquire no locks, perform no file I/O and make no system calls.
- Portable scalar C11 is the numerical reference for future architecture-specific kernels.
- Filter and limiter state is persistent and must be separate per channel/stream.
- WAV import/export remains PCM16; the streaming engine accepts float spans.

## Kernels
- RBJ low-pass biquad in transposed direct-form II.
- FIR convolution with caller-provided coefficients and circular history.
- Envelope limiter with configurable attack/release smoothing.
- Block peak/RMS/DC measurement and non-finite sample counting.

## Processor integration roadmap
1. Portable scalar baseline and bounded tests.
2. Runtime dispatch only after hardware ISA and OS-enabled vector-state checks.
3. SIMD kernels validated against scalar output, including tails, alignment, aliasing, denormals, NaN/Inf and saturation.
4. Optional aligned scratch buffers outside the callback; DMA only in a hardware-specific backend.
5. A DSP coprocessor requires a versioned command queue, bounded descriptors, completion fences, timeout/recovery and scalar fallback.
6. RTL/FPGA/ASIC requires fixed-point specification, rounding/saturation rules, pipeline latency, reset semantics, clock-domain crossing design, bus protocol and formal verification.

## Real-time rules
Allocate buffers and initialize DSP state before starting the audio callback. Do not call WAV functions, allocation, logging or device discovery inside the callback. Measure end-to-end latency and worst-case execution time on every target before claiming real-time guarantees.

## Acceptance gates
Future accelerated backends must pass scalar-reference differential tests, bounded callback-time tests and per-target performance measurements. Raw CPU feature flags are not sufficient evidence for safe SIMD dispatch.
