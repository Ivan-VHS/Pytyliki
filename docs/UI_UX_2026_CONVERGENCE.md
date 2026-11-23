# UI/UX 2026 — shared product interface contract

**Maintainer:** Ivan-VHS · **Product:** Pytyliki  
**Design language:** AETHER-ANAS / Anatidae — consistent white/blue/red identity adapted to native platform behavior.

This is an implementation contract for an operational desktop tool, not a marketing dashboard. The next useful task should be obvious; information should remain legible; visual polish must never interfere with processing, diagnostics or file operations.

## Semantic palette

| Token | Light | Dark | Role |
|---|---|---|---|
| bg | #F4F8FD | #071A2D | Workspace |
| surface | #FFFFFF | #0F2A45 | Work panels |
| elevated | #EAF1FA | #173A5E | Secondary surfaces |
| fg | #122C45 | #E8EDF2 | Primary text |
| secondary | #314B66 | #D0DCE8 | Supporting text |
| muted | #4D6177 | #B3C4D6 | Hints and metadata |
| blue | #004E98 | #70A5FF | Primary actions/navigation |
| blue_hover | #1A5FDB | #8AB6FF | Hover and focus |
| on_blue | #FFFFFF | #071A2D | Text on blue |
| red | #B5122D | #FF7B86 | Critical/destructive states |
| red_hover | #9C1026 | #FF9AA2 | Critical-action hover |
| on_red | #FFFFFF | #071A2D | Text on red |
| border | #CAD8E8 | #31516F | Boundaries |
| focus | #1A5FDB | #8AB6FF | Keyboard focus |
| selection | #D9E8FF | #244F82 | Selected content |
| selection_fg | #122C45 | #FFFFFF | Text on selection |
| disabled | #6B7C90 | #9BAFC4 | Disabled text |

Blue is the action, navigation and focus language. Red is reserved for critical or destructive states, never a generic focus ring. Prefer opaque work surfaces; blur and translucency do not replace contrast.

## Hierarchy and density

- Task-first workflow: primary operation first, supporting context second, logs and diagnostics in separate reachable panes.
- Use an 8 px spacing rhythm. Keep labels, values and units visible.
- Prefer 40–44 px action targets and 38–44 px inputs where native controls allow it. Dense tables may use 34–36 px rows when readable.
- Use native system typography for controls and monospace text for logs, hashes and numeric diagnostics.
- Never communicate success, failure or risk through color alone.

## Accessibility and input

- Give interactive controls meaningful accessible names and visible keyboard focus.
- Preserve native keyboard navigation, shortcuts, selection behavior, text scaling and high-DPI handling.
- Target 4.5:1 contrast for normal text, 3:1 for large text/focus, and 7:1 for primary body text where practical.
- Reduced Motion overrides Expressive Motion. Eco suppresses nonessential transitions.
- Respect OS accessibility preferences and `UI_REDUCED_MOTION`; hover, color or motion must never be the only cue.

## Motion 2026

| Event | Timing |
|---|---:|
| Direct acknowledgement | 0–40 ms |
| Hover/focus/state | 80–140 ms |
| Pane/graph transition | 160–240 ms |
| Context transition | 260–380 ms |
| Reduced-motion replacement | ≤80 ms |

Motion is functional feedback, not ambient decoration. Expressive Motion may scale supported transitions by 1.35×, capped at 380 ms. Reduced Motion removes spatial movement and caps retained fades at 80 ms; Eco follows the same conservative path. OS preferences and `UI_REDUCED_MOTION` take precedence.

Animation stays outside processor benchmarks, C11 workers, FFI and DSP paths. Cancel transitions when their owner is destroyed. Do not invent BPM/transport sync without a real tempo source.

## Responsive and high-DPI

Reflow panels instead of compressing labels or hiding actions. Test narrow/wide windows, keyboard-only use, and 100%, 125%, 150% and 200% scaling. Manual monitor review remains an acceptance item until performed.

## Product-specific implementation

Native Win32 processor workstation: retain responsive control layout, owner-drawn feedback, DPI awareness, persisted Normal / Expressive / Reduced / Eco preferences and registry compatibility. Do not add browser-only blur/parallax or motion to precision controls. Keep visuals separate from C11 processor, FFI, benchmarks and DSP execution. The GUI remains Windows-only.

## Engineering references

- [Qt Accessibility](https://doc.qt.io/qt-6/accessible.html) · [Qt High DPI](https://doc.qt.io/qt-6/highdpi.html)
- [WCAG 2.2 Focus Appearance](https://www.w3.org/WAI/WCAG22/Understanding/focus-appearance.html)
- [Microsoft Windows app design](https://learn.microsoft.com/windows/apps/design/)

References define targets; this document does not claim all manual tests have passed.


## Mora Amber profile

The native Win32 workstation defaults to the shared [Mora Amber profile](MORA_AMBER_DESIGN_SYSTEM.md). Light and Dark remain selectable. The palette does not change processor or report behavior; Reduce Motion and Eco remain authoritative.
