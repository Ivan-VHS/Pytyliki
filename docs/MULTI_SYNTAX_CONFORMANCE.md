# Reaper multi-syntax conformance corpus

**Project author/developer:** Ivan-VHS

Pytyliki owns a small, versioned conformance corpus that checks the public Reaper syntax-validation interface. The corpus covers native Reaper, the safe Python subset, pipelines, Lisp, Forth, Brainfuck, Befunge-93 and a source-ordered polyglot file. It includes both valid and intentionally invalid input.

Run the harness against an explicit local Reaper checkout:

```sh
python tools/multisyntax_conformance.py --reaper-root ../Reaper --json
```

The harness invokes only `python -m reaper syntax --check`; it never runs the fixture programs. It requires the target checkout to contain `reaper/multisyntax.py`, rejects symlinks and path traversal in fixtures, caps input size and case count, and launches the validator with `shell=False` and a bounded timeout.

This corpus validates syntax-check contracts and the documented frontend subsets. It does not prove full CPython, Scheme, ANS Forth or other language-standard conformance, and it does not benchmark runtime performance.
