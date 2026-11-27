# Architecture

| Layer | Responsibility |
|---|---|
| CLI / Win32 GUI | arguments, interaction, reports |
| C11 core | CPU probes, bounded tests, parser and metrics |
| Safety layer | read-only defaults, checked arithmetic and safety boundaries |
| PTFR | bounded fault-event cadence analysis and recovery decisions | caller-owned state; no kernel or hardware writes |
| RAS adapters | WHEA/ETW and EDAC ingestion plus evidence checkpointing | operating-system published data only |
| Placement adapter | current application-thread placement with restore snapshot | explicit opt-in; no processor hotplug |
| TRACE | context-bound measurement records |
| Tests | unit, regression and platform checks |

Flow: input → validation → bounded module → structured result → optional export. processor discovery and source inspection are read-only. GUI is Windows-only; portable core availability depends on platform APIs.
