#ifndef PYTYLIKI_NEURO_SUITE_H
#define PYTYLIKI_NEURO_SUITE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_NEURO_ABI_VERSION 1u
#define PL_NEURO_CONTRACT_VERSION 1u
#define PL_NEURO_MAX_LAYERS 32u
#define PL_NEURO_MAX_NEURONS 4096u
#define PL_NEURO_MAX_PARAMETERS 4194304u
#define PL_NEURO_MAX_BATCH 1024u
#define PL_NEURO_MAX_PEERS 32u
#define PL_NEURO_MAX_PEER_ID 64u

typedef enum {
    PL_NEURO_FRAMEWORK_ANY = 0,
    PL_NEURO_FRAMEWORK_SOKK = 1,
    PL_NEURO_FRAMEWORK_ABOVE_AL = 2,
    PL_NEURO_FRAMEWORK_JUMPJU = 3,
    PL_NEURO_FRAMEWORK_DIOGO = 4,
    PL_NEURO_FRAMEWORK_GHOSTERY = 5,
    PL_NEURO_FRAMEWORK_SELITOL = 6,
    PL_NEURO_FRAMEWORK_COUNT = 7
} pl_neuro_framework_id;

typedef enum {
    PL_NEURO_ACTIVATION_LINEAR = 0,
    PL_NEURO_ACTIVATION_RELU = 1,
    PL_NEURO_ACTIVATION_TANH = 2,
    PL_NEURO_ACTIVATION_SIGMOID = 3
} pl_neuro_activation;

enum {
    PL_NEURO_CONTRACT_ALLOW_TRAINING = 1u << 0,
    PL_NEURO_CONTRACT_ALLOW_BATCH = 1u << 1,
    PL_NEURO_CONTRACT_ALLOW_FEDERATION = 1u << 2
};

enum {
    PL_NEURO_CAP_TOPOLOGY = 1u << 0,
    PL_NEURO_CAP_CONTRACTS = 1u << 1,
    PL_NEURO_CAP_TRAINING = 1u << 2,
    PL_NEURO_CAP_BATCH_INFERENCE = 1u << 3,
    PL_NEURO_CAP_PRIVACY_GATES = 1u << 4,
    PL_NEURO_CAP_FEDERATION = 1u << 5
};

enum {
    PL_NEURO_FEDERATION_REQUIRE_SAME_GENERATION = 1u << 0,
    PL_NEURO_FEDERATION_ALLOW_MIXED_FRAMEWORKS = 1u << 1
};

typedef struct {
    pl_neuro_framework_id id;
    const char *name;
    const char *neural_model;
    const char *contract_name;
    const char *engineering_discipline;
    uint32_t capabilities;
} pl_neuro_framework_descriptor;

typedef struct {
    size_t input_count;
    size_t output_count;
    double *weights; /* Row-major: weights[output * input_count + input]. */
    double *biases;
    pl_neuro_activation activation;
} pl_neuro_layer;

typedef struct {
    uint32_t abi_version;
    pl_neuro_framework_id framework;
    size_t input_count;
    size_t layer_count;
    pl_neuro_layer *layers;
    uint64_t generation;
} pl_neuro_model;

typedef struct {
    uint32_t abi_version;
    uint32_t contract_version;
    pl_neuro_framework_id framework;
    size_t max_layers;
    size_t max_neurons_per_layer;
    size_t max_parameters;
    size_t max_batch;
    uint32_t flags;
} pl_neuro_contract;

typedef struct {
    size_t parameter_count;
    size_t maximum_width;
    size_t output_count;
    uint64_t generation;
} pl_neuro_model_report;

typedef struct {
    uint32_t abi_version;
    uint32_t allow_external_transfer;
    uint32_t allow_telemetry;
    uint32_t persist_input_data;
    uint32_t allow_remote_model_updates;
    uint32_t allow_dynamic_code;
} pl_neuro_privacy_policy;

typedef struct {
    const char *peer_id;
    uint32_t abi_version;
    uint32_t contract_version;
    pl_neuro_framework_id framework;
    uint64_t model_generation;
} pl_neuro_peer;

typedef struct {
    uint32_t abi_version;
    const pl_neuro_peer *peers;
    size_t peer_count;
    size_t quorum;
    pl_neuro_framework_id required_framework; /* ANY permits a mixed framework set. */
    uint32_t contract_flags;
    uint32_t flags;
} pl_neuro_federation_contract;

typedef struct {
    size_t peer_count;
    size_t quorum;
    uint64_t common_generation;
    uint32_t same_generation;
} pl_neuro_federation_report;

/* Immutable registry for the six domain frameworks. */
size_t pl_neuro_framework_count(void);
const pl_neuro_framework_descriptor *pl_neuro_framework_get(pl_neuro_framework_id id);

/* Builds a conservative, bounded contract for one framework. */
int pl_neuro_contract_default(pl_neuro_framework_id framework,
                              uint32_t flags,
                              pl_neuro_contract *out);
int pl_neuro_contract_validate(const pl_neuro_contract *contract);
int pl_neuro_model_validate(const pl_neuro_model *model,
                            const pl_neuro_contract *contract,
                            pl_neuro_model_report *out);

/* Workspace sizes are measured in doubles. Caller owns all buffers. */
int pl_neuro_inference_workspace_size(const pl_neuro_model *model, size_t *out_doubles);
int pl_neuro_training_workspace_size(const pl_neuro_model *model, size_t *out_doubles);

/* Sokk: deterministic feed-forward inference; output is committed only on success. */
int pl_neuro_sokk_infer(const pl_neuro_model *model,
                        const pl_neuro_contract *contract,
                        const double *input,
                        size_t input_count,
                        double *output,
                        size_t output_capacity,
                        double *workspace,
                        size_t workspace_capacity);

/* Above AL: verify the model against its versioned neural contract. */
int pl_neuro_above_al_check_contract(const pl_neuro_contract *contract,
                                     const pl_neuro_model *model,
                                     pl_neuro_model_report *out);

/* Jumpju: one deterministic, bounded, single-example SGD update (MSE loss). */
int pl_neuro_jumpju_train_step(pl_neuro_model *model,
                               const pl_neuro_contract *contract,
                               const double *input,
                               size_t input_count,
                               const double *target,
                               size_t target_count,
                               double learning_rate,
                               double *workspace,
                               size_t workspace_capacity,
                               double *out_loss);

/* Diogo: row-major batch inference. Earlier rows remain valid if a later row fails. */
int pl_neuro_diogo_infer_batch(const pl_neuro_model *model,
                               const pl_neuro_contract *contract,
                               const double *inputs,
                               size_t batch_count,
                               size_t input_stride,
                               double *outputs,
                               size_t output_capacity,
                               double *workspace,
                               size_t workspace_capacity);

/* Ghostery: admission gate only; it does not claim OS-level sandboxing. */
int pl_neuro_ghostery_validate_privacy(const pl_neuro_privacy_policy *policy);

/* SELITOL: validates peer manifests and quorum; it performs no network I/O. */
int pl_neuro_selitol_validate_federation(const pl_neuro_federation_contract *federation,
                                         pl_neuro_federation_report *out);

#ifdef __cplusplus
}
#endif

#endif
