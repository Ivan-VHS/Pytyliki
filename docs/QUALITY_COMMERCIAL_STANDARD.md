# Quality and commercial readiness

The machine-readable `quality-commercial.v1.json` is the 20 × 3 control register.

| Axis | Evidence gate |
|---|---|
| Quality | tests, bounds, accessibility, regressions |
| Execution | repeatability, build, support, recovery |
| Commercial | license, rights, distribution, costs, user validation |

Run `python tools/quality_commercial.py --format json`. The report is not a certification of market fit, sample rights or profitability.
