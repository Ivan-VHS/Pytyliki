# Orthogonal metric engine

| Model | API family |
|---|---|
| Euclidean/L1/L∞ norms and distances | `pl_metric_*` |
| Dot, projection, orthogonality, cosine | `pl_metric_*` |
| Quadratic forms / weighted metrics | `pl_metric_*` |
| Law of cosines / de Gua / Parseval | `pl_metric_*` |
| Spherical / hyperbolic / Minkowski | `pl_metric_*` |
| Variance, RMS, covariance | `pl_metric_*` |

C11, deterministic and allocation-free. Inputs must be finite and dimension-bounded. Positive definiteness, normalized bases and covariance assumptions remain caller responsibilities.
