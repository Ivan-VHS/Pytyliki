# Feature matrix

| Area | Scope | Boundary |
|---|---|---|
| CPU | identity, topology, memory, ISA/XCR0 | read-only |
| Benchmarks | CPU, memory, matrices, partitioning | bounded local tests |
| Source review | static diagnostics | no compile/run/rewrite |
| Memory | typed views, checked slices, arenas, pools | fixed limits |
| Metrics/TRACE | norms, distances, provenance, variance | context-checked |
| Embedded C | MCU profiles, FLASH/SRAM, masked MMIO | simulation |
| QCS | lexer, parser, AST | syntax-only |
| Processor languages | 66 unique assembly, systems, VM/IR and HDL profiles; detect, bounded inspection, target checks and argv planning | static; no tool execution |
| ABI | versioned C ABI v1 | explicit load |

Catalogues: 66 unique processor-language profiles, 14 processor profiles and 33 systems-lab functions. Counts are inventory, not compiler coverage or certification.
