# Pytyliki CPU Suite Pro 78X

CPU Suite Pro adds a deterministic, allocation-free catalogue of 78 specialized CPU workstation modules.

## Module groups

| Group | Module IDs | Scope |
|---|---:|---|
| CPU Hub and original workstation tools | 1–14 | Hub, tuning-plan design, affinity inspection, thermal profiles, stress planning, cache benchmarks, power-state inspection, bottleneck analysis, silicon health evidence, microcode audit, process policy, crash correlation, virtual-core simulation, telemetry visualization |
| Architecture and processor intelligence | 15–26 | Topology, threads, ISA, branch prediction, cache coherency, memory controller, idle states, thermal prediction, scheduler, benchmarks, degradation trends, kernel events |
| Thermal and power intelligence | 27–38 | Frequency residency, voltage telemetry, package power, per-core thermals, turbo residency, clock drift, thermal/power throttling, acoustics, cooling simulation, sensor trust, thermal budgets |
| Memory and cache laboratory | 39–50 | NUMA, bandwidth, L1/L2/L3, TLB, page locality, prefetch, false sharing, memory stalls, cache-miss attribution |
| Scheduling and concurrency | 51–60 | Context switches, interrupts, migration, priority inversion, foreground policy, hybrid-core mapping, CPU sets, Linux scheduler inspection, real-time budgets, spinlock contention |
| Diagnostics, security and benchmark laboratory | 61–78 | CPUID, microcode, ISA runtime gating, mitigation audit, security inventory, machine checks, WHEA/ECC/crash correlation, evidence ledger, scalar/AVX/atomic/fence/branch/power benchmarks, regression and session recording |

## C API

- `pl_cpu_suite_catalog`: enumerate all 78 descriptors.
- `pl_cpu_suite_get`: retrieve a descriptor by zero-based index.
- `pl_cpu_suite_find_id` / `pl_cpu_suite_find_name`: stable lookup.
- `pl_cpu_suite_plan_module`: create a bounded advisory plan for a module.

## Safety and implementation status

This increment implements the complete 78-entry module registry, stable identifiers, six domain groupings, lookups, advisory planning, and exhaustive catalogue tests. It does **not** claim that all 78 are fully-featured hardware tools. Live sensor backends, vendor-specific controls, GUI panels, OS-specific counters and physical tuning require separate validated adapters and tests.

No voltage, clock, firmware, microcode, fan, interrupt, kernel, or processor-register writes are performed. Tuning-related modules only describe plans until a separately reviewed, vendor-supported backend exists. AVX workloads must be bounded and dispatched only after runtime ISA **and OS vector-state** validation. No macOS, ARM64, x86_64-specific performance guarantee is implied.

## Validation

CTest checks all 78 IDs, unique names, deterministic enumeration, lookup consistency, advisory-plan generation and invalid-input rejection.
