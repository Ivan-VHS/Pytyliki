# Mora integration · Pytyliki

Pytyliki's Win32 workstation now reads its desktop page, surface, header, log, selection, border and primary-action colors from `src/mora_theme.h`. The shared semantic tokens are mirrored in `mora/theme.json`.

The processor data model, measurement paths, command IDs, read-only behavior and native window layout remain unchanged. Mora supplies the shared visual shell primitives; Pytyliki keeps its cyan processor/telemetry identity where those colors encode domain information.

The native UI continues to use the existing contrast helper and respects the existing Reduce Motion / Eco Visuals preferences. The header is intentionally toolkit-native rather than a web view.
