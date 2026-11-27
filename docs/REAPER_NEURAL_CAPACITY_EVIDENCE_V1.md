# Pytyliki Neural Capacity Evidence Contract v1

**Canonical model schema:** Reaper reaper.neural-network.v1  
**Adapter:** automation/reaper/neural_metrics.rpr  
**Status:** deterministic evidence assessment; no hardware control

## Evidence envelope

The nine-field Reaper list contains:

1. pytyliki.neural-capacity-evidence.v1;
2. snapshot or evidence identifier;
3. observed CPU load in permille (0–1000);
4. observed memory bytes;
5. elapsed workload time in microseconds;
6. observed operation-step count;
7. caller-declared step budget;
8. caller-declared memory budget in bytes;
9. workload mode: inference or offline_batch.

## Decision policy

- REJECT: malformed schema, missing identity, invalid units, or out-of-range evidence.
- DEFER: CPU load exceeds 850 permille, memory exceeds the caller's budget, observed steps exceed the declared budget, or elapsed time exceeds five seconds.
- ALLOW: supplied measurements fit the declared local workload budget.

The output uses pytyliki.neural-capacity-plan.v1 and records evidence provenance as read_only_evidence and recommendation_only.

## Trust and maturity boundary

The Reaper adapter evaluates values supplied to it. It does not enumerate processors, read sensors, benchmark a machine, change processor affinity, tune voltage/frequency, alter IRQs, touch firmware, or install drivers. The existing Pytyliki C11/native ABI remains the source of processor measurements.

A recommendation is not a scheduler reservation or a guarantee of future runtime. Production scheduling must revalidate live resource state at admission time.

## Verification

The Reaper language workflow checks and executes automation/reaper/neural_metrics.rpr and requires PYTYLIKI_REAPER_NEURAL_METRICS:PASS.
