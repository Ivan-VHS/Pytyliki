# Mental Wellbeing Awareness reference

Pytyliki exposes this topic as a small native C11 reference command, separate from processor telemetry and DSP measurements.

```sh
pytyliki-wellbeing --catalog
pytyliki-wellbeing --topic antidepressant-myths
```

The command presents only source-attributed education. It does not assess a user, inspect host signals, or make medical recommendations. The source bundle and schema are in `data/wellbeing/` and `schemas/`.


## Source and evidence boundary

Source: [ебаут — «Депресія — тема табу? Ознаки депресії, міфи довкола антидепресантів, як допомогти собі»](https://www.youtube.com/watch?v=brNbnipJ-i8), published 2025-11-23, duration 1:20:01. The supplied transcript contains only 00:00–00:07:55 and 01:17:16–01:19:53. The large middle portion is missing. Each topic carries evidence status and timestamp offsets. The 280-million figure is recorded as a claim voiced in the episode, not as a current verified statistic.

## Product safety contract

- Education and destigmatisation only; no diagnosis, screening, risk score, treatment plan or symptom interpretation.
- No inference from CPU, device, audio, project, work-pattern or user telemetry.
- No medication, dosage, start/stop or substitution instructions. The episode's closing advice says medication should be taken only by a psychiatrist's prescription.
- The speakers' experiences and the Stutz pyramid are attributed as personal statements, not medical protocols.
- Missing transcript sections remain explicitly unavailable. The title's reference to antidepressant myths is not evidence of what was said about those myths.

## Data contract

`data/wellbeing/depression-awareness.v1.json` follows `schemas/wellbeing-reference.v1.schema.json`. `evidence_status=insufficient_transcript` means a topic is named by the video title but cannot be summarised from the supplied excerpts.
