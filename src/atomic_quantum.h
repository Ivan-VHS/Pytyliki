#ifndef PYTYLIKI_ATOMIC_QUANTUM_H
#define PYTYLIKI_ATOMIC_QUANTUM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_AQ_MAX_QUBITS 10u
#define PL_AQ_MAX_AMPLITUDES (1u << PL_AQ_MAX_QUBITS)

typedef struct {
    size_t qubits;
    size_t amplitude_count;
    double *real;
    double *imag;
} pl_aq_state;

typedef struct {
    unsigned outcome;
    double probability_one;
} pl_aq_measurement;

/* Catalogue lookup includes all 133 stable IDs. */
size_t pl_aq_function_count(void);
const char *pl_aq_function_id(size_t index);
int pl_aq_function_find(const char *function_id);

/* State-vector primitives. Return 0 on success and -1 on invalid input. */
int pl_aq_state_init(pl_aq_state *state, size_t qubits);
void pl_aq_state_free(pl_aq_state *state);
int pl_aq_state_set_basis(pl_aq_state *state, size_t basis_index);
int pl_aq_state_normalize(pl_aq_state *state);
int pl_aq_state_valid(const pl_aq_state *state);
int pl_aq_hadamard(pl_aq_state *state, size_t target);
int pl_aq_phase_rotate(pl_aq_state *state, size_t target, double theta);
int pl_aq_pauli_x(pl_aq_state *state, size_t target);
int pl_aq_pauli_z(pl_aq_state *state, size_t target);
int pl_aq_cnot(pl_aq_state *state, size_t control, size_t target);
int pl_aq_swap(pl_aq_state *state, size_t left, size_t right);
int pl_aq_measure_z(pl_aq_state *state, size_t target, double sample,
                    pl_aq_measurement *measurement);
int pl_aq_dephase(pl_aq_state *state, size_t target, double probability,
                  uint64_t seed);
int pl_aq_amplitude_damp(pl_aq_state *state, size_t target, double gamma,
                         double sample);
int pl_aq_state_probability(const pl_aq_state *state, size_t basis_index,
                             double *probability);
int pl_aq_fidelity(const pl_aq_state *left, const pl_aq_state *right,
                   double *fidelity);

#ifdef __cplusplus
}
#endif
#endif
