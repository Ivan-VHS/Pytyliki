# Unified Engineering Standard

**Profile:** ISO C11 processor, memory, metric and DSP workstation.

This standard turns the shared quality vocabulary into rules that can be implemented, tested and reviewed. It guides new work and consolidation; it does not claim every proposed capability is already implemented.

## Product boundary

Pytyliki owns processor-oriented numerical primitives, bounded workloads, memory/DSP kernels, hardware-capability interpretation and the stable Pytyliki C ABI. It remains independently buildable. OPlague owns device inventory and driver-package workflows; Reaper owns language parsing, execution and orchestration.

## Twenty quality groups, expressed as engineering rules

| Quality vocabulary | Enforceable meaning |
|---|---|
| Unified · Consolidated · Centralized | Keep one authoritative implementation for each primitive and one canonical contract for each exported interface. |
| Unique · Distinctive · Original | Preserve Pytyliki's processor-workstation purpose; do not duplicate Reaper's language runtime or OPlague's driver manager. |
| Universal · Cross-platform · Compatible | Keep portable C11 logic separate from OS-specific discovery and presentation adapters. Report unsupported capabilities explicitly. |
| Standardized · Normalized · Systematized | Use fixed-width public types, explicit units, stable status codes, documented limits and deterministic report fields. |
| Versatile · Multifunctional · Multipurpose | Expose the same safe core operations through CLI, GUI and opt-in C ABI instead of reimplementing behavior in each frontend. |
| Integrated · Interoperable · Interconnected | Exchange data through versioned JSON or the documented C ABI; never expose internal C structs as a cross-project wire format. |
| Harmonized · Synchronized · Aligned | Keep public headers, documentation, tests and ABI declarations in sync; treat ABI changes as explicit version changes. |
| Adaptive · Flexible · Modular | Add backends behind narrow interfaces with scalar/portable fallbacks and explicit capability checks. |
| Extensible · Scalable · Bounded | Bound dimensions, iteration counts, memory, passes and payload sizes; reject overflow before allocation or execution. |
| Comprehensive · Holistic · Multidisciplinary | Cover numerical correctness, architecture support, memory behavior, diagnostics, reproducible builds and evidence. |
| Exceptional · Unparalleled · Evidence-led | Promote claims only when supported by tests, CI, measurements or reproducible external evidence. Heuristic scores are not benchmarks. |
| Singular · Coherent · Consistent | Keep one error model and consistent validation semantics across related C APIs. |
| Exclusive · Explicit · Opt-in | Native loading, FFI consumers and non-default execution paths require deliberate configuration and a trusted target. |
| Coordinated · Combined · Composable | Compose small tested primitives into larger workflows; avoid hidden global state and implicit cross-module mutation. |
| Cross-language · Stable · Interoperable | Preserve the documented Pytyliki ABI v1 contract until a separately versioned ABI is introduced. |
| All-purpose · Generalized · Hybrid | Generalize numerical primitives only where semantics remain precise; keep processor policy in processor-facing modules. |
| Remarkable · Auditable · Reproducible | Make behavior inspectable through source, tests, clear diagnostics and traceable build results. |
| Rare · One-of-a-kind · Purposeful | Prefer differentiated useful capabilities over duplicate feature catalogues. |
| Unmatched · Unrivaled · Reliable | Fail closed on invalid dimensions, non-finite inputs, unsupported ISA state and unrepresentable results. |
| Integrated · Maintainable · Independent | Keep OPlague/Reaper adapters optional; standalone builds and tests must work without sibling repositories. |

## Existing integration contracts

- **Pytyliki C ABI v1:** public interface in `src/interop.h`; consumers verify `pytyliki_abi_version() == 1` and pass bounded contiguous vectors.
- **OPlague:** may use the ABI only through an explicit absolute path to a trusted shared library. The library is not auto-discovered or loaded.
- **Reaper:** consumes versioned OPlague snapshots and may use the same explicit ABI bridge; project-level contracts remain independent of private C layouts.

## Required review gates for new modules

1. Define ownership, input/output types, units, limits, failure behavior and supported platforms.
2. Validate dimensions and arithmetic before allocation; reject non-finite values where numeric semantics require finite inputs.
3. Keep hardware access read-only or simulated unless a separately reviewed, vendor-documented and explicitly approved interface exists.
4. Add deterministic tests for normal, boundary, malformed and unsupported cases.
5. Preserve portable fallback behavior; ISA flags alone do not prove that the OS has enabled the required vector state.
6. Update the public header, architecture documentation and user-facing capability claims together.
7. Verify the CMake/CTest Windows and Linux matrix and configured sanitizer job before calling a change validated.

## Non-goals and truth rules

The standard does not authorize firmware, BIOS/UEFI, microcode, voltage, clock, power-limit, kernel or device-register writes. It does not turn advisory plans into hardware certification or promise speedups. A proposal is **planned** until code, public contract, tests and CI evidence exist.

## Related contracts

- [Architecture and safety contract](ARCHITECTURE.md)
- [Federated integration](FEDERATED_INTEGRATION.md)
- [C ABI header](../src/interop.h)
- [Memory contract](MEMORY_ARENA.md)
