# Java ecosystem capability adaptation for the native C11 core

Pytyliki is a local processor-systems workstation implemented in C11. The
adaptation imports mature *engineering properties* of Java ecosystems, not a
JVM service stack. There is no HTTP backend, relational database, hosted API or
distributed data-processing cluster in the workstation.

## Typed metric registry query

The metric registry now offers a versioned-in-source typed query contract:

- `query` and `category` are bounded to 256 UTF-8 bytes of C string input;
- required and excluded capability masks cannot overlap;
- required and excluded type masks cannot overlap;
- offset/limit pagination is explicit, with a maximum page size of 4096;
- `total_matches` is distinct from the number of pointers written to the
  caller-owned result buffer;
- caller buffer capacity is independently respected;
- the legacy `pl_metric_registry_search` API remains as a compatibility
  wrapper over the typed query path.

This is the C equivalent of a jOOQ-style explicit query contract: filters and
result bounds are inspectable and the search has no hidden allocation or
ownership transfer.

## Framework capability crosswalk

| Java ecosystem capability | Pytyliki adaptation |
|---|---|
| Spring IoC / DI | Explicit C structs, function parameters and caller-owned contexts |
| Spring Boot configuration | CMake options and validated native configuration structures |
| Quarkus / Micronaut build-time checks | C11 compile/link validation, deterministic tests and static registries |
| Jakarta CDI / MicroProfile contracts | Stable C ABI v1 and explicit plugin descriptor metadata |
| Hibernate / Spring Data JPA | Not applicable: no relational database or entity persistence layer |
| jOOQ / MyBatis | Typed bounded metric registry queries over static operation descriptors |
| Netty / Vert.x | Not applicable to the local workstation; no network server or event loop |
| Spark / Flink / Kafka Streams | Not applicable to bounded host-side CPU and profile analysis |
| Camel EIP | Existing explicit C ABI and file-based federated contracts, not a generic integration bus |
| Actuator / OpenTelemetry | Existing local reports, benchmark evidence and trace modules; no remote exporter is claimed |

## Cross-repository C ABI

The typed metric query is also exposed as the additive ABI v1 function
`pytyliki_v1_metric_registry_search`. It uses fixed-width, NUL-terminated
query structures and caller-owned result records. It reports both
`total_matches` and `written`, retains no caller pointers and does not execute
a metric operation. The OPlague bridge in
[tools/pytyliki_ffi.py](https://github.com/Ivan-VHS/OPlague/blob/feature/java-ecosystem-capability-adaptation/tools/pytyliki_ffi.py)
uses an explicit absolute shared-library path and applies matching UTF-8,
mask and pagination validation. This is an opt-in native integration; it does
not install or search for shared libraries implicitly.

## Compatibility

The original metric search function and its count-return contract remain
available. New consumers can use `pl_metric_registry_search_ex` and receive
both total matching rows and written rows. Search results are pointers into
the static registry; callers must not free them or retain them across a binary
replacement. No caller pointer is retained by the registry.

## Validation

The new CTest target is `metric-registry-query-tests`. Build and run the
existing native suite:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The registry remains deterministic, read-only and allocation-free during
search. The test target is added in the adaptation branch and must pass CI
before merge.
