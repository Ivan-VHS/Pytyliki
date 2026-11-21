# Visual documentation and graphics contract

**Project:** Pytyliki · **Scope:** README, Markdown, diagrams, screenshots, rendered documentation and native UI references.

This contract standardizes how technical information is presented. It does not claim that a rendering engine, theme, animation, shader or visual effect exists unless source and tests show it.

## Canonical documentation layout

| Surface | Responsibility | Source of truth |
|---|---|---|
| `README.md` | Product summary, feature matrix, build/test, real screenshot | Main branch and CI |
| `README.ru.md`, `README.uk.md`, `README.zh-CN.md` | Maintained translations of the public entry point | Same facts and paths as English |
| `docs/README.md` | Navigation index for technical documentation | Files present in `docs/` |
| `docs/VISUAL_DOCUMENTATION.md` | This visual, graphics and rendering contract | Project architecture and UI contracts |
| `assets/` | Editable vector diagrams and verified application captures | Original SVG/source or CI capture |

Keep one canonical root `README.md`. Do not create a second `README.mdx` that can drift from the GitHub entry point. Use MDX only inside a real documentation site when interactive components are required.

## Typography and Markdown hierarchy

- Use one H1 per page, a short lead paragraph, then H2 sections and H3 subsections without skipped levels.
- Use sentence-case headings, concise paragraphs, descriptive link labels and stable relative links.
- Use inline code for paths, flags, identifiers, API names and filenames; use fenced code blocks with language tags for commands and source.
- Use tables for compact capability matrices, not long prose. Mark status explicitly: **implemented**, **partial**, **planned**, **not applicable**.
- Every meaningful image needs concise alternative text. Captions identify whether an asset is a real capture, an illustrative diagram or a conceptual preview.
- Never encode status, warnings or meaning using colour, emoji, gradients or animation alone.

## Graphics, geometry and rendering

### Vector masters

Use SVG for architecture maps, flow diagrams, icons and documentation infographics. Keep SVG source in version control and export raster derivatives only when a consumer requires them. Every SVG must have a `viewBox`, descriptive `<title>` and `<desc>`, readable labels, adequate contrast and a predictable aspect ratio. Keep SVG static and self-contained: no scripts, event handlers, embedded fonts, external trackers or animation. Avoid `foreignObject` when the graphic must render on GitHub.

### Screenshots and raster derivatives

The existing Windows screenshot is the authoritative application capture. Refresh it only from a successful real desktop build/run; never redraw a screenshot or label an SVG mock-up as a live application. Keep PNG as the compatibility copy for GitHub README pages. WebP or AVIF may be added as optional web-site derivatives only when the source PNG remains available as a fallback. Preserve the SVG master when rasterizing diagrams; do not upscale a small source image to imply extra detail.

### Colour, light, texture and composition

Pytyliki's visual reference uses semantic **Pond Noir**, **Golden Reed** and **Arctic Glacier** palettes. Keep contrast and token roles consistent across canvas, panel, border, primary text, muted text, accent and focus. Gradients, transparency, texture, glow and post-processing are restrained presentation layers: never place them over dense code, measurements or diagnostic content. Avoid noisy backgrounds and decorative geometry that competes with data.

### Motion and effects

README graphics are static. Native motion rules are defined in [UI_MOTION.md](UI_MOTION.md) and the [Anatidae motion constitution](ANATIDAE_MOTION_CONSTITUTION.md). Motion is functional feedback, not environmental decoration. Respect system reduced-motion settings, `UI_REDUCED_MOTION`, Eco mode and the actual native UI implementation. Do not claim 60/120 FPS, zero jitter or measured performance without a reproducible benchmark.

## Diagram and documentation toolchain

| Tool / format | Appropriate use | Repository policy |
|---|---|---|
| GitHub Markdown + Mermaid | Small flow, sequence, state and dependency diagrams | Preferred source-first format when GitHub can render it |
| PlantUML (`.puml`) | Detailed sequence, component or deployment models | Keep source beside generated static SVG |
| D2 (`.d2`) | Readable architecture and relationship maps | Use when it materially clarifies a dense diagram |
| SVG infographic | Stable visual fallback for READMEs and release notes | Provide when a host cannot render diagram source |
| MDX / `.mdx` | Reusable interactive documentation components | Only in a configured site; ordinary repository docs stay Markdown |
| Docusaurus | Versioned docs and sidebars | Optional site adapter; not a project dependency |
| VitePress | Markdown documentation with a Vue-powered site shell | Optional site adapter; not a project dependency |
| Astro Starlight | Structured Markdown/MDX docs and navigation | Optional site adapter; not a project dependency |
| MkDocs Material | Python-oriented docs site and navigation | Optional site adapter; not a project dependency |
| Shields.io | CI, license and release badges | Real endpoints only; no decorative/fabricated status |
| GitHub Pages | Publishing a built documentation site | Only with an actual site build and deployment workflow |

Do not add all documentation frameworks to one repository. GitHub-flavoured Markdown remains the portable baseline; select one site generator only when there is a real publishing requirement. Keep diagram sources editable and provide static SVG/PNG output for viewers without the relevant renderer.

## Project-specific visual boundaries

Pytyliki is a native C11 processor-systems workstation. Diagrams must distinguish the shared C11 core, Windows GUI, CLI tools and bounded local modules. CPU discovery is read-only; workload plans are advisory and do not launch threads or alter CPU affinity; embedded profiles are reference/simulation data; DSP operates on local buffers.

## Review checklist

- [ ] README links resolve and all translations agree on capabilities and safety boundaries.
- [ ] Screenshots come from the real desktop workflow and have factual captions.
- [ ] SVG has title/description, viewBox, readable text and no scripts or remote dependencies.
- [ ] Mermaid / PlantUML / D2 source has a static fallback where the host cannot render it.
- [ ] Badges link to live workflows, license or releases and do not overstate verification.
- [ ] Motion, rendering and performance statements match the native implementation and test evidence.
- [ ] `docs/README.md` lists every canonical guide.

## References

- [GitHub: rendering images and SVG](https://docs.github.com/en/repositories/working-with-files/using-files/working-with-non-code-files)
- [GitHub: Mermaid diagrams](https://docs.github.com/en/get-started/writing-on-github/working-with-advanced-formatting/creating-diagrams)
- [Docusaurus Markdown and MDX](https://docusaurus.io/docs/markdown-features)
- [VitePress Markdown](https://vitepress.dev/guide/markdown)
- [Astro Starlight authoring content](https://starlight.astro.build/guides/authoring-content/)
