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


## 9. Anatidae identity grammar — lexical and visual expansion

### 9.1 Design intent

The supplied Anatidae word family is a design vocabulary for approachability, recognition, movement and coherent surfaces. It is **not** an instruction to turn technical software into a children's product. Do not scatter duck illustrations, quack jokes or diminutive words through command names, diagnostics, settings, reports or safety notices. Product copy stays precise and professional.

### 9.2 Consolidated lexical matrix

Repeated forms are normalized; spelling variants are retained here as creative source terms rather than duplicated interface labels.

| Design role | Source vocabulary | Product interpretation |
|---|---|---|
| First contact and guidance | Утёнок, Утёныш, Утеныш, Уточёнок, Утёночек, Утеночек, Утёнушка, Утюша, Утя, Утюня, Утюняш, Утюняшка, Утяшка, Кряквёнок, Крякушонок, Крякалыш, Птенчик, Птенец, Птеныш, Пухляш, Пушистик, Пернатик, Крылыш, Водоплавчик | Welcoming first-run flow, plain-language empty states, clear defaults, progressive disclosure and compact help |
| Product identity and recognition | Утка, Уточка, Крякуша, Кряква, Кряквашка, Птица, Пташка, Пернатая, Крылатая | A recognizable abstract brand mark and consistent navigation language; never a mascot-heavy UI |
| Shape, surface and motion | Пуховичок, Пёрышко, Лапчатая, Прудовая, Озерная, Водоплавающая, Водяная птица | Softly layered surfaces, fine feather-like separators, stable touch targets, subtle waterline/ripple cues and responsive flow |
| Experimental vocabulary only | Крякозябра, Крякальщица | Internal creative references only; do not expose them as production feature names or status labels |

### 9.3 Shared visual grammar

- **Water:** use calm, continuous page-to-panel hierarchy and subtle progress/wave cues. Water is a compositional metaphor, not a translucent effect.
- **Plumage:** use restrained fine-line dividers, compact grouped metadata and consistent icon strokes. Never add noisy feather textures behind text or data.
- **Wing:** use directional alignment, clear action grouping and predictable focus travel. Do not use decorative flight animations.
- **Webbed-foot tactility:** preserve generous hit targets, stable controls and visible pressed/focused states.
- **Young-bird vocabulary:** translate it into a first-run/quick-start experience, progressive disclosure and helpful empty states—not infantilized copy.
- **Pond and lake:** retain the established semantic white/blue/red palette and opaque light/dark surfaces. No teal, amber, green or violet accent palette is introduced by this metaphor.
- Keep the shared 8pt spacing rhythm, 44 logical px preferred action targets, readable text measure, keyboard operation and reduced-motion behavior.

### 9.4 Cross-product expression

The shared identity is deliberately quiet; each product expresses it through its own work:

- **Pytyliki:** contained instrument panels, stable measurement surfaces and clearly separated processor, source-review and DSP workbenches.
- **Reaper Studio:** a source-first flow surface, directional command grouping and inspectable output/debug panes that never displace the editor.
- **OPlague:** layered evidence from concise summary to exact identifiers and package details; provenance and read-only boundaries remain more visible than decorative status.
- **VentuSample:** rhythmic grid and phrase-oriented grouping, with waveform/sample-chop cues and deterministic progress. Do not imply live playback, real-time rendering or hardware monitoring that the application does not provide.

### 9.5 Acceptance

- The brand is recognizable without a cartoon duck or a background illustration.
- The lexical source remains a design metaphor; all production labels stay technically meaningful.
- Existing light/dark tokens, accessibility, safety limits, data contracts and keyboard behavior remain authoritative.
- Motion is state-led, nonessential and disabled or minimized under reduced-motion settings.
- A visual refresh cannot promote a planned capability to implemented status.


### Pytyliki application profile
Treat the workstation as a calm, precise instrument deck: processor identity and measurements occupy stable opaque surfaces; DSP controls remain a separate, rhythmically grouped lane; source review and export remain visibly distinct. Use the pond metaphor only in page hierarchy and restrained separators. A first-run path should explain Scan → Measure → Inspect → Export in plain language without masking the read-only and bounded-workload rules.
