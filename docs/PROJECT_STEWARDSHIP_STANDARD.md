# Project stewardship and anonymous operating principles

**Status:** normative project-governance guidance. This document defines review expectations; it does not claim that every organizational practice is automatically enforced by the application.

## Purpose

Build a durable engineering workstation through disciplined execution, accountable decisions, respectful collaboration and evidence-led communication. The project's value is measured by safe, repeatable outcomes—not by visibility, status or promotional activity.

## Operating principles

1. **Purpose before attention.** Every feature, benchmark and public statement must answer a concrete user or engineering need. Prefer a useful, measured result over a headline, vanity metric or exaggerated claim.
2. **Discipline without opacity.** Keep scope, owner, status, dependencies, acceptance criteria and rollback conditions explicit. A confident presentation never replaces a reproducible result.
3. **Authority follows responsibility.** Decisions belong to the role that owns the affected system boundary. Record material trade-offs; avoid informal or hidden approval paths.
4. **Evidence over reputation.** Evaluate proposals by source, measurement context, tests and observed behavior—not by the seniority, identity, public profile or personal relationships of the proposer.
5. **Adapt communication to the work.** Use concise asynchronous updates for routine progress, synchronous discussion for ambiguity or high-impact coordination, and durable written records for decisions.
6. **Protect continuity.** Document operational knowledge, rotate review responsibility where practical and avoid critical workflows that depend on one person being permanently available.

## Decision record

For a material architecture, safety, release or product decision, record:

- **Outcome:** the user or engineering problem being addressed.
- **Owner:** the accountable role, not an informal power relationship.
- **Evidence:** relevant code, measurements, tests, constraints and unknowns.
- **Alternatives:** credible options considered and why they were rejected.
- **Risk and recovery:** likely failure modes, containment and rollback.
- **Review point:** the condition or date that will trigger reassessment.

Small reversible choices can remain lightweight. Irreversible, safety-sensitive or externally visible choices require a stronger record.

## Execution rhythm

1. **Intake:** classify the request by user impact, safety, urgency, reproducibility and confidence.
2. **Plan:** split work into bounded, reviewable changes with explicit acceptance criteria.
3. **Execute:** preserve the read-only default for inspection and keep measurements within declared limits.
4. **Review:** inspect the diff, test the behavior, verify documentation and check that claims match the implementation.
5. **Release:** publish only validated changes; describe known limits and remaining work plainly.
6. **Learn:** capture incidents and user feedback as actionable follow-up, not as personal blame.

Keep work-in-progress limited enough that critical reviews, fixes and handoffs do not stall behind unfinished initiatives.

## Communication, feedback and public credibility

- Separate **observed fact**, **interpretation**, **proposal** and **unknown**.
- Describe benchmark hardware, operating conditions, units, sampling window and uncertainty. Do not present local comparisons as certification or universal rankings.
- Treat criticism as input to investigate. Record reproduction steps and impact; do not retaliate, expose private information or turn disagreement into spectacle.
- Prioritize feedback by severity, affected users, reproducibility, confidence and remediation cost. Popularity alone is not priority.
- Announce incidents with confirmed scope, user impact, containment and next update. Correct earlier statements visibly when evidence changes.
- Avoid manufactured controversy, engagement bait, false urgency and marketing language that obscures technical limitations.

## Incident and conflict protocol

Use the sequence **stabilize → assess → contain → communicate → recover → review**.

Preserve relevant logs and measurement context; limit access to sensitive material; assign one incident coordinator; state what is known and unknown; provide a safe workaround where possible; verify recovery with tests; and publish a blameless retrospective with owners and due dates. Security-sensitive details belong in the repository's approved private reporting path, not in a public issue.

## Sustainable stewardship

Maintain a current risk register with impact, likelihood, owner, mitigation and review date. Turn long-term ambitions into observable milestones. Keep personal, family and other private-life information out of issue reports, telemetry, screenshots and release notes unless a person has deliberately supplied information that is strictly necessary and consented to its publication.

Respectful collaboration is role-neutral: access, credit and review standards do not depend on gender, age, family status, popularity or personal relationships. Clear professional boundaries protect both contributors and project decisions.

## Pytyliki-specific acceptance gates

- Processor discovery, source inspection and hardware-facing reports remain read-only; this doctrine does not authorize BIOS, firmware, microcode, driver, clock, voltage or device-register writes.
- Every benchmark claim must preserve its hardware/software context and measurement provenance. Incompatible measurement contexts must not be merged into a single score.
- DSP and memory workloads remain bounded, input-validated and testable. Report failure and uncertainty instead of silently substituting values.
- Distinguish implemented capabilities from simulations, reference profiles, advisory scores and future work.
- A release candidate is not ready until the relevant CMake builds, CTest suite, platform checks and applicable sanitizer checks have passed or their gaps are explicitly disclosed.

## Release review checklist

- [ ] The change solves a named user or engineering problem.
- [ ] Scope, owner, risks and acceptance criteria are documented.
- [ ] Relevant tests and builds have been run and results are recorded.
- [ ] Hardware and measurement safety boundaries remain intact.
- [ ] Public claims match the current source and test evidence.
- [ ] Privacy, accessibility, localization and recovery paths were considered.
- [ ] Remaining limitations and follow-up owners are visible.

**Interpretation rule:** this standard strengthens planning and accountability. It does not expand product permissions, certify hardware, replace code review or convert a proposal into an implemented capability.
