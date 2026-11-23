# Documentation control

| # | Rule | Check |
|---:|---|---|
| 1 | Evidence | implemented claims link to code/tests |
| 2 | Privacy | no private uploads, identifiers or local paths |
| 3 | Status | separate implemented / partial / planned |
| 4 | Contracts | document formats, limits, exclusions |
| 5 | Compatibility | version breaking interfaces |
| 6 | Review | verify links, numbers and examples |

## Internal information architecture

| # | Rule | Acceptance |
|---:|---|---|
| 7 | Topic clusters | Every important page belongs to a clear topic hub and is listed in the central documentation map. |
| 8 | Navigation depth | Important pages are reachable in no more than three to four steps from the root README. |
| 9 | No orphan pages | Every important page has at least two useful incoming paths: the central catalog and its topic hub or an equivalent contextual page. |
| 10 | Contextual links | Place links where the surrounding text explains why the destination is relevant; prefer body links over repeated global menus. |
| 11 | Descriptive anchors | Name the target concept or document precisely; avoid “here”, “more” and keyword-stuffed anchors. |
| 12 | Cluster coherence | Cross-link documents that share a real technical dependency; keep unrelated contracts separate. |
| 13 | Canonical targets | Use direct relative paths for same-repository files and canonical HTTPS URLs for cross-repository resources. Avoid avoidable redirects and duplicate targets. |
| 14 | Breadcrumbs | Cluster hubs identify the documentation home and the parent information map. Add equivalent breadcrumbs to new deep-level pages. |
| 15 | Link density | Add links only where they improve navigation; avoid repetitive link blocks and link-heavy paragraphs. |
| 16 | New-document propagation | When adding a document, update its topic hub and central catalog in the same change; link new material from relevant existing pages. |
| 17 | Maintenance | Periodically check links, remove obsolete destinations and correct renamed paths. Include relative-link checks in CI where available. |
| 18 | Measurement | Track clicks and navigation depth only on a published documentation surface with supported, privacy-respecting analytics; do not invent telemetry for GitHub-rendered Markdown. |
| 19 | Accessibility | Use descriptive link text, preserve normal same-tab navigation and ensure rendered links remain distinguishable and usable on small screens. |

The [information architecture map](INFORMATION_ARCHITECTURE.md) is the navigation source for this repository. The product README, topic hubs, detailed contracts, code, tests and schemas must agree on names, status and boundaries; the map never overrides implementation evidence.
