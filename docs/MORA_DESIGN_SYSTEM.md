# Mora integration · Pytyliki

Pytyliki's Win32 workstation now reads its desktop page, surface, header, log, selection, border and primary-action colors from `src/mora_theme.h`. The shared semantic tokens are mirrored in `mora/theme.json`.

The processor data model, measurement paths, command IDs, read-only behavior and native window layout remain unchanged. Mora supplies the shared desktop frame and action language; Pytyliki keeps its cyan processor/telemetry identity where those colors encode domain information.

The native UI continues to use its existing contrast-safe text helper and respects the existing Reduce Motion / Eco Visuals preferences. The header is intentionally toolkit-native rather than a web view.

## Implementation map

| Surface | Mora token |
|---|---|
| Main page | dark/light page |
| Window header | dark/light header |
| Input panels | dark/light surface |
| Log output | dark/light log |
| Primary actions | violet brand |
| Hover and keyboard selection | brand hover / selection |
| Borders | shared semantic border |

The Windows GUI includes `mora_theme.h` directly. No remote stylesheet, browser dependency or network fetch is added to the native workstation.
