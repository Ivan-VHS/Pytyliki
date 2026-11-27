# Contributing to Pytyliki

Pytyliki is a native C11 processor-systems workstation. Contributions should keep hardware inspection read-only, numerical work bounded and public contracts evidence-backed.

## Before opening a pull request

1. Describe the user-visible problem and provide a minimal reproduction, host OS and compiler version.
2. Do not add implicit register writes, firmware changes, CPU affinity changes, clock/voltage control or hardware deployment.
3. Validate malformed input, numeric limits, allocation sizes and error paths.
4. Preserve the public C ABI unless the change explicitly versions it; add tests for public contract changes.
5. Distinguish measured host data from synthetic fixtures, estimates and reference models.
6. Update affected English, Ukrainian, Russian and Simplified Chinese documentation.

## Local verification

~~~sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
python scripts/validate_errors.py
git diff --check
~~~

Optional sanitizer build on supported GCC/Clang toolchains:

~~~sh
cmake -S . -B build-sanitized -DBUILD_TESTING=ON -DPYTYLIKI_ENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-sanitized --parallel
ctest --test-dir build-sanitized --output-on-failure
~~~

## Pull request expectations

Explain the affected layer, compatibility impact, resource bounds, safety implications and test evidence. Attach screenshots only for real application captures; label authored diagrams as illustrations. Keep claims aligned with [FEATURES.md](docs/FEATURES.md) and [DOCUMENTATION_CONTROL.md](docs/DOCUMENTATION_CONTROL.md).

## Security

Do not include private hardware identifiers, secrets or personal machine data in fixtures. See [SECURITY.md](SECURITY.md) for the repository security policy.
