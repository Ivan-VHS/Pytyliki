# Feature matrix

| Area | Scope | Boundary |
|---|---|---|
| CPU | identity, topology, memory, ISA/XCR0 | read-only |
| Benchmarks | CPU, memory, matrices, partitioning | bounded local tests |
| Source review | static diagnostics | no compile/run/rewrite |
| Memory | typed views, checked slices, arenas, pools | fixed limits |
| Metrics/TRACE | norms, distances, provenance, variance | context-checked |
| Embedded C | MCU profiles, FLASH/SRAM, masked MMIO | simulation |
| Low-level hardware contract | target descriptor, bounded memory map, capability-gated RAM/MMIO backend, IRQ registry | fake backend only; no physical backend shipped |
| QCS | lexer, parser, AST | syntax-only |
| ABI | versioned C ABI v1 | explicit load |

Catalogue: 66 language capabilities, 14 processor profiles, 33 systems-lab functions. Counts are inventory, not certification.

The low-level contract is a reusable boundary for target adapters, not a universal hardware driver. Physical MMIO, DMA, cache maintenance, context switching and privilege changes require explicit target implementations.
