# Pytyliki native QCS frontend

Pytyliki exposes a portable C11 QCS lexer, frame-aware parser, compact AST index and command-line inspector.

## Build and run

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure

    ./build/pytyliki-qcs check examples/qcs_irq.qcs
    ./build/pytyliki-qcs tokens examples/qcs_irq.qcs
    ./build/pytyliki-qcs ast examples/qcs_irq.qcs

On Windows, use the generated executable under the selected CMake configuration directory.

## Scope

The C11 module validates frame boundaries, required domain attributes, supported state markers, nested delimiters, registered domain identifiers and flow-operator placement. It emits a flat, parent-indexed AST summary for modules, frames, functions and flow operators.

The Python frontend in Reaper contains the richer expression/declaration AST. The Pytyliki native parser is intentionally a small frame-aware validation and inspection layer; it is not yet a complete Swift-like parser, type checker, ownership verifier, formal proof engine or hardware backend. Declaring a domain does not grant physical hardware access.
