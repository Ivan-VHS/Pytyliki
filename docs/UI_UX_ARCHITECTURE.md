# Pytyliki UI/UX and Language-Environment Architecture

**Status:** target architecture and implementation contract. This document does not claim that the full theme engine, adaptive 8K workspace or localization layer is already implemented.

## 1. Product fit

Pytyliki is a native C11 processor-systems workstation with a Windows Win32 desktop front end and portable command-line core. Its UI serves processor discovery, bounded benchmarks, source review, local DSP, metrics, Embedded C reference profiles and inspectable reports.

The current Win32 shell has a five-action top row, audio-action grid, parameter fields and a read-only session log. The goal is not to turn it into a generic IDE: make specialist workbenches discoverable and responsive while keeping C11 kernels independent of presentation code.

## 2. Six Anatidae articles as UI invariants

| Article | Invariant | Pytyliki application |
|---|---|---|
| I. Water-repellent plumage | Containment; no visual/state leakage | Each workbench owns controls, validation, status and result view. Errors remain beside the action that caused them. |
| II. Calm surface, strong underwater motion | Quiet surface; bounded work underneath | Benchmarks and DSP must not block the message loop. Progress, cancellation where supported, limits and completion are explicit. |
| III. Rudder and balance | Stable orientation | Active workbench, profile, input file and report destination remain visible through resizing. |
| IV. Webbed feet | Fluid layout | Controls reflow from compact to multi-column layouts without assumptions about one monitor. |
| V. Precise bill | Precise input and diagnostics | Numeric fields validate units, ranges and finite values before calling C APIs. Reports distinguish observation, heuristic and failure. |
| VI. Flock and wake | Coordinated modules and rights | UI calls public C APIs; it does not duplicate numerical kernels or bypass ABI, ownership or licensing boundaries. |

The existing safety posture remains mandatory: inventory is read-only, benchmarks are bounded, source review does not execute inspected source, and DSP processes local buffers.

## 3. Adaptive workstation

Breakpoints are based on the logical client area after OS DPI scaling, not raw panel resolution. Support 7680×4320 at 100–300% OS scaling.

- **Compact (≤1280 logical px):** one active workbench; persistent command strip; DSP actions and parameters in a collapsible group; session log can collapse without losing contents.
- **Standard (1281–2560):** three zones—workbench navigation, active workbench, result/diagnostic inspector. Profile, benchmark and DSP results use structured fields instead of log text alone.
- **Wide / 4K (2561–3839):** side-by-side profile, benchmark and metric summaries; source-review findings and Embedded C tables as optional inspectors.
- **8K (≥3840 logical px):** navigation, active workbench, metric/architecture inspector and trace are visible together. Use space for comparison and provenance, not oversized controls. Keep text measure near 68–96 characters.

## 4. Tokenized visual system

RGB values must not be scattered through control creation and paint handlers. All controls consume semantic tokens.

| Token | Meaning |
|---|---|
| `bg-pond`, `bg-surface`, `bg-elevated` | workstation, workbench and dialog surfaces |
| `fg-primary`, `fg-secondary`, `fg-muted` | primary text, explanations and metadata |
| `accent-primary`, `accent-secondary`, `accent-warm` | main action, linked metrics and active DSP/exploration |
| `status-ok`, `status-warn`, `status-danger` | verified, advisory and failed states |

Do not expose alternate accent presets. The production identity uses the single white-blue-red semantic token set in [ANATIDAE_DESKTOP_DESIGN_SYSTEM.md](ANATIDAE_DESKTOP_DESIGN_SYSTEM.md), with light/dark surfaces and normal/high/extra-high contrast modes. Contrast and text/icon status labels remain mandatory; theme variants may change luminance, not the product's semantic color roles.

Theme changes update controls, focus rings, tables, legends and log selection atomically. Include `uiScale` 80–250%, typography and Compact/Balanced/Spacious density. On 8K, separators/focus outlines scale to about 1.5–3 physical pixels while text retains readable logical size.

## 5. 20×20 meaning matrix

The concepts form a navigable interaction vocabulary, not 400 independent UI features:

- E1–E4: language composition, adoption, code representation and algorithms.
- E5–E9: machine model, formal constraints, developer tools, compute and system operations.
- E10–E13: command/document catalogue, CLI, syntax and semantics.
- E14–E16: declarative configuration, imperative flow and native procedures.
- E17–E19: object ownership, module author rights and third-party rights.
- E20: responsive live feedback and exploratory workflow.

Each directed pair `Ei ⊗ Ej` is a traceable design relation. Use all 400 projections to tag catalogue entries, reports and future workbench transitions. A matrix entry is a specification projection, not proof of a compiler, backend or hardware feature. The canonical detailed matrix is [Reaper MOST 400 matrix](https://github.com/Ivan-VHS/Reaper/blob/main/docs/MOST_400_COMBINATORIAL_MATRIX.md).

## 6. Language and content environment

- Keep C11 API identifiers, ABI structures and serialized report keys stable and locale-neutral.
- Localize labels, help, validation messages and documentation independently from C symbols and JSON keys.
- Target locales: English, Ukrainian, Russian and Simplified Chinese. Use UTF-8 resources; keep user-facing strings out of numerical kernels.
- The 66-entry systems-language catalogue is descriptive metadata. Label entries as detected, inspectable, partially supported or executable; catalogue presence is not compiler implementation.
- Source-review findings are heuristics, not confirmed defects.

## 7. Interaction, accessibility and performance

- Keyboard-first operation, visible focus, logical tab order, accelerators and accessible names.
- Interactive targets at least 40 logical px high; 44 preferred.
- Motion 120–220 ms where it clarifies state; honor reduced-motion preferences.
- Heavy work must not run inside the Win32 window procedure. Deliver bounded progress/results through a safe message handoff; keep the C core independently testable.
- No layout clipping at supported scale; never shrink text merely to fit a wide table.

## 8. Implementation sequence and acceptance gates

1. Extract semantic colors, typography, spacing and density into one theme table.
2. Add DPI-aware layout profiles and resize tests for compact, standard, 4K and 8K logical widths.
3. Separate workbench views from C11 operation handlers; keep the core free of Win32 dependencies.
4. Add locale resources and translation-completeness checks.
5. Add structured result panels and matrix tags for the engineering catalogue.

Acceptance: no clipped controls; visible keyboard focus; atomic theme changes; report provenance and safety wording preserved; benchmark/DSP does not freeze the shell; C11 tests and desktop CI pass.

## 9. Related architecture

- [Core architecture](ARCHITECTURE.md) · [Unified engineering standard](UNIFIED_ENGINEERING_STANDARD.md)
- [Federated integration](FEDERATED_INTEGRATION.md)
- [Reaper language and MOST matrix](https://github.com/Ivan-VHS/Reaper)


## White-blue-red Anatidae design authority

The product palette and platform implementation rules are standardized in [ANATIDAE_DESKTOP_DESIGN_SYSTEM.md](ANATIDAE_DESKTOP_DESIGN_SYSTEM.md). That document supersedes earlier exploratory color-preset examples in this architecture. The production UI uses semantic white, blue and red tokens with separate light/dark surfaces; unrelated accent palettes are not part of the product identity.
