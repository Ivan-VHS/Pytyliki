#include "atomic_quantum.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static const char *const kFunctionIds[133] = {
    "atom_superpose_basis_01",
    "qubit_phase_rotate_02",
    "atom_ground_reset_03",
    "state_amplitude_normalize_04",
    "atom_rabi_pulse_05",
    "qubit_bloch_project_06",
    "atom_stark_shift_07",
    "state_coherent_blend_08",
    "atom_zeeman_split_09",
    "qubit_hadamard_twin_10",
    "atom_metastable_hold_11",
    "state_polar_encode_12",
    "atom_hyperfin_lock_13",
    "qubit_zx_twist_14",
    "atom_optical_trap_15",
    "state_phase_flip_mask_16",
    "atom_level_detune_17",
    "qubit_iswap_core_18",
    "atom_recoil_damp_19",
    "state_vacuum_seed_20",
    "atom_quadrupole_tilt_21",
    "qubit_sqrt_not_22",
    "entangle_bell_pair_23",
    "entangle_ghz_ring_24",
    "entangle_w_chain_25",
    "correlate_swap_gate_26",
    "entangle_cluster_grid_27",
    "correlate_concurrence_28",
    "entangle_singlet_lock_29",
    "entangle_triplet_mix_30",
    "correlate_bell_witness_31",
    "entangle_dicke_slice_32",
    "correlate_mutual_info_33",
    "entangle_star_hub_34",
    "correlate_negativity_35",
    "entangle_spin_squeeze_36",
    "correlate_discord_flux_37",
    "entangle_ladder_weave_38",
    "correlate_purity_check_39",
    "entangle_cat_super_40",
    "correlate_schmidt_rank_41",
    "entangle_braid_step_42",
    "correlate_trace_dist_43",
    "entangle_mesh_sync_44",
    "measure_projective_z_45",
    "measure_weak_probe_46",
    "measure_povm_frame_47",
    "feedback_phase_steer_48",
    "measure_parity_check_49",
    "measure_homodyne_quad_50",
    "feedback_error_synd_51",
    "measure_tomo_slice_52",
    "measure_bell_project_53",
    "feedback_gain_adapt_54",
    "measure_arrival_tick_55",
    "measure_double_click_56",
    "feedback_reset_gate_57",
    "measure_ramsey_fringe_58",
    "measure_fidelity_ref_59",
    "feedback_drift_trim_60",
    "measure_count_bucket_61",
    "measure_threshold_hit_62",
    "feedback_cooldown_hold_63",
    "measure_joint_moment_64",
    "measure_null_result_65",
    "feedback_loop_clamp_66",
    "noise_depolarize_ch_67",
    "noise_dephase_markov_68",
    "noise_amplitude_damp_69",
    "ecc_bitflip_code_70",
    "ecc_phaseflip_code_71",
    "ecc_surface_plaquette_72",
    "ecc_vertex_star_73",
    "noise_thermal_bath_74",
    "ecc_decoder_mwpm_75",
    "noise_crosstalk_leak_76",
    "ecc_logical_x_77",
    "ecc_logical_z_78",
    "noise_burst_spike_79",
    "ecc_flag_qubit_80",
    "noise_1f_flicker_81",
    "ecc_lattice_surgery_82",
    "noise_loss_erase_83",
    "ecc_magic_distill_84",
    "noise_non_markov_back_85",
    "ecc_steane_block_86",
    "noise_gate_overrot_87",
    "ecc_syndrome_history_88",
    "lattice_hop_tunnel_89",
    "lattice_hubbard_onsite_90",
    "atom_cool_sideband_91",
    "transport_shuttle_move_92",
    "lattice_peierls_flux_93",
    "atom_recoil_kick_94",
    "lattice_mott_gap_95",
    "transport_junction_split_96",
    "lattice_superfluid_phase_97",
    "atom_dark_state_98",
    "lattice_disorder_ramp_99",
    "transport_merge_wells_100",
    "lattice_rydberg_block_101",
    "atom_raman_transfer_102",
    "lattice_boundary_wrap_103",
    "transport_adiabatic_ramp_104",
    "lattice_dimer_pair_105",
    "atom_doppler_shift_106",
    "lattice_spin_exchange_107",
    "transport_trap_handoff_108",
    "lattice_anyon_braid_109",
    "atom_sisyphus_step_110",
    "algo_qaoa_cost_111",
    "algo_qaoa_mixer_112",
    "algo_vqe_ansatz_113",
    "algo_qpe_phase_114",
    "algo_grover_oracle_115",
    "algo_grover_diffuser_116",
    "algo_hhl_invert_117",
    "algo_walk_coin_118",
    "algo_walk_shift_119",
    "telem_coherence_time_120",
    "telem_gate_error_121",
    "telem_readout_conf_122",
    "telem_temp_jitter_123",
    "telem_entangle_depth_124",
    "telem_flux_balance_125",
    "algo_anneal_schedule_126",
    "algo_fourier_qft_127",
    "algo_amplitude_est_128",
    "telem_spectral_gap_129",
    "telem_drift_rate_130",
    "algo_teleport_link_131",
    "telem_packet_loss_132",
    "telem_system_health_133"
};

static int valid_probability(double value) {
    return isfinite(value) && value >= 0.0 && value <= 1.0;
}

static int valid_target(const pl_aq_state *state, size_t target) {
    return pl_aq_state_valid(state) && target < state->qubits;
}

static int valid_state_impl(const pl_aq_state *state, int require_normalized) {
    if (state == NULL || state->qubits == 0u || state->qubits > PL_AQ_MAX_QUBITS ||
        state->amplitude_count != ((size_t)1u << state->qubits) ||
        state->real == NULL || state->imag == NULL) return 0;
    long double norm = 0.0L;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if (!isfinite(state->real[i]) || !isfinite(state->imag[i])) return 0;
        norm += (long double)state->real[i] * state->real[i] +
                (long double)state->imag[i] * state->imag[i];
        if (!isfinite(norm)) return 0;
    }
    if (norm <= 0.0L) return 0;
    return !require_normalized || fabsl(norm - 1.0L) <= 1e-8L;
}

size_t pl_aq_function_count(void) {
    return sizeof(kFunctionIds) / sizeof(kFunctionIds[0]);
}

const char *pl_aq_function_id(size_t index) {
    return index < pl_aq_function_count() ? kFunctionIds[index] : NULL;
}

int pl_aq_function_find(const char *function_id) {
    if (function_id == NULL) return -1;
    for (size_t i = 0u; i < pl_aq_function_count(); ++i) {
        if (strcmp(kFunctionIds[i], function_id) == 0) return (int)i;
    }
    return -1;
}

int pl_aq_state_init(pl_aq_state *state, size_t qubits) {
    if (state == NULL || qubits == 0u || qubits > PL_AQ_MAX_QUBITS) return -1;
    pl_aq_state result = {0u, 0u, NULL, NULL};
    result.qubits = qubits;
    result.amplitude_count = (size_t)1u << qubits;
    result.real = (double *)calloc(result.amplitude_count, sizeof(double));
    result.imag = (double *)calloc(result.amplitude_count, sizeof(double));
    if (result.real == NULL || result.imag == NULL) {
        free(result.real);
        free(result.imag);
        return -1;
    }
    result.real[0] = 1.0;
    *state = result;
    return 0;
}

void pl_aq_state_free(pl_aq_state *state) {
    if (state == NULL) return;
    free(state->real);
    free(state->imag);
    state->qubits = 0u;
    state->amplitude_count = 0u;
    state->real = NULL;
    state->imag = NULL;
}

int pl_aq_state_valid(const pl_aq_state *state) {
    return valid_state_impl(state, 1);
}

int pl_aq_state_set_basis(pl_aq_state *state, size_t basis_index) {
    if (state == NULL || !valid_state_impl(state, 0) || basis_index >= state->amplitude_count) return -1;
    memset(state->real, 0, state->amplitude_count * sizeof(double));
    memset(state->imag, 0, state->amplitude_count * sizeof(double));
    state->real[basis_index] = 1.0;
    return 0;
}

int pl_aq_state_normalize(pl_aq_state *state) {
    if (!valid_state_impl(state, 0)) return -1;
    long double norm_squared = 0.0L;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        norm_squared += (long double)state->real[i] * state->real[i] +
                        (long double)state->imag[i] * state->imag[i];
    }
    if (!isfinite(norm_squared) || norm_squared <= 0.0L) return -1;
    const double scale = 1.0 / sqrt((double)norm_squared);
    if (!isfinite(scale)) return -1;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        state->real[i] *= scale;
        state->imag[i] *= scale;
    }
    return pl_aq_state_valid(state) ? 0 : -1;
}

int pl_aq_hadamard(pl_aq_state *state, size_t target) {
    if (!valid_target(state, target)) return -1;
    const size_t mask = (size_t)1u << target;
    const double scale = 1.0 / sqrt(2.0);
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if ((i & mask) == 0u) {
            const size_t j = i | mask;
            const double r0 = state->real[i], im0 = state->imag[i];
            const double r1 = state->real[j], im1 = state->imag[j];
            state->real[i] = (r0 + r1) * scale;
            state->imag[i] = (im0 + im1) * scale;
            state->real[j] = (r0 - r1) * scale;
            state->imag[j] = (im0 - im1) * scale;
        }
    }
    return pl_aq_state_valid(state) ? 0 : -1;
}

int pl_aq_phase_rotate(pl_aq_state *state, size_t target, double theta) {
    if (!valid_target(state, target) || !isfinite(theta)) return -1;
    const size_t mask = (size_t)1u << target;
    const double c = cos(theta), s = sin(theta);
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if (i & mask) {
            const double r = state->real[i], im = state->imag[i];
            state->real[i] = r * c - im * s;
            state->imag[i] = r * s + im * c;
        }
    }
    return pl_aq_state_valid(state) ? 0 : -1;
}

int pl_aq_pauli_x(pl_aq_state *state, size_t target) {
    if (!valid_target(state, target)) return -1;
    const size_t mask = (size_t)1u << target;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if ((i & mask) == 0u) {
            const size_t j = i | mask;
            double temp = state->real[i]; state->real[i] = state->real[j]; state->real[j] = temp;
            temp = state->imag[i]; state->imag[i] = state->imag[j]; state->imag[j] = temp;
        }
    }
    return pl_aq_state_valid(state) ? 0 : -1;
}

int pl_aq_pauli_z(pl_aq_state *state, size_t target) {
    if (!valid_target(state, target)) return -1;
    const size_t mask = (size_t)1u << target;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if (i & mask) { state->real[i] = -state->real[i]; state->imag[i] = -state->imag[i]; }
    }
    return pl_aq_state_valid(state) ? 0 : -1;
}

int pl_aq_cnot(pl_aq_state *state, size_t control, size_t target) {
    if (!valid_target(state, control) || !valid_target(state, target) || control == target) return -1;
    const size_t control_mask = (size_t)1u << control;
    const size_t target_mask = (size_t)1u << target;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if ((i & control_mask) && !(i & target_mask)) {
            const size_t j = i | target_mask;
            double temp = state->real[i]; state->real[i] = state->real[j]; state->real[j] = temp;
            temp = state->imag[i]; state->imag[i] = state->imag[j]; state->imag[j] = temp;
        }
    }
    return pl_aq_state_valid(state) ? 0 : -1;
}

int pl_aq_swap(pl_aq_state *state, size_t left, size_t right) {
    if (!valid_target(state, left) || !valid_target(state, right)) return -1;
    if (left == right) return 0;
    const size_t left_mask = (size_t)1u << left;
    const size_t right_mask = (size_t)1u << right;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if (!(i & left_mask) && (i & right_mask)) {
            const size_t j = i ^ left_mask ^ right_mask;
            double temp = state->real[i]; state->real[i] = state->real[j]; state->real[j] = temp;
            temp = state->imag[i]; state->imag[i] = state->imag[j]; state->imag[j] = temp;
        }
    }
    return pl_aq_state_valid(state) ? 0 : -1;
}

int pl_aq_state_probability(const pl_aq_state *state, size_t basis_index, double *probability) {
    if (!pl_aq_state_valid(state) || probability == NULL || basis_index >= state->amplitude_count) return -1;
    *probability = state->real[basis_index] * state->real[basis_index] +
                   state->imag[basis_index] * state->imag[basis_index];
    return isfinite(*probability) ? 0 : -1;
}

int pl_aq_measure_z(pl_aq_state *state, size_t target, double sample,
                    pl_aq_measurement *measurement) {
    if (!valid_target(state, target) || !isfinite(sample) || sample < 0.0 || sample >= 1.0 ||
        measurement == NULL) return -1;
    const size_t mask = (size_t)1u << target;
    long double p1 = 0.0L;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if (i & mask) p1 += (long double)state->real[i] * state->real[i] +
                            (long double)state->imag[i] * state->imag[i];
    }
    if (!isfinite(p1) || p1 < 0.0L || p1 > 1.0L + 1e-8L) return -1;
    const unsigned outcome = (long double)sample < p1 ? 1u : 0u;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if (((i & mask) != 0u) != (outcome != 0u)) {
            state->real[i] = 0.0;
            state->imag[i] = 0.0;
        }
    }
    if (pl_aq_state_normalize(state) != 0) return -1;
    measurement->outcome = outcome;
    measurement->probability_one = (double)p1;
    return 0;
}

static uint64_t next_random(uint64_t *state) {
    uint64_t x = *state;
    if (x == 0u) x = UINT64_C(0x9E3779B97F4A7C15);
    x ^= x >> 12u; x ^= x << 25u; x ^= x >> 27u;
    *state = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

static double uniform01(uint64_t *state) {
    return (double)(next_random(state) >> 11u) * (1.0 / 9007199254740992.0);
}

int pl_aq_dephase(pl_aq_state *state, size_t target, double probability, uint64_t seed) {
    if (!valid_target(state, target) || !valid_probability(probability)) return -1;
    if (uniform01(&seed) < probability) return pl_aq_pauli_z(state, target);
    return 0;
}

int pl_aq_amplitude_damp(pl_aq_state *state, size_t target, double gamma, double sample) {
    if (!valid_target(state, target) || !valid_probability(gamma) ||
        !isfinite(sample) || sample < 0.0 || sample >= 1.0) return -1;
    const size_t mask = (size_t)1u << target;
    long double p1 = 0.0L;
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if (i & mask) p1 += (long double)state->real[i] * state->real[i] +
                            (long double)state->imag[i] * state->imag[i];
    }
    const long double jump_probability = (long double)gamma * p1;
    const int jump = (long double)sample < jump_probability;
    const double scale = jump ? sqrt(gamma) : sqrt(1.0 - gamma);
    for (size_t i = 0u; i < state->amplitude_count; ++i) {
        if ((i & mask) == 0u) {
            const size_t j = i | mask;
            const double r0 = state->real[i], im0 = state->imag[i];
            const double r1 = state->real[j], im1 = state->imag[j];
            if (jump) {
                state->real[i] = r1 * scale; state->imag[i] = im1 * scale;
                state->real[j] = 0.0; state->imag[j] = 0.0;
            } else {
                state->real[i] = r0; state->imag[i] = im0;
                state->real[j] = r1 * scale; state->imag[j] = im1 * scale;
            }
        }
    }
    return pl_aq_state_normalize(state);
}

int pl_aq_fidelity(const pl_aq_state *left, const pl_aq_state *right, double *out) {
    if (!pl_aq_state_valid(left) || !pl_aq_state_valid(right) || out == NULL ||
        left->qubits != right->qubits) return -1;
    long double real = 0.0L, imag = 0.0L;
    for (size_t i = 0u; i < left->amplitude_count; ++i) {
        real += (long double)left->real[i] * right->real[i] +
                (long double)left->imag[i] * right->imag[i];
        imag += (long double)left->real[i] * right->imag[i] -
                (long double)left->imag[i] * right->real[i];
    }
    const long double value = real * real + imag * imag;
    if (!isfinite(value) || value < -1e-10L || value > 1.0L + 1e-8L) return -1;
    *out = (double)value;
    if (*out < 0.0) *out = 0.0;
    if (*out > 1.0) *out = 1.0;
    return 0;
}
