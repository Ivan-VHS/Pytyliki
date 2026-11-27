# Architecture

| Layer | Responsibility |
|---|---|
| CLI / Win32 GUI | arguments, interaction, reports |
| C11 core | CPU probes, bounded tests, parser and metrics |
| Safety layer | read-only defaults, checked arithmetic and safety boundaries |
| PTFR | bounded fault-event cadence analysis and recovery decisions | caller-owned state; no OS scheduling or hardware writes |
| TRACE | context-bound measurement records |
| Tests | unit, regression and platform checks |

Flow: input → validation → bounded module → structured result → optional export. processor discovery and source inspection are read-only. GUI is Windows-only; portable core availability depends on platform APIs.
