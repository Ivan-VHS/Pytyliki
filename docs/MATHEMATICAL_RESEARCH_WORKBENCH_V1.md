# Mathematical Research Workbench v1

**Status:** implemented bounded C11 research kernels; experimental numerical and finite-model evidence  
**Language/runtime ownership:** Pytyliki owns the native research kernels; Reaper owns their canonical typed contracts  
**Catalog:** 17 open-problem domains, 3 kernels per domain, 51 stable function identifiers

## Purpose and claim boundary

This workbench turns selected famous mathematical problems into reproducible computational experiments. It does **not** claim to solve any of the open conjectures or Clay Millennium problems.

Each function has a deliberately narrow contract. A finite witness can establish that a supplied candidate satisfies a finite statement. A failed bounded search means only that the configured search did not find a witness within its domain and budget. Floating-point observations are numerical evidence, not exact proofs. A result is never upgraded to a theorem by this library.

The general Halting Problem is different: its undecidability is a theorem. The VM functions below execute and check finite traces of a small formal machine; they are not a general halting decider.

## Architecture

- **Pytyliki C11:** bounded arithmetic, finite-field linear algebra, exact integer checks, numerical diagnostics, small graph search and a bounded abstract-machine executor.
- **Reaper:** canonical typed function metadata, argument validation, effect/budget contracts and a future FFI invocation plan. Reaper does not duplicate these native kernels.
- **Evidence:** preserve input parameters, build identity, compiler/platform details, function identifier, status code, checked-count/budget, and output digest with every research run.

The public C API is in `src/research_math.h`. The implementation is in `src/research_math.c`. The 51-entry stable catalog is exposed by `pl_math_research_catalog_count()` and `pl_math_research_catalog_name()`.

## Function catalog

### 01. Riemann Hypothesis

| Function | Contract |
|---|---|
| `pl_math_riemann_dirichlet` | Finite partial sum for the Dirichlet series of ζ(s), restricted to Re(s) > 1 |
| `pl_math_riemann_eta` | Finite alternating Dirichlet eta-series approximation for Re(s) > 0 |
| `pl_math_riemann_critical_residual` | Computes the magnitude of an eta-derived ζ(1/2+it) approximation at a supplied height |

The critical-line result is an unaccelerated finite sum without a certified truncation bound. A small value is not a zero certificate, and this API does not enumerate or certify all non-trivial zeros.

### 02. P versus NP

| Function | Contract |
|---|---|
| `pl_math_cnf_validate` | Validates a bounded CNF instance and its literal/offset representation |
| `pl_math_sat_verify_assignment` | Checks one supplied Boolean assignment exactly |
| `pl_math_sat_find_model_bounded` | Exhaustively searches at most the supplied number of assignments |

The implementation caps instances at 24 variables. This is a small-instance SAT workbench, not a proof of P = NP or P ≠ NP and not a claim about the asymptotic complexity of SAT.

### 03. Collatz Conjecture

| Function | Contract |
|---|---|
| `pl_math_collatz_step` | Computes one exact uint64 transition with overflow detection |
| `pl_math_collatz_trajectory` | Records a trajectory up to a step and output-capacity budget |
| `pl_math_collatz_stopping_time` | Computes the observed stopping time if 1 is reached within the budget |

The trajectory is limited by uint64 arithmetic and an explicit fuel bound. Overflow, capacity exhaustion and budget exhaustion are distinct outcomes.

### 04. Navier–Stokes Existence and Smoothness

| Function | Contract |
|---|---|
| `pl_math_grid3d_divergence` | Computes finite-difference divergence on a rectangular 3D grid |
| `pl_math_grid3d_vorticity` | Computes finite-difference vorticity on a rectangular 3D grid |
| `pl_math_navier_stokes_cfl_dt` | Returns a conservative grid/time-step estimate from speed, spacing and viscosity inputs |

The grid routines use one-sided differences at boundaries and centered differences in the interior. They do not integrate the full nonlinear PDE system, prove convergence, or establish global regularity or blow-up.

### 05. Birch–Swinnerton-Dyer

| Function | Contract |
|---|---|
| `pl_math_elliptic_curve_nonsingular_mod_prime` | Checks the short Weierstrass discriminant over a prime field p > 3 |
| `pl_math_elliptic_curve_point_count_mod_prime` | Counts points, including infinity, for bounded prime p |
| `pl_math_elliptic_curve_frobenius_trace_mod_prime` | Computes p + 1 - #E(Fp) for the same finite-field model |

The point-count routine is limited to p ≤ 1,000,003. Finite-field counts and traces do not determine the global rank or the leading coefficient of the L-function.

### 06. Hodge Conjecture

| Function | Contract |
|---|---|
| `pl_math_hodge_type_check` | Checks the elementary degree/type condition 2p = degree and p = q |
| `pl_math_hodge_cycle_span_rank_mod_prime` | Computes the rank of a finite cycle-vector matrix over Fp |
| `pl_math_hodge_class_in_cycle_span_mod_prime` | Checks membership in the span of supplied finite vectors over Fp |

These are finite-dimensional linear-algebra tests. They do not construct algebraic cycles on a general complex projective variety or prove the rational Hodge conjecture.

### 07. Yang–Mills Existence and Mass Gap

| Function | Contract |
|---|---|
| `pl_math_u1_plaquette_phase` | Forms an oriented U(1) plaquette phase from four link angles |
| `pl_math_u1_wilson_action` | Evaluates the one-plaquette Wilson action |
| `pl_math_correlator_effective_mass` | Extracts a single-step effective mass from positive correlator samples |

This is a U(1) lattice toy model. It is not a construction of a four-dimensional interacting non-abelian Yang–Mills quantum field theory and does not prove a mass gap.

### 08. Goldbach Conjecture

| Function | Contract |
|---|---|
| `pl_math_is_prime_u64` | Deterministic 64-bit primality test using the documented Miller–Rabin base set |
| `pl_math_goldbach_verify` | Verifies one supplied prime-pair witness for an even integer |
| `pl_math_goldbach_find_pair_bounded` | Searches candidate first summands under an explicit candidate budget |

A found pair is an exact finite witness. A bounded miss is not a counterexample unless the full finite interval was exhaustively searched.

### 09. Twin Prime Conjecture

| Function | Contract |
|---|---|
| `pl_math_twin_prime_verify` | Verifies whether p and p+2 are both prime |
| `pl_math_twin_prime_next_bounded` | Searches for the next pair under a candidate budget |
| `pl_math_twin_prime_count_bounded` | Counts pairs up to a finite upper bound and reports budget exhaustion |

These functions provide finite counts and witnesses only; they do not establish infinitude.

### 10. Halting Problem

| Function | Contract |
|---|---|
| `pl_math_vm_step` | Applies one instruction to a two-counter abstract machine state |
| `pl_math_vm_run_bounded` | Executes the machine for at most the supplied fuel |
| `pl_math_vm_trace_verify` | Replays and validates adjacent state transitions in a supplied trace |

The machine has explicit state, two uint64 counters and a finite program. `PL_MATH_LIMIT` means that the fuel budget ended before a halt was observed; it does not mean the program will never halt.

### 11. Hadwiger–Nelson Problem

| Function | Contract |
|---|---|
| `pl_math_unit_graph_build` | Builds a finite unit-distance graph from scaled integer planar coordinates |
| `pl_math_graph_coloring_verify` | Checks a coloring against every supplied graph edge |
| `pl_math_graph_coloring_search_bounded` | Backtracks through color assignments under a node budget |

The graph is finite and has at most 128 vertices. It does not cover the entire Euclidean plane and cannot determine the plane's chromatic number.

### 12. Inscribed Square Problem

| Function | Contract |
|---|---|
| `pl_math_point_on_polyline` | Checks distance from a point to a finite polygonal curve within a tolerance |
| `pl_math_square_on_polyline_verify` | Verifies four supplied points form a square and lie on a closed polygonal curve |
| `pl_math_inscribed_square_search_sampled` | Searches a finite sample set for a square candidate |

A sampled miss says nothing decisive about all points on the continuous curve. The implementation handles polygonal curves, not arbitrary continuous or fractal curves.

### 13. Legendre Conjecture

| Function | Contract |
|---|---|
| `pl_math_legendre_interval` | Computes the open interval (n², (n+1)²) within the supported uint64-safe n range |
| `pl_math_legendre_witness_verify` | Verifies a supplied prime lies strictly inside the interval |
| `pl_math_legendre_find_prime_bounded` | Searches the interval under a candidate budget |

A witness is exact for one n. Finite successes do not prove the statement for all positive n.

### 14. Beal Conjecture

| Function | Contract |
|---|---|
| `pl_math_beal_power_sum_check` | Compares checked uint64 values of Aˣ + Bʸ and Cᶻ |
| `pl_math_beal_common_prime_divisor` | Finds a prime divisor common to three positive integer bases, if one exists |
| `pl_math_beal_counterexample_screen` | Screens one candidate with x,y,z > 2 for equality and absence of a common prime divisor |

Power overflow is reported rather than wrapped. The screen covers only supplied uint64-representable candidates.

### 15. Perfect Cuboid

| Function | Contract |
|---|---|
| `pl_math_cuboid_face_diagonals` | Computes exact integer square roots of the three face-diagonal squares where they exist |
| `pl_math_perfect_cuboid_verify` | Checks all three face diagonals and the space diagonal exactly |
| `pl_math_perfect_cuboid_search_bounded` | Searches ordered edge triples under maximum-edge and node budgets |

A bounded search miss does not prove non-existence.

### 16. Tate Conjecture

| Function | Contract |
|---|---|
| `pl_math_tate_fixed_vector_verify` | Checks whether a supplied vector is fixed by a finite-field linear map |
| `pl_math_tate_invariant_dimension_mod_prime` | Computes dim ker(F - I) over a prime field |
| `pl_math_tate_cycle_span_rank_mod_prime` | Computes the rank of a finite list of supplied cycle vectors |

These routines operate on a user-supplied matrix modulo a prime. They do not build étale cohomology, a Galois representation, or the required comparison between algebraic cycles and invariant classes.

### 17. Matrix Multiplication Exponent ω = 2

| Function | Contract |
|---|---|
| `pl_math_matrix_multiply_f64` | Multiplies two bounded dense square matrices using long-double accumulation and a fixed scratch buffer |
| `pl_math_matrix_product_verify_f64` | Checks a proposed matrix product against a tolerance |
| `pl_math_matrix_scaling_exponent` | Estimates the empirical two-point runtime exponent from two measurements |

The scaling estimate is a measurement descriptor, not an asymptotic exponent proof. Benchmark runs must record warm-up, repetitions, hardware, compiler, threading and memory-layout details.

## Resource limits and status codes

The public limits are compile-time constants in `research_math.h`:

- Riemann series: at most 1,000,000 terms.
- CNF/SAT: at most 24 variables.
- Dense matrices: at most 128 × 128.
- Finite-field matrices: at most 64 dimensions.
- Unit-distance graph: at most 128 vertices.
- Polygonal curve sample buffer: at most 128 points.
- Elliptic point count: prime p ≤ 1,000,003.
- Perfect-cuboid bounded search: maximum edge ≤ 100,000, plus explicit node budget.

Return values:

| Status | Meaning |
|---|---|
| `PL_MATH_OK` | Operation completed within its declared domain |
| `PL_MATH_INVALID` | Invalid inputs or unsupported domain |
| `PL_MATH_OVERFLOW` | Exact integer or finite numeric range exceeded |
| `PL_MATH_LIMIT` | Explicit budget or capacity exhausted |
| `PL_MATH_NOT_FOUND` | The declared finite search completed without a witness |

A status is not an epistemic verdict on a conjecture.

## Build and test

CMake adds `src/research_math.c` to `pytyliki_core`, installs `research_math.h`, and registers the native test executable:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

The focused test source is `tests/test_research_math.c`. It covers representative cases across all 17 domains and verifies that the catalog contains exactly 51 stable function names.

## Evidence discipline

Every external consumer must preserve the exact function identifier and versioned input contract. Future run reports should distinguish:

- exact finite witness;
- exhaustive finite search;
- bounded search with remaining unexplored space;
- floating-point approximation;
- numerical discretization;
- model-only observable;
- compiler/target measurement.

No report may label a conjecture `PROVED` or `DISPROVED` based only on this workbench. Proof objects require a separately specified proof calculus, proof checker, source provenance and independently reproducible verification.
