# TriMatrix AutoForge 100

## Purpose

TriMatrix is a bounded, deterministic planning engine for composing future automation workflows. It provides a catalogue and traversal contract; it is not a self-modifying runtime.

## Three axes

- **Execution line:** Build, Evolve, Verify.
- **Capability matrix:** Function, Automation, Meta-Automation.
- **Lifecycle stage:** Discover, Design, Generate.

The Cartesian product contains 3 × 3 × 3 = 27 stable catalogue cells. Each logical generation traverses all 27 cells in catalogue order. A plan may contain 1–100 generations, for a maximum of 2,700 steps.

## Runtime contract

- C11, deterministic iteration, no dynamic allocation, no recursion on the C call stack.
- `pl_tma_plan_init` accepts only depths 1 through 100.
- `pl_tma_plan_next` returns 1 for a step, 0 for clean completion, and -1 for invalid arguments or corrupted plan state.
- The public plan state is validated against its expected generation, cell index and emitted-step count before every transition.
- The catalogue is immutable and has static lifetime.

## CLI

```sh
pytyliki --trimatrix          # print all 100 generations (2,700 steps)
pytyliki --trimatrix 1        # inspect one generation (27 steps)
pytyliki --trimatrix 12       # inspect a bounded 12-generation plan
```

The output is a deterministic plan listing. It does not create files or execute generated code.

## Integration and promotion

A future executor may consume plans only through separate reviewed interfaces. Any generated artifact must pass, at minimum:

1. Schema and input validation.
2. Static analysis, compiler warnings and sanitizers.
3. Unit, integration, differential and fuzz tests appropriate to the artifact.
4. Resource and execution limits.
5. Independent review and explicit authorization for repository writes, deployment or merge.
6. Evidence tied to the exact commit SHA; stale evidence blocks promotion.

No step in this planner grants permissions, executes commands, writes repositories, deploys software, or merges pull requests. The planner itself cannot certify performance claims or platform-specific guarantees.

## Tests

CTest target: `trimatrix-autoforge-tests`. Coverage includes all 27 unique cells, bounds, invalid arguments, full 100-generation traversal, clean completion and corrupted-state rejection. CLI and APSIA smoke tests are registered separately.
