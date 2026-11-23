# Mora integration

This directory defines a future opt-in launch registration for the Pytyliki Windows desktop GUI.

## Manifest

\`manifest.json\` follows the DuckOS Mora application manifest v1.0.0 contract and points to the Windows Release GUI executable. Pytyliki is a native C11 workstation; this declaration does not wrap the CLI, change processor APIs or add a web runtime.

The registration is **specified, not activated**. Mora's browser shell cannot start native processes, and this repository does not yet include a native Mora process broker. The manifest therefore remains \`maturity_status: specified\` and \`verification_status: not_run\`.

## Build and launch

From a Windows developer environment, build with the project's documented CMake workflow:

\`\`\`sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
\`\`\`

The GUI entry point is \`build/Release/pytyliki-gui.exe\`. The GUI target is Windows-only; portable CLI/core builds remain independent.

## Security boundary

A native Mora launcher must validate the manifest and executable path under the trusted Pytyliki installation root, avoid shell execution, and request explicit operator approval before launch. Processor inventory and analysis remain owned by Pytyliki.

Project: [Ivan-VHS/Pytyliki](https://github.com/Ivan-VHS/Pytyliki). Project author/developer: Ivan-VHS.