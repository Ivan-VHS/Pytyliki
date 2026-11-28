# Vyunenko Theory K3 v0.4.0 — external mathematical reference

> **Editorial update · 2026-11-28.** This page records a documentation-only relationship to Kaskad. It does not declare a product integration or implementation.

The [Kaskad / Vyunenko Theory K3 v0.4.0 proposal](https://github.com/Ivan-VHS/Kaskad/pull/36) adds a formal treatment of the radix-1000 divisibility completion:
- compatible CRT projectors and the orthogonal idempotents associated with the (2)-adic and (5)-adic components;
- the topological-ring decomposition (widehat{mathbb Z}_{1000}congmathbb Z_2	imesmathbb Z_5);
- a unique infinite positional expansion by coefficients (0,ldots,999);
- the distinction between finite K3 words and infinite completion elements.

## Relationship to Pytyliki

The Kaskad theory formalizes radix-1000 positional arithmetic, CRT idempotent splitting of its divisibility completion, and unique infinite coefficient expansions. This is a documentation-only mathematical reference; it does not change Pytyliki's C11 implementation, metrics API, C ABI, benchmark semantics or hardware-access boundaries.

This repository does not import Kaskad source code, gain a new runtime or build dependency, change a local API/ABI/schema, or inherit the Kaskad license. Any future implementation-level adoption would require a separate, versioned contract in the owning repository, plus tests and an explicit compatibility decision.

See the [Psychoporter 666 documentation profile](https://github.com/Ivan-VHS/DAK/blob/main/docs/PSYCHOPORTER_DOCUMENTATION_STYLE_V1.md) for the shared editorial standard. Existing author credits, licensing terms, notices and third-party attribution remain authoritative.
