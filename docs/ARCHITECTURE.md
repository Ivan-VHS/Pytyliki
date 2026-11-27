# Architecture

| Layer | Responsibility |
|---|---|
| CLI / Win32 GUI | arguments, interaction, reports |
| C11 core | CPU probes, bounded tests, parser, processor-language registry and metrics |
| Safety layer | read-only defaults, checked arithmetic and safety boundaries |
| TRACE | context-bound measurement records |
| Language workbench | profile registry, bounded source scanner, target masks and inert toolchain plans |
| Tests | unit, regression and platform checks |

Flow: input → validation → bounded module → structured result → optional export. processor discovery and source inspection are read-only. GUI is Windows-only; portable core availability depends on platform APIs.
