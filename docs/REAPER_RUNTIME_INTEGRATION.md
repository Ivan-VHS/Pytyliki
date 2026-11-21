# Reaper native runtime integration

Pytyliki remains a standalone C11 processor, metric, memory and DSP library. Its Reaper integration is optional and never replaces those native subsystems.

To link a sibling Reaper source checkout:

\`\`\`sh
cmake -S . -B build -DREAPER_SOURCE_DIR=/absolute/path/to/Reaper
cmake --build build --parallel
ctest --test-dir build --output-on-failure
\`\`\`

CMake adds Reaper's native C17 target and links the versioned \`reaper_ffi\` shared ABI into \`pytyliki_ffi\`. The integration exposes three optional calls from \`src/interop.h\`:

- \`pytyliki_v1_reaper_available\` — report whether this build contains the optional bridge.
- \`pytyliki_v1_reaper_eval_i64\` — evaluate a bounded Reaper integer expression.
- \`pytyliki_v1_reaper_selfhood_lookup\` — exact lookup of the Reaper selfhood/autonomy catalogue.

Without \`REAPER_SOURCE_DIR\`, default builds remain self-contained. The functions report \`PYTYLIKI_ABI_BACKEND_UNAVAILABLE\`; they do not search PATH, load a dynamic library at runtime, access a sibling directory implicitly or download a dependency.

When available, the Reaper evaluator receives explicit UTF-8 source bytes and uses the native lexer, parser, checked IR, optimizer and signed-i64 execution pipeline. It does not gain filesystem, process, network, device or plugin capabilities. This is an expression engine bridge, not a sandbox.

Existing Pytyliki metric, C11 compute, embedded-memory and DSP APIs are unchanged. Reaper is an additional optional language/control runtime, not a replacement for the product's specialist C implementation.
