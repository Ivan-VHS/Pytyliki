#include "neuro_suite.h"

#include <float.h>
#include <math.h>
#include <string.h>

static const pl_neuro_framework_descriptor frameworks[] = {
    { PL_NEURO_FRAMEWORK_SOKK, "Sokk", "Bounded dense artificial neural network", "Neural topology contract", "System design and feed-forward programming", PL_NEURO_CAP_TOPOLOGY },
    { PL_NEURO_FRAMEWORK_ABOVE_AL, "Above AL", "Typed neural model and intelligent system", "Versioned neural model contract", "System architecture and contract engineering", PL_NEURO_CAP_CONTRACTS },
    { PL_NEURO_FRAMEWORK_JUMPJU, "Jumpju", "Trainable adaptive neural model", "Bounded learning contract", "Model training and optimization", PL_NEURO_CAP_TRAINING },
    { PL_NEURO_FRAMEWORK_DIOGO, "Diogo", "Deterministic multi-sample neural execution", "Batch inference agreement", "Modular and multi-level development", PL_NEURO_CAP_BATCH_INFERENCE },
    { PL_NEURO_FRAMEWORK_GHOSTERY, "Ghostery", "Privacy-constrained cognitive execution", "Local-only privacy contract", "Trust, privacy and safe system construction", PL_NEURO_CAP_PRIVACY_GATES },
    { PL_NEURO_FRAMEWORK_SELITOL, "SELITOL", "Distributed neural-system compatibility", "Federation and peer contract", "Platform and infrastructure integration", PL_NEURO_CAP_FEDERATION }
};

static int framework_valid(pl_neuro_framework_id id) {
    return id >= PL_NEURO_FRAMEWORK_SOKK && id < PL_NEURO_FRAMEWORK_COUNT;
}

static int checked_add(size_t left, size_t right, size_t *out) {
    if (out == NULL || left > SIZE_MAX - right) return -1;
    *out = left + right;
    return 0;
}

static int checked_mul(size_t left, size_t right, size_t *out) {
    if (out == NULL || (right != 0u && left > SIZE_MAX / right)) return -1;
    *out = left * right;
    return 0;
}

static int finite_values(const double *values, size_t count) {
    if (values == NULL) return 0;
    for (size_t i = 0u; i < count; ++i) {
        if (!isfinite(values[i])) return 0;
    }
    return 1;
}

static int valid_activation(pl_neuro_activation activation) {
    return activation >= PL_NEURO_ACTIVATION_LINEAR &&
           activation <= PL_NEURO_ACTIVATION_SIGMOID;
}

size_t pl_neuro_framework_count(void) {
    return sizeof(frameworks) / sizeof(frameworks[0]);
}

const pl_neuro_framework_descriptor *pl_neuro_framework_get(pl_neuro_framework_id id) {
    if (!framework_valid(id)) return NULL;
    return &frameworks[(size_t)id - 1u];
}

int pl_neuro_contract_default(pl_neuro_framework_id framework,
                              uint32_t flags,
                              pl_neuro_contract *out) {
    const uint32_t allowed = PL_NEURO_CONTRACT_ALLOW_TRAINING |
                             PL_NEURO_CONTRACT_ALLOW_BATCH |
                             PL_NEURO_CONTRACT_ALLOW_FEDERATION;
    if (!framework_valid(framework) || out == NULL || (flags & ~allowed) != 0u) return -1;
    pl_neuro_contract next;
    next.abi_version = PL_NEURO_ABI_VERSION;
    next.contract_version = PL_NEURO_CONTRACT_VERSION;
    next.framework = framework;
    next.max_layers = PL_NEURO_MAX_LAYERS;
    next.max_neurons_per_layer = PL_NEURO_MAX_NEURONS;
    next.max_parameters = PL_NEURO_MAX_PARAMETERS;
    next.max_batch = PL_NEURO_MAX_BATCH;
    next.flags = flags;
    *out = next;
    return 0;
}

int pl_neuro_contract_validate(const pl_neuro_contract *contract) {
    const uint32_t allowed = PL_NEURO_CONTRACT_ALLOW_TRAINING |
                             PL_NEURO_CONTRACT_ALLOW_BATCH |
                             PL_NEURO_CONTRACT_ALLOW_FEDERATION;
    if (contract == NULL || contract->abi_version != PL_NEURO_ABI_VERSION ||
        contract->contract_version != PL_NEURO_CONTRACT_VERSION ||
        !framework_valid(contract->framework) ||
        contract->max_layers == 0u || contract->max_layers > PL_NEURO_MAX_LAYERS ||
        contract->max_neurons_per_layer == 0u ||
        contract->max_neurons_per_layer > PL_NEURO_MAX_NEURONS ||
        contract->max_parameters == 0u ||
        contract->max_parameters > PL_NEURO_MAX_PARAMETERS ||
        contract->max_batch == 0u || contract->max_batch > PL_NEURO_MAX_BATCH ||
        (contract->flags & ~allowed) != 0u) return -1;
    return 0;
}

int pl_neuro_model_validate(const pl_neuro_model *model,
                            const pl_neuro_contract *contract,
                            pl_neuro_model_report *out) {
    if (pl_neuro_contract_validate(contract) != 0 || model == NULL ||
        model->abi_version != PL_NEURO_ABI_VERSION ||
        model->framework != contract->framework ||
        model->input_count == 0u || model->input_count > contract->max_neurons_per_layer ||
        model->layer_count == 0u || model->layer_count > contract->max_layers ||
        model->layers == NULL) return -1;

    size_t parameters = 0u;
    size_t previous_width = model->input_count;
    size_t maximum_width = model->input_count;
    for (size_t i = 0u; i < model->layer_count; ++i) {
        const pl_neuro_layer *layer = &model->layers[i];
        size_t weight_count = 0u;
        size_t layer_parameters = 0u;
        if (layer->input_count != previous_width || layer->output_count == 0u ||
            layer->output_count > contract->max_neurons_per_layer ||
            !valid_activation(layer->activation) || layer->weights == NULL ||
            layer->biases == NULL ||
            checked_mul(layer->input_count, layer->output_count, &weight_count) != 0 ||
            checked_add(weight_count, layer->output_count, &layer_parameters) != 0 ||
            checked_add(parameters, layer_parameters, &parameters) != 0 ||
            parameters > contract->max_parameters ||
            !finite_values(layer->weights, weight_count) ||
            !finite_values(layer->biases, layer->output_count)) return -1;
        if (layer->output_count > maximum_width) maximum_width = layer->output_count;
        previous_width = layer->output_count;
    }
    if (out != NULL) {
        pl_neuro_model_report report;
        report.parameter_count = parameters;
        report.maximum_width = maximum_width;
        report.output_count = previous_width;
        report.generation = model->generation;
        *out = report;
    }
    return 0;
}

int pl_neuro_inference_workspace_size(const pl_neuro_model *model, size_t *out_doubles) {
    if (model == NULL || out_doubles == NULL || model->layers == NULL ||
        model->layer_count == 0u || model->layer_count > PL_NEURO_MAX_LAYERS ||
        model->input_count == 0u || model->input_count > PL_NEURO_MAX_NEURONS) return -1;
    size_t maximum_width = model->input_count;
    for (size_t i = 0u; i < model->layer_count; ++i) {
        if (model->layers[i].output_count == 0u ||
            model->layers[i].output_count > PL_NEURO_MAX_NEURONS) return -1;
        if (model->layers[i].output_count > maximum_width) maximum_width = model->layers[i].output_count;
    }
    return checked_mul(maximum_width, 2u, out_doubles);
}

int pl_neuro_training_workspace_size(const pl_neuro_model *model, size_t *out_doubles) {
    if (model == NULL || out_doubles == NULL || model->layers == NULL ||
        model->layer_count == 0u || model->layer_count > PL_NEURO_MAX_LAYERS ||
        model->input_count == 0u || model->input_count > PL_NEURO_MAX_NEURONS) return -1;
    size_t activations = model->input_count;
    size_t deltas = 0u;
    for (size_t i = 0u; i < model->layer_count; ++i) {
        if (model->layers[i].output_count == 0u ||
            model->layers[i].output_count > PL_NEURO_MAX_NEURONS ||
            checked_add(activations, model->layers[i].output_count, &activations) != 0 ||
            checked_add(deltas, model->layers[i].output_count, &deltas) != 0) return -1;
    }
    return checked_add(activations, deltas, out_doubles);
}

static double activate(double value, pl_neuro_activation activation) {
    switch (activation) {
        case PL_NEURO_ACTIVATION_LINEAR:
            return value;
        case PL_NEURO_ACTIVATION_RELU:
            return value > 0.0 ? value : 0.0;
        case PL_NEURO_ACTIVATION_TANH:
            return tanh(value);
        case PL_NEURO_ACTIVATION_SIGMOID:
            if (value >= 0.0) {
                double e = exp(-value);
                return 1.0 / (1.0 + e);
            } else {
                double e = exp(value);
                return e / (1.0 + e);
            }
        default:
            return NAN;
    }
}

static double activation_derivative_from_output(double output,
                                                pl_neuro_activation activation) {
    switch (activation) {
        case PL_NEURO_ACTIVATION_LINEAR:
            return 1.0;
        case PL_NEURO_ACTIVATION_RELU:
            return output > 0.0 ? 1.0 : 0.0;
        case PL_NEURO_ACTIVATION_TANH:
            return 1.0 - output * output;
        case PL_NEURO_ACTIVATION_SIGMOID:
            return output * (1.0 - output);
        default:
            return NAN;
    }
}

static int infer_core(const pl_neuro_model *model,
                      const pl_neuro_model_report *report,
                      const double *input,
                      size_t input_count,
                      double *output,
                      size_t output_capacity,
                      double *workspace,
                      size_t workspace_capacity) {
    size_t required_workspace = 0u;
    if (model == NULL || report == NULL || input == NULL || output == NULL ||
        workspace == NULL || input_count != model->input_count ||
        output_capacity < report->output_count || !finite_values(input, input_count) ||
        pl_neuro_inference_workspace_size(model, &required_workspace) != 0 ||
        workspace_capacity < required_workspace) return -1;

    double *first = workspace;
    double *second = workspace + required_workspace / 2u;
    const double *source = input;
    size_t source_count = input_count;
    for (size_t layer_index = 0u; layer_index < model->layer_count; ++layer_index) {
        const pl_neuro_layer *layer = &model->layers[layer_index];
        double *destination = (layer_index % 2u == 0u) ? first : second;
        if (source_count != layer->input_count) return -1;
        for (size_t neuron = 0u; neuron < layer->output_count; ++neuron) {
            long double sum = (long double)layer->biases[neuron];
            size_t weight_offset = neuron * layer->input_count;
            for (size_t input_index = 0u; input_index < layer->input_count; ++input_index) {
                sum += (long double)layer->weights[weight_offset + input_index] *
                       (long double)source[input_index];
            }
            if (!isfinite(sum) || fabsl(sum) > (long double)DBL_MAX) return -1;
            double value = activate((double)sum, layer->activation);
            if (!isfinite(value)) return -1;
            destination[neuron] = value;
        }
        source = destination;
        source_count = layer->output_count;
    }
    memmove(output, source, report->output_count * sizeof(double));
    return 0;
}

int pl_neuro_sokk_infer(const pl_neuro_model *model,
                        const pl_neuro_contract *contract,
                        const double *input,
                        size_t input_count,
                        double *output,
                        size_t output_capacity,
                        double *workspace,
                        size_t workspace_capacity) {
    pl_neuro_model_report report;
    if (model == NULL || model->framework != PL_NEURO_FRAMEWORK_SOKK ||
        pl_neuro_model_validate(model, contract, &report) != 0) return -1;
    return infer_core(model, &report, input, input_count, output, output_capacity,
                      workspace, workspace_capacity);
}

int pl_neuro_above_al_check_contract(const pl_neuro_contract *contract,
                                     const pl_neuro_model *model,
                                     pl_neuro_model_report *out) {
    if (pl_neuro_contract_validate(contract) != 0) return -1;
    return pl_neuro_model_validate(model, contract, out);
}

static int activation_offset(const pl_neuro_model *model, size_t layer_index,
                             size_t *out_offset) {
    size_t offset = model->input_count;
    for (size_t i = 0u; i < layer_index; ++i) {
        if (checked_add(offset, model->layers[i].output_count, &offset) != 0) return -1;
    }
    *out_offset = offset;
    return 0;
}

static int delta_offset(const pl_neuro_model *model, size_t layer_index,
                        size_t *out_offset) {
    size_t offset = 0u;
    for (size_t i = 0u; i < layer_index; ++i) {
        if (checked_add(offset, model->layers[i].output_count, &offset) != 0) return -1;
    }
    *out_offset = offset;
    return 0;
}

int pl_neuro_jumpju_train_step(pl_neuro_model *model,
                               const pl_neuro_contract *contract,
                               const double *input,
                               size_t input_count,
                               const double *target,
                               size_t target_count,
                               double learning_rate,
                               double *workspace,
                               size_t workspace_capacity,
                               double *out_loss) {
    pl_neuro_model_report report;
    size_t required_workspace = 0u;
    if (model == NULL || model->framework != PL_NEURO_FRAMEWORK_JUMPJU ||
        pl_neuro_model_validate(model, contract, &report) != 0 ||
        (contract->flags & PL_NEURO_CONTRACT_ALLOW_TRAINING) == 0u ||
        input == NULL || target == NULL || workspace == NULL || out_loss == NULL ||
        input_count != model->input_count || target_count != report.output_count ||
        !isfinite(learning_rate) || learning_rate <= 0.0 || learning_rate > 1.0 ||
        !finite_values(input, input_count) || !finite_values(target, target_count) ||
        model->generation == UINT64_MAX ||
        pl_neuro_training_workspace_size(model, &required_workspace) != 0 ||
        workspace_capacity < required_workspace) return -1;

    size_t activation_total = required_workspace;
    size_t delta_total = 0u;
    for (size_t i = 0u; i < model->layer_count; ++i) {
        if (checked_add(delta_total, model->layers[i].output_count, &delta_total) != 0) return -1;
    }
    activation_total -= delta_total;
    double *activations = workspace;
    double *deltas = workspace + activation_total;
    memcpy(activations, input, input_count * sizeof(double));

    size_t previous_offset = 0u;
    size_t current_offset = model->input_count;
    for (size_t layer_index = 0u; layer_index < model->layer_count; ++layer_index) {
        const pl_neuro_layer *layer = &model->layers[layer_index];
        for (size_t neuron = 0u; neuron < layer->output_count; ++neuron) {
            long double sum = (long double)layer->biases[neuron];
            size_t weight_offset = neuron * layer->input_count;
            for (size_t j = 0u; j < layer->input_count; ++j) {
                sum += (long double)layer->weights[weight_offset + j] *
                       (long double)activations[previous_offset + j];
            }
            if (!isfinite(sum) || fabsl(sum) > (long double)DBL_MAX) return -1;
            double value = activate((double)sum, layer->activation);
            if (!isfinite(value)) return -1;
            activations[current_offset + neuron] = value;
        }
        previous_offset = current_offset;
        if (checked_add(current_offset, layer->output_count, &current_offset) != 0) return -1;
    }

    long double loss_sum = 0.0L;
    size_t last_activation_offset = 0u;
    size_t last_delta_offset = 0u;
    if (activation_offset(model, model->layer_count - 1u, &last_activation_offset) != 0 ||
        delta_offset(model, model->layer_count - 1u, &last_delta_offset) != 0) return -1;
    const pl_neuro_layer *last_layer = &model->layers[model->layer_count - 1u];
    for (size_t j = 0u; j < report.output_count; ++j) {
        double difference = activations[last_activation_offset + j] - target[j];
        double derivative = activation_derivative_from_output(
            activations[last_activation_offset + j], last_layer->activation);
        long double squared = (long double)difference * (long double)difference;
        loss_sum += squared;
        double delta = (2.0 / (double)report.output_count) * difference * derivative;
        if (!isfinite(delta)) return -1;
        deltas[last_delta_offset + j] = delta;
    }
    long double loss_value = loss_sum / (long double)report.output_count;
    if (!isfinite(loss_value) || loss_value > (long double)DBL_MAX) return -1;

    for (size_t reverse = model->layer_count - 1u; reverse > 0u; --reverse) {
        size_t layer_index = reverse - 1u;
        const pl_neuro_layer *layer = &model->layers[layer_index];
        const pl_neuro_layer *next_layer = &model->layers[layer_index + 1u];
        size_t layer_activation_offset = 0u;
        size_t next_delta_offset = 0u;
        size_t layer_delta_start = 0u;
        if (activation_offset(model, layer_index, &layer_activation_offset) != 0 ||
            delta_offset(model, layer_index + 1u, &next_delta_offset) != 0 ||
            delta_offset(model, layer_index, &layer_delta_start) != 0) return -1;
        for (size_t neuron = 0u; neuron < layer->output_count; ++neuron) {
            long double propagated = 0.0L;
            for (size_t next = 0u; next < next_layer->output_count; ++next) {
                propagated += (long double)next_layer->weights[next * next_layer->input_count + neuron] *
                              (long double)deltas[next_delta_offset + next];
            }
            double activation_value = activations[layer_activation_offset + neuron];
            double derivative = activation_derivative_from_output(activation_value, layer->activation);
            long double delta = propagated * (long double)derivative;
            if (!isfinite(delta) || fabsl(delta) > (long double)DBL_MAX) return -1;
            deltas[layer_delta_start + neuron] = (double)delta;
        }
    }

    /* Preflight every parameter update so a failed step never partially mutates the model. */
    previous_offset = 0u;
    for (size_t layer_index = 0u; layer_index < model->layer_count; ++layer_index) {
        pl_neuro_layer *layer = &model->layers[layer_index];
        size_t layer_activation_offset = 0u;
        size_t layer_delta_start = 0u;
        if (activation_offset(model, layer_index, &layer_activation_offset) != 0 ||
            delta_offset(model, layer_index, &layer_delta_start) != 0) return -1;
        for (size_t neuron = 0u; neuron < layer->output_count; ++neuron) {
            long double next_bias = (long double)layer->biases[neuron] -
                                    (long double)learning_rate *
                                    (long double)deltas[layer_delta_start + neuron];
            if (!isfinite(next_bias) || fabsl(next_bias) > (long double)DBL_MAX) return -1;
            for (size_t j = 0u; j < layer->input_count; ++j) {
                size_t index = neuron * layer->input_count + j;
                long double next_weight = (long double)layer->weights[index] -
                                          (long double)learning_rate *
                                          (long double)deltas[layer_delta_start + neuron] *
                                          (long double)activations[previous_offset + j];
                if (!isfinite(next_weight) || fabsl(next_weight) > (long double)DBL_MAX) return -1;
            }
        }
        previous_offset = layer_activation_offset;
    }

    previous_offset = 0u;
    for (size_t layer_index = 0u; layer_index < model->layer_count; ++layer_index) {
        pl_neuro_layer *layer = &model->layers[layer_index];
        size_t layer_activation_offset = 0u;
        size_t layer_delta_start = 0u;
        (void)activation_offset(model, layer_index, &layer_activation_offset);
        (void)delta_offset(model, layer_index, &layer_delta_start);
        for (size_t neuron = 0u; neuron < layer->output_count; ++neuron) {
            long double delta = (long double)deltas[layer_delta_start + neuron];
            layer->biases[neuron] = (double)((long double)layer->biases[neuron] -
                                              (long double)learning_rate * delta);
            for (size_t j = 0u; j < layer->input_count; ++j) {
                size_t index = neuron * layer->input_count + j;
                long double next_weight = (long double)layer->weights[index] -
                                          (long double)learning_rate * delta *
                                          (long double)activations[previous_offset + j];
                layer->weights[index] = (double)next_weight;
            }
        }
        previous_offset = layer_activation_offset;
    }
    model->generation += 1u;
    *out_loss = (double)loss_value;
    return 0;
}

int pl_neuro_diogo_infer_batch(const pl_neuro_model *model,
                               const pl_neuro_contract *contract,
                               const double *inputs,
                               size_t batch_count,
                               size_t input_stride,
                               double *outputs,
                               size_t output_capacity,
                               double *workspace,
                               size_t workspace_capacity) {
    pl_neuro_model_report report;
    size_t required_workspace = 0u;
    size_t required_outputs = 0u;
    size_t required_inputs = 0u;
    if (model == NULL || model->framework != PL_NEURO_FRAMEWORK_DIOGO ||
        pl_neuro_model_validate(model, contract, &report) != 0 ||
        (contract->flags & PL_NEURO_CONTRACT_ALLOW_BATCH) == 0u ||
        inputs == NULL || outputs == NULL || workspace == NULL || batch_count == 0u ||
        batch_count > contract->max_batch || input_stride < model->input_count ||
        checked_mul(batch_count, report.output_count, &required_outputs) != 0 ||
        checked_mul(batch_count - 1u, input_stride, &required_inputs) != 0 ||
        checked_add(required_inputs, model->input_count, &required_inputs) != 0 ||
        output_capacity < required_outputs ||
        pl_neuro_inference_workspace_size(model, &required_workspace) != 0 ||
        workspace_capacity < required_workspace) return -1;
    (void)required_inputs;
    for (size_t row = 0u; row < batch_count; ++row) {
        if (infer_core(model, &report, inputs + row * input_stride, model->input_count,
                       outputs + row * report.output_count, report.output_count,
                       workspace, workspace_capacity) != 0) return -1;
    }
    return 0;
}

int pl_neuro_ghostery_validate_privacy(const pl_neuro_privacy_policy *policy) {
    if (policy == NULL || policy->abi_version != PL_NEURO_ABI_VERSION ||
        policy->allow_external_transfer != 0u || policy->allow_telemetry != 0u ||
        policy->persist_input_data != 0u || policy->allow_remote_model_updates != 0u ||
        policy->allow_dynamic_code != 0u) return -1;
    return 0;
}

static int valid_peer_id(const char *peer_id) {
    if (peer_id == NULL) return 0;
    size_t length = 0u;
    while (peer_id[length] != '\0') {
        unsigned char value = (unsigned char)peer_id[length];
        if (length >= PL_NEURO_MAX_PEER_ID || value < 0x21u || value > 0x7eu) return 0;
        ++length;
    }
    return length > 0u;
}

int pl_neuro_selitol_validate_federation(const pl_neuro_federation_contract *federation,
                                         pl_neuro_federation_report *out) {
    const uint32_t allowed = PL_NEURO_FEDERATION_REQUIRE_SAME_GENERATION |
                             PL_NEURO_FEDERATION_ALLOW_MIXED_FRAMEWORKS;
    if (federation == NULL || out == NULL ||
        federation->abi_version != PL_NEURO_ABI_VERSION ||
        federation->peers == NULL || federation->peer_count == 0u ||
        federation->peer_count > PL_NEURO_MAX_PEERS || federation->quorum == 0u ||
        federation->quorum > federation->peer_count ||
        (federation->contract_flags & PL_NEURO_CONTRACT_ALLOW_FEDERATION) == 0u ||
        (federation->contract_flags & ~(PL_NEURO_CONTRACT_ALLOW_TRAINING |
                                        PL_NEURO_CONTRACT_ALLOW_BATCH |
                                        PL_NEURO_CONTRACT_ALLOW_FEDERATION)) != 0u ||
        (federation->required_framework != PL_NEURO_FRAMEWORK_ANY &&
         !framework_valid(federation->required_framework)) ||
        (federation->flags & ~allowed) != 0u) return -1;

    uint64_t common_generation = federation->peers[0].model_generation;
    pl_neuro_framework_id first_framework = federation->peers[0].framework;
    for (size_t i = 0u; i < federation->peer_count; ++i) {
        const pl_neuro_peer *peer = &federation->peers[i];
        if (!valid_peer_id(peer->peer_id) || peer->abi_version != PL_NEURO_ABI_VERSION ||
            peer->contract_version != PL_NEURO_CONTRACT_VERSION ||
            !framework_valid(peer->framework) ||
            (federation->required_framework != PL_NEURO_FRAMEWORK_ANY &&
             peer->framework != federation->required_framework) ||
            (((federation->flags & PL_NEURO_FEDERATION_ALLOW_MIXED_FRAMEWORKS) == 0u) &&
             peer->framework != first_framework) ||
            (((federation->flags & PL_NEURO_FEDERATION_REQUIRE_SAME_GENERATION) != 0u) &&
             peer->model_generation != common_generation)) return -1;
        for (size_t j = 0u; j < i; ++j) {
            if (strcmp(peer->peer_id, federation->peers[j].peer_id) == 0) return -1;
        }
    }
    pl_neuro_federation_report report;
    report.peer_count = federation->peer_count;
    report.quorum = federation->quorum;
    report.common_generation = common_generation;
    report.same_generation = 1u;
    for (size_t i = 1u; i < federation->peer_count; ++i) {
        if (federation->peers[i].model_generation != common_generation) {
            report.same_generation = 0u;
            break;
        }
    }
    *out = report;
    return 0;
}
