# Pytyliki speculative hardware profiles

**Status:** reference dialect examples, not a new executable language mode  
**Structural frontend:** existing C11 hardware-construct parser  
**Canonical compiler foundation:** Reaper typed IR / ABI contracts

## 1. Scope

Two compound profiles have been added as design specimens:

- [Hyper-Switch Reality Synthesizer](../examples/profiles/01_hyper_switch_reality_synthesizer.syntax) — a systems/audio workload graph joining scheduling, bounded data preparation, callback gates, parallel work, authenticated connectivity, audio processing and integrity recovery.
- [Epoch 4M Vacuum Lattice](../examples/profiles/02_epoch_4m_vacuum_lattice.syntax) — a speculative processor profile with register-bank declarations, target memory regions, phase-labeled operations, bounded sweeps, invariants and read-only tracing.

Both files use one outer `dag` construct so that the current structural frontend can validate the source envelope, comments, strings and delimiter nesting. **The nested declarations and marker spellings are not yet parsed as individual language constructs.** Passing `pytyliki-hw-syntax --check` is not semantic validation, compilation, simulation or hardware execution.

The profile descriptions are fictional/speculative. Names such as *quantum*, *vacuum lattice*, *chronon* and *star-core* are labels in a design scenario; they do not claim physical quantum computation, vacuum-energy extraction or a realizable 4,000,000 AD substrate.

## 2. Hyper-Switch mapping to existing Pytyliki constructs

| Profile area | Intended language model | Required future contract |
|---|---|---|
| Reality-layer and state labels | `contract`, typed enums, configuration data | State is symbolic; no implicit device or privilege transition |
| Tournament scheduler | `schedule` + `table` + `fn` | Deterministic tie-break, bounded queue, measured cost units |
| Data forge | `stream` + storage region | Output-capacity bounds, authenticated chunk encryption, key handles |
| Selective operation gates | `rule` + typed callbacks | Callback signatures, effects, cleanup and error propagation |
| Parallel engine | `dag` + `schedule` | Worker count derived from available workers; cancellation and bounded merge |
| Remote connectivity | `device` / `react` | Trusted authenticated-channel implementation and explicit network capability |
| Sound reality engine | `stream` + `fn` | Sample rate, channel layout, latency budget and backend ownership |
| System integrity and main loop | `invariant` + `fsm` | Safe-stop state, bounded recovery and verified transition rules |

### Corrections required before semantic implementation

1. In the supplied sketch, `promote_winner` references `max_impact` outside the scope where it is computed. Return a `TOURNAMENT_RESULT` containing both the winner and score, then pass that result to promotion and logging.
2. `energy_cost` is measured in joules, while the penalty is described as latency. Keep energy and latency as distinct typed quantities and only combine normalized cost-model values through an explicit policy.
3. A fixed 128-worker pool is not evidence that 128 workers exist. Bound the actual worker count by platform capacity, split count and resource budget; define behavior for empty or undersized input.
4. A 4096-byte payload cannot safely receive an unspecified compressed output. Compression must accept an output capacity and return a checked length or an explicit overflow result.
5. Alignment must be provided by an allocator/region contract; changing a payload pointer with an unchecked alignment macro can invalidate ownership or bounds.
6. Per-slice encryption needs authenticated encryption, unique nonces, key-handle ownership and failure propagation. Do not define a home-grown cipher or expose raw key material.
7. Priority boosts, branch-predictor controls, DAC writes, DMA, driver loading and recovery actions require target-specific capabilities and platform services. Syntax alone must never perform them.

## 3. Epoch 4M marker vocabulary

The supplied token spellings are retained in comments in the reference example. The proposed grouping is:

| Marker spelling | Proposed role | Status |
|---|---|---|
| `verison ///. *` | Version/profile identity | Unspecified; spelling retained for compatibility discussion |
| `verificated /// !!` | Build/profile attestation label | Not a cryptographic verification operation |
| `CC based rules ^^^` | Constraint/coherence policy header | Semantics and scope TBD |
| `& system = tray,overlay+;` | Resource overlay directive | Declaration only; no storage mutation |
| `& system = overload"load"save+;` | Load/save policy directive | Declaration only; persistence contract TBD |
| `begin iron )` / `stop option ;` | Compound profile framing | Not yet part of the parser grammar |
| `() core --`, `[ system ]-`, `] level .` | Core/system/privilege sections | Target capability and privilege semantics TBD |
| `{ selective /`, `} operation P)-` | Selection and operation blocks | Must become typed control/effect constructs |
| `?? massive <>`, `< start >?`, `grown new-work >/ ` | Workload, start and spawn directives | Bounded task and admission semantics TBD |
| `> concrete //*`, `## utility _`, `# unify TAB` | Data, utility and formatting sections | Descriptive sections only |
| `setup brachpass ^^`, `!+ hyper tournament ^` | Branch-hint and tournament policy | Target backend hooks, not guaranteed CPU controls |
| `level jump to run *(`, `TRUE connective *+` | Context transition and conjunction | Typed capability/boolean contracts required |

The marker forms are a proposed vocabulary, not aliases for machine instructions. The parser does not yet tokenize or assign semantics to these exact spellings.

## 4. Epoch 4M technical corrections and safety boundaries

- `0xP400` is not a valid hexadecimal integer because `P` is not a hexadecimal digit. The reference uses the string identifier `"P400"`; a future opcode namespace should be a typed symbol or target-defined enum.
- `regit`, `VACUUM_BUS_0`, `PHASE_RING`, and all instruction mnemonics are target-defined names. They must be declared by a selected target profile before type checking or lowering.
- Every absolute address must resolve through a non-overlapping, overflow-checked `memory_map`; placeholder addresses do not authorize physical memory access.
- The arena loop needs a checked relationship between iteration count, stride, and declared region size. The sample makes the stride explicit and keeps the scan read-only.
- `phase_lanes`, latency, chronon jitter and star-flux values are profile metadata until supported by measured target data. No timing or physical-energy guarantee follows from the literal values.
- A privilege ring cannot be selected by source text. Effective privilege is supplied by the execution environment and checked against the target policy.
- Branch predictor hints and phase-fold operations are optional target features. Unsupported features must produce a diagnostic, not silently emit a guessed opcode.
- Debug tracing is read-only by default. No firmware update, microcode change, voltage/clock change, MMIO write, DMA submission or live patch is performed by this frontend.

## 5. Implementation sequence

1. Define a grammar for compound profile framing and each marker family, with stable token IDs and source spans.
2. Specify typed nodes for resource overlays, load/save policy, phase profiles, privilege requirements, spawn/admission and branch hints.
3. Add type/effect/capability validation and target-profile resolution.
4. Lower supported operations to Pytyliki Hardware IR, then use the Reaper canonical typed IR and ABI adapter where applicable.
5. Add negative tests for unknown opcodes, invalid addresses, privilege escalation, unbounded parallelism, missing output capacity, and unsupported target features.

Until those stages exist, these profiles remain structurally checked design references—not compiled programs.
