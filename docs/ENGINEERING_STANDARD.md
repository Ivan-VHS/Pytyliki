# Engineering Standard

Status: repository-wide development contract  
Applies to: source, tests, CI, documentation and the Windows desktop interface

## 1. Natural, coherent implementation

- Prefer idiomatic C11, explicit ownership, small functions and descriptive names.
- Add a capability only when it has a distinct user-visible or engineering outcome. Do not multiply wrappers, aliases or catalogue entries merely to inflate function counts.
- Reuse the tested core from the CLI and Win32 interface. UI event handlers are adapters; they must not become a second implementation of processor, DSP or policy logic.
- Keep platform-specific code behind narrow boundaries. A feature unavailable on a platform must be reported honestly, not simulated as if it ran natively.

## 2. Function composition and interaction coverage

"Apply every function to every function" means test every *valid, contract-compatible interaction*, not blindly execute an N×N Cartesian product.

1. Inventory public functions, input/output types, side effects, limits and failure modes.
2. Mark compatible pairs and meaningful multi-stage paths in a small interaction matrix.
3. Add positive integration tests for valid pairs and representative chains.
4. Add negative tests for incompatible inputs, invalid state transitions and resource-limit violations.
5. Use exhaustive combinations only when the domain is finite and small; use pairwise or risk-based coverage for larger spaces.
6. Record intentionally unsupported combinations and why they are unsupported.

The existing CTest suite remains the executable source of truth. New matrix entries must point to real tests; a catalogue label is not proof that a capability is implemented.

## 3. Tests and automatic execution

- Run CMake configure, build and CTest on Linux and Windows.
- Keep AddressSanitizer and UndefinedBehaviorSanitizer coverage on Linux.
- Run targeted regression tests for every changed subsystem and interaction.
- CI may start automatically on feature-branch pushes and pull requests; manual dispatch remains available for controlled diagnostics.
- Workflows must be bounded, deterministic where practical, and least-privilege (contents: read unless a narrowly scoped write is essential).
- Do not create self-triggering, unbounded recursive workflow loops.

## 4. Push, review and merge policy

- Work on a dedicated feature or maintenance branch. Never push directly to main, force-update it, or bypass repository controls.
- Automation may validate a change and prepare a PR. It must not approve its own work.
- Merge only when the applicable CI checks are green, an independent human review is complete, and repository rules permit the merge.
- Native GitHub auto-merge is acceptable only when it remains gated by independent approval and required checks. A green job alone is not merge authorization.
- Preserve a simple rollback path: revert the isolated commit or PR rather than rewriting shared history.

## 5. Native desktop UX and visual language

Pytyliki already builds a native Win32 desktop interface on Windows. Keep the GUI thin, keyboard-operable and consistent with the core APIs.

Use this white–red–blue design vocabulary for new or revised UI surfaces:

- White: primary canvas and readable content surfaces — #FFFFFF.
- Blue: primary actions, navigation and focus indicators — #1D4ED8.
- Red: destructive actions, warnings and critical states — #DC2626.
- Neutral text: #111827; secondary surfaces: #F8FAFC.

Do not use color as the only status signal. Preserve visible focus, clear labels and readable contrast. Red is a semantic accent, not a default color for every control. Linux remains a CLI-supported build unless a native desktop backend is actually implemented and tested.

## 6. Feature lifecycle and maintainability

Every new feature must have:
- a clear problem statement and acceptance criteria;
- a small public contract and explicit error behavior;
- unit tests plus integration tests where it composes with other subsystems;
- updated English documentation and applicable translated documentation;
- updated examples or CLI/GUI help where users discover the capability;
- performance or resource claims backed by reproducible measurements;
- a stated platform scope and rollback strategy.

Refactor for clarity without changing behavior accidentally. Avoid duplicate logic, hidden global state, unexplained magic numbers and cosmetic churn unrelated to the change.

## Definition of done

A change is complete when its behavior is implemented, tested, documented, reviewed, compatible with the stated platforms, and reversible. Planned, heuristic or reserved capabilities must remain labeled as such.
