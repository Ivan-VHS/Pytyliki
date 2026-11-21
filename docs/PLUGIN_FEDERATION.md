# Cross-repository plugin catalog

Pytyliki publishes the shared 18-plugin registry to native C11 consumers through its existing stable \`pytyliki_ffi\` ABI v1. The registry is an enumeration and discovery surface, not an execution engine.

## C ABI discovery

\`\`\`c
uint64_t count = 0;
pytyliki_plugin_descriptor_v1 descriptor;

if (pytyliki_v1_plugin_count(&count) == PYTYLIKI_ABI_OK) {
    for (uint64_t i = 0; i < count; ++i) {
        if (pytyliki_v1_plugin_at(i, &descriptor) == PYTYLIKI_ABI_OK) {
            /* Inspect descriptor.id, descriptor.name and capability metadata. */
        }
    }
}

pytyliki_v1_plugin_find("Keel Koh", &descriptor);
\`\`\`

The descriptor contains fixed-width, null-terminated ID, display name, domain, service and semicolon-separated capability fields. Lookup accepts plugin ID or display name, case-insensitively; spaces and underscores normalize to hyphens.

## Execution boundary

All 18 entries are currently catalogue metadata in Pytyliki. The ABI does not load modules, call foreign handlers, execute scripts, or automatically bridge into Reaper, VentuSample or OPlague. Native computation, processor discovery, memory, metrics and DSP remain owned by Pytyliki's C11 core. A future native handler must be implemented and tested in this repository before its host manifest can claim executable status.

## Compatibility

The shared catalog and host declarations are in \`federation/plugins/\`. Catalog schema: \`schemas/ivan-vhs.plugin-catalog.v1.schema.json\`. Host contract: \`ivan-vhs.plugin-host.v1\`, API version 1. The C ABI remains version 1 and the new descriptor functions are additive.
