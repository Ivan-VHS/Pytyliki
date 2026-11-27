# Pytyliki — Duck Desktop integration

> Optional native launch registration · **Status:** specified, not activated · **Reviewed:** 2026-11-27 UTC

[Pytyliki project README](../../README.md) · [Duck repository](https://github.com/Ivan-VHS/DAK)

This directory defines a future opt-in launch registration for the Pytyliki Windows desktop GUI.

## Manifest

`manifest.json` follows the Duck host-desktop application manifest v1.0.0 contract and points to the Windows Release GUI executable. Pytyliki remains a native C11 workstation; this declaration does not wrap the CLI, change processor APIs or add a web runtime.

The registration is **specified, not activated**. The browser reference preview cannot start native processes. The current Qt 6 host-native desktop can create real top-level host windows, but it does not execute manifest command strings and this repository does not yet include a native application process broker. The manifest therefore remains `maturity_status: specified` and `verification_status: not_run`.

## Build and launch

From a Windows developer environment, build with the project's documented CMake workflow:

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

The GUI entry point is `build/Release/pytyliki-gui.exe`. The GUI target is Windows-only; portable CLI/core builds remain independent.

## Security boundary

A future Duck native application broker must validate the manifest and executable path under the trusted Pytyliki installation root, avoid shell execution and request explicit operator approval before launch. Processor inventory and analysis remain owned by Pytyliki.

This manifest does not grant hardware permissions or enable automatic launch. Verify the referenced Duck manifest schema and native broker contract before changing its maturity status.
