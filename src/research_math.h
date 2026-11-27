#ifndef PYTYLIKI_RESEARCH_MATH_H
#define PYTYLIKI_RESEARCH_MATH_H

#include <stddef.h>
#include <stdint.h>

#define PL_MATH_MAX_TERMS 1000000u
#define PL_MATH_MAX_CNF_VARIABLES 24u
#define PL_MATH_MAX_MATRIX_DIMENSION 128u
#define PL_MATH_MAX_FINITE_FIELD_DIMENSION 64u
#define PL_MATH_MAX_GRAPH_VERTICES 128u
#define PL_MATH_MAX_CURVE_SAMPLES 128u

typedef enum {
    PL_MATH_OK = 0,
    PL_MATH_INVALID = -1,
    PL_MATH_OVERFLOW = -2,
    PL_MATH_LIMIT = -3,
    PL_MATH_NOT_FOUND = -4
} pl_math_status;

typedef struct { double real; double imag; } pl_math_complex;
typedef struct { double x; double y; } pl_math_point2;
typedef struct { size_t left; size_t right; } pl_math_edge;

typedef enum {
    PL_MATH_VM_HALT = 0,
    PL_MATH_VM_INCREMENT = 1,
    PL_MATH_VM_DECREMENT_OR_JUMP = 2,
    PL_MATH_VM_JUMP = 3
} pl_math_vm_opcode;

typedef struct {
    uint8_t opcode;
    uint8_t reg;
    uint32_t target;
} pl_math_vm_instruction;

typedef struct {
    uint32_t pc;
    uint64_t registers[2];
    uint64_t steps;
    uint8_t halted;
} pl_math_vm_state;

size_t pl_math_research_catalog_count(void);
const char *pl_math_research_catalog_name(size_t index);

/* 01. Riemann hypothesis: truncated approximations, never a proof oracle. */
int pl_math_riemann_dirichlet(double sigma, double t, size_t terms, pl_math_complex *out);
int pl_math_riemann_eta(double sigma, double t, size_t terms, pl_math_complex *out);
int pl_math_riemann_critical_residual(double t, size_t terms, double *out_abs_value);

/* 02. P versus NP: finite CNF instances and bounded SAT search. */
int pl_math_cnf_validate(size_t variables, const int32_t *literals, size_t literal_count,
                         const size_t *clause_offsets, size_t clause_count);
int pl_math_sat_verify_assignment(size_t variables, const int32_t *literals, size_t literal_count,
                                  const size_t *clause_offsets, size_t clause_count,
                                  const uint8_t *assignment, int *satisfied);
int pl_math_sat_find_model_bounded(size_t variables, const int32_t *literals, size_t literal_count,
                                   const size_t *clause_offsets, size_t clause_count,
                                   uint64_t assignment_budget, uint8_t *assignment,
                                   uint64_t *assignments_checked);

/* 03. Collatz: exact uint64 trajectories with an explicit step budget. */
int pl_math_collatz_step(uint64_t value, uint64_t *next);
int pl_math_collatz_trajectory(uint64_t start, uint64_t step_budget, uint64_t *sequence,
                               size_t sequence_capacity, size_t *sequence_count, int *reached_one);
int pl_math_collatz_stopping_time(uint64_t start, uint64_t step_budget, uint64_t *steps);

/* 04. Navier-Stokes: finite-grid diagnostics and an explicit stability bound. */
int pl_math_grid3d_divergence(const double *u, const double *v, const double *w,
                              size_t nx, size_t ny, size_t nz,
                              double dx, double dy, double dz, double *out);
int pl_math_grid3d_vorticity(const double *u, const double *v, const double *w,
                             size_t nx, size_t ny, size_t nz,
                             double dx, double dy, double dz,
                             double *omega_x, double *omega_y, double *omega_z);
int pl_math_navier_stokes_cfl_dt(double max_speed, double dx, double dy, double dz,
                                 double viscosity, double safety, double *out_dt);

/* 05. Birch-Swinnerton-Dyer: exact finite-field elliptic-curve evidence. */
int pl_math_elliptic_curve_nonsingular_mod_prime(uint32_t a, uint32_t b, uint32_t prime, int *out);
int pl_math_elliptic_curve_point_count_mod_prime(uint32_t a, uint32_t b, uint32_t prime,
                                                 uint32_t *point_count);
int pl_math_elliptic_curve_frobenius_trace_mod_prime(uint32_t a, uint32_t b, uint32_t prime,
                                                     int64_t *trace);

/* 06. Hodge: finite-dimensional and finite-field checks only. */
int pl_math_hodge_type_check(unsigned cohomology_degree, unsigned p, unsigned q, int *is_hodge_type);
int pl_math_hodge_cycle_span_rank_mod_prime(const uint32_t *cycles, size_t cycle_count,
                                            size_t dimension, uint32_t prime, size_t *rank);
int pl_math_hodge_class_in_cycle_span_mod_prime(const uint32_t *cycles, size_t cycle_count,
                                                size_t dimension, const uint32_t *candidate,
                                                uint32_t prime, int *in_span);

/* 07. Yang-Mills: U(1) lattice toy observables, not a 4D existence proof. */
int pl_math_u1_plaquette_phase(const double link_angles[4], const int8_t orientations[4], double *out_phase);
int pl_math_u1_wilson_action(double beta, double plaquette_phase, double *out_action);
int pl_math_correlator_effective_mass(double c_t, double c_t1, double lattice_spacing, double *out_mass);

/* 08. Goldbach: finite witnesses. */
int pl_math_is_prime_u64(uint64_t value, int *is_prime);
int pl_math_goldbach_verify(uint64_t even_value, uint64_t prime_a, int *valid);
int pl_math_goldbach_find_pair_bounded(uint64_t even_value, uint64_t candidate_budget,
                                       uint64_t *prime_a, uint64_t *prime_b,
                                       uint64_t *candidates_checked);

/* 09. Twin primes: finite witness search. */
int pl_math_twin_prime_verify(uint64_t lower_prime, int *valid);
int pl_math_twin_prime_next_bounded(uint64_t lower_bound, uint64_t candidate_budget,
                                    uint64_t *lower_prime, uint64_t *candidates_checked);
int pl_math_twin_prime_count_bounded(uint64_t upper_bound, uint64_t candidate_budget,
                                     uint64_t *pair_count, uint64_t *candidates_checked);

/* 10. Halting: bounded two-counter machine execution, not a halting decider. */
int pl_math_vm_step(const pl_math_vm_instruction *program, size_t program_length,
                    const pl_math_vm_state *before, pl_math_vm_state *after);
int pl_math_vm_run_bounded(const pl_math_vm_instruction *program, size_t program_length,
                           pl_math_vm_state *state, uint64_t fuel);
int pl_math_vm_trace_verify(const pl_math_vm_instruction *program, size_t program_length,
                            const pl_math_vm_state *trace, size_t trace_length);

/* 11. Hadwiger-Nelson: finite unit-distance graphs and bounded coloring. */
int pl_math_unit_graph_build(const int64_t *xy, size_t vertex_count, uint32_t scale,
                             pl_math_edge *edges, size_t edge_capacity, size_t *edge_count);
int pl_math_graph_coloring_verify(size_t vertex_count, const pl_math_edge *edges,
                                  size_t edge_count, const uint8_t *colors,
                                  size_t color_count, int *valid);
int pl_math_graph_coloring_search_bounded(size_t vertex_count, const pl_math_edge *edges,
                                          size_t edge_count, uint8_t color_count,
                                          uint64_t node_budget, uint8_t *colors,
                                          uint64_t *nodes_visited);

/* 12. Inscribed square: polygonal-curve membership and sampled search. */
int pl_math_point_on_polyline(const pl_math_point2 *curve, size_t point_count,
                              pl_math_point2 point, double tolerance, int *on_curve);
int pl_math_square_on_polyline_verify(const pl_math_point2 *curve, size_t point_count,
                                      const pl_math_point2 square[4], double tolerance,
                                      int *valid);
int pl_math_inscribed_square_search_sampled(const pl_math_point2 *curve, size_t point_count,
                                            unsigned subdivisions, double tolerance,
                                            pl_math_point2 square[4], uint64_t *triples_checked);

/* 13. Legendre: exact finite interval construction and witness checks. */
int pl_math_legendre_interval(uint64_t n, uint64_t *lower_exclusive, uint64_t *upper_exclusive);
int pl_math_legendre_witness_verify(uint64_t n, uint64_t prime_candidate, int *valid);
int pl_math_legendre_find_prime_bounded(uint64_t n, uint64_t candidate_budget,
                                        uint64_t *prime_candidate, uint64_t *candidates_checked);

/* 14. Beal: checked integer-power identities and bounded candidate screening. */
int pl_math_beal_power_sum_check(uint64_t a, uint32_t x, uint64_t b, uint32_t y,
                                 uint64_t c, uint32_t z, int *equal);
int pl_math_beal_common_prime_divisor(uint64_t a, uint64_t b, uint64_t c, uint64_t *prime_factor);
int pl_math_beal_counterexample_screen(uint64_t a, uint32_t x, uint64_t b, uint32_t y,
                                       uint64_t c, uint32_t z, int *candidate);

/* 15. Perfect cuboid: checked integer geometry and bounded search. */
int pl_math_perfect_cuboid_verify(uint64_t a, uint64_t b, uint64_t c, int *perfect);
int pl_math_cuboid_face_diagonals(uint64_t a, uint64_t b, uint64_t c,
                                  uint64_t diagonals[3], int *all_integer);
int pl_math_perfect_cuboid_search_bounded(uint64_t max_edge, uint64_t node_budget,
                                          uint64_t edges[3], uint64_t *nodes_checked);

/* 16. Tate: Frobenius invariants over a declared finite field. */
int pl_math_tate_fixed_vector_verify(const uint32_t *frobenius, size_t dimension,
                                     const uint32_t *vector, uint32_t prime, int *fixed);
int pl_math_tate_invariant_dimension_mod_prime(const uint32_t *frobenius, size_t dimension,
                                               uint32_t prime, size_t *invariant_dimension);
int pl_math_tate_cycle_span_rank_mod_prime(const uint32_t *cycle_vectors, size_t cycle_count,
                                           size_t dimension, uint32_t prime, size_t *rank);

/* 17. Matrix multiplication exponent: finite kernels and empirical scaling. */
int pl_math_matrix_multiply_f64(const double *left, const double *right, size_t dimension, double *out);
int pl_math_matrix_product_verify_f64(const double *left, const double *right, const double *candidate,
                                      size_t dimension, double tolerance, int *valid);
int pl_math_matrix_scaling_exponent(size_t n1, double seconds1, size_t n2, double seconds2,
                                    double *estimated_exponent);

#endif
