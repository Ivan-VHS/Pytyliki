# UI/UX system

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

See [UI_UX_2026_CONVERGENCE.md](UI_UX_2026_CONVERGENCE.md) for shared semantic tokens, accessibility, responsive behavior and motion budgets. Native Win32 and product-specific constraints above remain authoritative.

## UI/UX Codex 40026

See [UI/UX Codex 40026](UI_UX_CODEX_40026.md) for the six-cycle / 666-rule engineering contract. The Win32 workstation now has a UI-only text contrast policy with a 4.5:1 ordinary-text and 3:1 large-text threshold, a preferred 44 px action target, contrast-safe text fallback, and Escape/keyboard-containment support in Motion Settings. The helper is deliberately not linked into the DSP/FFI/processing core.
