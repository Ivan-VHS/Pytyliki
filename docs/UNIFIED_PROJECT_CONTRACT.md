# Unified project contract v1

The ecosystem contains independent products connected by explicit versioned interfaces.

| Field | Rule |
|---|---|
| Capabilities | stable ID, implemented/partial status, evidence path |
| Interfaces | format, direction, version, peer, opt-in status |
| Safety | read-only defaults, bounded inputs, explicit native loading |
| Validation | schema + repository-local evidence files |
| Promotion | exact PR head and required checks |

Run `python tools/validate_project_contract.py`. This manifest is an inventory, not a roadmap.


## Shared documentation semantics

This project follows the [Ivan-VHS Unified Documentation and Semantics Standard](UNIFIED_DOCUMENTATION_STANDARD.md). Capability maturity, runtime state, verification result and integration policy are separate dimensions. In capability entries, maturity describes implementation progress; terms such as `optional` and `opt_in` describe integration policy and must not be interpreted as maturity.

This addition clarifies documentation semantics only. It does not change the existing project-contract v1 schema, field names or stored values. Any machine-readable schema migration must be versioned and pass the repository's contract validator.
