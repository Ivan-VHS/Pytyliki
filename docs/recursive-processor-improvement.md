# Recursive Processor Improvement Engine

The C11 improvement engine turns the 20-row capability matrix into a deterministic processor-engineering catalogue and a gated lifecycle:

observe -> diagnose -> propose -> implement -> verify -> evaluate -> review -> integrate -> learn -> improve-engine.

## Iteration target

The default plan is **666,666 evidence-gated improvement iterations**. This is a configurable upper target, not a promise that the repository can produce 666,666 useful changes or that every pass will improve every metric. A cycle completes only after its ten phases have recorded the required evidence. The target can be reduced for tests and controlled runs; values above 666,666 and zero are rejected.

The requested **66,666,666× improvement factor** is retained as an aspirational slogan, not a measured multiplier or a guaranteed result. Actual progress must be reported through reproducible measures such as build/test pass rate, regression count, review rework, benchmark distributions, memory use, and rollback rate. No iteration may claim improvement without before/after evidence. If a change is neutral, noisy, unsafe, or regressive, reject it and preserve the last verified state.

## Capability catalogue

Each row contains three coordinated capabilities: engineering transformation, optimization activity and automation direction. The catalogue is descriptive and stable; it does not claim that every transformation is automatically performed.

## Evidence-gated cycle

The API requires a phase-specific evidence token before advancing. Phases cannot be skipped. Integration requires an explicit approval token, representing authorization by a human maintainer. On completion of the final phase, the engine increments the completed-iteration counter. It resets to observe until the configured target is reached, then enters complete.

The iteration counter does not itself run builds, rewrite source, create commits, or call GitHub. Those operations belong to a separately permissioned orchestration workflow. Every proposed source change must remain isolated, tested and reviewable.

## Safety boundaries

- Processor discovery and source inspection remain read-only.
- The engine does not edit source files, execute generated code, flash firmware, alter microcode, write device registers, or change voltage, clocks, power limits, kernels or drivers.
- Implementation evidence means a proposed change was implemented in a separately controlled development workflow; this library does not perform that implementation.
- Verification evidence must come from the project's tests and CI. The library records gate state; it does not certify hardware.
- The engine cannot approve or merge its own pull requests, weaken tests, bypass branch protection or grant itself permissions.
- Stop an iteration on failed tests, a security finding, a regression, missing evidence, or an unrepeatable performance result. Never auto-retry by weakening the gate.

## API

- pl_improvement_catalog: return the fixed 20-entry capability catalogue.
- pl_improvement_cycle_init: initialize a cycle with the 666,666 iteration target.
- pl_improvement_cycle_set_target: set a smaller or equal target before recording evidence.
- pl_improvement_record: record the one evidence item valid for the current phase.
- pl_improvement_advance: move one phase, or begin the next iteration after all gates pass.
- pl_improvement_phase_name: retrieve the phase label.

Build and test with the normal CMake/CTest workflow.