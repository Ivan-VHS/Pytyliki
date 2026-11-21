# Bounded memory kernel

| Primitive | Contract |
|---|---|
| Typed view | pointer + element count |
| Slice | checked offset/length |
| Arena | fixed capacity |
| Checkpoint | rewind to valid offset |
| Pool | fixed-size blocks |
| Telemetry | used, capacity, high-water mark |

Check integer overflow before allocation/access. Invalid ranges fail explicitly; no implicit unbounded growth.
