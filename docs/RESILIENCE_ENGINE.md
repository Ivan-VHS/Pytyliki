# Resilience policy engine

Pytyliki includes a deterministic circuit-breaker evaluator. It converts
caller-supplied telemetry into a recommended state; it does not connect to
services, inspect live host metrics, or enforce traffic limits by itself.

## States

- CLOSED: normal operation; 100% concurrency budget.
- DEGRADED: low resource headroom or queue pressure; shed optional work and
  recommend a 50% concurrency budget.
- OPEN: the error-rate threshold has been crossed or recovery has failed;
  stop primary work and recommend stale-cache service.
- HALF_OPEN: after a 30-second caller-measured cooldown, allow a limited
  recovery probe (five probe slots and a 5% concurrency budget).

Defaults: 25% error threshold, 15% minimum CPU or memory headroom, 85% queue
utilization threshold, and five successful probes before closing a recovering
circuit. All percentages must be finite and between 0 and 100.

## CLI

    pytyliki --resilience 30 60 55 40
    pytyliki --resilience 2 60 55 40 --breaker-state open --cooldown-elapsed 30
    pytyliki --resilience 1 60 55 40 --breaker-state half-open --probe-successes 5

The four values are error rate, CPU headroom, memory headroom, and queue
utilization percentages. They are caller-provided evidence, not live
measurements. The CLI prints a recommendation only; it does not apply it.

## C API

Include resilience.h and call pl_resilience_evaluate. The result reports the
next state, primary-work permission, stale-cache recommendation, optional-work
shedding, and suggested concurrency/probe budgets. The function is pure and
has no side effects. Malformed telemetry is rejected; OPEN does not recover
before cooldown, and HALF_OPEN returns to OPEN when the error threshold recurs.
