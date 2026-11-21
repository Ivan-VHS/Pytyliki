# Correction, coverage and defect standard

The `quality-remediation.v1.json` file holds 20 triplets and their acceptance evidence.

| Stage | Required record |
|---|---|
| Detect | affected path, input and expected/actual output |
| Reproduce | regression test or bounded observation |
| Diagnose | confirmed root cause |
| Remediate | smallest coherent fix |
| Verify | targeted + full regression checks |
| Close | evidence, limitation and reviewer |

Run `python tools/quality_remediation.py --format json`. Broad coverage never means unbounded resources.
