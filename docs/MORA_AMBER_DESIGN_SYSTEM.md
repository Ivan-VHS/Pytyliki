# Imperial Noir & Glass native theme — Pytyliki

**Maintainer:** Ivan-VHS

Pytyliki now starts in Imperial Noir & Glass. The existing Light and Dark profiles remain available through the theme button and are persisted in the current user's registry settings. `MORA_THEME_MODE=amber|mora|light|dark` overrides the saved value for one launch.

## Canonical palette

| Role | Value |
|---|---|
| Workspace | `#050505` |
| Header and log | `#18120D` |
| Input / card | `#101010` |
| Primary text | `#FFFFFF` |
| Supporting text | `#D8D8D8` |
| Muted metadata | `#A3A3A3` |
| gold action | `#FFD700` |
| Hover / focus | `#FFE36E` / `#FFE36E` |
| Critical signal | `#FF7777` |
| Olive support | `#79B8FF` |
| Selection | `#3B3000` |
| Border | `#343434` |

## Implementation boundaries

The native Win32 renderer consumes the shared `mora_theme.h` token source. The theme changes painting only; CPU measurements, benchmarks, reports, C11 execution and worker behavior remain independent. Logs stay opaque and monospaced; no animation is added to precision controls or processor operations.

## Verification

The C UI policy test contains explicit Imperial Noir & Glass contrast assertions. Native scaling, keyboard and physical-display checks remain part of the desktop release gate.
