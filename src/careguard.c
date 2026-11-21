#include "careguard.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

static int contains_ci(const char *text, const char *needle) {
    size_t i, j;
    if (text == NULL || needle == NULL || needle[0] == '\0') return 0;
    for (i = 0; text[i] != '\0'; ++i) {
        for (j = 0; needle[j] != '\0'; ++j) {
            unsigned char a = (unsigned char)text[i + j];
            unsigned char b = (unsigned char)needle[j];
            if (a == '\0' || tolower(a) != tolower(b)) break;
        }
        if (needle[j] == '\0') return 1;
    }
    return 0;
}

static int nonempty(const char *text) {
    if (text == NULL) return 0;
    while (*text != '\0' && isspace((unsigned char)*text)) ++text;
    return *text != '\0';
}

CareGuardAssessment careguard_assess_project_load(const CareGuardWorkload *workload) {
    CareGuardAssessment out = { CAREGUARD_LOAD_INVALID, 0u, 0u };
    uint32_t score = 0u;
    double utilization;
    if (workload == NULL || workload->worker_capacity == 0u) return out;
    if (workload->queued_items >= 16u) score += 25u;
    else if (workload->queued_items >= 8u) score += 12u;
    if (workload->failed_checks >= 3u) score += 30u;
    else if (workload->failed_checks > 0u) score += 12u;
    if (workload->alerts_per_hour >= 20u) score += 30u;
    else if (workload->alerts_per_hour >= 8u) score += 12u;
    utilization = (double)workload->active_workers / (double)workload->worker_capacity;
    if (utilization >= 1.0) score += 25u;
    else if (utilization >= 0.85) score += 12u;
    out.pressure_score = score > 100u ? 100u : score;
    if (score >= 60u) {
        out.status = CAREGUARD_LOAD_PAUSE;
        out.reserve_percent = 30u;
    } else if (score >= 25u) {
        out.status = CAREGUARD_LOAD_WATCH;
        out.reserve_percent = 20u;
    } else {
        out.status = CAREGUARD_LOAD_NORMAL;
        out.reserve_percent = 10u;
    }
    return out;
}

uint32_t careguard_review_language(const char *text) {
    static const char *const phrases[] = {
        "weakness", "lazy", "attention seeking", "just snap out of it",
        "слабость", "слабкість", "ленивый", "лінивий"
    };
    uint32_t count = 0u;
    size_t i;
    if (text == NULL) return 0u;
    for (i = 0u; i < sizeof(phrases) / sizeof(phrases[0]); ++i) {
        if (contains_ci(text, phrases[i])) ++count;
    }
    return count;
}

int careguard_accept_help_request(const char *message) {
    return nonempty(message);
}

CareGuardSupportPath careguard_select_support_path(const char *path) {
    if (path == NULL) return CAREGUARD_PATH_INVALID;
    if (strcmp(path, "trusted_person") == 0) return CAREGUARD_PATH_TRUSTED_PERSON;
    if (strcmp(path, "peer_support") == 0) return CAREGUARD_PATH_PEER_SUPPORT;
    if (strcmp(path, "professional") == 0) return CAREGUARD_PATH_PROFESSIONAL;
    if (strcmp(path, "urgent_human") == 0) return CAREGUARD_PATH_URGENT_HUMAN;
    return CAREGUARD_PATH_INVALID;
}

int careguard_review_contextual_claim(const char *claim, const char *evidence_ref) {
    return nonempty(claim) && nonempty(evidence_ref);
}

int careguard_check_clinical_boundary(const char *action) {
    static const char *const blocked[] = {
        "diagnose", "diagnosis", "prescribe", "prescription", "medication",
        "antidepressant", "dosage", "поставить диагноз", "поставити діагноз",
        "назначить препарат", "призначити препарат"
    };
    size_t i;
    if (!nonempty(action)) return 0;
    for (i = 0u; i < sizeof(blocked) / sizeof(blocked[0]); ++i) {
        if (contains_ci(action, blocked[i])) return 0;
    }
    return 1;
}

int careguard_allocate_resilience_buffer(uint32_t capacity_units, uint32_t reserve_percent,
                                         uint32_t *reserved_units, uint32_t *usable_units) {
    uint64_t reserved;
    if (reserved_units == NULL || usable_units == NULL || reserve_percent > 50u) return 0;
    reserved = ((uint64_t)capacity_units * (uint64_t)reserve_percent + 99u) / 100u;
    if (reserved > capacity_units) return 0;
    *reserved_units = (uint32_t)reserved;
    *usable_units = capacity_units - *reserved_units;
    return 1;
}
