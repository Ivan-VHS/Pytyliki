# UI/UX system

**Maintainer:** Ivan-VHS · **Project:** Pytyliki

| Area | Contract |
|---|---|
| Product | processor workstation; task-first, not marketing dashboard |
| Layout | distinct CPU, tests, source, DSP and Embedded C groups |
| Theme | light/dark semantic tokens; visible focus |
| Surface | opaque work areas for logs and numeric output |
| Input | keyboard-native controls, explicit action labels |
| Platform | native Win32 GUI; portable C11 core/CLI |

No color-only status signals. GUI claims remain Windows-only unless a separate platform adapter is implemented and tested.

## UI/UX 2026 convergence

See [UI_UX_2026_CONVERGENCE.md](UI_UX_2026_CONVERGENCE.md) for the current shared semantic tokens, accessibility, responsive behavior, high-DPI requirements and motion budgets. Native Win32 and product-specific constraints above remain authoritative.

## Motion contract

See [UI_MOTION.md](UI_MOTION.md) for the motion implementation contract. Reduced Motion and Eco remain authoritative; motion must not enter processor benchmarks, C11 workers, FFI or DSP execution.

