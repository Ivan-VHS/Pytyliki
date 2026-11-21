# QCS native frontend

Pipeline: UTF-8 source → lexer → frame-aware parser → compact AST → diagnostics.

| Included | Excluded |
|---|---|
| tokenization/source locations | compilation |
| syntax/frame validation | execution |
| CLI diagnostics | hardware control |

Malformed input returns a bounded diagnostic with no external effects.
