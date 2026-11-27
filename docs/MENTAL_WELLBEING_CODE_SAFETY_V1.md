# Mental Wellbeing & Depression Awareness v1 — Code Safety Standard

**Policy ID:** `mental-wellbeing-code-safety.v1`  
**Applies to:** Pytyliki C11 processor workstation  
**Status:** Engineering guardrails; not a clinical protocol

## Why this exists

This standard translates the non-stigmatizing, help-seeking and self-care themes in the supplied *Mental Wellbeing & Depression Awareness v1* material into software-engineering safeguards. The supplied transcript contains only the opening and closing segments; its missing middle is intentionally not reconstructed. Personal stories, the sponsor segment, timestamps and the quoted prevalence figure are not used as clinical evidence or executable rules.

Pytyliki is a processor, source-diagnostics and local-DSP workstation. It is not a mental-health product. It must not diagnose, screen, score, infer or store a person's mental-health state.

## Non-negotiable safeguards

- **No diagnosis or inference:** never infer depression, distress, motivation, disability or psychological traits from CPU use, benchmark results, source code, error patterns, audio, usage time or performance.
- **No medical advice:** do not recommend medication, treatment, dosage, therapy or clinical decisions. Do not present benchmark output as a health or wellbeing measure.
- **Data minimization:** do not add telemetry for source contents, file paths, device identifiers, audio, personal logs or wellbeing-related text. Any future optional telemetry needs a separate, explicit, informed opt-in and documented retention/deletion rules.
- **Safe diagnostics:** static source review must remain inspection-only. Do not execute inspected code, upload it, or include source snippets and local paths in diagnostics by default.
- **Bounded native processing:** validate sizes and ranges before allocation, indexing, pointer arithmetic, RIFF parsing and integer conversion. Keep sanitizer-enabled tests in the existing C11 CI path.
- **Respectful wording:** errors describe the operation and recovery step, never label a person as weak, lazy, careless or incompetent.
- **Accessible control:** destructive actions require clear intent; errors remain actionable; keyboard operation and readable contrast must not be sacrificed for decorative motion.
- **Blameless engineering:** incidents and regressions are documented as system conditions and corrective actions, not personal fault. Do not introduce developer mood, health or productivity surveillance.

## Pytyliki-specific threat model

| Surface | Required protection |
|---|---|
| Source diagnostics | Treat file content as untrusted; parse/inspect only; bound input and report sizes; redact paths and snippets |
| CPU / ISA discovery | Read-only queries; no user profiling or health interpretation |
| Benchmarking | Bounded workloads; report measured performance only; avoid ranking people or inferring capacity |
| PCM16 WAV / DSP | Validate RIFF chunk lengths and sample bounds; do not infer emotion or wellbeing from audio |
| Memory / C ABI | Preserve checked slices, overflow checks, caller-owned buffers and explicit ABI boundaries |
| Reports | Emit only data needed for the requested report; no hidden personal-state fields |

## Change acceptance checklist

A change touching parsing, diagnostics, reports, hardware discovery, DSP or persistence is ready only when reviewers can answer **yes** to all applicable items:

1. Inputs are bounded and malformed input fails closed without executing it.
2. No new personal or mental-health data is collected, inferred, logged or transmitted.
3. Output language is factual, non-stigmatizing and does not claim clinical meaning.
4. Reports disclose uncertainty and distinguish measurements from estimates.
5. Tests cover invalid lengths, boundary values, overflow and safe failure paths.
6. Existing C11, CTest and sanitizer checks pass.

## Automated enforcement

`mental-wellbeing-code-safety.v1.json` is the machine-readable policy contract.  
`scripts/check_mental_wellbeing_guardrails.py` validates its required prohibitions, project-specific boundary and this document. The dedicated GitHub Actions workflow runs that validator on pushes and pull requests. This gate verifies policy integrity; it does not replace code review, C sanitizers or security testing.

## Scope and limitations

This is a software privacy, safety and respectful-language standard. It is not diagnosis, screening, counselling, crisis response or treatment guidance. Anyone seeking medical help should use an appropriately qualified healthcare professional; the workstation must not attempt to replace one.
