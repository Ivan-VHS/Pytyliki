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

## 40026 Codex

The normative six-cycle / 666-rule contract is maintained in [UI/UX Codex 40026](UI_UX_CODEX_40026.md). A header-only C11 UI policy validates WCAG text contrast and preferred 44 px targets; the Win32 workstation applies a contrast-safe text fallback and the Motion Settings dialog supports Escape and larger primary controls. The helper remains UI-only and is not part of the DSP, FFI or processing core.

