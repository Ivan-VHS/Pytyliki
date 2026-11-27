# Pytyliki Processor Fabric (PPF) v1

**Status:** bounded C11 inventory, catalogue aggregation, graph validation and advisory process-pool planner  
**ABI major:** 1  
**Report schema:** `pytyliki.processor-fabric-report.v1`  
**Runtime default:** read-only; planning only; no OS scheduler or hardware control  
**Compiler boundary:** Reaper remains the canonical typed-IR, Span/SourceMap and ABI foundation

## 1. Purpose

Processor Fabric is the central aggregation and coordination layer for the processor-engineering subsystems in Pytyliki. It turns separately produced CPU topology, architecture profiles, register/ISA catalogues, address-space descriptions, process pools, resource relationships and verification artifacts into one bounded, versioned model.

It is not a replacement for the operating-system scheduler, an instruction decoder, an MMU/IOMMU, a device driver or an assembler. It is the inventory and planning layer that lets those independently owned components describe their objects and relationships consistently.

The target architecture is **one processor-system graph, multiple explicit execution domains, stable identifiers and evidence-backed plans**.

## 2. Consolidation of the requested directions

The repeated descriptions reduce to twenty complementary capabilities:

| Requested direction | PPF responsibility |
|---|---|
| Machine-oriented structural collection | Typed processor-system node inventory |
| Hardware-oriented architectural cataloguing | Target, ISA, feature and architecture-profile references |
| Machine infrastructure aggregation | Aggregation of existing processor catalogues |
| Hardware-complex consolidation | One validated graph of hardware and software objects |
| Assembler-centralized accumulation | ISA profiles, register-bank references and produced build artifacts |
| Native integrated accumulation | C11 API and caller-owned report buffers |
| Direct fundamental union | Stable IDs and explicit graph relations |
| Fundamental base centralization | Versioned ABI, schema and bounded validation |
| System-wide systematization | Node lifecycle and state accounting |
| Register-platform cataloguing | Register-bank nodes tied to architectural owners |
| Microarchitecture modular orchestration | Execution units and named execution domains |
| Processor hierarchy coordination | Package → Die → Core → Logical Processor |
| Microprocessor dispatch | Deterministic worker-selection plan |
| Bit-level concentration | Feature masks and domain-aware admission |
| Address-space grouping | Address-space and memory-region nodes |
| Instruction integration | ISA-profile nodes and feature requirements |
| System process management | Process nodes and dependency edges |
| Hardware functional aggregation | Typed uses/maps-to/produces relationships |
| Code-generation packaging | Artifact nodes linked to producing processes |
| Near-machine-code processor-pool formation | Explicit process-pool nodes and resource-bounded worker assignments |

The terms are not twenty independent runtimes. They are views over the same model.

## 3. Object hierarchy

The v1 node catalogue contains thirteen object kinds:

- **Physical hierarchy:** package, die, core, logical processor.
- **Architecture resources:** register bank, ISA profile, address space, memory region, device, execution unit.
- **Process organisation:** process pool, process, artifact.

Parent links express containment and ownership. For example, a logical processor belongs to a core, a memory region belongs to an address space, a process belongs to a pool, and an artifact belongs to the process that produced it.

Each object has a stable nonzero ID, snapshot generation, kind, state, domain mask and optional feature/affinity/capacity metadata. IDs are unique inside one snapshot. A generation mismatch is rejected rather than silently mixing stale CPU topology with newer process state.

## 4. Relationship graph

V1 supports four typed relationships:

| Relationship | Direction | Meaning |
|---|---|---|
| `depends-on` | prerequisite process → dependent process | Execution order requirement |
| `maps-to` | memory region → address space | Address-space binding |
| `uses` | process → resource | Declared dependency on a worker, register bank, ISA profile, memory region or device |
| `produces` | process → artifact | Provenance link for a generated artifact |

Containment is represented by `parent_id`, not an untyped duplicate edge.

Validation rejects missing endpoints, duplicate IDs, duplicate edges, illegal endpoint-kind combinations, invalid parent-kind relations, mismatched generations, invalid capacities and cycles in the process dependency graph. The fixed v1 workspace bounds graph validation to at most 256 nodes and 512 edges.

## 5. Execution domains and feature admission

The domain mask distinguishes host work, firmware, architectural models, microarchitecture models, structural logic, simulation and verification. A task can name one or more required domains; a worker must support every required domain bit.

Feature masks are 64-bit target/profile evidence. A process requirement is a subset of the worker's declared feature mask. They are not a substitute for full ISA feature-closure validation: target manifests and OS-usable feature evidence remain authoritative.

An affinity mask is a constraint on logical processor IDs 0–63. A zero mask means that the process does not specify an affinity preference. It does not mean that affinity is changed by PPF.

## 6. Deterministic process-pool planning

A READY process carries:

- required execution-domain mask;
- required feature mask;
- optional logical-processor affinity mask;
- requested load in basis points (0–10,000);
- an 8-bit priority;
- dependency edges to prerequisite processes.

An available worker carries:

- logical processor or execution-unit identity;
- supported domains and feature mask;
- logical ID;
- capacity and already-reserved load in basis points.

The planner:

1. Validates the entire model and dependency graph.
2. Considers READY processes in descending priority, then ascending task ID.
3. Leaves a process in `waiting-dependency` until every prerequisite is COMPLETED.
4. Filters workers by availability, domain, feature subset and affinity.
5. Rejects a worker whose reserved plus planned load would exceed its capacity.
6. Chooses the candidate with the lowest projected utilization; ties resolve by logical ID and then worker ID.
7. Returns a caller-owned assignment record for every READY process.

The plan is deterministic for identical inputs. It does not spawn threads, execute tasks, change process priority, set CPU affinity, reserve a physical core, change clocks or write registers. The load values are planning units supplied by the caller, not measured CPU percentages or performance guarantees.

## 7. Existing catalogue aggregation

PPF does not copy or recreate subsystem catalogues. It calls their existing enumeration APIs or reads their stable header-level catalogue size and records their counts in the report:

| Catalogue | v1 count at integration |
|---|---:|
| CPU Suite modules | 78 |
| Processor engineering rules | 3 |
| Hardware construct forms | 20 |
| SystemArray sigil operators | 13 |
| Iron Core markers | 20 |

These are catalogue entry counts, not counts of implemented hardware backends. The hardware-construct frontend remains structural, the SystemArray sigil registry remains metadata-only, and Iron Core remains a bounded structural source format.

## 8. C11 API and use

The API is in `src/processor_fabric.h`; implementation is in `src/processor_fabric.c`.

~~~c
pl_processor_fabric_workspace workspace;
pl_processor_fabric_summary summary;
pl_processor_fabric_error error;
pl_processor_fabric_assignment plan[PL_PROCESSOR_FABRIC_MAX_NODES];
size_t plan_count = 0u;

pl_processor_fabric_status status =
    pl_processor_fabric_plan(
        &model,
        &workspace,
        plan,
        PL_PROCESSOR_FABRIC_MAX_NODES,
        &plan_count,
        &summary,
        &error
    );
~~~

The model is a read-only view over caller-owned node and edge arrays. No pointers are retained. Workspace and output storage are caller-owned. The API performs no heap allocation and does not execute any task.

The report serializer writes `pytyliki.processor-fabric-report.v1` to a caller-owned character buffer. The schema requires the fields `execution_performed`, `processor_control_changed` and `hardware_access_performed` to be false.

The `pytyliki-fabric` utility exposes `--version`, `--catalogues` and `--catalogues-json`. The catalogue summary has its own schema, `pytyliki.processor-fabric-catalogues.v1`, and is metadata-only.

## 9. Error and lifecycle model

The API reports structured status codes for invalid arguments, limits, duplicate or missing IDs, invalid parent/edge relations, parent/dependency cycles, generation mismatches, invalid capacity/domain metadata, and insufficient caller output storage.

Node states are `declared`, `verified`, `ready`, `available`, `blocked`, `completed`, `failed`, `disabled` and `retired`. A worker is schedulable only when it is `available`; a process enters the planner only when it is `ready`.

A source-authored state or report is not itself proof that CI, a signature or a platform has verified the object. Verification evidence must keep its own provenance.

## 10. Reaper ownership and integration

Pytyliki owns the native processor-system inventory, host evidence, catalogue aggregation and deterministic advisory worker plan.

Reaper owns project manifests, canonical language semantics, typed IR, source spans/SourceMap, ABI and compiler/backend policy. Reaper's Processor Engineering Platform v1 already defines a host-evidence workflow based on `pytyliki.report.v1`. PPF adds a separate optional report schema; it does not silently change the existing report contract or claim that Reaper already consumes PPF reports.

The next cross-repository stage is a versioned Reaper adapter that validates this report, preserves target/profile generation and evidence provenance, and lowers only supported metadata into Reaper's canonical typed IR. It must not turn a planning assignment into an operating-system scheduling command.

## 11. Current implementation maturity

| Capability | Status |
|---|---|
| Bounded typed node/edge model | Implemented in this branch |
| Parent and relationship validation | Implemented in this branch |
| Process dependency cycle detection | Implemented in this branch |
| Existing catalogue count aggregation | Implemented in this branch |
| Deterministic domain/feature/affinity/capacity planner | Implemented in this branch |
| Versioned report buffer serialization and schema | Implemented in this branch |
| Live OS process scheduler adapter | Not implemented |
| CPU affinity / priority control | Not implemented |
| Privileged register, MSR, firmware or voltage writes | Not implemented |
| Real MMIO, DMA or IOMMU mapping | Not implemented |
| Full instruction selection / assembler / linker | Not implemented by PPF |
| Reaper PPF report importer | Planned cross-repository adapter |
| Architectural emulator / cycle-accurate simulator / RTL synthesis | Separate future backends |

## 12. Acceptance tests

The native CTest suite covers:

- inventory counts and aggregation of the five existing catalogues;
- deterministic worker selection and task ordering;
- dependency waiting and cycle rejection;
- invalid parent references and duplicate IDs;
- feature mismatch and capacity exhaustion;
- caller buffer bounds;
- report schema identity and explicit false side-effect flags;
- no mutation of process state or reserved worker load during planning.

The report schema is `schemas/pytyliki.processor-fabric-report.v1.schema.json`.

## 13. Roadmap beyond v1

1. **Evidence adapters:** normalize CPU topology, usable ISA and resource measurements from the existing read-only inventory APIs.
2. **Architecture manifests:** versioned target/profile, register-bank and memory-region descriptors with provenance.
3. **Scheduling analytics:** NUMA locality, cache-sharing groups, thermal budgets, load history and fairness policies; still advisory until a separately reviewed OS adapter exists.
4. **Typed process contracts:** connect Pytyliki hardware AST constructs to process-pool requirements and explicit effects.
5. **Reaper bridge:** validate and import PPF reports through a schema-versioned adapter preserving Span, SourceMap and ABI metadata.
6. **Backend orchestration:** build-graph integration for explicitly available native assembler, linker, simulator, RTL and verification tools. No tool is installed or launched merely because it appears in a catalogue.

Every stage needs implementation evidence, negative tests and a declared maturity state before its capability is promoted.
