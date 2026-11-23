# Mora Amber native theme — Pytyliki

**Maintainer:** Ivan-VHS

Pytyliki now starts in Mora Amber. The existing Light and Dark profiles remain available through the theme button and are persisted in the current user's registry settings. `MORA_THEME_MODE=amber|mora|light|dark` overrides the saved value for one launch.

## Canonical palette

| Role | Value |
|---|---|
| Workspace | `#211913` |
| Header and log | `#18120D` |
| Input / card | `#33261D` |
| Primary text | `#FFF3E2` |
| Supporting text | `#E7D2B8` |
| Muted metadata | `#C4A98A` |
| Amber action | `#D58B3D` |
| Hover / focus | `#EAA653` / `#F3C16B` |
| Critical signal | `#B64A40` |
| Olive support | `#526347` |
| Selection | `#60442E` |
| Border | `#75543A` |

## Implementation boundaries

The native Win32 renderer consumes the shared `mora_theme.h` token source. The theme changes painting only; CPU measurements, benchmarks, reports, C11 execution and worker behavior remain independent. Logs stay opaque and monospaced; no animation is added to precision controls or processor operations.

## Verification

The C UI policy test contains explicit Mora Amber contrast assertions. Native scaling, keyboard and physical-display checks remain part of the desktop release gate.
