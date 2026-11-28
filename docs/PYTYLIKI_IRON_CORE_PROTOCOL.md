# Pytyliki Iron Core Protocol

**Specification:** Iron Core v1.0  
**Implementation:** bounded native C11 structural validator and AST catalogue  
**Language owner:** Pytyliki  
**Semantic/compiler foundation:** Reaper  
**Maturity:** structural validation and marker catalogue; no code generation or execution

## Purpose

Iron Core gives Pytyliki processor-development and repository-rollout source a versioned structural vocabulary. The twenty spellings below are exact source tokens. The historical spellings "verison" and "brachpass" are preserved rather than silently normalized.

A marker selects a structural or declarative role. It is not an instruction to the host, operating system, processor, peripheral or GitHub service.

## Marker catalogue

| Token | Semantic identifier | Current role |
|---|---|---|
| begin iron ) | module.begin | Opens the module root |
| () core -- | core.context | Core declaration block |
| [ system ]- | system.context | System declaration block |
| stop option ; | block.end | Closes the most recent block |
| ] level . | level.context | Declares a requested level, without changing privilege |
| { selective / | selection.filter | Filtering and routing plan |
| } operation P)- | operation.unit | Operation declaration |
| ?? massive <> | work.massive | Bounded bulk-work plan |
| < start >? | startup.plan | Startup metadata, not a boot action |
| grown new-work >/ | work.spawn.plan | Work-queue declaration |
| > concrete //* | implementation.binding | Concrete implementation binding |
| ## utility _ | utility.helper | Helper declaration |
| # unify TAB | layout.unify | Tabular layout declaration |
| setup brachpass ^^ | branchpass.plan | Offline branch-analysis hint |
| !+ hyper tournament ^ | tournament.rank | Deterministic candidate-ranking plan |
| CC based rules ^^^ | consistency.rules | Consistency-policy declaration |
| level jump to run *( | execution.jump.plan | Control-flow plan, not an executed jump |
| TRUE connective *+ | logic.true_connective | Boolean connective declaration |
| verison ///. * | module.version | Version label |
| verificated /// !! | source.verification_claim | Untrusted source-authored seal label |

## Framing and limits

A source document contains one root, one or more version labels, at least one consistency-policy label, one or more blocks, and a final source-authored verification claim. Version and policy identifiers accept ASCII letters, digits, underscore, dot and hyphen. Block markers occupy their own logical line. Blocks may nest to depth 32. A document may contain at most 64 blocks and one MiB of source. Strings, line comments and block comments are skipped while locating markers. Every open block must have a matching stop marker.

The parser records byte offsets and source line numbers. It does not parse statements inside a block. Structural success is not semantic correctness.

## Ownership

Pytyliki owns token spelling, source framing, the marker catalogue and the hardware-oriented AST. Reaper owns language semantics, canonical typed IR, type/effect analysis, source maps, lowering and ABI contracts. A future adapter must map stable semantic identifiers to Reaper IR nodes and preserve source spans.

## Verification and safety

The "verificated" marker is a source claim only. It is never treated as a passing build, signed artifact, approved review, trusted repository state or merge authorization. Those facts must come from CI checks, review records, commit identity and repository policy.

The current implementation only reads bytes and emits a structural AST. It does not compile or execute instructions, change privilege, write predictor state, map memory, access MMIO, submit DMA, load drivers, flash firmware, change clocks or voltage, spawn workers, call GitHub, or merge pull requests.

## Native interface

The C11 API is in src/iron_core.h and the CLI target is pytyliki-iron:

    pytyliki-iron --version
    pytyliki-iron --catalog
    pytyliki-iron --catalog --json
    pytyliki-iron --check examples/iron/01_sequential_pr_rollout.iron
    pytyliki-iron --ast-json examples/iron/01_sequential_pr_rollout.iron

The JSON AST is defined by schemas/pytyliki.iron-core-ast.v1.schema.json.
