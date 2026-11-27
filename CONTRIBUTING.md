# Contributing to Pytyliki

Thank you for helping improve Pytyliki. The project is a native C11 processor-systems workstation with a Windows GUI and portable command-line tools. Contributions should make its implemented behavior clearer, safer to inspect, and easier to reproduce.

## Preserve the project boundary

Processor inventory remains read-only. Benchmarks are bounded, host-dependent measurements, not promises of speedup. Do not add scheduler, affinity, clock, firmware, register-write, or hardware-control behavior under the name of inspection. DSP and file-processing changes must document their input limits and output-format effects.

## Before opening a pull request

1. Read README.md and the relevant architecture and quality documents under docs/.
2. Keep public C interfaces explicit and preserve existing behavior unless a change is intentional and documented.
3. Add a focused regression test for every bug fix and boundary case.
4. Update README.md, README.uk.md, README.ru.md, and README.zh-CN.md when user-visible behavior, supported limits, commands, or examples change.
5. Update the relevant JSON contract or schema when a machine-readable format changes.
6. Describe limitations and platform-specific behavior instead of silently skipping unsupported work.

## Local checks

Use the same compiler family and commands as the CI workflow where possible.

    cmake -S . -B build -DBUILD_TESTING=ON
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure
    python .github/scripts/check_readme_links.py
    python .github/scripts/check_readme_parity.py

Run the architecture and project-contract validators used by CI when their related files change.

## Pull request expectations

A good pull request explains the user problem, the smallest viable change, compatibility effects, tests run, and any known limitations. Do not include generated binaries, machine-specific benchmark results, private data, or screenshots presented as genuine captures unless they were produced by the repository workflow.

## Security and review

Do not execute untrusted input in tests without explicit bounds. Report security concerns privately using the contact route in SECURITY.md. Keep changes focused; avoid broad formatting or unrelated generated-file churn.
