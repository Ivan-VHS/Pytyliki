#include "clinical_safety.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int test_digest_provider(
    const unsigned char *data, size_t data_size,
    unsigned char digest_out[32], void *provider_context
) {
    size_t i;
    unsigned char seed = provider_context != NULL
        ? *(const unsigned char *)provider_context : 0x5au;
    if (data == NULL || data_size == 0u || digest_out == NULL) return -1;
    for (i = 0u; i < 32u; ++i)
        digest_out[i] = (unsigned char)(seed ^ data[i % data_size] ^ (unsigned char)i);
    return 0;
}

int main(void) {
    {
        const pl_contact_event events[] = {
            {"person-a", "person-b", "zone-1", 100},
            {"person-a", "staff-c", "zone-1", 110},
            {"person-b", "staff-d", "zone-1", 120},
            {"person-e", "staff-f", "zone-2", 130}
        };
        const char *confirmed[] = {"person-a", "person-b"};
        pl_contact_graph_summary summary;
        assert(pl_contact_graph_analyze(events, 4u, confirmed, 2u, 90, 125, 2u, &summary) == 0);
        assert(summary.events_in_window == 3u);
        assert(summary.unique_participants == 4u);
        assert(summary.candidate_zones == 1u);
        assert(pl_contact_graph_analyze(events, 4u, confirmed, 2u, 90, 125, 3u, &summary) == 0);
        assert(summary.candidate_zones == 0u);
        assert(pl_contact_graph_analyze(events, 4u, confirmed, 2u, 125, 90, 2u, &summary) != 0);
    }
    {
        pl_sterilization_profile profile = {
            "profile-v1", "validated-protocol-v3",
            120.0, 135.0, 180.0, 220.0, 20.0, 80.0, 300.0, 900.0
        };
        pl_sterilization_cycle cycle = {
            "cycle-42", "sterilizer-2", "load-7", 130.0, 200.0, 55.0, 600.0
        };
        pl_sterilization_evaluation evaluation;
        unsigned char previous[32] = {0}, digest[32] = {0}, seed = 0x37u;
        assert(pl_sterilization_evaluate(&profile, &cycle, &evaluation) == 0);
        assert(evaluation.within_configured_profile == 1);
        assert(evaluation.out_of_profile_mask == 0u);
        assert(pl_sterilization_chain_digest(previous, &profile, &cycle, &evaluation,
                                             test_digest_provider, &seed, digest) == 0);
        cycle.exposure_seconds = 100.0;
        assert(pl_sterilization_evaluate(&profile, &cycle, &evaluation) == 0);
        assert(evaluation.within_configured_profile == 0);
        assert((evaluation.out_of_profile_mask & PL_STERILIZATION_EXPOSURE_OUT_OF_PROFILE) != 0u);
    }
    {
        const pl_cpr_bounds bounds = {100.0, 120.0, 0.0, 100.0, 0.25};
        const pl_cpr_observation observation = {110.0, 50.0, 0.10};
        unsigned flags = 99u;
        assert(pl_cpr_review_measurement(&bounds, &observation, &flags) == 0);
        assert(flags == 0u);
        {
            const pl_cpr_observation noisy = {95.0, 50.0, 0.50};
            assert(pl_cpr_review_measurement(&bounds, &noisy, &flags) == 0);
            assert((flags & PL_CPR_RATE_BELOW_CONFIGURED_RANGE) != 0u);
            assert((flags & PL_CPR_SIGNAL_QUALITY_LOW) != 0u);
        }
    }
    {
        pl_rsi_checklist checklist = {
            PL_RSI_OXYGENATION_CONFIRMED | PL_RSI_MONITORING_READY |
            PL_RSI_PRIMARY_AIRWAY_READY
        };
        unsigned missing = 0u;
        const unsigned required =
            PL_RSI_OXYGENATION_CONFIRMED | PL_RSI_SUCTION_READY |
            PL_RSI_MONITORING_READY | PL_RSI_PRIMARY_AIRWAY_READY |
            PL_RSI_BACKUP_AIRWAY_READY;
        assert(pl_rsi_review_checklist(&checklist, required, &missing) == 0);
        assert(missing == (PL_RSI_SUCTION_READY | PL_RSI_BACKUP_AIRWAY_READY));
    }
    {
        const pl_model_vote votes[] = {
            {"BI-RADS-3", 0.70}, {"BI-RADS-4A", 0.62}, {"BI-RADS-4A", 0.66}
        };
        pl_model_disagreement disagreement;
        assert(pl_model_disagreement_analyze(votes, 3u, &disagreement) == 0);
        assert(strcmp(disagreement.majority_label, "BI-RADS-4A") == 0);
        assert(disagreement.majority_votes == 2u);
        assert(disagreement.tied_majority == 0);
        assert(disagreement.disagreement_fraction > 0.33 &&
               disagreement.disagreement_fraction < 0.34);
    }
    {
        const pl_biomarker_point points[] = {{0.0, 10.0}, {30.0, 20.0}, {60.0, 40.0}};
        pl_biomarker_trend trend;
        assert(pl_biomarker_trend_analyze(points, 3u, &trend) == 0);
        assert(trend.increasing == 1);
        assert(trend.concentration_doubling_time_days > 29.9 &&
               trend.concentration_doubling_time_days < 30.1);
        {
            const pl_biomarker_point invalid[] = {{0.0, 0.0}, {10.0, 2.0}};
            assert(pl_biomarker_trend_analyze(invalid, 2u, &trend) != 0);
        }
    }
    puts("All clinical-safety engineering tests passed.");
    return 0;
}
