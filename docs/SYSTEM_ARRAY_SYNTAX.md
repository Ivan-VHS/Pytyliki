# Pytyliki \`system(array)\` syntax, version 1

## Status and scope

This is a syntax-front-end contract, not an executable hardware API. The native
C11 parser recognizes one framed directive, normalizes it to \`system(array)\`,
and emits a stable AST record. It does not allocate memory, map addresses,
access devices, execute code, or infer hardware permissions.

## Canonical spelling

\`\`\`pytyliki
system(array)
\`\`\`

The canonical spelling is case-insensitive for the \`system\` and \`array\` words.
The following aliases normalize to the same AST kind, \`system.array\`:

\`\`\`text
system(array)   System(Array)   SYSTEM(ARRAY)
system_array    systemArray    SYSTEM_ARRAY
system-array    system.array    system::array
system/array    system\\array    system[array]
\`\`\`

\`system\` by itself is a legacy shorthand. It is accepted only when accompanied
by one of the documented framing markers. It always normalizes to
\`system(array)\` and is reported with \`shorthand: true\`.

## Framing markers

The punctuation sequences in the design specimen are treated as explicit
framing markers, not operators. A marker is syntax metadata; it has no runtime
meaning and cannot grant privileges, alter execution order, change memory
ordering, or select a hardware device.

The parser recognizes these prefix markers:

\`\`\`text
{]+/*+?$  {]+/*  */+?  {]/*  $+?/  *]+$  {+/*?  ?+$}
[/+*]    {?$+*  *?{/+  {/*+?$  ]+?*/  $/*+}  {]?$+
\`\`\`

And these suffix markers:

\`\`\`text
^>  !==  >^  !!==  ^^>>  >>^^  ^>>  >>^  >^=
!^>  ^!>  >!^  ^>==!
\`\`\`

Markers are matched exactly and are not comments. Unknown punctuation around a
directive is rejected rather than silently discarded.

## Grammar

\`\`\`ebnf
system_array_directive = whitespace, [prefix_marker, whitespace],
                         system_array_name,
                         [whitespace, suffix_marker],
                         [whitespace, ";"], whitespace, EOF ;

system_array_name      = canonical_name | alias | framed_bare_system ;
canonical_name         = "system(array)" ;
alias                  = "system_array" | "systemArray" | "system-array"
                       | "system.array" | "system::array" | "system/array"
                       | "system\\array" | "system[array]" ;
framed_bare_system     = "system" ;
\`\`\`

\`framed_bare_system\` requires at least one prefix or suffix marker. The parser
is ASCII case-insensitive for aliases and whitespace is permitted between the
prefix marker, alias, suffix marker and optional semicolon.

## AST contract

Successful parsing produces \`pytyliki.system-array-ast.v1\`:

\`\`\`json
{
  "schema": "pytyliki.system-array-ast.v1",
  "kind": "system.array",
  "canonical": "system(array)",
  "spelling": "systemArray",
  "alias": "camel",
  "prefix_marker": "{]+/*",
  "suffix_marker": ">^",
  "shorthand": false,
  "source_execution_performed": false
}
\`\`\`

The original parser treats the suffix marker as syntax metadata. The opt-in hardware extension adds a separate [SystemArray sigil registry](SYSTEM_SIGIL_OPERATORS.md): it maps the same stable suffix-marker IDs to thirteen semantic operator identities. This mapping is also metadata-only; it does not perform overload resolution, evaluation or hardware access.

The AST preserves the original alias family and framing markers for tooling,
while semantic identity is always \`system.array\`. Marker metadata must not be
interpreted as a capability or an execution instruction.

## Rejected forms

The parser rejects unknown aliases, unknown markers, incomplete directives,
trailing tokens, unframed bare \`system\`, and inputs longer than 512 bytes.
It does not accept a declaration body yet. Array element types, capacities,
layout, ownership, address spaces and access rights belong to a future typed
\`system.array\` declaration grammar; they must not be guessed by this header
normalizer.

## CLI

\`\`\`sh
pytyliki-system-array --check '{]+/* systemArray >^'
pytyliki-system-array --normalize 'system_array'
pytyliki-system-array --json 'system(array) ^>'
\`\`\`

The CLI is a bounded syntax validator only. It never executes source or touches
hardware. `examples/system_array.syntax` is a conformance corpus: each non-empty
line is an independent directive sample, not a multi-statement program.
