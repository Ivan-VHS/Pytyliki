# Feature matrix

| Area | Scope | Boundary |
|---|---|---|
| CPU | identity, topology, memory, ISA/XCR0 | read-only |
| Benchmarks | CPU, memory, matrices, partitioning | bounded local tests |
| Source review | static diagnostics | no compile/run/rewrite |
| DSP | PCM16 WAV, filter, compression, saturation | local buffers |
| Memory | typed views, checked slices, arenas, pools | fixed limits |
| Metrics/TRACE | norms, distances, provenance, variance | context-checked |
| Embedded C | MCU profiles, FLASH/SRAM, masked MMIO | simulation |
| QCS | lexer, parser, AST | syntax-only |
| ABI | versioned C ABI v1 | explicit load |

Catalogue: 66 language capabilities, 14 processor profiles, 33 systems-lab functions. Counts are inventory, not certification.
