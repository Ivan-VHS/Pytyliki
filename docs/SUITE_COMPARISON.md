# Comparing Pytyliki suite runs

The CPU suite already emits repeated measurements. This tool compares two saved `pytyliki.report.v1` documents without converting a noisy wall-clock result into a performance promise.

## Capture two runs

Build and test Pytyliki as described in the root README. On the same host, capture a baseline before a code or configuration change:

```sh
./build/pytyliki --profile --suite --repeat 10 --json > baseline.json
```

After the change, repeat the exact command and save the second report:

```sh
./build/pytyliki --profile --suite --repeat 10 --json > candidate.json
python tools/compare_suite_runs.py baseline.json candidate.json --json comparison.json
```

Use the Windows executable path under `build/Release/` when running the desktop toolchain. Capture both files on the same machine, with the same power profile, background workload and thermal conditions as far as practical.

## What is checked

The comparator validates both report and suite schema identifiers, all measured samples, integrity flags and the workload settings. It refuses to compare different processor architectures, CPU models, vendors, logical-processor counts or suite workload configurations. Warm-up records are excluded; at least two measured samples are required in each file.

For `integer_ms`, `memory_ms` and `matrix_ms`, it reports the median and median absolute deviation (MAD). A difference is labelled `within_noise` when its absolute value does not exceed the larger of:

- the configured percentage of the baseline median (1% by default);
- three times the sum of the baseline and candidate MAD values.

Use `--noise-percent 2.0` to widen the percentage floor, or `--noise-percent 0` to rely only on the observed MAD values. Lower execution time is better. A result may be `improved`, `regressed` or `within_noise`; the overall result reports a regression if any of the three measures regresses.

## Exit codes and limits

- `0`: both reports are valid and comparable. This is not a CI performance gate.
- `2`: a file, schema, sample or option is invalid.
- `3`: the reports are valid but their host or workload contexts differ.

The machine-readable `pytyliki.suite-comparison.v1` report contains the medians, MAD values, deltas, noise bands and comparison decision. The calculation is a practical noise guard, not a statistical significance test. Results remain host-dependent; this tool does not rank CPUs or guarantee a speedup.