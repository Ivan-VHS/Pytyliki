#include "clinical_safety.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int valid_token(const char *value, size_t capacity) {
    size_t i;
    if (value == NULL || capacity == 0u || value[0] == '\0') return 0;
    for (i = 0u; i < capacity; ++i) {
        unsigned char c = (unsigned char)value[i];
        if (c == '\0') return 1;
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return 0;
    }
    return 0;
}

static int in_window(int64_t t, int64_t start, int64_t end) {
    return t >= start && t <= end;
}

static int participant_seen_before(
    const pl_contact_event *events, size_t current,
    const char *person_id, int64_t start, int64_t end
) {
    size_t j;
    for (j = 0u; j < current; ++j) {
        if (!in_window(events[j].timestamp_utc, start, end)) continue;
        if (strcmp(events[j].person_a, person_id) == 0 ||
            strcmp(events[j].person_b, person_id) == 0) return 1;
    }
    return 0;
}

int pl_contact_graph_analyze(
    const pl_contact_event *events, size_t event_count,
    const char *const *confirmed_ids, size_t confirmed_count,
    int64_t window_start_utc, int64_t window_end_utc,
    size_t min_confirmed_people_per_zone, pl_contact_graph_summary *out
) {
    size_t i;
    pl_contact_graph_summary result = {0u, 0u, 0u};
    if (out == NULL || window_start_utc > window_end_utc ||
        event_count > PL_CLINICAL_MAX_EVENTS ||
        confirmed_count > PL_CLINICAL_MAX_POSITIVE_IDS ||
        (event_count > 0u && events == NULL) ||
        (confirmed_count > 0u && confirmed_ids == NULL) ||
        (confirmed_count > 0u && min_confirmed_people_per_zone == 0u)) return -1;

    for (i = 0u; i < confirmed_count; ++i) {
        size_t j;
        if (!valid_token(confirmed_ids[i], PL_CLINICAL_ID_MAX)) return -1;
        for (j = 0u; j < i; ++j)
            if (strcmp(confirmed_ids[i], confirmed_ids[j]) == 0) return -1;
    }

    for (i = 0u; i < event_count; ++i) {
        const pl_contact_event *event = &events[i];
        if (!valid_token(event->person_a, sizeof(event->person_a)) ||
            !valid_token(event->person_b, sizeof(event->person_b)) ||
            !valid_token(event->zone_id, sizeof(event->zone_id))) return -1;
        if (!in_window(event->timestamp_utc, window_start_utc, window_end_utc)) continue;
        result.events_in_window++;
        if (!participant_seen_before(events, i, event->person_a,
                                     window_start_utc, window_end_utc))
            result.unique_participants++;
        if (strcmp(event->person_a, event->person_b) != 0 &&
            !participant_seen_before(events, i, event->person_b,
                                     window_start_utc, window_end_utc))
            result.unique_participants++;
    }

    for (i = 0u; i < event_count && confirmed_count > 0u; ++i) {
        size_t j;
        int zone_seen_before = 0;
        unsigned char confirmed_seen[PL_CLINICAL_MAX_POSITIVE_IDS] = {0};
        size_t confirmed_in_zone = 0u;

        if (!in_window(events[i].timestamp_utc, window_start_utc, window_end_utc)) continue;
        for (j = 0u; j < i; ++j) {
            if (in_window(events[j].timestamp_utc, window_start_utc, window_end_utc) &&
                strcmp(events[j].zone_id, events[i].zone_id) == 0) {
                zone_seen_before = 1;
                break;
            }
        }
        if (zone_seen_before) continue;

        for (j = i; j < event_count; ++j) {
            size_t k;
            if (!in_window(events[j].timestamp_utc, window_start_utc, window_end_utc) ||
                strcmp(events[j].zone_id, events[i].zone_id) != 0) continue;
            for (k = 0u; k < confirmed_count; ++k) {
                if (!confirmed_seen[k] &&
                    (strcmp(events[j].person_a, confirmed_ids[k]) == 0 ||
                     strcmp(events[j].person_b, confirmed_ids[k]) == 0)) {
                    confirmed_seen[k] = 1u;
                    confirmed_in_zone++;
                }
            }
            if (confirmed_in_zone >= min_confirmed_people_per_zone) break;
        }
        if (confirmed_in_zone >= min_confirmed_people_per_zone) result.candidate_zones++;
    }
    *out = result;
    return 0;
}

static int valid_range(double minimum, double maximum) {
    return isfinite(minimum) && isfinite(maximum) && minimum <= maximum;
}

int pl_sterilization_evaluate(
    const pl_sterilization_profile *profile,
    const pl_sterilization_cycle *cycle,
    pl_sterilization_evaluation *out
) {
    pl_sterilization_evaluation result;
    if (profile == NULL || cycle == NULL || out == NULL ||
        !valid_token(profile->profile_id, sizeof(profile->profile_id)) ||
        !valid_token(profile->reference_id, sizeof(profile->reference_id)) ||
        !valid_token(cycle->cycle_id, sizeof(cycle->cycle_id)) ||
        !valid_token(cycle->sterilizer_id, sizeof(cycle->sterilizer_id)) ||
        !valid_token(cycle->load_id, sizeof(cycle->load_id)) ||
        !valid_range(profile->min_temperature_c, profile->max_temperature_c) ||
        !valid_range(profile->min_pressure_kpa_abs, profile->max_pressure_kpa_abs) ||
        !valid_range(profile->min_humidity_pct, profile->max_humidity_pct) ||
        !valid_range(profile->min_exposure_seconds, profile->max_exposure_seconds) ||
        !isfinite(cycle->temperature_c) || !isfinite(cycle->pressure_kpa_abs) ||
        !isfinite(cycle->humidity_pct) || !isfinite(cycle->exposure_seconds)) return -1;

    result.out_of_profile_mask = 0u;
    if (cycle->temperature_c < profile->min_temperature_c ||
        cycle->temperature_c > profile->max_temperature_c)
        result.out_of_profile_mask |= PL_STERILIZATION_TEMP_OUT_OF_PROFILE;
    if (cycle->pressure_kpa_abs < profile->min_pressure_kpa_abs ||
        cycle->pressure_kpa_abs > profile->max_pressure_kpa_abs)
        result.out_of_profile_mask |= PL_STERILIZATION_PRESSURE_OUT_OF_PROFILE;
    if (cycle->humidity_pct < profile->min_humidity_pct ||
        cycle->humidity_pct > profile->max_humidity_pct)
        result.out_of_profile_mask |= PL_STERILIZATION_HUMIDITY_OUT_OF_PROFILE;
    if (cycle->exposure_seconds < profile->min_exposure_seconds ||
        cycle->exposure_seconds > profile->max_exposure_seconds)
        result.out_of_profile_mask |= PL_STERILIZATION_EXPOSURE_OUT_OF_PROFILE;
    result.within_configured_profile = result.out_of_profile_mask == 0u;
    *out = result;
    return 0;
}

int pl_sterilization_chain_digest(
    const unsigned char previous_digest[32],
    const pl_sterilization_profile *profile,
    const pl_sterilization_cycle *cycle,
    const pl_sterilization_evaluation *evaluation,
    pl_sha256_provider sha256, void *provider_context,
    unsigned char digest_out[32]
) {
    unsigned char input[32u + 768u];
    int written;
    if (previous_digest == NULL || profile == NULL || cycle == NULL ||
        evaluation == NULL || sha256 == NULL || digest_out == NULL ||
        !valid_token(profile->profile_id, sizeof(profile->profile_id)) ||
        !valid_token(profile->reference_id, sizeof(profile->reference_id)) ||
        !valid_token(cycle->cycle_id, sizeof(cycle->cycle_id)) ||
        !valid_token(cycle->sterilizer_id, sizeof(cycle->sterilizer_id)) ||
        !valid_token(cycle->load_id, sizeof(cycle->load_id)) ||
        !isfinite(cycle->temperature_c) || !isfinite(cycle->pressure_kpa_abs) ||
        !isfinite(cycle->humidity_pct) || !isfinite(cycle->exposure_seconds)) return -1;

    memcpy(input, previous_digest, 32u);
    written = snprintf((char *)input + 32u, sizeof(input) - 32u,
        "PYTYLIKI-STERILIZATION-LEDGER-v1|%s|%s|%s|%s|%a|%a|%a|%a|%u|%d",
        profile->profile_id, profile->reference_id,
        cycle->cycle_id, cycle->sterilizer_id,
        cycle->temperature_c, cycle->pressure_kpa_abs,
        cycle->humidity_pct, cycle->exposure_seconds,
        evaluation->out_of_profile_mask, evaluation->within_configured_profile);
    if (written < 0 || (size_t)written >= sizeof(input) - 32u) return -1;
    return sha256(input, 32u + (size_t)written, digest_out, provider_context) == 0 ? 0 : -1;
}

int pl_cpr_review_measurement(
    const pl_cpr_bounds *bounds,
    const pl_cpr_observation *observation,
    unsigned *out_flags
) {
    unsigned flags = 0u;
    if (bounds == NULL || observation == NULL || out_flags == NULL ||
        !valid_range(bounds->min_rate_per_minute, bounds->max_rate_per_minute) ||
        !valid_range(bounds->min_depth_mm, bounds->max_depth_mm) ||
        !isfinite(bounds->max_signal_noise_fraction) ||
        bounds->max_signal_noise_fraction < 0.0 ||
        bounds->max_signal_noise_fraction > 1.0 ||
        !isfinite(observation->observed_rate_per_minute) ||
        !isfinite(observation->observed_depth_mm) ||
        !isfinite(observation->signal_noise_fraction) ||
        observation->signal_noise_fraction < 0.0 ||
        observation->signal_noise_fraction > 1.0) return -1;

    if (observation->observed_rate_per_minute < bounds->min_rate_per_minute)
        flags |= PL_CPR_RATE_BELOW_CONFIGURED_RANGE;
    if (observation->observed_rate_per_minute > bounds->max_rate_per_minute)
        flags |= PL_CPR_RATE_ABOVE_CONFIGURED_RANGE;
    if (observation->observed_depth_mm < bounds->min_depth_mm)
        flags |= PL_CPR_DEPTH_BELOW_CONFIGURED_RANGE;
    if (observation->observed_depth_mm > bounds->max_depth_mm)
        flags |= PL_CPR_DEPTH_ABOVE_CONFIGURED_RANGE;
    if (observation->signal_noise_fraction > bounds->max_signal_noise_fraction)
        flags |= PL_CPR_SIGNAL_QUALITY_LOW;
    *out_flags = flags;
    return 0;
}

int pl_rsi_review_checklist(
    const pl_rsi_checklist *checklist,
    unsigned required_steps,
    unsigned *out_missing_steps
) {
    const unsigned known_steps =
        PL_RSI_OXYGENATION_CONFIRMED | PL_RSI_SUCTION_READY |
        PL_RSI_MONITORING_READY | PL_RSI_PRIMARY_AIRWAY_READY |
        PL_RSI_BACKUP_AIRWAY_READY | PL_RSI_TEAM_BRIEFED |
        PL_RSI_MEDICATION_PLAN_CHECKED;
    if (checklist == NULL || out_missing_steps == NULL || required_steps == 0u ||
        (required_steps & ~known_steps) != 0u ||
        (checklist->confirmed_steps & ~known_steps) != 0u) return -1;
    *out_missing_steps = required_steps & ~checklist->confirmed_steps;
    return 0;
}

int pl_model_disagreement_analyze(
    const pl_model_vote *votes, size_t vote_count, pl_model_disagreement *out
) {
    size_t i, best_count = 0u, best_index = 0u;
    int tied = 0;
    double confidence_sum = 0.0;
    if (votes == NULL || out == NULL || vote_count == 0u ||
        vote_count > PL_CLINICAL_MAX_MODEL_VOTES) return -1;

    for (i = 0u; i < vote_count; ++i) {
        size_t j, count = 0u;
        if (!valid_token(votes[i].label, sizeof(votes[i].label)) ||
            !isfinite(votes[i].confidence) ||
            votes[i].confidence < 0.0 || votes[i].confidence > 1.0) return -1;
        confidence_sum += votes[i].confidence;
        for (j = 0u; j < vote_count; ++j)
            if (strcmp(votes[i].label, votes[j].label) == 0) count++;
        if (count > best_count) {
            best_count = count;
            best_index = i;
            tied = 0;
        } else if (count == best_count &&
                   strcmp(votes[i].label, votes[best_index].label) != 0) {
            tied = 1;
        }
    }
    memset(out, 0, sizeof(*out));
    memcpy(out->majority_label, votes[best_index].label,
           strlen(votes[best_index].label) + 1u);
    out->vote_count = vote_count;
    out->majority_votes = best_count;
    out->tied_majority = tied;
    out->disagreement_fraction = (double)(vote_count - best_count) / (double)vote_count;
    out->mean_reported_confidence = confidence_sum / (double)vote_count;
    return 0;
}

int pl_biomarker_trend_analyze(
    const pl_biomarker_point *points, size_t point_count, pl_biomarker_trend *out
) {
    size_t i;
    double mean_x = 0.0, mean_y = 0.0, numerator = 0.0, denominator = 0.0;
    double slope;
    if (points == NULL || out == NULL || point_count < 2u ||
        point_count > PL_CLINICAL_MAX_BIOMARKER_POINTS) return -1;

    for (i = 0u; i < point_count; ++i) {
        if (!isfinite(points[i].elapsed_days) ||
            !isfinite(points[i].concentration) || points[i].concentration <= 0.0 ||
            (i > 0u && points[i].elapsed_days <= points[i - 1u].elapsed_days)) return -1;
        mean_x += points[i].elapsed_days;
        mean_y += log(points[i].concentration);
    }
    mean_x /= (double)point_count;
    mean_y /= (double)point_count;

    for (i = 0u; i < point_count; ++i) {
        const double dx = points[i].elapsed_days - mean_x;
        const double dy = log(points[i].concentration) - mean_y;
        numerator += dx * dy;
        denominator += dx * dx;
    }
    if (denominator <= 0.0 || !isfinite(denominator)) return -1;
    slope = numerator / denominator;
    if (!isfinite(slope)) return -1;

    out->sample_count = point_count;
    out->log_slope_per_day = slope;
    out->increasing = slope > 0.0;
    out->concentration_doubling_time_days = slope > 0.0 ? log(2.0) / slope : 0.0;
    return 0;
}
