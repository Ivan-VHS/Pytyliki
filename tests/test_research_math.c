#include "research_math.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define CHECK_OK(call) assert((call) == PL_MATH_OK)
#define NEAR(a, b, eps) assert(fabs((a) - (b)) <= (eps))

static void test_catalog_and_riemann(void) {
    pl_math_complex value;
    double residual = 0.0;
    assert(pl_math_research_catalog_count() == 51u);
    assert(pl_math_research_catalog_name(0u) != NULL);
    assert(pl_math_research_catalog_name(50u) != NULL);
    assert(pl_math_research_catalog_name(51u) == NULL);
    CHECK_OK(pl_math_riemann_dirichlet(2.0, 0.0, 50000u, &value));
    NEAR(value.real, 1.6449, 0.001);
    NEAR(value.imag, 0.0, 1e-12);
    CHECK_OK(pl_math_riemann_eta(1.0, 0.0, 20000u, &value));
    NEAR(value.real, log(2.0), 0.0001);
    CHECK_OK(pl_math_riemann_critical_residual(14.134725, 50000u, &residual));
    assert(isfinite(residual) && residual >= 0.0);
    assert(pl_math_riemann_dirichlet(0.5, 1.0, 100u, &value) == PL_MATH_INVALID);
}

static void test_sat(void) {
    const int32_t literals[] = {1, 2, -1};
    const size_t offsets[] = {0u, 2u, 3u};
    uint8_t assignment[2] = {0u, 1u};
    uint64_t checked = 0u;
    int satisfied = 0;
    CHECK_OK(pl_math_cnf_validate(2u, literals, 3u, offsets, 2u));
    CHECK_OK(pl_math_sat_verify_assignment(2u, literals, 3u, offsets, 2u, assignment, &satisfied));
    assert(satisfied == 1);
    assignment[0] = 0u;
    assignment[1] = 0u;
    CHECK_OK(pl_math_sat_find_model_bounded(2u, literals, 3u, offsets, 2u, 4u, assignment, &checked));
    assert(checked <= 4u);
    assert(pl_math_sat_verify_assignment(2u, literals, 3u, offsets, 2u, assignment, &satisfied) == PL_MATH_OK);
    assert(satisfied == 1);
    assert(pl_math_cnf_validate(2u, literals, 3u, offsets, 1u) == PL_MATH_INVALID);
}

static void test_collatz(void) {
    uint64_t next = 0u, steps = 0u, sequence[32];
    size_t count = 0u;
    int reached = 0;
    CHECK_OK(pl_math_collatz_step(7u, &next));
    assert(next == 22u);
    CHECK_OK(pl_math_collatz_trajectory(3u, 20u, sequence, 32u, &count, &reached));
    assert(reached == 1 && sequence[0] == 3u && sequence[count - 1u] == 1u);
    CHECK_OK(pl_math_collatz_stopping_time(3u, 20u, &steps));
    assert(steps == 7u);
    assert(pl_math_collatz_step(UINT64_MAX, &next) == PL_MATH_OVERFLOW);
    assert(pl_math_collatz_stopping_time(27u, 1u, &steps) == PL_MATH_LIMIT);
}

static void test_navier_stokes_diagnostics(void) {
    double u[8], v[8], w[8], div[8], ox[8], oy[8], oz[8], dt = 0.0;
    for (size_t z = 0u; z < 2u; ++z) for (size_t y = 0u; y < 2u; ++y) for (size_t x = 0u; x < 2u; ++x) {
        size_t i = x + 2u * (y + 2u * z);
        u[i] = (double)x; v[i] = (double)y; w[i] = (double)z;
    }
    CHECK_OK(pl_math_grid3d_divergence(u, v, w, 2u, 2u, 2u, 1.0, 1.0, 1.0, div));
    CHECK_OK(pl_math_grid3d_vorticity(u, v, w, 2u, 2u, 2u, 1.0, 1.0, 1.0, ox, oy, oz));
    for (size_t i = 0u; i < 8u; ++i) {
        NEAR(div[i], 3.0, 1e-12);
        NEAR(ox[i], 0.0, 1e-12);
        NEAR(oy[i], 0.0, 1e-12);
        NEAR(oz[i], 0.0, 1e-12);
    }
    CHECK_OK(pl_math_navier_stokes_cfl_dt(2.0, 0.5, 0.5, 0.5, 0.01, 0.5, &dt));
    assert(dt > 0.0 && dt < 0.1);
}

static void test_bsd_hodge_and_tate(void) {
    int nonsingular = 0, hodge = 0, fixed = 0;
    uint32_t point_count = 0u;
    int64_t trace = 0;
    const uint32_t cycles[] = {1u, 0u, 0u, 1u};
    const uint32_t candidate[] = {1u, 1u};
    size_t rank = 0u, invariant_dimension = 0u;
    const uint32_t identity[] = {1u, 0u, 0u, 1u};
    const uint32_t vector[] = {2u, 3u};
    CHECK_OK(pl_math_elliptic_curve_nonsingular_mod_prime(0u, 1u, 5u, &nonsingular));
    assert(nonsingular == 1);
    CHECK_OK(pl_math_elliptic_curve_point_count_mod_prime(0u, 1u, 5u, &point_count));
    CHECK_OK(pl_math_elliptic_curve_frobenius_trace_mod_prime(0u, 1u, 5u, &trace));
    assert(trace == 6 - (int64_t)point_count);
    CHECK_OK(pl_math_hodge_type_check(4u, 2u, 2u, &hodge));
    assert(hodge == 1);
    CHECK_OK(pl_math_hodge_cycle_span_rank_mod_prime(cycles, 2u, 2u, 5u, &rank));
    assert(rank == 2u);
    CHECK_OK(pl_math_hodge_class_in_cycle_span_mod_prime(cycles, 2u, 2u, candidate, 5u, &hodge));
    assert(hodge == 1);
    CHECK_OK(pl_math_tate_fixed_vector_verify(identity, 2u, vector, 5u, &fixed));
    assert(fixed == 1);
    CHECK_OK(pl_math_tate_invariant_dimension_mod_prime(identity, 2u, 5u, &invariant_dimension));
    assert(invariant_dimension == 2u);
    CHECK_OK(pl_math_tate_cycle_span_rank_mod_prime(cycles, 2u, 2u, 5u, &rank));
    assert(rank == 2u);
}

static void test_yang_mills_toy_model(void) {
    const double angles[4] = {0.5, 0.5, 0.5, 0.5};
    const int8_t orientation[4] = {1, -1, 1, -1};
    double phase = 0.0, action = 0.0, mass = 0.0;
    CHECK_OK(pl_math_u1_plaquette_phase(angles, orientation, &phase));
    NEAR(phase, 0.0, 1e-12);
    CHECK_OK(pl_math_u1_wilson_action(2.0, phase, &action));
    NEAR(action, 0.0, 1e-12);
    CHECK_OK(pl_math_correlator_effective_mass(1.0, 0.5, 0.25, &mass));
    NEAR(mass, log(2.0) / 0.25, 1e-12);
}

static void test_goldbach_and_twins(void) {
    uint64_t a = 0u, b = 0u, checked = 0u, count = 0u;
    int prime = 0, valid = 0;
    CHECK_OK(pl_math_is_prime_u64(97u, &prime)); assert(prime == 1);
    CHECK_OK(pl_math_goldbach_verify(100u, 3u, &valid)); assert(valid == 1);
    CHECK_OK(pl_math_goldbach_find_pair_bounded(100u, 50u, &a, &b, &checked));
    assert(a + b == 100u && checked > 0u);
    CHECK_OK(pl_math_twin_prime_verify(11u, &valid)); assert(valid == 1);
    CHECK_OK(pl_math_twin_prime_next_bounded(10u, 20u, &a, &checked)); assert(a == 11u);
    CHECK_OK(pl_math_twin_prime_count_bounded(20u, 20u, &count, &checked)); assert(count == 4u);
}

static void test_bounded_halting_machine(void) {
    const pl_math_vm_instruction program[] = {
        {PL_MATH_VM_INCREMENT, 0u, 0u},
        {PL_MATH_VM_HALT, 0u, 0u}
    };
    pl_math_vm_state state = {0u, {0u, 0u}, 0u, 0u};
    pl_math_vm_state trace[3];
    trace[0] = state;
    CHECK_OK(pl_math_vm_step(program, 2u, &trace[0], &trace[1]));
    CHECK_OK(pl_math_vm_step(program, 2u, &trace[1], &trace[2]));
    CHECK_OK(pl_math_vm_trace_verify(program, 2u, trace, 3u));
    CHECK_OK(pl_math_vm_run_bounded(program, 2u, &state, 2u));
    assert(state.halted == 1u && state.registers[0] == 1u);
    state.pc = 0u; state.halted = 0u; state.steps = 0u;
    assert(pl_math_vm_run_bounded(program, 2u, &state, 1u) == PL_MATH_LIMIT);
}

static void test_unit_distance_graph(void) {
    const int64_t points[] = {0,0, 1,0, 0,1};
    pl_math_edge edges[3];
    size_t edge_count = 0u;
    uint8_t colors[3] = {0u, 1u, 1u};
    uint64_t nodes = 0u;
    int valid = 0;
    CHECK_OK(pl_math_unit_graph_build(points, 3u, 1u, edges, 3u, &edge_count));
    assert(edge_count == 2u);
    CHECK_OK(pl_math_graph_coloring_verify(3u, edges, edge_count, colors, 2u, &valid));
    assert(valid == 1);
    CHECK_OK(pl_math_graph_coloring_search_bounded(3u, edges, edge_count, 2u, 100u, colors, &nodes));
    assert(nodes > 0u);
}

static void test_inscribed_square(void) {
    const pl_math_point2 curve[] = {{0,0},{1,0},{1,1},{0,1},{0,0}};
    const pl_math_point2 square[] = {{0,0},{1,0},{1,1},{0,1}};
    pl_math_point2 found[4];
    uint64_t triples = 0u;
    int valid = 0;
    CHECK_OK(pl_math_square_on_polyline_verify(curve, 5u, square, 1e-10, &valid));
    assert(valid == 1);
    CHECK_OK(pl_math_inscribed_square_search_sampled(curve, 5u, 1u, 1e-10, found, &triples));
    assert(triples > 0u);
}

static void test_legendre_beal_and_cuboid(void) {
    uint64_t lower = 0u, upper = 0u, prime = 0u, checked = 0u, common_factor = 0u;
    uint64_t diagonals[3], edges[3];
    int valid = 0, equal = 0, candidate = 0, all_integer = 0, perfect = 0;
    CHECK_OK(pl_math_legendre_interval(3u, &lower, &upper));
    assert(lower == 9u && upper == 16u);
    CHECK_OK(pl_math_legendre_witness_verify(3u, 11u, &valid));
    assert(valid == 1);
    CHECK_OK(pl_math_legendre_find_prime_bounded(3u, 6u, &prime, &checked));
    assert(prime == 11u);
    CHECK_OK(pl_math_beal_power_sum_check(2u, 3u, 1u, 3u, 3u, 2u, &equal));
    assert(equal == 1);
    CHECK_OK(pl_math_beal_common_prime_divisor(6u, 12u, 18u, &common_factor));
    assert(common_factor == 2u);
    assert(pl_math_beal_common_prime_divisor(1000003u * 1000033u,
                                             2u * 1000003u * 1000033u,
                                             3u * 1000003u * 1000033u,
                                             &common_factor) == PL_MATH_LIMIT);
    CHECK_OK(pl_math_beal_counterexample_screen(2u, 3u, 3u, 3u, 5u, 3u, &candidate));
    assert(candidate == 0);
    CHECK_OK(pl_math_cuboid_face_diagonals(1u, 2u, 2u, diagonals, &all_integer));
    assert(all_integer == 0);
    CHECK_OK(pl_math_perfect_cuboid_verify(1u, 2u, 2u, &perfect));
    assert(perfect == 0);
    assert(pl_math_perfect_cuboid_search_bounded(100000u, 1u, edges, &checked) == PL_MATH_LIMIT);
    assert(checked == 1u);
    assert(pl_math_cnf_validate(1u, NULL, 0u, (const size_t[]){0u}, PL_MATH_MAX_TERMS + 1u) == PL_MATH_INVALID);
}

static void test_matrix_multiplication(void) {
    const double a[] = {1,2,3,4};
    const double b[] = {5,6,7,8};
    const double expected[] = {19,22,43,50};
    double out[4], exponent = 0.0;
    int valid = 0;
    CHECK_OK(pl_math_matrix_multiply_f64(a,b,2u,out));
    for (size_t i = 0u; i < 4u; ++i) NEAR(out[i], expected[i], 1e-12);
    CHECK_OK(pl_math_matrix_product_verify_f64(a,b,expected,2u,1e-12,&valid));
    assert(valid == 1);
    CHECK_OK(pl_math_matrix_scaling_exponent(10u,1.0,100u,100.0,&exponent));
    NEAR(exponent,2.0,1e-12);
}

int main(void) {
    test_catalog_and_riemann();
    test_sat();
    test_collatz();
    test_navier_stokes_diagnostics();
    test_bsd_hodge_and_tate();
    test_yang_mills_toy_model();
    test_goldbach_and_twins();
    test_bounded_halting_machine();
    test_unit_distance_graph();
    test_inscribed_square();
    test_legendre_beal_and_cuboid();
    test_matrix_multiplication();
    puts("research-math: all bounded-kernel checks passed");
    return 0;
}
