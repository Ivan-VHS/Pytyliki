# Disney Motion System — Pytyliki

Pytyliki adopts a restrained, native Win32 interpretation of the twelve animation principles. Motion communicates state and spatial continuity; it is never decoration that delays processor diagnostics, audio processing, source review, or report generation.

## Motion contract

- Keep the existing Pytyliki visual identity, layout, and native controls.
- Use native Win32 APIs; do not add a browser runtime or UI framework.
- User input and diagnostic operations remain immediate. Animation must never imply that a CPU scan, benchmark, DSP operation, or report has completed before its real result exists.
- Respect Windows animation accessibility settings and the portable `UI_REDUCED_MOTION=1` override. Reduced mode skips nonessential movement and preserves all content and status feedback.
- Prefer short, interruptible transitions. Avoid flashing, looping decoration, parallax, and long stagger sequences.

## Timing tokens

| Token | Duration | Purpose |
|---|---:|---|
| instant | 0 ms | Reduced motion / immediate state |
| micro | 100 ms | Press and focus feedback |
| fast | 140 ms | Small control transition |
| standard | 180 ms | Menu and disclosure |
| spatial | 240 ms | Window or workspace transition |
| emphasis | 280 ms | Meaningful success or validation feedback |

Use ease-out for entry, ease-in for exit, ease-in-out for repositioning, and linear only for continuous progress. Cap stagger at 120 ms total.

## Twelve principles translated

1. **Squash and stretch:** restrained pressed-state feedback; never distort labels, values, or charts.
2. **Anticipation:** focus and hover states preview available actions without delaying clicks.
3. **Staging:** one visual focus at a time; keep the diagnostic log and controls readable.
4. **Straight-ahead action and pose-to-pose:** use explicit state transitions for predictable workflows; reserve continuous frame-by-frame motion for genuinely continuous interactions such as dragging or meters.
5. **Follow-through and overlap:** related controls may transition in a short bounded sequence; parent surfaces lead.
6. **Slow in / slow out:** consistent easing for entry, exit, and repositioning.
7. **Arcs:** optional for spatial navigation only; keep dense workstation layouts direct and predictable.
8. **Secondary action:** supporting icon/status feedback reinforces the real result without competing with it.
9. **Timing:** use the shared duration tokens; never block the message loop.
10. **Exaggeration:** reserve stronger emphasis for important validation errors and confirmed success.
11. **Solid drawing:** preserve geometry, clipping, z-order, contrast, and focus indicators at every frame.
12. **Appeal:** motion should feel precise and technical, matching the workstation's visual language.

## Component mapping

- Buttons: immediate native pressed/focused states; no layout shift.
- Menus and dialogs: reveal from their invoking control, preserve keyboard focus, and restore focus on close.
- Diagnostic log: append results immediately; never animate or delay the underlying text.
- Metrics and benchmark values: interpolate only when comparison benefits; keep exact values available immediately.
- Audio meters: linear continuous movement is acceptable; never use decorative motion as a substitute for measured data.
- Errors and completion: use text and semantic state in addition to any visual emphasis.

## Current native adapter

The Win32 workstation uses a short 140 ms alpha blend when the application window first appears. It checks the Windows client-area animation accessibility setting and the `UI_REDUCED_MOTION` environment override. If animation is disabled or unsupported, the window is shown normally. This is a deliberately safe first integration point; it does not change the geometry or timing of diagnostic operations.

## Accessibility and performance

1. Keep keyboard operation and visible focus available in every mode.
2. Never rely on color or movement alone to communicate an error or completion.
3. Do not animate large log contents, dense tables, or every row in a long list.
4. Keep animation work bounded and on the UI thread; never block message dispatch.
5. If frames cannot be rendered smoothly, skip intermediate frames and show the latest state.
6. Test on high-DPI and lower-powered Windows systems, with reduced motion both enabled and disabled.

## Review checklist

- [ ] Motion explains a state change or spatial relationship.
- [ ] Input and diagnostics remain immediate.
- [ ] Reduced motion removes nonessential effects.
- [ ] Text, metrics, focus, and control geometry remain stable.
- [ ] No decorative loops, flashes, or long cascades are introduced.
