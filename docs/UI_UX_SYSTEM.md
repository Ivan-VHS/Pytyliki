# Pytyliki UI/UX system

**Status:** implemented foundation for the Windows workstation; cross-platform desktop shells remain separate work.

## Product surface

Pytyliki is a processor and systems workstation, not a marketing dashboard. Its primary tasks are hardware capability inspection, bounded benchmarks, source review, DSP operations, metric experiments and embedded-profile inspection. The interface prioritizes task grouping, explicit operation labels, readable diagnostics and visible safety boundaries.

## Visual language

- **Fluent-inspired structure:** a calm page surface, strong dark header, one restrained red accent, consistent Segoe UI typography and compact grouped actions.
- **Opaque work surfaces:** logs, measurements and parameter fields use stable surfaces. Acrylic/Mica-like treatments are not simulated behind text or numerical output.
- **Light/dark modes:** the native Win32 shell exposes a direct theme switch. Both palettes preserve readable text, focused controls and selected states.
- **Responsive layout:** action groups reflow with the window width; the minimum window size protects the control row and parameter fields from collapsing.
- **DPI-aware rendering:** process DPI awareness is enabled before creating the main window; typography and layout remain legible on high-density displays.

## Platform adaptation

| Environment | UI direction | Implementation boundary |
|---|---|---|
| Windows | Fluent-inspired native Win32 controls, high-contrast surfaces, light/dark palette | Desktop GUI is currently Windows-only |
| macOS | Future native AppKit adapter; use native materials only in chrome | No macOS GUI is claimed |
| GNOME/Linux | Future GTK/Libadwaita adapter for a native shell | Portable C11 core and CLI remain usable independently |
| Web/analytics | Dashboard-like metric summaries only where dense comparisons help | No web frontend is implied by the workstation UI |

## Interaction and accessibility

- Keep hardware inspection, source review and DSP actions in distinct groups.
- Label actions with verbs and expected artifacts; do not rely on color alone for status.
- Maintain visible keyboard focus and native control semantics.
- Keep log text selectable, high contrast and read-only.
- A diagnostic flag is a review prompt, not a verified fault or certification.
- Do not add motion or transparency that obscures values, long paths, code or diagnostic traces.

## Shared visual contract

| Token | Light | Dark |
|---|---|---|
| Page | #F3F6FA | #101827 |
| Surface | #FFFFFF | #172337 |
| Text | #172337 | #EEF3FA |
| Muted | #5C6B80 | #A7B4C7 |
| Accent | #2563EB | #6EA8FE |
| Alert | #DC2626 | #F87171 |
| Diagnostic log | #111C2E | #0B1422 |

Red remains a small brand accent; blue is reserved for primary actions and focus. Status colors are supplementary, never the only signal.

## Related repositories

- [Reaper language and Studio](https://github.com/Ivan-VHS/Reaper/blob/main/docs/UI_UX_SYSTEM.md)
- [OPlague driver workbench](https://github.com/Ivan-VHS/OPlague/blob/main/docs/UI_UX_SYSTEM.md)

## Validation checklist

- Light and dark palettes keep log text and controls readable.
- The six primary actions fit without clipping at the supported minimum width.
- The theme switch repaints static labels, edit fields and the main surface.
- Existing CPU, DSP, review and report flows remain unchanged.
