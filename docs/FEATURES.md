# Feature matrix

| Area | Scope | Boundary |
|---|---|---|
| CPU | identity, topology, memory, ISA/XCR0 | read-only |
| PTFR | periodic fault correlation, CSV import, bounded recovery plans | advisory only; no host CPU control |\n| RAS provider | Windows WHEA Event Log snapshot | read-only; source metadata only; no worker attribution or severity guess |
| Benchmarks | CPU, memory, matrices, partitioning | bounded local tests |
| Source review | static diagnostics | no compile/run/rewrite |
| Memory | typed views, checked slices, arenas, pools | fixed limits |
| Metrics/TRACE | norms, distances, provenance, variance | context-checked |
| Embedded C | MCU profiles, FLASH/SRAM, masked MMIO | simulation |
| QCS | lexer, parser, AST | syntax-only |
| ABI | versioned C ABI v1 | explicit load |

Catalogue: 66 language capabilities, 14 processor profiles, 33 systems-lab functions. Counts are inventory, not certification.
