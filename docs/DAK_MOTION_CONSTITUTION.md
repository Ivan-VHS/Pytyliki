# Dak Motion Constitution — shared desktop profile

This profile translates the supplied shared motion / Dak Constitution visual language into four native workstations. It is a design-system integration, not a product rename: each application keeps its own information architecture, product vocabulary and toolkit.

## Visual tokens

| Token | Value | Meaning |
|---|---|---|
| Feather white | `#FFFFFF` | primary surface |
| Mist | `#F8FAFC` | quiet background |
| Blue wash | `#EFF6FF` | calm, navigational grouping |
| Red wash | `#FEF2F2` | explicit warning or action grouping |
| River blue | `#0039A6` | primary navigation and actions |
| Deep blue | `#0A1E46` | strong text and dark surfaces |
| Signal red | `#D52B1E` | focus / active signal |
| Critical red | `#B51A10` | destructive or safety-critical state |
| Gold | `#D97706` | optional ornamental accent only |

The white / blue / red composition target remains 60 / 25 / 15, not a measured quota. Red never becomes a generic hover colour, and gold never replaces semantic status. Keep focus indicators, text labels and WCAG contrast independent of colour.

## Type, surfaces and motion

Use native application fonts for controls and body text. Serif display type belongs only in a hero or welcome panel; monospaced type belongs to code, IDs, values and telemetry. Do not fetch remote fonts at runtime in offline products. Prefer opaque native surfaces and restrained borders.

- **Paddle:** `cubic-bezier(0.22, 1.18, 0.36, 1)`, a bounded spring-like acknowledgement.
- **Water:** `cubic-bezier(0.37, 0, 0.63, 1)`, slow symmetric drift for optional brand artwork.
- Direct acknowledgement: 0–40 ms; hover/focus: 80–140 ms; pane transitions: 160–240 ms; optional hero transitions: 260–380 ms.
- Reduced Motion retains no more than 80 ms and removes spatial movement.

## shared motion motifs

Dak bob, tail waggle, waves, ripples, voice particles, floating feathers and parallax are reusable brand-motion vocabulary, not mandatory product chrome. Keep mascot movement small (about 6 px and 2 degrees). Ripples may acknowledge one deliberate action but must not cover values, focus or hit targets. Particles require a pause/disable path. Parallax is limited to low-contrast hero/showcase surfaces; never place it behind editors, tables, plots, device inventory, logs, hashes, package evidence or precision controls. Do not force dak terminology into product features.

## Accessibility and performance

1. Operating-system reduced-motion and `UI_REDUCED_MOTION` overrides take precedence.
2. Reduce Motion removes parallax, bobbing, particles and spatial movement.
3. Eco / Low-CPU Visuals suppresses ambient effects and secondary refresh work.
4. Any optional ambient animation needs an explicit pause control.
5. Animation runs only on the UI thread, is cancelled when widgets are destroyed and never blocks work queues.
6. Never run UI motion inside DSP callbacks, audio buffers, generator workers, benchmarks, device scans, archive inspection or compiler/runtime execution.

## Native mapping

- **Pytyliki / Win32:** one native startup transition; no ambient loop.
- **Reaper / Tk:** bounded spring controller; no decoration behind editor/debugger panes.
- **OPlague / Tk:** persisted Expressive spring preference; evidence panes remain still and opaque.
- **VentuSample / Qt:** bounded Qt spring and project-BPM indicator; no motion in DSP or generator workers.

Product-specific `docs/UI_MOTION.md` contracts remain authoritative for enabled features.


## Motion Constitution v2026 precedence

The normative shared motion, layout and telemetry contract is [Motion Constitution v2026](MOTION_CONSTITUTION_V2026.md). This design document supplies visual-language guidance; it does not override the canonical token ranges or product-specific implementation status.

Shared tokens: `--ease-spring`, `--ease-snappy`, `--motion-scale` (0.4–2.2) and normalized `--spring-stiffness` (0.7–1.4). Operating-system reduced-motion and `UI_REDUCED_MOTION` take precedence; nonessential spatial/ambient motion is removed and the underlying state change is retained. Light/dark parity, keyboard focus and high-DPI readability apply to every supported visual state.

The 2×2 overview and focus scene are showcase layout targets. SVG/Canvas or native-paint graphics must be driven by real model values, expose units and textual equivalents, and distinguish unknown, stale and error states. Product-specific `docs/UI_MOTION.md` remains authoritative for shipped native behavior and safety boundaries.
