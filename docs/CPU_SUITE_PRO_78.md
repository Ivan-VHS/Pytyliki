# CPU Suite Pro — 78 modules

The catalogue has 78 descriptors across 6 groups. A registry entry is not a claim of full sensor/hardware implementation.

| IDs | Group | Scope |
|---|---|---|
| 1–14 | Workstation | affinity, thermal/power plans, cache, crash evidence |
| 15–26 | Architecture | topology, ISA, branch/cache/memory intelligence |
| 27–38 | Thermal/power | residency, throttling, sensor trust, budgets |
| 39–50 | Memory/cache | NUMA, bandwidth, TLB, locality, stalls |
| 51–60 | Scheduling | migration, interrupts, hybrid-core policy |
| 61–78 | Diagnostics | CPUID, microcode, mitigations, benchmark evidence |

API: `pl_cpu_suite_catalog`, `pl_cpu_suite_get`, `pl_cpu_suite_find_id/name`, `pl_cpu_suite_plan_module`. Planning is advisory; physical tuning remains out of scope.
