# Semantic Hardware Compiler v1

## Purpose and boundary

The Semantic Hardware Compiler is a bounded C11 semantic frontend for an explicitly small Pytyliki source subset. It resolves declarations, infers types, checks physical dimensions, enforces register access/effect contracts, constructs typed HIR and independently verifies that HIR.

It does **not** claim that all statements inside the 21 structural hardware constructs are now compiled. The v1 grammar below is the complete accepted source surface for this compiler slice. Other constructs are rejected with a source-located diagnostic rather than silently passed through.

## Pipeline

```text
.shw source
 -> bounded lexer
 -> declaration parser / expression parser
 -> symbol and type resolution
 -> dimension and unit normalization
 -> hardware effect inference / allowlist check
 -> typed topological Hardware IR
 -> HIR verifier
 -> Reaper-facing typed-HIR JSON envelope
 -> restricted pure C17 host lowering
```

The compiler has fixed limits: 64 KiB source, 8,192 tokens, 64 types, 512 HIR values, 192 symbols and 47-character identifiers/names.

## v1 source grammar

A module contains one module declaration, one target declaration, zero or more effect declarations, and value/register/operation declarations:

~~~pytyliki
module kinematics;
target host;

input distance: cm = 120;
input elapsed: s = 4;
derive speed: m/s = distance / elapsed;
derive travelled: m = speed * elapsed;
~~~

Supported top-level declarations:

| Form | Meaning |
|---|---|
| `module name;` | Stable module identity |
| `target host;` | Pure host C17 backend profile |
| `target sim;` | Simulation/reference target identity |
| `target riscv64;` / `target x86_64;` | Named target profile; no hardware emitter implied |
| `input name: unit = number;` | Typed input with a finite literal and explicit unit |
| `derive name: unit = expression;` | Typed derived expression |
| `register name: u32 access read_only;` | Fixed-width register descriptor |
| `allow mmio.read;` | Explicit effect allowlist entry |
| `operation name: u32 = mmio.read(register);` | Effectful HIR node, never executed by this frontend |
| `operation ack: unit = mmio.write(register, value);` | Effectful HIR node with checked width/access |

Expressions support finite numeric constants, earlier declared values, parentheses, unary negation, multiplication/division, addition/subtraction and left-to-right arithmetic precedence. Forward references, implicit numeric conversions, undeclared names, duplicate declarations and unsupported syntax are rejected.

## Type model and dimension vectors

Integer widths are represented explicitly (`i64`, `u32`, `u64`). `f64` is the floating numeric type. Physical quantities use an eight-component signed exponent vector:

`[length, mass, time, electric_current, temperature, amount, luminous_intensity, count]`.

| Unit | Dimension vector |
|---|---|
| m | [1, 0, 0, 0, 0, 0, 0, 0] |
| kg | [0, 1, 0, 0, 0, 0, 0, 0] |
| s | [0, 0, 1, 0, 0, 0, 0, 0] |
| Hz | [0, 0, -1, 0, 0, 0, 0, 0] |
| N | [1, 1, -2, 0, 0, 0, 0, 0] |
| J | [2, 1, -2, 0, 0, 0, 0, 0] |
| W | [2, 1, -3, 0, 0, 0, 0, 0] |
| C | [0, 0, 1, 1, 0, 0, 0, 0] |
| V | [2, 1, -3, -1, 0, 0, 0, 0] |
| count/s | [0, 0, -1, 0, 0, 0, 0, 1] |

Supported base/scaled units include m/cm/mm/km, kg/g, s/ms/us/ns, A, K, mol, cd, count, Hz, N, J, W, C and V. Scale prefixes normalize input literals to canonical base units before HIR is emitted. Unit exponents are bounded to [-16, 16]. Fixed-width integer literals are conservatively limited to exactly representable values in the v1 double-backed constant representation (up to 2^53-1; u32 retains its full range).

Type rules are strict: addition/subtraction require identical types and dimensions; multiplication adds dimension exponents; division subtracts dimension exponents; scalar operations retain only documented numeric types; implicit integer/floating conversions are rejected; known zero divisors and non-finite literals/results are rejected.

## Hardware entities and effects

A register descriptor has a fixed-width integer type and one access class: `read_only`, `write_only` or `read_write`. A register name cannot be read as an ordinary variable: access must be represented by `mmio.read` or `mmio.write`.

The effect vocabulary reserves memory read/write, MMIO read/write, DMA submit, IRQ, clock write and firmware write. In v1, the executable parser accepts explicit `allow mmio.read;` and `allow mmio.write;` declarations; other effect IDs are reserved metadata and do not imply an implemented operation.

The verifier recomputes observed effects from HIR and rejects disagreement with the module summary or an effect outside the allowlist. Read-only registers cannot be written; write-only registers cannot be read; write payload width and read result width must match the register descriptor.

No MMIO callback, address mapping, device access, DMA submission, IRQ installation, clock change or firmware write exists in this compiler slice. These opcodes are typed declarative IR only.

## Hardware IR and source provenance

The producer schema is `pytyliki.hardware-ir.v1`. The Reaper-facing interchange schema is `reaper.typed-hardware-ir.v1`. Both use stable value/type/symbol indices, topological operand references and byte/line/column source spans.

Every HIR value records its opcode, result type index, operand and symbol references, immediate constant where relevant, inferred effect label and source span. The independent verifier checks table bounds, type indices, non-forward operand references, operation-specific type invariants, source spans, register access, effect summaries and the declared effect allowlist.

The JSON shape is specified by `schemas/pytyliki.hardware-ir.v1.schema.json`.

## Reaper relationship

Reaper remains the canonical owner of language semantics, general source Span/SourceMap, typed IR foundations and shared ABI contracts. Pytyliki owns hardware-domain type descriptors, physical dimension vectors, target/register/effect semantics and domain HIR.

The `--reaper-ir` command emits a versioned interchange envelope with source spans and dimension/effect metadata intact. This envelope is **not** the existing Reaper scalar `CanonicalIR`, and no implicit cast into that scalar IR is performed. A Reaper-side import/validation adapter must accept the versioned schema and preserve typed metadata; only the pure scalar subset may later be lowered into the existing interpreter/VM IR. Effectful hardware nodes must stay outside the pure interpreter path.

## Lowering

The v1 C17 emitter accepts only `target host`, zero inferred effects, and `f64`/physical-quantity values. It emits one deterministic host function. It rejects MMIO, device effects, integer/register operations, untrusted target profiles and unsupported HIR opcodes. This is a verifiable pure numeric lowering slice, not an x86_64/RISC-V machine-code backend and not a firmware/RTL generator.

## Commands

~~~sh
pytyliki-semantic --version
pytyliki-semantic --check examples/semantic-hardware/kinematics.shw
pytyliki-semantic --ir-json examples/semantic-hardware/kinematics.shw
pytyliki-semantic --reaper-ir examples/semantic-hardware/kinematics.shw
pytyliki-semantic --emit-c examples/semantic-hardware/kinematics.shw
~~~

## Current maturity and next gates

| Area | v1 status |
|---|---|
| Bounded lexer and declaration/expression parser | Implemented for the grammar above |
| Scalar and quantity type checking | Implemented for the documented type slice |
| Unit scale normalization and dimension algebra | Implemented for the documented unit table |
| Register read/write access checks | Implemented as declarative descriptors/IR |
| Effect allowlist and inferred-effect verification | Implemented for MMIO read/write nodes |
| Typed HIR and source-span verification | Implemented for v1 nodes |
| Reaper typed-HIR interchange writer | Implemented; consumer import is a separate integration |
| Pure C17 quantity/f64 backend | Implemented for the explicitly supported subset |
| General hardware construct-body semantics | Not implemented |
| Ownership/borrow/region analysis for hardware resources | Not implemented |
| Native ISA/object/ELF backend | Not implemented by this change |
| RTL/netlist generation and timing analysis | Not implemented by this change |
| Physical/nuclear simulation engine | Not implemented by this change |
| Physical MMIO, DMA, IRQ or firmware execution | Not provided |

The next gates are Reaper-side schema import and metadata preservation, broader expression/type/ownership/effect semantics, canonical Hardware IR to Reaper typed-IR lowering for pure operations, ISA architectural models and a deterministic emulator, then target-specific object and RTL backends with independent conformance tests.
