# Pytyliki clinical-safety engineering matrix

Status: **prototype interfaces and deterministic calculations only**. These modules are not cleared for clinical use and do not implement a medical device, clinical workflow product or autonomous decision system.

## Scope and safety boundary

The subsystem accepts structured data from adapters and produces audit facts, data-quality flags, workflow completeness and mathematical summaries. It does not connect to RFID/UWB devices, defibrillators, ventilators, imaging systems, laboratory analyzers, pharmacy systems or hospital access-control systems. Patient identifiers must be pseudonymised upstream and protected by the deploying system.

The software must never autonomously diagnose infection or cancer, prescribe or administer medication, order a biopsy, set isolation status, lock or unlock a clinical area, suppress an alarm, or prevent a clinician from performing urgent care. Production deployment requires clinical governance, privacy/security review, device-specific validation, human-factors testing and applicable regulatory assessment.

## Module V — infection control and epidemiological safety

### 10. Contact-chain review

Engineering interface: **pl_contact_graph_analyze()**.

- Accept timestamped contact edges with pseudonymous person identifiers and zone identifiers.
- Restrict analysis to an explicit UTC time window.
- Count distinct participants and flag zones containing at least a caller-configured number of supplied confirmed identifiers.
- A candidate-zone count is a review cue only, not an outbreak determination or quarantine recommendation.
- RFID/UWB ingestion, identity resolution, clock synchronisation, retention rules and access control remain external adapters and governance responsibilities.

Not implemented: live tracking, contact-tracing identity resolution, automated quarantine recommendations or access-control interlocks.

## Module V — sterilization-cycle audit

### 11. Cycle parameter review

Engineering interfaces: **pl_sterilization_evaluate()** and **pl_sterilization_chain_digest()**.

- Compare recorded temperature, absolute pressure, relative humidity and exposure time with a locally approved, versioned profile supplied by the caller.
- Return a bit mask of parameters outside that configured profile.
- Optionally calculate a chain digest using a caller-supplied SHA-256 provider; the previous digest is included in the canonical payload.

A profile match is not proof of sterility, a biological-indicator result, a release certificate or authorization to use a load. No universal sterilization parameters are embedded. The library does not implement signatures, secure key storage, immutable/WORM storage, retention policy or a cryptographic provider. A hash chain alone cannot make a record immutable.

## Module VI — resuscitation and airway workflow support

### 12. CPR measurement review

Engineering interface: **pl_cpr_review_measurement()**.

- Compare measured compression rate, depth and signal-noise fraction with caller-configured limits.
- Return measurement-quality and configured-range flags.
- The 100–120/min range supplied in the architecture brief belongs in a clinically governed, versioned configuration; it is deliberately not hard-coded as a universal treatment instruction.
- Sensor fusion, transport-noise suppression, audio/haptic metronome output and defibrillator integration are not implemented.

### 13. RSI checklist review

Engineering interface: **pl_rsi_review_checklist()**.

- Accept a required-step mask from an approved local RSI checklist.
- Return the steps that remain unconfirmed.
- Checklist incompleteness is displayed as a reminder; it must not lock the interface, device or clinician out of urgent care.
- No drug names, doses, medication timing engine, airway automation or clinical gating is implemented.

## Module VII — oncology screening and biomarker trend utilities

### 14. Model disagreement summary

Engineering interface: **pl_model_disagreement_analyze()**.

- Compare labels and confidence values already produced by external models.
- Report the majority label, disagreement fraction, tie state and mean reported confidence.
- Labels such as BI-RADS or PI-RADS remain opaque values. This function does not calculate either clinical score, interpret an image, calibrate probabilities or substitute an expert read.
- Image ingestion, overlays, segmentation, model training, explainability and reader adjudication are not implemented.

### 15. Biomarker concentration trend

Engineering interface: **pl_biomarker_trend_analyze()**.

- Fit a least-squares straight line to log concentration against elapsed days for positive, consistently measured input values.
- Return the log slope and mathematical doubling time when the fitted slope is positive.
- The result is a concentration trend only. It is not tumour-volume doubling time, a diagnosis, a validated risk score, or a trigger for biopsy or any other procedure.
- Assay harmonisation, reference ranges, confounders, clinical thresholds and longitudinal validation are external clinical responsibilities.

## Build and tests

The library is C11 and is included in pytyliki_core. CTest exercises bounded synthetic examples for each API. Test fixtures are artificial and do not represent patient data or approved clinical thresholds.

Build commands:

    cmake -S . -B build -DBUILD_TESTING=ON
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

## Production-readiness gates

1. Approve each workflow and configuration through institutional clinical governance.
2. Add authenticated adapters, pseudonymisation, consent/legal basis, role-based access, data minimisation and retention controls.
3. Integrate a reviewed cryptographic provider, key management, signed audit records and protected storage where required.
4. Validate hardware timing, sensor quality, edge cases, time synchronisation and failure-mode behavior on target systems.
5. Complete clinical safety, usability, cybersecurity and jurisdiction-specific regulatory assessment before any clinical deployment.
