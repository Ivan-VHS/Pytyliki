# Documentation Control

**Status:** normative  
**Scope:** all public documentation, examples, schemas, release notes and repository metadata.

Published documentation is a controlled product interface. It describes supported behavior and stable contracts; it is not an archive of planning sessions, private inputs or implementation history.

## Publication requirements

1. **Evidence:** a statement marked as implemented MUST be supported by checked-in code and relevant tests. A schema or proposal alone does not establish implementation.
2. **Private material:** public documentation MUST NOT identify, quote or fingerprint private attachments, local working files, Library objects, chat threads, prompts, personal notes, internal share handles or unpublished reference material. Do not publish their filenames, hashes, sizes, line counts or storage identifiers.
3. **Public surface:** document observable behavior, prerequisites, supported formats, versioned interfaces, operational limits, security boundaries and known exclusions. Internal file maps, local paths and design history MUST be omitted unless required by a supported integration.
4. **Normative language:** use MUST, MUST NOT, SHOULD, SHOULD NOT and MAY consistently. Requirements MUST identify the affected component and a verifiable acceptance condition.
5. **Capability status:** distinguish implemented, partial, planned and unsupported behavior. Roadmap items MUST NOT be presented as shipped features.
6. **Data protection:** never publish credentials, access tokens, private endpoints, personal identifiers, device identifiers or unredacted local paths. Explain pseudonymization accurately; do not call linkable data anonymous.
7. **Integration:** expose only versioned contracts and the minimum consumer obligations. Do not make private implementation details or repository-internal layouts cross-project dependencies.
8. **Change control:** update affected specifications, schemas, examples and compatibility notes in the same change as a contract modification. Breaking changes require a new interface version or an explicit migration rule.

## Review gate

Before publication, verify that links resolve, examples match the current interface, quantitative claims have reproducible evidence, unsupported behavior is labelled, and no private-source provenance or unnecessary implementation detail remains. Tests and schemas take precedence over descriptive prose when they conflict.
