# TRACE cadence analyzer

API: `pl_trace_analyze_cadence(intervals, count, expected, tolerance, &report)`.

| Input/output | Limit |
|---|---:|
| Interval count | 2–1,000,000 |
| Input | finite positive seconds |
| Metrics | mean, sample SD, CV, bias, half-window drift |
| Score | 0–100 descriptive cadence score |

Invalid input is rejected; the input array is unchanged. The score is not hardware certification. Compare only records with matching TRACE provenance.
