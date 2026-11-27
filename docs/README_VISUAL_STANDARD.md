# README and Visual Documentation Standard

**Scope:** Pytyliki README files, documentation entry points, diagrams and visual assets. Source code, tests, schemas and CI remain authoritative for product behavior.

## Publishing principles

1. Lead with one factual sentence describing the product, audience and primary task.
2. Keep the hero, project name, current CI/license badges and quick navigation above the fold.
3. Treat this as documentation presentation, not a software release. Never invent a version, release date, passing CI state or shipped capability.
4. Separate implementation maturity from test verification and platform availability.
5. Keep the root README as an orientation page; put detailed contracts in their owning documents.

## Required README flow

Use this order where it fits the product: adaptive hero; one-line pitch; badges and navigation; scope/status note; requirements and quick start; documentation-revival comparison; implemented capability matrix; visual overview; Mermaid architecture; configuration; repository map; verification; roadmap/status links; contribution and license.

The comparison table describes documentation changes, not fictional product-version changes.

## Adaptive hero assets

Store the pair at `assets/readme/hero-banner-light.svg` and `assets/readme/hero-banner-dark.svg`. Both files MUST use the same viewBox and intrinsic dimensions. Reference them with `<picture>` and `prefers-color-scheme`; keep meaningful fallback alt text on `<img>`. SVGs MUST be self-contained, accessible, text-based and free of external network resources.

## Screenshots and illustrations

- Label real application captures as screenshots and link to the original image.
- Label workflow/architecture drawings as illustrations or diagrams; never present them as UI captures.
- Preserve source provenance and avoid mock metrics, fake devices or invented application states.
- Use a two-column, one-row visual grid on wide screens with descriptive alt text and captions.
- When no verified screenshot exists, say so plainly and use clearly labelled schematic illustrations.
- Keep raster screenshots in `assets/screenshots/` and authored diagrams in `assets/readme/`.

## Architecture, commands and configuration

Prefer Mermaid for editable source architecture. Diagram the implemented path and mark partial/planned branches. Every quick-start command MUST be checked against current build files and scripts. List only real environment variables and CMake options. If a hosted service, account, API key or `.env` file does not exist, say it is not required rather than inventing values.

## Localization and navigation

Keep English, Ukrainian, Russian and Simplified Chinese README variants aligned in structure, scope and commands. Preserve canonical code, file names and CLI flags. Use relative links and ensure each README links to `docs/README.md`, `CONTRIBUTING.md` and `SECURITY.md`.

## Acceptance checklist

- [ ] Hero renders in both light and dark themes.
- [ ] Purpose and product boundary are clear.
- [ ] Clone/build/test/run commands match current repository files.
- [ ] Every image is labelled as a screenshot or illustration.
- [ ] Mermaid describes implementation rather than an aspirational design.
- [ ] Maturity and platform limitations are explicit.
- [ ] Localized entry points and links are consistent.
- [ ] `git diff --check` and relevant checks pass.
