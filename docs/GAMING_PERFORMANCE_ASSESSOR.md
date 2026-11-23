# Pytyliki gaming-session evidence assessor

**Project author/developer:** Ivan-VHS

Pytyliki extends its read-only host monitor with a bounded game-session assessor. It consumes real Pytyliki Watch snapshots and, optionally, measured frame-time samples exported by a compatible capture tool.

## Evidence and outputs

The assessor reports sample counts, minimum/median/p95/maximum CPU, memory and swap utilization, pressure warnings, mean/p95/p99 frame time, average FPS and 1%/0.1% low FPS. The 0.1% low stays unavailable until at least 1,000 measured frames exist.

The report explicitly labels CPU/RAM/swap readings as `whole_host`; it does not infer which process caused the pressure. Missing counters remain unknown and no synthetic FPS or sensor data is created.

## Non-goals

This module does not change scheduling, process priority, affinity, paging, memory compression, GPU clocks, power plans, cooling, network settings or background services. It produces measurements and review recommendations only.

## Verification

```sh
python -m unittest tests.test_gaming_performance_assessor -v
```
