# Recursive Processor Improvement Engine

The C11 improvement engine turns the 20-row capability matrix into a deterministic processor-engineering catalogue and a gated lifecycle:

`observe -> diagnose -> propose -> implement -> verify -> evaluate -> review -> integrate -> learn -> improve-engine`.

## Capability catalogue

Each row contains three coordinated capabilities: engineering transformation, optimization activity and automation direction. The catalogue is descriptive and stable; it does not claim that every transformation is automatically performed.

## Evidence-gated cycle

The API requires a phase-specific evidence token before advancing. Phases cannot be skipped. Integration requires an explicit approval token, representing authorization by a human maintainer. A completed cycle increments its generation counter.

## Safety boundaries

- Processor discovery and source inspection remain read-only.
- The engine does not edit source files, execute generated code, flash firmware, alter microcode, write device registers, or change voltage, clocks, power limits, kernels or drivers.
- Implementation evidence means a proposed change was implemented in a separately controlled development workflow; this library does not perform that implementation.
- Verification evidence must come from the project's tests and CI. The library records gate state; it does not certify hardware.
- The engine cannot approve or merge its own pull requests, weaken tests, bypass branch protection or grant itself permissions.

## API

- `pl_improvement_catalog`: return the fixed 20-entry capability catalogue.
- `pl_improvement_cycle_init`: initialize a cycle.
- `pl_improvement_record`: record the one evidence item valid for the current phase.
- `pl_improvement_advance`: move exactly one phase after its evidence gate passes.
- `pl_improvement_phase_name`: retrieve the phase label.

Build and test with the normal CMake/CTest workflow.
