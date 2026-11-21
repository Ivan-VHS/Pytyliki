# Orthogonal Metric Engine

Pytyliki exposes a bounded C11 numerical kernel for the Pythagorean family of models. It is designed for benchmark vectors, processor telemetry, DSP measurements and engineering feature spaces. The API is deterministic, allocation-free and does not access hardware.

## Implemented operations

| Model | API | Engineering use |
|---|---|---|
| Euclidean norm and n-D distance | pl_metric_norm, pl_metric_distance | Compare benchmark and telemetry vectors |
| Inner product and orthogonality | pl_metric_dot, pl_metric_is_orthogonal | Measure alignment and near-independent axes |
| General quadratic form | pl_metric_quadratic_form | Apply an explicit symmetric weight or metric matrix |
| Law of cosines | pl_metric_law_of_cosines | Recover a resultant from two magnitudes and their angle |
| de Gua relation | pl_metric_de_gua_area | Combine three mutually perpendicular face areas |
| Parseval energy | pl_metric_parseval_energy | Sum squared coefficients in a declared orthogonal basis |
| Unit-circle identity | pl_metric_unit_circle_residual | Check numerical trigonometric residual |
| Spherical right triangle | pl_metric_spherical_hypotenuse | Unit-sphere geodesic model; sides are radians |
| Hyperbolic right triangle | pl_metric_hyperbolic_hypotenuse | Curvature -1 model; sides are dimensionless |
| Minkowski interval | pl_metric_minkowski_interval | (ct)^2 - sum(x_i^2), using the (+---) convention |
| Variance composition | pl_metric_combine_uncorrelated_variances | Add variances only for uncorrelated components |
| Reciprocal altitude | pl_metric_reciprocal_altitude | Altitude implied by the right-triangle reciprocal identity |

The variance rule requires zero covariance; independence is sufficient but stronger than necessary. Parseval's equality assumes coefficients use a normalized orthogonal basis. The quadratic-form API accepts any finite matrix; positive definiteness must be established by callers before interpreting its value as a distance or energy.

## Safety and numeric contract

- Vectors contain 1–4096 finite values.
- Quadratic-form matrices are row-major and limited to 512 × 512.
- No API allocates memory, reads hardware registers, changes device settings or performs I/O.
- Invalid dimensions, negative variances, invalid spherical sides and non-finite or overflowing results return -1.
- Spherical sides are restricted to [0, π/2]; hyperbolic sides must be non-negative.
- Orthogonality uses normalized cosine magnitude, avoiding an absolute-scale-dependent dot-product test.

## Build and test

The functions are linked into pytyliki_core. CMake registers metric-geometry-tests with CTest. Run the normal build and CTest suite to verify the workstation.
