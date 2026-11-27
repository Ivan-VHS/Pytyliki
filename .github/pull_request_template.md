This PR explains one coherent change to Pytyliki and the reason it belongs in the processor workstation.

## What changed
Describe the observable behavior, public API or user workflow. Name affected modules and keep the explanation concrete.

## Why
What problem does this solve? Why is this the smallest useful change?

## Native implementation
- [ ] C11 interfaces, ownership and error behavior are documented where relevant.
- [ ] Memory bounds, arithmetic limits and resource lifetimes are handled.
- [ ] Platform-specific behavior and portable-core boundaries are clear.
- [ ] ABI changes are additive or explicitly versioned.

## Verification
List the exact CMake, CTest, CLI or platform commands that were run. Mark checks that were not run as pending; do not imply that CI has passed before it has.

- [ ] Unit and regression tests cover normal and boundary cases.
- [ ] Integration or ABI tests cover affected consumers.
- [ ] Documentation and examples match the implementation.

## Compatibility and safety
Explain hardware access, permissions, input limits, persistence and any unsupported behavior. Pytyliki remains within its declared processor-analysis and workstation boundary.

## Dependencies
Link related PRs and branches. State merge order for stacked or cross-repository work.
