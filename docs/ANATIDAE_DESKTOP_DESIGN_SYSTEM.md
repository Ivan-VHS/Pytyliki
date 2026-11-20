# Anatidae Desktop Design System — Pytyliki

**Status:** implemented baseline for the native Win32 workstation; physical-monitor validation remains a release task. This specification is the authoritative white-blue-red UI contract for Pytyliki.

## 1. Design intent

The interface is calm, tactile and technical: large clear work areas, rounded controls, predictable focus, and concise feedback. The six Anatidae principles are containment, calm feedback, stable orientation, adaptive layout, precise input and explicit module boundaries. Visual styling must never weaken the existing read-only hardware posture, bounded benchmarks, source-review isolation or local-buffer DSP behavior.

## 2. White-blue-red tokens

| Semantic token | Light theme | Dark theme |
|---|---|---|
| Workspace | \`#F4F8FD\` | \`#071A2D\` |
| Surface / input | \`#FFFFFF\` | \`#0F2A45\` |
| Elevated surface | \`#EAF1FA\` | \`#173A5E\` |
| Primary text | \`#122C45\` | \`#E8EDF2\` |
| Secondary text | \`#314B66\` | \`#D0DCE8\` |
| Muted text | \`#4D6177\` | \`#B3C4D6\` |
| Primary blue | \`#004E98\` | \`#70A5FF\` |
| Blue hover | \`#1A5FDB\` | \`#8AB6FF\` |
| Action / danger red | \`#B5122D\` | \`#FF7B86\` |
| Red hover | \`#9C1026\` | \`#FF9AA2\` |
| Border | \`#CAD8E8\` | \`#31516F\` |
| Focus | \`#1A5FDB\` | \`#8AB6FF\` |
| Selection | \`#D9E8FF\` | \`#244F82\` |

Do not introduce unrelated green, yellow, violet or teal accents. Success and advisory states use a blue or red marker plus a readable word/icon; hue alone never carries meaning. Normal body text targets WCAG AAA (7:1); controls and large text must meet at least WCAG AA (4.5:1). The colors above are semantic tokens, not decorative hex literals repeated across controls.

## 3. Controls and typography

- Primary actions use blue fill with white text. Secondary actions use a white/surface fill, blue text and a fine semantic border. Destructive or clearing actions use red with white text.
- Native Win32 push controls are owner-drawn with 16px corner curves, a distinct pressed/hover state and a visible keyboard focus rectangle. Main actions are CPU scan, processor suite, source review and report save.
- Interactive control height is 44 logical pixels. Numeric fields have persistent labels and remain separate from their actions.
- Segoe UI is used for the application surface; Consolas is reserved for dense diagnostic output. Keep text at a readable logical size and avoid shrinking type to fit a wide screen.
- The 8pt spacing rhythm is 8/16/24/32 logical px. Standard window gutters are 22px, compact gutters 16px.

## 4. Adaptive layout and 8K

Layout uses the Win32 logical client area after OS DPI scaling, never raw physical resolution. The workbench reflows by available width: three or six primary-action columns, two or four audio-action columns, and three or five parameter columns. The command log remains below the controls and keeps its contents while resizing. A compact minimum window is retained without hiding key actions.

The app requests Per-Monitor V2 DPI awareness when the OS exposes it, falls back to system DPI awareness, and handles \`WM_DPICHANGED\`. On 8K displays, users should use Windows scaling so logical text/control sizes remain readable. 7680×4320 at all scale factors still needs real-device visual regression before being called fully certified.

## 5. Depth, motion and accessibility

The header uses deep blue, the work surface uses pearl white or pond blue, and red is reserved for action emphasis. Surfaces remain opaque and readable; Mica/Acrylic is not emulated. The current UI uses state feedback rather than decorative motion. Keep reduced-motion behavior as the default: no auto-playing or perpetual animation.

Keyboard tab order, focus visibility, label-to-field association and readable status text are mandatory. Numeric inputs reject non-finite or malformed values before calling the C core. A color change must repaint controls, fields, header and log atomically. Do not make the log the only place where the result of an action is visible in future workbench-specific panels.

## 6. Platform implementation ledger

| Area | Current implementation |
|---|---|
| Theme | Light/dark token functions in \`src/win_gui.c\`; a theme action switches palettes |
| Buttons | Owner-drawn rounded Win32 buttons, distinct primary/secondary/danger states and focus rectangle |
| Responsive layout | Width-dependent primary, DSP and parameter grids |
| DPI | Per-monitor DPI-awareness request, \`WM_DPICHANGED\` layout refresh |
| Data safety | UI still calls shared C11 APIs; no UI-side duplicate numerical kernels |
| Native materials | Opaque Win32 surfaces; Mica/Acrylic and animated shared-element transitions are not claimed |
| 8K verification | DPI-aware implementation; real 8K monitor/scale regression is still required |

## 7. Visual review capture plan

The release review should capture: primary/secondary/danger button states; the full workstation at compact and standard widths; 4K/8K logical layouts at OS scaling; focus and selection; both light and dark themes; and a close-up of diagnostic text. Captures must come from a running application. SVG mockups or illustrative previews must be labelled as illustrations, never as live screenshots.

## 8. Release checklist

- [ ] White-blue-red semantic palette only; no unrelated status colors.
- [ ] Light and dark themes repaint all native controls.
- [ ] 44px action targets, rounded buttons and keyboard focus are visible.
- [ ] Compact, standard and wide layouts have no clipped controls.
- [ ] DPI changes trigger a full layout refresh.
- [ ] Contrast tests/inspection cover body text, button labels and focus.
- [ ] CPU discovery remains read-only; benchmarks stay bounded; source review never executes inspected source.
- [ ] DSP remains local-buffer processing; numerical kernels remain in the C11 core.
- [ ] Real-device 8K screenshots are captured before claiming certification.
