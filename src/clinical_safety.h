#ifndef PYTYLIKI_CLINICAL_SAFETY_H
#define PYTYLIKI_CLINICAL_SAFETY_H

/*
 * Pytyliki clinical-safety engineering interfaces.
 * Data quality, workflow audit and mathematical utilities only.
 * Not a medical device; do not use to diagnose, prescribe, control equipment,
 * restrict care, or replace an approved protocol and qualified judgement.
 */
#include <stddef.h>
#include <stdint.h>

#define PL_CLINICAL_ID_MAX 48u
#define PL_CLINICAL_MAX_EVENTS 4096u
#define PL_CLINICAL_MAX_POSITIVE_IDS 256u
#define PL_CLINICAL_MAX_MODEL_VOTES 32u
#define PL_CLINICAL_LABEL_MAX 32u
#define PL_CLINICAL_MAX_BIOMARKER_POINTS 128u

typedef struct {
    char person_a[PL_CLINICAL_ID_MAX];
    char person_b[PL_CLINICAL_ID_MAX];
    char zone_id[PL_CLINICAL_ID_MAX];
    int64_t timestamp_utc;
} pl_contact_event;

typedef struct {
    size_t events_in_window;
    size_t unique_participants;
    size_t candidate_zones;
} pl_contact_graph_summary;

/* Identifiers must be pseudonymised by the caller. Candidate zones are review flags only. */
int pl_contact_graph_analyze(
    const pl_contact_event *events, size_t event_count,
    const char *const *confirmed_ids, size_t confirmed_count,
    int64_t window_start_utc, int64_t window_end_utc,
    size_t min_confirmed_people_per_zone,
    pl_contact_graph_summary *out
);

typedef struct {
    char profile_id[PL_CLINICAL_ID_MAX];
    char reference_id[PL_CLINICAL_ID_MAX];
    double min_temperature_c, max_temperature_c;
    double min_pressure_kpa_abs, max_pressure_kpa_abs;
    double min_humidity_pct, max_humidity_pct;
    double min_exposure_seconds, max_exposure_seconds;
} pl_sterilization_profile;

typedef struct {
    char cycle_id[PL_CLINICAL_ID_MAX];
    char sterilizer_id[PL_CLINICAL_ID_MAX];
    char load_id[PL_CLINICAL_ID_MAX];
    double temperature_c;
    double pressure_kpa_abs;
    double humidity_pct;
    double exposure_seconds;
} pl_sterilization_cycle;

enum {
    PL_STERILIZATION_TEMP_OUT_OF_PROFILE = 1u << 0,
    PL_STERILIZATION_PRESSURE_OUT_OF_PROFILE = 1u << 1,
    PL_STERILIZATION_HUMIDITY_OUT_OF_PROFILE = 1u << 2,
    PL_STERILIZATION_EXPOSURE_OUT_OF_PROFILE = 1u << 3
};

typedef struct {
    unsigned out_of_profile_mask;
    int within_configured_profile;
} pl_sterilization_evaluation;

int pl_sterilization_evaluate(
    const pl_sterilization_profile *profile,
    const pl_sterilization_cycle *cycle,
    pl_sterilization_evaluation *out
);

/* The caller supplies a reviewed SHA-256 provider. This does not provide
 * storage, signatures, access control, WORM retention or physical immutability. */
typedef int (*pl_sha256_provider)(
    const unsigned char *data, size_t data_size,
    unsigned char digest_out[32], void *provider_context
);

int pl_sterilization_chain_digest(
    const unsigned char previous_digest[32],
    const pl_sterilization_profile *profile,
    const pl_sterilization_cycle *cycle,
    const pl_sterilization_evaluation *evaluation,
    pl_sha256_provider sha256, void *provider_context,
    unsigned char digest_out[32]
);

typedef struct {
    double min_rate_per_minute, max_rate_per_minute;
    double min_depth_mm, max_depth_mm;
    double max_signal_noise_fraction;
} pl_cpr_bounds;

typedef struct {
    double observed_rate_per_minute;
    double observed_depth_mm;
    double signal_noise_fraction;
} pl_cpr_observation;

enum {
    PL_CPR_RATE_BELOW_CONFIGURED_RANGE = 1u << 0,
    PL_CPR_RATE_ABOVE_CONFIGURED_RANGE = 1u << 1,
    PL_CPR_DEPTH_BELOW_CONFIGURED_RANGE = 1u << 2,
    PL_CPR_DEPTH_ABOVE_CONFIGURED_RANGE = 1u << 3,
    PL_CPR_SIGNAL_QUALITY_LOW = 1u << 4
};

int pl_cpr_review_measurement(
    const pl_cpr_bounds *bounds,
    const pl_cpr_observation *observation,
    unsigned *out_flags
);

enum {
    PL_RSI_OXYGENATION_CONFIRMED = 1u << 0,
    PL_RSI_SUCTION_READY = 1u << 1,
    PL_RSI_MONITORING_READY = 1u << 2,
    PL_RSI_PRIMARY_AIRWAY_READY = 1u << 3,
    PL_RSI_BACKUP_AIRWAY_READY = 1u << 4,
    PL_RSI_TEAM_BRIEFED = 1u << 5,
    PL_RSI_MEDICATION_PLAN_CHECKED = 1u << 6
};

typedef struct { unsigned confirmed_steps; } pl_rsi_checklist;
/* required_steps must come from an approved local checklist; this never blocks care. */
int pl_rsi_review_checklist(
    const pl_rsi_checklist *checklist,
    unsigned required_steps,
    unsigned *out_missing_steps
);

typedef struct {
    char label[PL_CLINICAL_LABEL_MAX];
    double confidence;
} pl_model_vote;

typedef struct {
    char majority_label[PL_CLINICAL_LABEL_MAX];
    size_t vote_count;
    size_t majority_votes;
    int tied_majority;
    double disagreement_fraction;
    double mean_reported_confidence;
} pl_model_disagreement;

int pl_model_disagreement_analyze(
    const pl_model_vote *votes, size_t vote_count,
    pl_model_disagreement *out
);

typedef struct {
    double elapsed_days;
    double concentration;
} pl_biomarker_point;

typedef struct {
    size_t sample_count;
    double log_slope_per_day;
    int increasing;
    double concentration_doubling_time_days;
} pl_biomarker_trend;

int pl_biomarker_trend_analyze(
    const pl_biomarker_point *points, size_t point_count,
    pl_biomarker_trend *out
);

#endif
