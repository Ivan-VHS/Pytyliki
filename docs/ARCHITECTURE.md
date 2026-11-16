# Pytyliki architecture

## 1. Runtime shape

The repository is a C11 application composed of a platform-neutral static library, a portable CLI, and an optional native Win32 presentation layer.

The shared library currently contains:
- CPU profile and bounded benchmark;
- read-only source scanner and JSON report writer;
- deny-by-default role/action policy;
- deterministic C11/C23 workflow reference planner;
- clinical workflow metadata catalogue.

The Windows GUI is presentation only. It calls the same profile and benchmark functions as the CLI.

## 2. Data flow

1. The CLI or Win32 window requests a CPU profile.
2. The profile function reports the CPU vendor, brand string when exposed by the processor, logical processor count, and family/model identifiers.
3. The benchmark runs a fixed-count integer workload with a hard upper bound.
4. The scanner reads one explicitly selected source file and flags selected patterns for human review.
5. The policy module evaluates a local role/action pair and denies unknown or unsafe actions.
6. The pipeline module describes the ordered stages expected from a future transpilation implementation.
7. The clinical catalogue exposes workflow shells that require a configured approved source and expert sign-off.

## 3. Policy model

Roles are viewer, analyst, contributor, maintainer, and admin. Permissions increase by responsibility:
- viewer: read and analyze;
- analyst: viewer permissions plus benchmark;
- contributor: analyst permissions plus patch proposals;
- maintainer: contributor permissions plus patch application;
- admin: policy and access administration.

Execution of inspected code, firmware writes, and CPU-control changes are explicitly denied by the local policy function for every role.

This is an application-level policy check. It does not grant GitHub API permissions, operating-system privileges, or authority over another service.

## 4. Transpilation boundary

The C11/C23 pipeline module currently builds a deterministic reference plan with these stages:
1. lexical and syntax analysis;
2. type and dependency validation;
3. intermediate representation;
4. C source generation;
5. compiler validation;
6. semantic and regression tests.

The planner is not a source parser, compiler, intermediate representation implementation, or general-purpose transpiler. A real implementation must prove semantic equivalence and preserve defined behavior, error handling, ownership, and concurrency semantics.

## 5. Clinical workflow boundary

The catalogue contains three metadata templates:
- infection-control workflow;
- emergency-response workflow;
- oncology-screening workflow.

The templates deliberately contain no patient-specific thresholds, triage rules, eligibility criteria, diagnostic statements, or treatment advice. An entry is rejected until its approved source field is configured. A configured source is not proof of correctness: qualified clinical review, version control, validation scenarios, and regulatory assessment are still required before any clinical use.

## 6. Security and safety contract

- No privileged elevation or kernel driver.
- No BIOS/UEFI, microcode, firmware, voltage, frequency, or power-limit writes.
- No source rewriting, automatic patch application, remote execution, or automatic download of code.
- Benchmark cap: 500,000,000 iterations.
- Static findings are heuristic prompts, not proof of a vulnerability.
- User-selected files are opened read-only.
- JSON output is written only to the path explicitly provided by the caller.
- CI has contents: read permission and runs a Windows/Linux build-test matrix.

## 7. Known limitations

- The scanner reads one file per invocation and uses heuristic pattern matching rather than an AST.
- CPU brand strings depend on the processor exposing the extended CPUID brand leaves.
- The benchmark is a local micro-workload, not a standardized benchmark suite.
- The pipeline module is descriptive; transpilation is not implemented.
- Policy evaluation is local and does not mirror GitHub App installation permissions.
- Clinical entries are templates and are not a medical device or clinical decision-support system.
- The GUI currently displays system profile information; richer navigation and workflow controls remain future work.
