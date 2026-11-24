# Documentation control

**Scope:** Pytyliki documentation, Markdown entry points, visual assets, architecture diagrams and linked machine-readable contracts.

## Acceptance rules

| Gate | Requirement | Evidence |
|---|---|---|
| Evidence before claims | Implemented claims point to current source or tests | Source path, test or CI link |
| Explicit maturity | Separate implemented, partial, reference/simulated and planned behavior | Feature matrix and implementation source |
| Privacy | Exclude secrets, personal identifiers and local machine paths | Diff and fixture review |
| Contract accuracy | State accepted formats, bounds, defaults and exclusions | Schemas, code and tests |
| Compatibility | Version public ABI/schema changes and explain migration | Versioned contract and tests |
| Documentation structure | Every important page has an owner and information-map entry | Documentation index and topic hub |
| Visual integrity | Screenshots are real captures; diagrams are labelled | Asset provenance and alt text |
| Adaptive rendering | Hero has matching light/dark SVGs and a fallback | Both SVG assets and README picture block |
| Example accuracy | Clone/build/run/test commands match build files | CI/build configuration |
| Localization | Language variants preserve scope, limits and commands | README language comparison |
| Review hygiene | Links, code fences, Mermaid, tables and diffs are checked | Link check and git diff --check |

## Maturity vocabulary

- **Implemented:** behavior exists in current source.
- **Partial:** only a documented subset is implemented.
- **Reference / simulated:** a model or simulation, not physical hardware operation.
- **Planned:** design intent without an available implementation.
- **Verified:** a defined check passed on a stated revision and environment; it is not a synonym for implemented.

## Publication checklist

When behavior changes, update the root README and the owning detailed contract together. Add new documents to the information architecture map and topic hub. Do not infer performance, compatibility, safety or certification from a successful build alone.
