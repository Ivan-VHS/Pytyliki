#include "atomic_quantum.h"

#include <assert.h>
#include <math.h>
#include <string.h>

int main(void) {
    assert(pl_aq_function_count() == 133u);
    assert(strcmp(pl_aq_function_id(0u), "atom_superpose_basis_01") == 0);
    assert(strcmp(pl_aq_function_id(132u), "telem_system_health_133") == 0);
    assert(pl_aq_function_find("entangle_bell_pair_23") == 22);
    assert(pl_aq_function_find("not_a_quantum_function") == -1);
    assert(pl_aq_function_id(133u) == NULL);

    pl_aq_state state = {0};
    assert(pl_aq_state_init(&state, 2u) == 0);
    assert(pl_aq_state_valid(&state));
    assert(pl_aq_hadamard(&state, 0u) == 0);
    assert(pl_aq_cnot(&state, 0u, 1u) == 0);
    double p0 = 0.0, p3 = 0.0;
    assert(pl_aq_state_probability(&state, 0u, &p0) == 0);
    assert(pl_aq_state_probability(&state, 3u, &p3) == 0);
    assert(fabs(p0 - 0.5) < 1e-12 && fabs(p3 - 0.5) < 1e-12);

    pl_aq_state copy = {0};
    assert(pl_aq_state_init(&copy, 2u) == 0);
    assert(pl_aq_hadamard(&copy, 0u) == 0);
    assert(pl_aq_cnot(&copy, 0u, 1u) == 0);
    double overlap = 0.0;
    assert(pl_aq_fidelity(&state, &copy, &overlap) == 0);
    assert(fabs(overlap - 1.0) < 1e-12);

    pl_aq_measurement measurement = {0u, 0.0};
    assert(pl_aq_measure_z(&state, 0u, 0.2, &measurement) == 0);
    assert(measurement.outcome == 1u);
    assert(pl_aq_state_valid(&state));

    assert(pl_aq_dephase(&copy, 0u, 1.0, UINT64_C(17)) == 0);
    assert(pl_aq_amplitude_damp(&copy, 1u, 0.2, 0.2) == 0);
    assert(pl_aq_state_valid(&copy));

    assert(pl_aq_phase_rotate(&copy, 0u, 0.5) == 0);
    assert(pl_aq_pauli_x(&copy, 0u) == 0);
    assert(pl_aq_pauli_z(&copy, 0u) == 0);
    assert(pl_aq_swap(&copy, 0u, 1u) == 0);
    assert(pl_aq_state_set_basis(&copy, 3u) == 0);
    assert(pl_aq_state_valid(&copy));

    pl_aq_state_free(&copy);
    pl_aq_state_free(&state);
    assert(pl_aq_state_init(&state, PL_AQ_MAX_QUBITS + 1u) == -1);
    return 0;
}
