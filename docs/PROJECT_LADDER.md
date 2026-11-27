# Pytyliki project maturity

**Current stage:** 04 — Ascension  
**Assessment:** provisional  
**Next stage:** 05 — Vanguard  
**Canonical ladder:** [DAK Engineering Maturity Ladder](https://github.com/Ivan-VHS/DAK/blob/main/docs/PROJECT_EVOLUTION_LADDER.md)  
**Machine record:** [`.github/project-ladder.v1.json`](../.github/project-ladder.v1.json)

The recorded stage is the highest cumulative gate currently supported by repository evidence. It is not a release-quality score and does not imply that later-stage capabilities exist.

## Next gate — Vanguard

- Exercise the primary C11 core and CLI on each declared x86-64 target in CI.
- Record compiler-matrix, sanitizer and repeatable resource/performance evidence.
- Keep hardware inspection read-only and tie every target claim to named evidence.

## Assessment limits

This record is provisional because the full target build, test matrix and relevant integration workflow were not re-run during ladder adoption. Existing documentation and CI definitions are evidence references, not a new certification.

## Promotion procedure

Update this record in the same pull request as implementation. A stage may be marked `met` only when every required evidence category is linked to an existing repository artifact. `verified` additionally requires a pinned commit, workflow, environment, CI run URL, passing test evidence and passing CI evidence. Higher stages remain pending until all lower cumulative gates remain satisfied.
