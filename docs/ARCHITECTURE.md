# Architecture

| Layer | Responsibility |
|---|---|
| CLI / Win32 GUI | arguments, interaction, reports |
| C11 core | CPU probes, bounded tests, parser and metrics |
| PTFR | worker-fault cadence analysis and bounded recovery decisions |
| Platform adapters | WHEA event-log pull and optional Rust EDAC sysfs snapshots |
| Evidence ledger | event SHA-256, append-only chain, JSONL export |
| Language consumers | Reaper contract, x86-64 CPUID helper, Swift reader, TypeScript verifier |
| Affinity lease | reversible current-thread placement within the existing OS-allowed set |
| Safety layer | read-only hardware defaults, checked arithmetic and explicit privilege boundaries |
| TRACE | context-bound measurement records |
| Tests | unit, regression and platform checks |

Flow: provider → normalized event → evidence ledger → PTFR correlation/policy → structured report. Unknown source location is preserved as unknown, never inferred as a CPU or worker. Hardware discovery and event collection are read-only; affinity is an explicit opt-in operation scoped to the calling application thread.
