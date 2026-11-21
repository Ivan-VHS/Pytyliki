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
