# Conversation-derived project context

This file records the project requirements that were available in the current conversation context when Pytyliki 0.5.0 was prepared. It is a traceability note, not a transcript archive.

## Consolidated engineering streams

### 1. Pytyliki as the central program
Requested direction: one native application that inventories and analyzes project components, coordinates safe transformations, runs checks, and reports results. The current repository was verified to be a small C11 CPU Lab foundation. The implementation therefore extends that codebase instead of claiming a finished universal engineering platform.

### 2. Carrisime and CPU optimization
Requested direction: CPU identification, bounded benchmarking, profiling, architecture-aware optimization, and performance reporting. Current implementation covers local CPU identification and a bounded integer benchmark. It does not overclock, alter firmware, tune voltage/frequency, or automatically rewrite processor microcode or hardware controls.

### 3. ISO C11/C23 and deterministic transpilation
Requested direction: transform higher-level source into portable low-level C, use an intermediate representation, validate semantics, and test across compilers and architectures. Current implementation adds an explicit reference pipeline plan only. Parser, IR, code generator, and semantic-equivalence engine remain future work.

### 4. GitHub integration and authorization
Requested direction: inspect repositories, manage branches and Pull Requests, diagnose GitHub App permissions, and automate CI without bypassing review. The local policy module provides a role/action model; it does not grant or replace external GitHub permissions. Repository writes must remain reviewable and should use feature branches and Pull Requests.

### 5. Architecture code and security automation
Requested direction: modular boundaries, least privilege, reproducible builds, static analysis, test gates, security scanning, and auditability. The repository currently applies read-only workflow token permissions and a Windows/Linux build-test matrix. Additional scanners and repository-wide orchestration remain planned.

### 6. Clinical rule-to-engineering matrix
Requested direction: formalize infection-control, emergency-response, and oncology-screening requirements as traceable system workflows. The application now exposes three metadata templates. No clinical thresholds, patient-specific logic, diagnostic rules, or treatment instructions are embedded.

### 7. Visual identity
Requested direction: a clear white / blue / red appearance across project presentation. The README banner and Win32 window now use a white surface, deep-blue structure, and restrained red accent.

## Source coverage and limits

The current assistant context contained the latest project summaries and selected recent conversation excerpts, including the five-topic consolidation immediately preceding this change. It did **not** contain 666 complete conversation transcripts or a complete export of the user's chat history. Therefore this file records only verifiable, available project requirements; it does not claim to have ingested, searched, or implemented 666 unseen threads.

To extend this register accurately, provide an export or files containing the additional conversations. Each imported requirement should be assigned an ID, source date, scope, implementation status, and acceptance test before it is treated as an engineering requirement.
