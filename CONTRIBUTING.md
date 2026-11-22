# Contributing to Pytyliki

Pytyliki is a C11 processor-systems and DSP workstation. Keep changes measurable, bounded and honest about which facts are detected, simulated, planned or described.

## Before opening a pull request

1. File an issue for a defect or substantial design change. Include the observed behaviour, host platform and a minimal reproduction.
2. Keep processor inspection read-only. Do not add implicit register writes, firmware changes, affinity changes, voltage or clock control.
3. Keep DSP work bounded and validate malformed WAV input, numeric limits and allocation sizes before processing.
4. Preserve the public C ABI unless the change explicitly versions it. Add a test for every new public contract.
5. Update English, Russian, Ukrainian and Simplified Chinese documentation when public behaviour changes.

## Local verification

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
python scripts/validate_errors.py
```

For memory-safety-sensitive changes, also use the repository sanitizer configuration where supported.

## Pull request expectations

Explain the user-visible result, compatibility impact, resource bounds and evidence for performance claims. Include CLI output or a screenshot when useful. Do not describe a roadmap item as implemented until code and tests are present. Maintainers decide issue closure after review.
