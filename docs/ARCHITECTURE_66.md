# Architecture 66 — concise contract

**66 rules** define one modular C11 product, explicit interfaces and bounded local operation.

| # | Domain | Rule summary |
|---:|---|---|
| 1–11 | Core | one shared core; narrow APIs; **default to deny**; explicit policy |
| 12–22 | Safety | read-only discovery; bounded inputs; human authorization |
| 23–33 | Automation | observe → diagnose → propose → verify; rollback path |
| 34–44 | Evidence | reproducible tests; source-linked claims; measured changes |
| 45–55 | UX | accessible diagnostics; CLI/GUI parity; explicit consent |
| 56–66 | Release | reviewable changes; exact PR head; protected promotion |

Statuses are implemented, partial or planned. A catalogue is not implementation proof.
