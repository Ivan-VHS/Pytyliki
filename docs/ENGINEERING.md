# Engineering gates

| # | Gate | Acceptance |
|---:|---|---|
| 1 | Claims | implemented / partial / planned, backed by code/tests |
| 2 | Safety | processor discovery and source review are read-only |
| 3 | Bounds | lengths, counts, overflow and workload limits checked |
| 4 | Inputs | untrusted files are never executed |
| 5 | Evidence | metric, unit, context, digest, window and samples recorded |
| 6 | UX | keyboard, focus, contrast, localization and accessibility |
| 7 | Release | build, CTest, license and reproducible commands |

Benchmarks are local comparisons; profiles and simulations are advisory. Hardware writes require separate design, human authorization and dedicated tests.
