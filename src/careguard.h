#ifndef PYTYLIKI_CAREGUARD_H
#define PYTYLIKI_CAREGUARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CareGuardLoadStatus {
    CAREGUARD_LOAD_INVALID = -1,
    CAREGUARD_LOAD_NORMAL = 0,
    CAREGUARD_LOAD_WATCH = 1,
    CAREGUARD_LOAD_PAUSE = 2
} CareGuardLoadStatus;

typedef enum CareGuardSupportPath {
    CAREGUARD_PATH_INVALID = 0,
    CAREGUARD_PATH_TRUSTED_PERSON = 1,
    CAREGUARD_PATH_PEER_SUPPORT = 2,
    CAREGUARD_PATH_PROFESSIONAL = 3,
    CAREGUARD_PATH_URGENT_HUMAN = 4
} CareGuardSupportPath;

typedef struct CareGuardWorkload {
    uint32_t queued_items;
    uint32_t failed_checks;
    uint32_t alerts_per_hour;
    uint32_t active_workers;
    uint32_t worker_capacity;
} CareGuardWorkload;

typedef struct CareGuardAssessment {
    CareGuardLoadStatus status;
    uint32_t pressure_score;
    uint32_t reserve_percent;
} CareGuardAssessment;

CareGuardAssessment careguard_assess_project_load(const CareGuardWorkload *workload);
uint32_t careguard_review_language(const char *text);
int careguard_accept_help_request(const char *message);
CareGuardSupportPath careguard_select_support_path(const char *path);
int careguard_review_contextual_claim(const char *claim, const char *evidence_ref);
int careguard_check_clinical_boundary(const char *action);
int careguard_allocate_resilience_buffer(uint32_t capacity_units, uint32_t reserve_percent,
                                         uint32_t *reserved_units, uint32_t *usable_units);

#ifdef __cplusplus
}
#endif
#endif
