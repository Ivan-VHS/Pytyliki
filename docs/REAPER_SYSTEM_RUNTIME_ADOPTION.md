# Reaper System Runtime ABI adoption contract

**Consumer:** Pytyliki  
**Provider:** Reaper System Runtime ABI v1  
**Status:** integration contract; Pytyliki's existing C ABI remains authoritative.

## Scope

Pytyliki focuses on processor and ISA discovery, read-only hardware evidence, bounded measurements, memory utilities, metrics, QCS and its explicit native ABI. Reaper System Runtime ABI v1 adds a capability-gated host service boundary. It is optional and must not become a hidden dependency for existing C11 APIs or offline tooling.

The new Reaper ABI exposes 42 operation IDs in seven domains. Its operation catalogue is not proof that the host supports all services. Pytyliki must detect the ABI and each granted capability explicitly.

## Compatibility

- Preserve pytyliki.c-abi.v1 and every existing exported symbol and structure layout.
- Keep Pytyliki's own C11 ABI stable and versioned independently from REAPER_SYSTEM_ABI_VERSION.
- Do not expose Reaper internal AST/IR structures through Pytyliki's public ABI.
- Keep native interop opt-in and bound to an operator-selected, trusted library path.
- No automatic PATH, environment, current-directory or plugin-folder library discovery.

## Allowed usage

The initial integration profile is intentionally narrow:

| Capability | Initial policy | Rationale |
|---|---|---|
| CLOCK | Optional | Timestamping and bounded measurement metadata |
| CRYPTO | Optional | Host-provided hashing/entropy for explicit evidence workflows |
| FS_HANDLE / FS_READ | Optional | Read a caller-selected evidence file under a host-approved root |
| FS_WRITE | Deny by default | Do not mutate evidence or source material implicitly |
| PROCESS_CONTROL | Deny | Processor inspection does not need process launch/termination |
| THREAD_CONTROL | Deny by default | Existing worker model owns its own scheduling contract |
| SYNC | Deny by default | Existing worker model owns its own synchronization contract |
| NETWORK | Deny by default | Offline-first and reproducible evidence workflows |
| MMIO / DMA / IRQ | Not provided by this ABI | Must use a separate reviewed device adapter |

The capability table is a recommended default, not an unconditional grant. An application must operate correctly when the host provides no System Runtime capabilities.

## Domain boundaries

- CPU discovery and feature reporting remain evidence-driven and read-only.
- SIMD and accelerator dispatch must check the actual target feature set and a scalar fallback. No compiler feature flag is proof that the running processor supports an instruction.
- Embedded profile and FLASH/SRAM budget APIs remain metadata/evaluation interfaces; they do not map physical MCU memory or flash firmware.
- Generic memory helpers validate ranges and capacities; unsafe address access is a separate adapter.
- Reaper's operation payloads must be copied or consumed synchronously. No host pointer may be retained after dispatch returns.
- Errors must preserve the originating operation, status and request context. Never turn a denied capability into a false “unsupported CPU” result.

## Audio ownership

Pytyliki must not reintroduce its old DSP implementation. Audio processing and its migrated source compatibility module are owned by VentuSample under the existing migration contract. Reaper integration does not change that ownership.

## Verification gates

1. Add ABI-version and structure-size checks before use.
2. Add fake-adapter tests for granted, denied, missing and malformed capabilities.
3. Test maximum payload limits, null pointer/length pairs and host output-length violations.
4. Keep processor probes read-only and test that the adapter cannot trigger driver installation or firmware writes.
5. Add target-specific build coverage for x86-64 Intel/AMD as the primary path; preserve Elbrus as a distinct supported architecture backend with its own evidence.
6. Keep the existing Pytyliki C ABI tests and CTest suite independent of an installed Reaper shared library.

This document defines the integration contract. It does not claim that Pytyliki currently invokes the new System Runtime ABI or that all seven service domains are implemented.