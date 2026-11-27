# Pytyliki SystemArray sigil operators

**Specification:** 0.4 draft  
**Frontend implementation:** C11 metadata registry and bounded suffix parser  
**Canonical language owner:** Pytyliki  
**Compiler foundation:** Reaper Canonical Typed IR, Span, SourceMap and shared ABI contracts  
**Execution status:** no operator evaluation, overload resolution, memory access or hardware access is implemented by this registry.

## Purpose

The punctuation forms used by the SystemArray design are reserved hardware-dialect operators. The operator token chooses an operation; the spelling of the SystemArray type does not. All supported aliases normalize to one SystemArray AST identity.

The C11 registry in `system_sigil.c` assigns stable semantic IDs, type signatures, array-plan signatures, effect classes, applicable domains and safety notes to the thirteen suffix tokens. It is a language metadata boundary, not an execution engine.

## Stable operator catalog

| ID | Token | Semantic identity | Scalar contract | Effect |
|---:|---|---|---|---|
| 1 | `^>` | `bits.rotate_left` | `Bit<N>, u32 -> Bit<N>`; `Logic<N>, Logic<S> -> Logic<N>` | Pure |
| 2 | `!==` | `compare.not_equal` | `T, T -> bool`, `T: HardwareComparable` | Pure |
| 3 | `>^` | `bits.rotate_right` | `Bit<N>, u32 -> Bit<N>`; `Logic<N>, Logic<S> -> Logic<N>` | Pure |
| 4 | `!!==` | `bits.pattern_not_equal` | `Bit<N>, Bit<N> -> bool` | Pure |
| 5 | `^^>>` | `vector.lane_shift_left_zero_fill` | `Vector<W,L>, u32 -> Vector<W,L>` | Pure |
| 6 | `>>^^` | `vector.lane_shift_right_zero_fill` | `Vector<W,L>, u32 -> Vector<W,L>` | Pure |
| 7 | `!^>` | `bits.any_masked_bit_set` | `Bit<N>, Mask<N> -> bool` | Pure |
| 8 | `^!>` | `bits.all_masked_bits_match` | `Bit<N>, Mask<N> -> bool` | Pure |
| 9 | `>!^` | `bits.xor` | `Bit<N>, Bit<N> -> Bit<N>` | Pure |
| 10 | `>^=` | `bits.rotate_right_assign` | `mut Bit<N>, u32 -> Unit` | StagedMutation |
| 11 | `^>>` | `bits.arithmetic_shift_right` | `SBit<N>, u32 -> SBit<N>`; `LogicSigned<N>, Logic<S> -> LogicSigned<N>` | Pure |
| 12 | `>>^` | `bits.logical_shift_right` | `Bit<N>, u32 -> Bit<N>`; `Logic<N>, Logic<S> -> Logic<N>` | Pure |
| 13 | `^>==!` | `logic.equal_with_validity` | `Logic<N>, Logic<N> -> LogicBit` | LogicRead |

### Normative behavior

- `^>` and `>^` rotate within width `N`; `N` must be nonzero and the shift amount is reduced modulo `N`.
- `!==` compares values according to the operand type's declared equality semantics.
- `!!==` compares fixed-width bit patterns. For two-state `Bit<N>`, its truth value equals not-equal, but its AST identity remains distinct for diagnostics and trace readability.
- `^^>>` and `>>^^` shift each vector lane independently and zero-fill. No bit crosses a lane boundary.
- `^>>` is arithmetic right shift with sign extension. For an amount greater than or equal to the width, the result is all sign bits.
- `>>^` is logical right shift with zero fill. For an amount greater than or equal to the width, the result is zero.
- For four-state `Logic<S>` shift counts, an unknown X/Z count yields an all-X result. Known counts use the ordinary width rule.
- `!^>` is true iff at least one bit selected by the mask is set: `(value & mask) != 0`.
- `^!>` is true iff all selected bits are set: `(value & mask) == mask`; an empty mask is true by definition.
- `>!^` is width-preserving bitwise XOR.
- `^>==!` returns a four-state `LogicBit`: known equality is 1, a known mismatch is 0, and unresolved equality is X. It is not a two-state boolean comparison.

## SystemArray overloads are lazy plans

The SystemArray overload creates a typed plan; it does not read RAM, MMIO, a register bank, a DMA buffer or a device.

| Token | SystemArray plan |
|---|---|
| `^>` | `SystemArrayPlan<..., RotateLeft>` |
| `!==` | `SystemArrayComparePlan<..., AnyNotEqual>` |
| `>^` | `SystemArrayPlan<..., RotateRight>` |
| `!!==` | `SystemArrayComparePlan<..., BitPatternNotEqual>` |
| `^^>>` | `SystemArrayPlan<..., LaneShiftLeft>` |
| `>>^^` | `SystemArrayPlan<..., LaneShiftRight>` |
| `!^>` | `SystemArrayComparePlan<..., AnyMaskedBitSet>` |
| `^!>` | `SystemArrayComparePlan<..., AllMaskedBitsSet>` |
| `>!^` | `SystemArrayPlan<..., XorArray>` |
| `^>>` | `SystemArrayPlan<..., ArithmeticShiftRight>` |
| `>>^` | `SystemArrayPlan<..., LogicalShiftRight>` |
| `^>==!` | `LogicSystemArrayComparePlan<..., EqualWithValidity>` |
| `>^=` | only `SystemArrayMutationTransaction<...>`; never an ordinary descriptor |

Plan construction is pure. An eventual `evaluate(plan, read_capabilities)` operation must validate region provenance, bounds, layout, shape, address space and the required read capabilities before transferring data. The execution engine for this operation is not part of the current registry.

The assignment form `>^=` is only legal for a mutable place inside an active explicit mutation transaction holding independently granted read and write capabilities. The transaction must stage writes, validate its write set and document whether commit is all-or-nothing or may expose partial writes. An ordinary `SystemArray` descriptor is not mutable by operator syntax alone.

## Alias normalization

All historic forms denote one type/construct identity when used in an allowed SystemArray type or constructor position:

~~~text
system(array)   System(Array)   SYSTEM(ARRAY)
system_array    systemArray     SystemArray
SYSTEM_ARRAY    system-array    system.array
system::array   system/array    system\array
system[array]
~~~

Canonical source output is `system::array`; canonical type identity is `SystemArray<W,N,Space>`. Compatibility forms are parser aliases, not distinct storage models. The legacy bare `system` shorthand requires explicit `@allow_legacy_system_shorthand` in semantic source. Existing syntax-only frontends may continue recognizing the historical framed forms while the stricter typed frontend is introduced.

Prefix framing tokens are retained as source metadata. They never grant privileges or change execution. Suffix token IDs retain their historical stable mapping and are used by the new registry to identify an operator.

## Precedence and overload resolution target

The language specification defines maximal-munch tokenization. The intended precedence from high to low is:

1. postfix mutation assignment `>^=` (only inside a valid mutable transaction);
2. unary operators;
3. shifts and rotations: `^>`, `>^`, `^^>>`, `>>^^`, `^>>`, `>>^`;
4. bitwise XOR: `>!^`;
5. comparisons: `!==`, `!!==`, `!^>`, `^!>`, `^>==!`;
6. boolean AND;
7. boolean OR.

Assignments associate right; non-assignment operators associate left unless comparisons, which are non-associative. Chained comparisons are rejected; use explicit boolean conjunction. There is no implicit conversion between `Bit<N>`, `SBit<N>`, `Logic<N>`, and integer values.

Only one canonical built-in overload is selected by types, widths, address spaces and execution domain. Users cannot redefine reserved sigil token meanings. Missing or ambiguous overloads are compile errors. The C11 registry does not yet perform these checks.

## Current implementation boundary

Implemented in the PR branch:

- immutable metadata catalog for thirteen operators;
- exact suffix token lookup using the existing SystemArray suffix-marker IDs;
- bounded parse wrapper for one framed SystemArray sigil directive;
- stable JSON AST and catalog output;
- tests for all tokens, semantic IDs, transaction requirement and no-execution flags;
- CLI catalog, syntax-check and AST output.

Not implemented by this registry:

- full expression parsing or precedence parsing;
- type-directed overload resolution;
- capability checking or effect checking;
- plan evaluation, memory access or bus transfer;
- transaction execution or register mutation;
- Reaper IR lowering or machine-code generation.

The metadata registry must not be advertised as a working hardware operator engine.

## CLI

~~~sh
pytyliki-system-sigil --version
pytyliki-system-sigil --catalog
pytyliki-system-sigil --catalog-json
pytyliki-system-sigil --check "{]+/* systemArray ^>"
pytyliki-system-sigil --ast-json "systemArray ^>==!"
~~~

The AST explicitly reports `overload_resolution_performed: false`, `operator_execution_performed: false`, `hardware_access_performed: false` and `source_execution_performed: false`.

## Acceptance requirements for the semantic implementation

1. Parse all tokens using maximal-munch lexical rules and preserve source spans.
2. Resolve the exact overload from static operand types and generic width constraints.
3. Reject implicit signed/unsigned/four-state conversions.
4. Prove the width and vector-lane constraints before lowering.
5. Treat SystemArray plans as deferred reads and keep MMIO read side effects explicit.
6. Require separate read and write capabilities; no capability is inferred from syntax.
7. Permit `>^=` only on an active transactional mutable place.
8. Lower each operator to a stable semantic opcode, never to token spelling.
9. Preserve source `Span`, target features, memory space, effects, privilege and ownership in Reaper IR.
10. Test scalar semantics against reference vectors and differential target/emulator results before claiming executable support.
