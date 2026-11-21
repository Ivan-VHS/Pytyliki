# Hardware rule engine

Inputs: detected capabilities, platform profile and constraints.

| Result | Meaning |
|---|---|
| pass | evidence satisfies rule |
| warn | incomplete/conditional evidence |
| unknown | insufficient evidence |
| fail | explicit contradiction |

Scores are advisory; they do not certify stability or performance. Discovery is read-only. No voltage, clock, firmware, BIOS, driver or device-register changes are made.
