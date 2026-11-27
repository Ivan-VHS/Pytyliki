# Architecture

| Layer | Responsibility |
|---|---|
| CLI / Win32 GUI | arguments, interaction, reports |
| C11 core | CPU probes, bounded tests, parser and metrics |
| Safety layer | read-only defaults, checked arithmetic and safety boundaries |
| TRACE | context-bound measurement records |
| Tests | unit, regression and platform checks |

Flow: input → validation → bounded module → structured result → optional export. processor discovery and source inspection are read-only. GUI is Windows-only; portable core availability depends on platform APIs.


## MOS switch-level model

The C11 MOS model is a bounded leaf subsystem owned by Pytyliki. It represents ideal N-channel and P-channel switches and resolves connectivity against explicit VDD/VSS rails. It is deterministic and allocation-free within fixed node/device limits.

This subsystem is below Boolean-gate descriptions, but it is not an analog circuit solver. It does not calculate current, continuous voltage, capacitance, delay, leakage, temperature or transistor process effects. It never accesses physical hardware. See [MOS switch-level model v1](PHYSICAL_SWITCH_MODEL_V1.md).
