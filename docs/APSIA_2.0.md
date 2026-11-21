# APSIA 2.0 — bounded routing

APSIA selects a suitable workload backend; it is not an autonomous code writer or deployment agent.

| Stage | Implemented boundary |
|---|---|
| Observe | local build/test/host evidence |
| Diagnose | bounded gap and workload classification |
| Route | native C11 decision function |
| Contract | capability and least-privilege declarations |
| Verify | tests + promotion evidence |

| Policy | Rule |
|---|---|
| Language choice | prefer host-native implementation |
| FFI | reject when transfer/bridge cost exceeds benefit |
| Capabilities | explicit, least privilege |
| Dependencies | no automatic install |
| Promotion | exact PR head, required checks, human approval |

LLM synthesis, eBPF collection, plugin execution, dynamic hot-swap and unattended merge are not implemented by the routing core.
