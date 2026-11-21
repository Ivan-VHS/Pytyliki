# TRACE measurement record

| Field | Meaning |
|---|---|
| metric / unit | named value and unit |
| context | CPU, workload, build, environment |
| source_digest | source/config identity |
| window | exact interval/duration |
| sample_count | observation count |
| uncertainty | declared error metadata |

Comparison requires compatible context, unit and sampling policy. TRACE records evidence, not hardware certification.
