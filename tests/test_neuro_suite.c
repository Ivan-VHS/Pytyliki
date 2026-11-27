#include "neuro_suite.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    assert(pl_neuro_framework_count() == 6u);
    assert(strcmp(pl_neuro_framework_get(PL_NEURO_FRAMEWORK_SOKK)->name, "Sokk") == 0);
    assert(strcmp(pl_neuro_framework_get(PL_NEURO_FRAMEWORK_ABOVE_AL)->name, "Above AL") == 0);
    assert(strcmp(pl_neuro_framework_get(PL_NEURO_FRAMEWORK_JUMPJU)->name, "Jumpju") == 0);
    assert(strcmp(pl_neuro_framework_get(PL_NEURO_FRAMEWORK_DIOGO)->name, "Diogo") == 0);
    assert(strcmp(pl_neuro_framework_get(PL_NEURO_FRAMEWORK_GHOSTERY)->name, "Ghostery") == 0);
    assert(strcmp(pl_neuro_framework_get(PL_NEURO_FRAMEWORK_SELITOL)->name, "SELITOL") == 0);
    assert(pl_neuro_framework_get(PL_NEURO_FRAMEWORK_ANY) == NULL);

    pl_neuro_contract contract;
    assert(pl_neuro_contract_default(PL_NEURO_FRAMEWORK_SOKK,
                                     PL_NEURO_CONTRACT_ALLOW_BATCH, &contract) == 0);
    double weights[] = {1.0, 0.0, 0.0, 1.0};
    double biases[] = {0.0, 0.0};
    double output_weights[] = {1.0, 1.0};
    double output_biases[] = {0.5};
    pl_neuro_layer layers[] = {
        {2u, 2u, weights, biases, PL_NEURO_ACTIVATION_RELU},
        {2u, 1u, output_weights, output_biases, PL_NEURO_ACTIVATION_LINEAR}
    };
    pl_neuro_model model = {PL_NEURO_ABI_VERSION, PL_NEURO_FRAMEWORK_SOKK,
                            2u, 2u, layers, 4u};
    pl_neuro_model_report report;
    assert(pl_neuro_above_al_check_contract(&contract, &model, &report) == 0);
    assert(report.parameter_count == 9u && report.maximum_width == 2u &&
           report.output_count == 1u && report.generation == 4u);

    size_t inference_workspace_size = 0u;
    assert(pl_neuro_inference_workspace_size(&model, &inference_workspace_size) == 0);
    double inference_workspace[8] = {0.0};
    assert(inference_workspace_size <= 8u);
    const double input[] = {2.0, 3.0};
    double output = -99.0;
    assert(pl_neuro_sokk_infer(&model, &contract, input, 2u, &output, 1u,
                               inference_workspace, 8u) == 0);
    assert(output == 5.5);

    pl_neuro_contract diogo_contract;
    assert(pl_neuro_contract_default(PL_NEURO_FRAMEWORK_DIOGO,
                                     PL_NEURO_CONTRACT_ALLOW_BATCH, &diogo_contract) == 0);
    pl_neuro_model diogo_model = model;
    diogo_model.framework = PL_NEURO_FRAMEWORK_DIOGO;
    const double batch_inputs[] = {2.0, 3.0, 4.0, 5.0, -2.0, 1.0};
    double batch_outputs[3] = {0.0, 0.0, 0.0};
    assert(pl_neuro_diogo_infer_batch(&diogo_model, &diogo_contract, batch_inputs, 3u, 2u,
                                      batch_outputs, 3u, inference_workspace, 8u) == 0);
    assert(batch_outputs[0] == 5.5 && batch_outputs[1] == 9.5 && batch_outputs[2] == 1.5);
    assert(pl_neuro_diogo_infer_batch(&diogo_model, &diogo_contract, batch_inputs, 3u, 2u,
                                      batch_outputs, 2u, inference_workspace, 8u) == -1);

    double bad_input[] = {NAN, 1.0};
    output = 123.0;
    assert(pl_neuro_sokk_infer(&model, &contract, bad_input, 2u, &output, 1u,
                               inference_workspace, 8u) == -1);
    assert(output == 123.0);

    pl_neuro_contract wrong_contract = contract;
    wrong_contract.framework = PL_NEURO_FRAMEWORK_DIOGO;
    assert(pl_neuro_above_al_check_contract(&wrong_contract, &model, NULL) == -1);

    pl_neuro_privacy_policy privacy = {PL_NEURO_ABI_VERSION, 0u, 0u, 0u, 0u, 0u};
    assert(pl_neuro_ghostery_validate_privacy(&privacy) == 0);
    privacy.allow_external_transfer = 1u;
    assert(pl_neuro_ghostery_validate_privacy(&privacy) == -1);

    pl_neuro_peer peers[] = {
        {"node-a", PL_NEURO_ABI_VERSION, PL_NEURO_CONTRACT_VERSION, PL_NEURO_FRAMEWORK_SOKK, 7u},
        {"node-b", PL_NEURO_ABI_VERSION, PL_NEURO_CONTRACT_VERSION, PL_NEURO_FRAMEWORK_SOKK, 7u}
    };
    pl_neuro_federation_contract federation = {
        PL_NEURO_ABI_VERSION, peers, 2u, 2u, PL_NEURO_FRAMEWORK_SOKK,
        PL_NEURO_CONTRACT_ALLOW_FEDERATION,
        PL_NEURO_FEDERATION_REQUIRE_SAME_GENERATION
    };
    pl_neuro_federation_report federation_report;
    assert(pl_neuro_selitol_validate_federation(&federation, &federation_report) == 0);
    assert(federation_report.peer_count == 2u && federation_report.quorum == 2u &&
           federation_report.common_generation == 7u && federation_report.same_generation == 1u);
    peers[1].model_generation = 8u;
    assert(pl_neuro_selitol_validate_federation(&federation, &federation_report) == -1);
    peers[1].model_generation = 7u;
    peers[1].peer_id = "node-a";
    assert(pl_neuro_selitol_validate_federation(&federation, &federation_report) == -1);
    peers[1].peer_id = "node-b";

    double train_weight[] = {0.0};
    double train_bias[] = {0.0};
    pl_neuro_layer train_layer = {1u, 1u, train_weight, train_bias, PL_NEURO_ACTIVATION_LINEAR};
    pl_neuro_model train_model = {PL_NEURO_ABI_VERSION, PL_NEURO_FRAMEWORK_JUMPJU,
                                  1u, 1u, &train_layer, 0u};
    pl_neuro_contract train_contract;
    assert(pl_neuro_contract_default(PL_NEURO_FRAMEWORK_JUMPJU,
                                     PL_NEURO_CONTRACT_ALLOW_TRAINING, &train_contract) == 0);
    size_t training_workspace_size = 0u;
    assert(pl_neuro_training_workspace_size(&train_model, &training_workspace_size) == 0);
    double training_workspace[4] = {0.0};
    assert(training_workspace_size <= 4u);
    const double train_input[] = {1.0};
    const double target[] = {2.0};
    double loss = -1.0;
    assert(pl_neuro_jumpju_train_step(&train_model, &train_contract,
                                     train_input, 1u, target, 1u, 0.1,
                                     training_workspace, 4u, &loss) == 0);
    assert(fabs(train_weight[0] - 0.4) < 1e-12);
    assert(fabs(train_bias[0] - 0.4) < 1e-12);
    assert(loss == 4.0 && train_model.generation == 1u);

    double old_weight = train_weight[0];
    train_contract.flags = 0u;
    assert(pl_neuro_jumpju_train_step(&train_model, &train_contract,
                                     train_input, 1u, target, 1u, 0.1,
                                     training_workspace, 4u, &loss) == -1);
    assert(train_weight[0] == old_weight && train_model.generation == 1u);
    return 0;
}
