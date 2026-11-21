# Reaper native frontend integration

## Purpose

Pytyliki vendors a small, isolated part of the Reaper language implementation so native C11 applications can validate Reaper source without booting the Python reference package.

**Upstream:** [Ivan-VHS/Reaper](https://github.com/Ivan-VHS/Reaper)  
**Pinned commit:** `34a5970bbdf0a2bc186233aee6631acdeaeb9904)  
**Local path:** `vendor/reaper_frontend/`  
**License:** Apache-2.0; the upstream license text is preserved in `vendor/reaper_frontend/LICENSE`.

## Included components

| Component | Role |
|---|---|
| `lexer.c` | UTF-8-aware tokenization, keyword aliases and source coordinates |
| `parser.c` | Pratt parser for one expression; bounded nesting |
| `token_catalog.c` | Reserved three-character operator catalogue |
| Public headers | Token, AST and catalogue contracts |
| `pytyliki-reaper` | Native CLI for token dumps, expression AST JSON and expression validation |

The library is a separate build target. It does not become a dependency of the existing processor/DSP core and does not alter existing ABI v1 behavior.

## Commands

```sh
pytyliki-reaper --version
pytyliki-reaper --tokens expression.rpr
pytyliki-reaper --ast-json expression.rpr
pytyliki-reaper --check-expression expression.rpr
```

The input file is bounded to 2 MiB. Embedded NUL bytes are rejected. `--ast-json` and `--check-expression` accept one expression, not an entire multi-statement program.

## Verification

The CTest target `reaper-native-frontend` checks:

- UTF-8 identifiers and Russian keyword canonicalization;
- arithmetic precedence in the resulting AST;
- rejection of malformed expressions with source coordinates.

The target `reaper-native-frontend-version` verifies that the CLI executable is linked and runnable.

## Explicitly not implemented

This integration does not claim:

- complete `.rpr` statement or block parsing;
- name/type resolution or semantic analysis;
- interpretation or execution of Reaper programs;
- module loading, filesystem access, process execution or host effects;
- Reaper VM, canonical IR parity, optimizer parity or self-hosted compiler bootstrap.

The Reaper repository's own Python reference runtime therefore remains the compatibility oracle. The native frontend is a first shared component, not a completed Reaper runtime migration.

## Synchronization

When updating the vendored code, update the upstream commit in `vendor/reaper_frontend/NOTICE.md`, preserve license notices, sync headers and sources together, and run the full Pytyliki CMake/CTest suite on Windows and Linux. Never copy generated or unverified runtime claims into the capability matrix.
