# Embedded C workbench

| Tool | Result |
|---|---|
| MCU profile | reference CPU/ISA and memory map |
| Memory budget | FLASH/SRAM usage and remaining capacity |
| MMIO model | masked register simulation |
| Silicon runtime | opt-in C11 HAL for barriers, counters, exact MMIO allowlists, guarded states, faults and bounded machine-stream inspection |
| Starter generator | hardware-neutral project skeleton and build hints |

The profile catalogue and original register bank are planning/simulation facilities. The silicon runtime adds explicit architecture backends, but physical access remains disabled unless the firmware build and board-specific HAL opt in. Reference addresses must be verified against the exact MCU datasheet. No physical register, firmware, bootloader or device is written by the workstation CLI.

- [Reaper-style silicon runtime (English)](EMBEDDED_SILICON.md)
- [Руководство по реальному кремнию (русский)](EMBEDDED_SILICON_RU.md)
