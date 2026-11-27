# Pytyliki Neural Framework Suite v1

## Purpose

The Neural Framework Suite adds a bounded, native C11 neural-model execution layer to Pytyliki. It is deliberately separate from the Win32 GUI neuro-autoimprovement profile: that profile governs supervised UI changes and does not run a model.

The initial runtime implements dense feed-forward networks with \`double\` parameters, fixed iteration order, caller-owned memory and explicit model contracts. It is CPU-only and does not depend on an external ML runtime.

## Six frameworks

| Framework | Domain model | Contract focus | Implemented responsibility |
|---|---|---|---|
| **Sokk** | Artificial neural network | Topology contract | Validate bounded layer graphs and run feed-forward inference |
| **Above AL** | Typed neural model / intelligent system | Versioned model contract | Create and validate limits, ABI, framework identity and shape compatibility |
| **Jumpju** | Trainable adaptive model | Learning contract | One-example SGD backpropagation with mean-squared-error loss |
| **Diogo** | Multi-sample neural execution | Batch agreement | Bounded row-major batch inference |
| **Ghostery** | Privacy-constrained cognitive execution | Local-only privacy contract | Reject external transfer, telemetry, input persistence, remote model updates and dynamic code |
| **SELITOL** | Distributed neural-system compatibility | Federation / peer contract | Validate peer manifests, framework/version compatibility, generation and quorum |

The twenty Russian-language system / contract / engineering combinations supplied for this suite are preserved in [\`contracts/neuro-suite.v1.json\`](../contracts/neuro-suite.v1.json) under \`conceptual_matrix\` and mapped to these six frameworks. That matrix is a taxonomy; an entry does not imply that every model family in its wording has shipped.

## Native API

Public declarations are in \`src/neuro_suite.h\`; implementation is in \`src/neuro_suite.c\`.

- \`pl_neuro_framework_count\` / \`pl_neuro_framework_get\` expose the immutable six-framework registry.
- \`pl_neuro_contract_default\` creates a bounded v1 contract. Training, batch and federation are opt-in flags.
- \`pl_neuro_model_validate\` validates ABI/version, framework identity, layer dimensions, supported activation, finite parameters and aggregate parameter limits.
- \`pl_neuro_sokk_infer\` runs a dense feed-forward model. Weights are row-major as \`[output_neuron][input_neuron]\`; the caller supplies a two-bank workspace sized by \`pl_neuro_inference_workspace_size\`.
- \`pl_neuro_jumpju_train_step\` performs one SGD backpropagation step with MSE loss. It accepts learning rates in \`(0, 1]\`, increments the model generation after success, and preflights every parameter update before mutating any weight or bias.
- \`pl_neuro_diogo_infer_batch\` processes row-major inputs under the contract's batch cap. Earlier output rows remain valid if a later row fails.
- \`pl_neuro_ghostery_validate_privacy\` is a policy gate. A successful check means the supplied policy denies external transfer, telemetry, input persistence, remote model updates and dynamic code; it does not create an OS sandbox.
- \`pl_neuro_selitol_validate_federation\` checks peer identifiers, ABI and contract versions, framework compatibility, optional generation agreement and quorum. It performs no network I/O.

All memory is caller-owned. The runtime does not allocate, retain, persist or transmit model inputs, weights, outputs or workspace buffers. Callers must provide valid arrays large enough for the declared shapes and must not mutate a model concurrently with inference or training.

## Mathematical model

For layer \(l\), neuron \(j\), and input feature \(i\):

\[
z_j^{(l)} = b_j^{(l)} + \sum_i W_{j i}^{(l)} a_i^{(l-1)}, \qquad
a_j^{(l)} = \phi_l(z_j^{(l)})
\]

For one training sample with target vector \(t\), the runtime uses mean-squared error:

\[
L = \frac{1}{m}\sum_{j=1}^{m}(a_j^{(L)}-t_j)^2
\]

and one gradient-descent update:

\[
W \leftarrow W - \eta\nabla_W L, \qquad b \leftarrow b - \eta\nabla_b L
\]

Supported activations are linear, ReLU, tanh and sigmoid. The current implementation is a dense layered graph; it does not implement sparse or cyclic graphs, convolution, recurrence, attention, transformers or generative decoding.

## Resource limits

| Resource | v1 limit |
|---|---:|
| Layers | 32 |
| Neurons per layer | 4,096 |
| Parameters (weights + biases) | 4,194,304 |
| Batch rows | 1,024 |
| Federation peers | 32 |
| Peer identifier | 64 printable ASCII bytes |
| Learning rate | greater than 0 and at most 1 |

The implementation checks size arithmetic before workspace calculations and model traversal. Workspace sizes are returned in \`double\` elements, not bytes.

## Safety and maturity boundaries

- The inference and training code runs locally on the CPU. There is no network transport, telemetry, plugin loader, dynamic code execution or GPU backend.
- Training is disabled unless \`PL_NEURO_CONTRACT_ALLOW_TRAINING\` is set in the supplied contract.
- The federation feature validates metadata only. It does not exchange weights, perform secure aggregation, elect a leader or guarantee distributed consensus.
- Ghostery's checks are policy validation, not differential privacy, encryption, sandboxing or a proof that a model is safe.
- No model accuracy, fairness, bias or suitability for medical, financial, legal or other high-impact decisions is certified.
- Generative-model, recurrent, convolutional and transformer execution are future extension tracks, not part of v1.
- Floating-point results use a fixed operation order in this implementation but are not promised to be bitwise identical across all compilers and CPU architectures.

## Build and tests

The module is part of \`pytyliki_core\`; the public header is installed with the core development headers. Its focused CTest target is \`neuro-framework-suite-tests\`.

~~~sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build -R neuro-framework-suite-tests --output-on-failure
~~~

The test covers registry identity, contract/model compatibility, inference, batch inference, SGD parameter changes, failed-training non-mutation, privacy rejection and federation quorum/identity checks.
