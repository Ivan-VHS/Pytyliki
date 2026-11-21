# Vendored Reaper native frontend

This directory contains the native C lexer, expression parser, AST JSON serializer, reserved-token catalogue and public headers from **Ivan-VHS/Reaper**.

- Upstream repository: https://github.com/Ivan-VHS/Reaper
- Upstream commit: `34a5970bbdf0a2bc186233aee6631acdeaeb9904`
- Upstream license: Apache License 2.0 (full text retained in `LICENSE`)
- Integration: compiled as an isolated C static library and exposed through a dedicated Pytyliki command.

## Local integration adjustments

- Added the upstream AST JSON serializer because the CLI exposes deterministic AST output.
- Replaced the upstream parser's `strcpy` call with length-checked allocation plus `memcpy` to keep the same semantics under MSVC's secure-CRT diagnostics.

## Scope and boundaries

The imported implementation supports tokenization and parsing of a single expression into a validated AST. It does **not** yet implement Reaper statements, name resolution, evaluation, module loading, the Reaper VM, or full `.rpr` program execution.

This integration is a reusable native frontend, not a claim that Pytyliki has migrated to the complete Reaper runtime. Keep the source provenance and upstream commit updated when synchronizing.
