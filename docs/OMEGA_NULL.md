# Omega-Null — Pytyliki IRON research profile

> **Research profile v0.0.1 · 2026-11-28**
>
> Omega-Null is a physics-inspired experimental profile inside Pytyliki IRON CORE. It preserves the `begin iron )` visual language and the original resonance markers while assigning them explicit, typed, testable meanings.
>
> **It is not a claim that software can control the quantum vacuum, rewrite spacetime, reverse physical time, create matter from thought or bypass physical law.** Such statements remain metaphor or speculative fiction, not executable operations.

## 1. What Omega-Null is

Omega-Null explores how one coherent programming surface can describe the deepest engineering layers Pytyliki intends to study:

- bits, architectural state and machine instructions;
- digital logic, timing, circuit and electrical models;
- quantum circuits and measurement models;
- numerical field and spacetime-model simulations;
- material/atomic-scale models;
- reproducible computational experiments;
- CAD, RTL, EDA and manufacturing plans;
- consent-based sensor and brain-computer-interface adapters.

The phrase **"absolute bottom"** is an engineering aspiration, not a literal level below physics. A computer program runs on physical devices and cannot operate beneath their physical substrate. The deepest practical software-facing layer is a declared device/electrical model or a real, documented hardware interface.

Omega-Null therefore has five semantic layers:

| Layer | Name | Meaning |
|---|---|---|
| O0 | Marker surface | The resonant symbols and visual block structure |
| O1 | Typed research operations | Explicit quantum, field, material, experiment, time and BCI operations |
| O2 | Model and simulator | Bounded deterministic or seeded probabilistic models |
| O3 | Artifact and tool adapters | IR, netlists, RTL, CAD, EDA and manufacturing job data |
| O4 | Physical target | A named, qualified device or laboratory target behind a trusted adapter |

A marker is never a secret hardware instruction. It normalizes into a documented operation, then passes Reaper's type/effect checks and Pytyliki's target admission.

## 2. Ownership and compiler boundary

- **Reaper** owns canonical grammar, parsing, type/effect semantics, diagnostics, `Span`/`SourceMap`, typed IR and compiler passes.
- **Pytyliki** owns the Omega-Null research profile, physics/hardware model descriptors, adapters, simulation orchestration, evidence and qualification.
- **Duck OS** mediates process isolation, protected device access, sensors, BCI devices, driver lifecycle and privileged services.
- **Board and laboratory backends** supply exact board, quantum-device, materials, manufacturing and EDA tool interfaces, including calibration and revision metadata.

Omega-Null must not fork Reaper's language grammar or invent a parallel IR. Its markers map into canonical Reaper AST/typed IR nodes plus explicit effects. A syntax prototype is not considered an executable frontend until the pinned Reaper checker accepts it and golden Span/IR tests pass.

## 3. Resonance marker dictionary

The original marker spellings are retained as a visual layer. The normalized names below define the intended engineering meaning.

| Original marker | Normalized operation | Concrete meaning |
|---|---|---|
| `begin iron )` | `iron.begin` | Start a bounded session with target, mode, effect budget and provenance |
| `() core --` | `core.scope` | Open a root scope with versioned initial state |
| `[ system ]-` | `system.bind` | Bind a named CPU, simulator, device, board, model or lab target |
| `stop option ;` | `stop.policy` | Declare timeout, cancellation, safe-stop and cleanup |
| `] level .` | `model.level` | Select transistor, gate, circuit, ISA, microarchitecture, device or system model resolution |
| `{ selective /` | `select.filter` | Apply a typed predicate, threshold or constraint to candidates |
| `} operation P)-` | `operation.apply` | Execute or simulate an admitted operation under declared effects |
| `?? massive <>` | `data.region` | Allocate a bounded state-vector, tensor, memory or dataset region |
| `< start >?` | `resource.query` | Read-only query of a declared model or resource provider |
| `grown new-work >/` | `pipeline.expand` | Expand bounded child jobs, generated source or model steps |
| `> concrete //*` | `artifact.materialize` | Emit source, IR, netlist, RTL, CAD or a manufacturing plan |
| `## utility _` | `utility.import` | Resolve a versioned local utility module or pinned adapter |
| `# unify TAB` | `timeline.align` | Align logical clocks, signal phases or event streams |
| `setup brachpass ^^` | `experiment.branch_plan` | Prepare bounded branch-and-bound or search paths |
| `!+ hyper tournament ^` | `experiment.tournament` | Evaluate candidates in parallel on finite compute resources |
| `CC based rules ^^^` | `physics.constraints` | Enforce declared units, equations, conservation laws and boundary conditions |
| `level jump to run *(` | `workflow.transition` | Perform a validated state-machine or pipeline transition |
| `TRUE connective *+` | `state.entangle_model` | Represent correlated quantum subsystems in a model or qualified device |
| `verison ///. *` | `artifact.version` | Attach version, digest, model identity and reproducibility metadata |
| `verificated /// !!` | `verify.seal` | Run named checks and emit a scoped verification report |

The odd spelling `verison` is retained for compatibility with the submitted surface. A future spelling correction must be a versioned alias, not a silent grammar change.

## 4. Operation model

Every operation must declare:

1. a target kind and target identity;
2. allowed execution modes;
3. input and result types;
4. memory/device/model effects;
5. required capability, where applicable;
6. deterministic or probabilistic behavior;
7. resource, time and memory bounds;
8. evidence mode and provenance;
9. failure and safe-stop behavior.

### Initial research operations

| Operation | Intended purpose | Critical boundary |
|---|---|---|
| `quantum.state.prepare` | Prepare a declared state using a circuit or calibrated device procedure | Does not promise an arbitrary state on every device |
| `quantum.gate.apply` | Apply a supported gate in a simulator or device adapter | Gate set, connectivity and noise are target-specific |
| `quantum.measure` | Measure in a declared basis and return an observation/distribution | Outcomes are probabilistic; no API can force a desired result |
| `field.solve` | Solve declared field/evolution equations on a bounded numerical domain | Simulates a model; does not bend real spacetime |
| `experiment.tournament` | Run parallel, resource-bounded candidate evaluations | Uses finite compute resources, not alternate physical realities |
| `sandbox.create` | Create a process, VM or simulator sandbox with quotas | A sandbox is isolation, not a separate universe |
| `chrono.checkpoint` | Capture a versioned logical/model state | Does not snapshot the physical world |
| `chrono.replay` | Reproduce a recorded model execution or event log | Does not reverse real-world time or external events |
| `material.structure.build` | Construct an atomic/molecular/material model | Produces model data, not a fabricated object |
| `fabrication.plan` | Generate a constrained CAD/EDA/3D-printing or manufacturing plan | Real production needs feedstock, equipment, energy and operator approval |
| `bci.capture` | Read a selected BCI/sensor stream through an OS-mediated adapter | Requires informed consent and visible activation |
| `bci.intent.decode` | Classify user-calibrated signal features into intent hypotheses | Probabilistic hypothesis, not direct access to thoughts |

All twelve operations are research/planned interfaces. Their inclusion in the registry does not mean that the corresponding hardware or simulator backend is shipped.

## 5. Translating the reality-manipulation examples into engineering

### 5.1 `collapse_wavefunction(target)`

**Retained concept:** quantum state preparation, gates and measurement.

**Correct operation:** `quantum.measure(target, basis)` returns an observed outcome and, in simulation, a distribution with a named random seed and noise model. Measurement is not a command to select an arbitrary preferred result. State preparation and measurement are distinct operations.

It cannot be used to retrieve a password by putting all possible passwords into superposition and asking the system to choose the correct one.

### 5.2 `bend_space_time(coords, factor)`

**Retained concept:** numerical field/geometry research.

**Correct operation:** `field.solve(model, domain, boundary_conditions, tolerance)` solves a declared numerical model. Inputs must declare equations, dimensions, coordinate system, discretization, units, boundary conditions and solver limits.

It cannot compress petabytes into an atom, alter the actual gravitational field or slow real execution time. Storage optimization remains a memory/data-layout problem.

### 5.3 `hyper_tournament_select(algorithms)`

**Retained concept:** broad parallel search over candidate algorithms.

**Correct operation:** `experiment.tournament(candidates, objective, budget, seed)` schedules real candidate runs or simulations. It records compute limits, inputs, failures, measurements and confidence, then returns a reproducible winner or an explicit no-winner result.

It does not create alternate universes. More candidates require more actual compute capacity; model results are only as reliable as their assumptions.

### 5.4 `materialize_thought(concept)`

**Retained concept:** turning an abstract design into a concrete artifact.

**Correct operation:** `artifact.materialize(design, backend)` can emit a design file, netlist, RTL, CAD model, atomistic structure or manufacturing job plan. A qualified manufacturing adapter may submit a plan only after an explicit operator action.

A physical object requires matter/feedstock, energy, suitable equipment and a validated process. The relation (E=mc^2) is not a practical API for turning ordinary electricity into arbitrary quantities of material.

## 6. The four proposed frameworks

### Aegis-Shield — execution and model safety

| Original component | Pytyliki implementation direction |
|---|---|
| Reality Sandbox | Process/VM isolation or deterministic simulator sandbox |
| Entropy Monitor | Model-specific numerical error, stability, queue pressure or entropy telemetry |
| Causality Lock | Validate causal event order and reject inconsistent replay mutations |
| Stop Policy | Bounded timeout, cancellation, cleanup and safe-stop ledger |

In technical APIs, prefer **Model Sandbox** over Reality Sandbox. A sandbox's isolation strength must name the actual OS or simulator mechanism.

### Chrono-Weave — logical time and reproducibility

| Original component | Pytyliki implementation direction |
|---|---|
| Time Bookmark | Versioned model/application checkpoint |
| Future Preview | Scenario or Monte Carlo forecast with assumptions and uncertainty |
| Parallel Thread Manager | Bounded worker pool and deterministic branch scheduler |
| Rollback | Restore an isolated logical snapshot |

Chrono-Weave can replay a simulation. It cannot undo an external event, reverse physical time or guarantee a future prediction.

### Matter-Forge — material models and fabrication

| Original component | Pytyliki implementation direction |
|---|---|
| Atom Library | Versioned element/isotope/species and material-property database |
| Structure Blueprint | Atomic/molecular geometry, crystal lattice, CAD or device design |
| Energy Converter | Dimensioned process-energy model and budget |
| Fabrication Adapter | Machine-specific, reviewed manufacturing workflow |

No matter-from-vacuum operation is defined. Each material model must identify its assumptions and source data; a manufacturing plan must identify feedstock and the actual machine/process.

### Mind-Link — neural input

| Original component | Pytyliki implementation direction |
|---|---|
| Thought Parser | Signal processing and user-calibrated intent classification |
| Emotion Stabilizer | Replace with signal-quality checks, neutral calibration and user pause controls |
| Focus Amplifier | Optional user-controlled interface assistance, without claims of changing cognition |
| Privacy Gate | Local-first processing, explicit consent, minimal retention and deletion controls |

BCI acquisition requires a named supported device, OS-mediated permissions, clear user consent and an obvious active state. No hidden recording, emotion suppression or guaranteed thought-to-code translation is permitted.

## 7. Evidence and verification

Omega-Null defines distinct evidence modes:

- `static_analysis`
- `reference_fixture`
- `deterministic_simulation`
- `probabilistic_simulation`
- `live_os_observation`
- `qualified_device_measurement`
- `hardware_in_loop`
- `physical_signoff`

A report must include source revision, model/device identity and revision, toolchain version, input digest, random seed or device-run ID, timestamp, units, uncertainty, result and limitations.

Additional domain requirements:

- Quantum runs identify state representation, gate set, noise model, measurement basis and seed/device provenance.
- Field runs identify equations, units, discretization, boundary conditions and solver tolerance.
- Material runs identify species, model/potential, temperature/pressure assumptions and source database.
- Fabrication runs identify feedstock, process, machine profile and operator approval.
- BCI runs identify consent state, device/provider, sampling metadata and privacy/retention policy.

A simulator pass is never relabeled as live hardware evidence. A model of a transistor is never relabeled as direct access to the transistors in a commercial CPU.

## 8. Access and safety invariants

The default mode is `inspect`. Supported mode names align with IRON CORE: `inspect`, `plan`, `simulate`, `verify`, `target`, `privileged` and `physical_design`.

- No implicit hardware access.
- No hidden network activity.
- No automatic firmware or driver installation.
- No generic "reality mutation" API.
- Privileged device operations require an exact target profile and narrow capability.
- Unsupported semantics return an explicit unsupported result.
- All simulations are bounded by time, memory, operation and recursion budgets.
- Nondeterministic models carry an explicit seed or device-run identity.
- Causality, conservation and dimensional constraints are modeled as invariants, not optional decoration.
- Sensitive BCI signals remain local by default and require informed consent.

## 9. Illustrative surface

The following block preserves the visual Omega-Null language. It is a **design sketch**, not a parser-accepted program:

```omega
begin iron )
() core --
    [ system ]-
        MODE = SIMULATE
        TARGET = QUANTUM_STATE_VECTOR
        SEED = 481516
        BUDGET = BOUNDED
    stop option ;

    ] level . QUANTUM_MODEL

    ?? massive <>
        STATE = PREPARE_STATE(CIRCUIT, MODEL)
    < start >?

    { selective /
        VALIDATE_GATE_SET(CIRCUIT, MODEL)
    } operation P)-

    !+ hyper tournament ^
        EVALUATE_CANDIDATES(
            CANDIDATES,
            OBJECTIVE,
            BUDGET,
            SEED
        )

    CC based rules ^^^
        CHECK_UNITARITY()
        CHECK_RESOURCE_LIMITS()
        CHECK_MODEL_PROVENANCE()

    > concrete //*
        EMIT_ARTIFACT("omega-null-report.json")

    verificated /// !!
end iron )
```

The eventual implementation is a Reaper-owned frontend/grammar profile with Omega-Null marker normalization and typed effect nodes. Pytyliki supplies the models/backends. The sketch above must not be presented as executable until the canonical Reaper checker and golden AST/IR/Span tests accept it.

## 10. Implementation roadmap

1. **Profile contract:** keep this marker dictionary, operation registry, claim translations, capability model and schema versioned.
2. **Reaper normalization:** add the grammar/profile and AST/typed-IR effects upstream in Reaper; retain original marker spans and diagnostics.
3. **Model-only core:** implement bounded experiment orchestration, model sandbox, checkpoint/replay and evidence metadata in Pytyliki.
4. **Quantum/field adapters:** add reference models and seeded simulation, then differential tests against independently specified fixtures.
5. **Material pipeline:** add versioned species/material models and design artifacts; only later integrate actual CAD/manufacturing tools.
6. **BCI adapters:** require an explicitly supported device, Duck OS permissions, user consent and privacy review.
7. **Hardware qualification:** introduce named lab targets and operator-approved tests; do not generalize one device result to all hardware.

Each stage is a separate reviewable PR with tests and explicit unsupported behavior. The initial Omega-Null profile remains research-only.

## 11. Extended research domains and three verification gates

The experimental concepts from the wider IRON surface are assigned to explicit research modules rather than represented as magical runtime operations.

| Research term | Module | Defined engineering scope |
|---|---|---|
| `quantum.bit` | `pyty.quantum` | Qubit, state-vector, circuit and measurement models; qualified device adapters remain separate |
| `neuro.point` | `pyty.bci` | Neuromorphic network models and consented neural signal adapters |
| `liquid.ram` | `pyty.material` | Research models for fluidic, phase-change or other emerging memory concepts; no assumed shipping device |
| `bio.sync` | `pyty.bci` | Timestamp and synchronize biosignal streams with clock provenance and uncertainty |
| `holo.stack` | `pyty.field` | Three-dimensional scene/spatial visualization; real holographic output requires a named display |
| `zero.lat` | `pyty.time` | Latency budgets and lower-bound measurement; not literal zero latency |
| `energy.syn` | `pyty.power` | Dimensioned energy and power models/measurements |
| `mind.hive` | `pyty.experiment` | Multi-agent or distributed simulation and search; not telepathy or shared consciousness |
| `dim.jump` | `pyty.sim` | Change simulation resolution or model layer; not travel between physical dimensions |

Every Omega-Null run follows three separate gates:

1. **Syntax/type/effect gate:** parse, type-check, preserve source spans and verify targets, effects and capabilities.
2. **Model/invariant gate:** check units, conservation constraints, numerical stability, resource budgets and evidence provenance.
3. **Physical-target gate:** when a real device is involved, verify its exact identity/revision, calibration, approved procedure and measured behavior. A simulator cannot satisfy this gate.

Every experiment also declares wall-clock, compute, memory, output and recursion/branch limits plus cancellation/watchdog behavior. Checkpoints apply only to logical application or model state. Forecasts remain uncertainty-bearing scenarios, not guaranteed future knowledge.

## 12. Source of truth

- [Machine-readable profile](../omega-null/omega-null.v0.json)
- [JSON Schema](../schemas/omega-null.v0.schema.json)
- [IRON CORE parent contract](../IRON_CORE.md)

Omega-Null's ambition is to push the boundary of what Pytyliki can model, inspect and verify. Its credibility comes from explicit physical semantics and reproducible evidence—not from claiming that a programming language can suspend the laws of nature.
