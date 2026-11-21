# Platform and Capability Validation Policy

## Purpose

This policy prevents documentation, release notes, badges, examples and product descriptions from presenting unverified platform support, performance guarantees, data-format integrations or hardware-design capabilities as implemented facts.

## Current declared scope

| Area | Current evidence-backed statement |
|---|---|
| Operating systems | Windows and Linux are represented in the project CI matrix. |
| Language | The implementation is ISO C11. C23 is referenced only by the reference pipeline plan. |
| Processor architectures | CPU/ISA discovery and bounded local benchmarks are advisory; they do not establish architecture-wide support or performance guarantees. |
| Data formats | JSON reports and PCM16 WAV audio are the documented formats. |
| Hardware design | The project does not claim production RTL generation or FPGA synthesis. |

## Claims that must remain unclaimed until separately validated

- macOS support or compatibility.
- ARM64-specific or x86_64-specific performance guarantees.
- Protobuf, Parquet or Apache Arrow integration.
- Production-ready RTL generation, FPGA implementation or synthesis.

Mentioning a technology in a roadmap, issue, example, dependency proposal or design document is not evidence that the capability exists.

## Blockchain and processor systems lab claims

The 33-function systems lab is an implemented C API catalogue, not a claim that Pytyliki operates every named external platform or protocol. The following wording rules apply:

- Merkle, ZK, cross-chain proof and firmware-signature operations are callback contracts. Production use requires protocol-specific algorithms, trusted roots, key management, external vectors, adversarial tests and independent review.
- RPC scheduling is a local quota-respecting token bucket; never describe it as bypassing or defeating provider limits.
- VM opcode hooks and micro-op tables target caller-owned simulators only. ISA extensions are encoded for a simulator/compiler integration and do not add instructions to a physical CPU.
- FPGA synthesis is available only through a supplied vendor adapter and validated target flow. A callback smoke test is not evidence of production RTL synthesis, place-and-route, timing closure or a deployable bitstream.
- Virtual clock/governor, scheduler affinity, cache partitioning, prefetch, vectorization and PMU APIs are local models or plans. They do not write device registers or guarantee performance gains.

Promotion of an adapter to a production integration requires named protocol/device/toolchain versions, known-answer and negative tests, documented trust assumptions, and reproducible CI or vendor-flow evidence for the exact target.

## Evidence required to change the scope

A claim may be promoted only through a reviewed pull request that supplies the relevant evidence:

1. **Platform support:** a maintained CI runner or reproducible supported-host test, successful configure/build/test results, documented toolchain and OS versions, and any platform-specific implementation notes.
2. **Architecture performance:** reproducible benchmark methodology and source, named CPU models and microarchitecture, compiler/version/flags, OS/kernel, workload and input sizes, repeated measurements, baseline comparison, variance, and clearly bounded conclusions. A successful compile or CPUID feature flag alone is insufficient.
3. **Data-format integration:** an implemented reader/writer or adapter, dependency and version/licensing details, round-trip or interoperability tests using representative fixtures, malformed-input tests, and documented limits.
4. **RTL/FPGA synthesis:** synthesizable RTL sources, target device/toolchain versions, reproducible synthesis and implementation logs, timing/resource reports, and hardware or vendor-flow validation appropriate to the claim. A conceptual pipeline, software model or generated diagram is not production synthesis.

## Documentation and release gate

- Keep the compatibility statements in all four top-level README editions aligned.
- Distinguish **implemented**, **tested**, **planned** and **not validated** capabilities.
- Do not infer support from compiler portability, CPU feature detection, dependency presence or a successful build on a different target.
- Do not use badges, benchmark headlines or release notes to imply broader support than the evidence covers.
- Any scope expansion must link its validation evidence from the pull request and update this policy and all affected documentation.
- If evidence is missing, keep the capability explicitly marked as unvalidated and out of the supported scope.

## Review checklist

- [ ] The implementation exists and is reachable through documented interfaces.
- [ ] Tests exercise success, failure and relevant boundary cases.
- [ ] CI or reproducible validation covers the exact claimed target.
- [ ] Performance claims include reproducible methodology and bounded conclusions.
- [ ] All four README editions and release notes agree with the evidence.
- [ ] Independent review confirms that no roadmap item is described as a delivered feature.

**Default rule:** absence of target-specific validation means no support or performance guarantee is claimed.
