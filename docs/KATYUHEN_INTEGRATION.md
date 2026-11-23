# Katyuhen integration — Pytyliki

## Role in the ecosystem

Pytyliki is Katyuhen's processor-systems workstation. It owns bounded CPU and memory measurements, inventory, profile import/export, benchmark statistics, source diagnostics and its versioned C ABI.

## Registered interfaces

| Interface | Direction | State | Boundary |
|---|---|---|---|
| Pytyliki C ABI v1 | Provides | Implemented | Consumers select an explicit trusted library |
| Device snapshot v1 | Optional consumer | Specified | Read-only local JSON input |
| VentuSample pack v1 | Optional consumer | Specified | Metadata cross-check only |

## Integration rules

- Keep hardware discovery and measurements observational; no implicit overclocking, voltage, scheduler or firmware changes.
- Native consumers must select a trusted C ABI library path explicitly and verify the expected ABI version.
- Treat imported device and pack metadata as untrusted input and enforce size, schema and numeric bounds.
- Preserve standalone operation when all optional integrations are disabled.

## Workspace and visual identity

Mora is the ecosystem's desktop entry point. Pytyliki retains its engineering workstation layout and evidence panels; native adapters may use local aliases for DuckOS's `mora.theme.v1` semantic tokens.

The Katyuhen registry is maintained in [DuckOS](https://github.com/Ivan-VHS/DuckOS/blob/main/ecosystem/katyuhen.v1.json). The local declaration is [integrations/katyuhen/manifest.json](../integrations/katyuhen/manifest.json).

## Validation

Run the C11 CI and Windows desktop build. C ABI consumers must have a separate integration test for the selected library and platform.
