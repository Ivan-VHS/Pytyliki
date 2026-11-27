# Pytyliki Iron Syntax v1

**Canonical owner:** Pytyliki  
**Compiler foundation:** Reaper  
**Status:** exact vocabulary and line-oriented syntax validation; semantic execution is not defined

## Purpose

Iron Syntax v1 records the twenty exact forms supplied for Pytyliki. It is a bounded syntax profile and interchange vocabulary, not a second processor instruction set. The spellings are preserved exactly, including `brachpass` and `verison`.

## Canonical record catalogue

| # | Exact form | Concept | Trailing marker | Family |
|---:|---|---|---|---|
| 0 | `begin iron )` | iron | `)` | boundary |
| 1 | `() core --` | core | `--` | section |
| 2 | `[ system ]-` | system | `]-` | section |
| 3 | `stop option ;` | option | `;` | control |
| 4 | `] level .` | level | `.` | control |
| 5 | `{ selective /` | selective | `/` | control |
| 6 | `} operation P)-` | operation | `P)-` | control |
| 7 | `?? massive <>` | massive | `<>` | modifier |
| 8 | `< start >?` | start | `>?` | boundary |
| 9 | `grown new-work >/ ` | new-work | `>/` | modifier |
| 10 | `> concrete //*` | concrete | `//*` | modifier |
| 11 | `## utility _` | utility | `_` | directive |
| 12 | `# unify TAB` | unify | `TAB` | directive |
| 13 | `setup brachpass ^^` | brachpass | `^^` | directive |
| 14 | `!+ hyper tournament ^` | hyper tournament | `^` | modifier |
| 15 | `CC based rules ^^^` | based rules | `^^^` | directive |
| 16 | `level jump to run *(` | jump to run | `*(` | control |
| 17 | `TRUE connective *+` | connective | `*+` | operator |
| 18 | `verison ///. *` | `///.` | `*` | metadata |
| 19 | `verificated /// !!` | `///` | `!!` | metadata |

The family labels group entries for tooling and do not define runtime effects.

## Grammar

```ebnf
iron_document = { blank_line | iron_record, line_ending } ;
iron_record = begin_iron | core | system | stop_option | level
            | selective | operation | massive | start | new_work
            | concrete | utility | unify | setup_brachpass
            | hyper_tournament | based_rules | level_jump_to_run
            | true_connective | verison | verificated ;
begin_iron = "begin", ws, "iron", ws, ")" ;
core = "()", ws, "core", ws, "--" ;
system = "[", ws, "system", ws, "]-" ;
stop_option = "stop", ws, "option", ws, ";" ;
level = "]", ws, "level", ws, "." ;
selective = "{", ws, "selective", ws, "/" ;
operation = "}", ws, "operation", ws, "P)-" ;
massive = "??", ws, "massive", ws, "<>" ;
start = "<", ws, "start", ws, ">?" ;
new_work = "grown", ws, "new-work", ws, ">/" ;
concrete = ">", ws, "concrete", ws, "//*" ;
utility = "##", ws, "utility", ws, "_" ;
unify = "#", ws, "unify", ws, "TAB" ;
setup_brachpass = "setup", ws, "brachpass", ws, "^^" ;
hyper_tournament = "!+", ws, "hyper", ws, "tournament", ws, "^" ;
based_rules = "CC", ws, "based", ws, "rules", ws, "^^^" ;
level_jump_to_run = "level", ws, "jump", ws, "to", ws, "run", ws, "*(" ;
true_connective = "TRUE", ws, "connective", ws, "*+" ;
verison = "verison", ws, "///.", ws, "*" ;
verificated = "verificated", ws, "///", ws, "!!" ;
ws = one_or_more(" " | "\\t" | "\\v" | "\\f") ;
line_ending = "\\n" | "\\r\\n" | "\\r" ;
```

Each non-empty line is exactly one record. Horizontal spacing between atoms is flexible; case and atom spelling are exact. Blank lines are ignored. Record order and repetition are preserved but not constrained because no ordering semantics were supplied. Unknown tokens, extra tokens and multiple records on one line are rejected.

## AST and limits

The parser emits `pytyliki.iron-syntax-ast.v1`, with a stable record name and identifier, exact phrase, concept, trailing marker, family and source location. Input is limited to 64 KiB and 4,096 records. The core parser uses caller-owned record storage and performs no allocation.

## Runtime and hardware boundary

This version formalizes spelling and structural recognition only. It does not define what a tournament, selective operation, level jump, system section or option does at runtime. A successful parse must not cause code execution, MMIO/DMA access, privilege changes, clock/voltage changes, instruction emission, RTL synthesis, driver loading or device writes. A parsed record is data, not authorization.

## CLI

```sh
pytyliki-iron --catalog
pytyliki-iron --check examples/iron.syntax
pytyliki-iron --ast-json examples/iron.syntax
pytyliki-iron --check - < examples/iron.syntax
```

The CLI reads a local file or standard input and validates syntax or writes a JSON AST.
