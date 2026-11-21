# CLife Immortality Pod — Pytyliki C11 adapter

This is a software-resilience analogue of the supplied pod simulation. "Cryo faults" map to
storage I/O, artifact-integrity, resource-pressure and execution-interruption faults.
"Consciousness snapshots" map only to caller-owned project bytes. No physical or biological
state is represented.

## API

- Capture copies a bounded payload (maximum 64 MiB) into an owned checkpoint.
- Verify checks schema version, project identifier, bounds and payload checksum.
- Restore requires a matching project ID and enough output capacity; it never partially copies.
- Recovery selection returns normal resume, verified restore, partial hold, safe stop or invalid.
- Fault mitigation is reported only when normal resume or verified restore is selected.

Build integration is part of pytyliki_core and CTest. Run with the standard CMake build.

## Important integrity boundary

The dependency-free native implementation currently uses FNV-1a 64-bit as an accidental
corruption checksum. It is **not cryptographic**, does not authenticate a checkpoint and must
not be used as a security boundary. Store exported checkpoint data in access-controlled storage;
use a separately reviewed signature/HMAC layer before trusting snapshots against hostile writers.
The API operates on in-memory payloads; the host application owns durable file I/O and retention.
