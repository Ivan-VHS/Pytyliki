# Processor Engineering Report v1

**Producer:** Pytyliki C11 processor-systems workstation  
**Report schema:** `pytyliki.report.v1`  
**JSON Schema:** `schemas/pytyliki.report.v1.schema.json`  
**Production command:** `pytyliki --profile --suite --json`

## Purpose

This report is the stable, file-based evidence interface for local processor engineering workspaces such as Reaper. It combines two independent observations from the machine running Pytyliki:

- a read-only processor and memory profile, including ISA flags filtered through OS-managed vector-state checks;
- bounded integer, memory and matrix benchmark runs, with warm-up markers, checksums, integrity flags and descriptive timing statistics.

The report is useful for validating a caller-selected target profile against the current host and for recording reproducible input data. It is not a cross-compiler, target simulator, performance certification or privilege grant.

## Command

Build the workstation, then generate a single profile/suite envelope:

```sh
cmake -S . -B build -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target pytyliki
./build/pytyliki --profile --suite --json > build/processor-report.json
node tools/validate_processor_report.mjs build/processor-report.json
```

Use the generated report as an explicit local file input. The report contains no live daemon connection and Pytyliki does not push data to Reaper.

## Report shape

The top-level object is:

```json
{
  "schema": "pytyliki.report.v1",
  "program": { "name": "Pytyliki", "version": "0.8.0" },
  "profile": { "architecture": "x86-64", "isa": {} },
  "suite": { "schema": "pytyliki.suite.v1", "runs": [], "statistics": {} }
}
```

The checked-in schema is authoritative for the full field set and bounds. Unknown fields are rejected so a future incompatible producer must bump the schema version or publish a new report contract.

## Evidence semantics

- Architecture, CPU vendor/model, logical processor count and memory capacities describe the host where the command ran.
- ISA usability is reported only when both the CPU instruction feature and the operating system's enabled vector state permit it. Hardware presence alone is not sufficient.
- Each benchmark run contains fixed workload parameters, elapsed time, checksum and an integrity flag.
- Warm-up and measured runs are identified separately. Aggregate statistics are descriptive and derived from measured runs.
- Timing values are host-dependent. They must not be interpreted as guaranteed throughput, expected application latency or a comparison across unlike machines without a controlled methodology.
- The report's physical memory and CPU facts are host observations, not a description of an arbitrary target device.

## Reaper integration contract

Reaper may consume the file through its explicit `read_json` effect. The consumer should:

1. validate `schema == pytyliki.report.v1`;
2. verify architecture and every required ISA feature against `profile.isa`;
3. compare declared resource requirements with available memory and logical processor capacity;
4. retain the producer version, workload parameters and timing note in its output report;
5. refuse to infer missing ISA support, toolchain availability or target compatibility from a benchmark score.

A Reaper project manifest remains the authority for the intended compilation target. Pytyliki supplies host evidence, not a substitute target description.

## Safety boundary

Processor inspection is read-only. The profile and suite do not write CPU registers, affinity, frequency, voltage, firmware, microcode, drivers, BIOS/UEFI or device state. Benchmarks allocate a bounded memory buffer in the running process and execute bounded CPU work; they are not real-time qualification tests.

## Verification

The dedicated GitHub Actions workflow builds Pytyliki, generates the report from the runner's actual host, and checks the JSON contract and ISA invariants. The validator does not assert specific CPU models or benchmark values, so it remains portable across CI host changes.
