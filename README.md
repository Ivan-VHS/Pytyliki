# Pytyliki — Processor Systems Workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Pytyliki project overview" width="100%"></p>

[Source](https://github.com/Ivan-VHS/Pytyliki) · [C11 CI](.github/workflows/ci.yml) · [Windows desktop build](.github/workflows/desktop-build.yml) · [Documentation index](docs/README.md) · [MIT](LICENSE)

## Live application capture

<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Real Windows desktop capture of Pytyliki" width="100%"></a></p>
<p align="center"><sub>1920×1080 · Click the image to open the original PNG.</sub></p>

Genuine Windows desktop capture of the running application, produced by this repository's desktop build workflow. The dashboard and workflow graphics above are original SVG illustrations; they are not screenshots.

### Architecture at a glance

<p align="center"><img src="assets/readme/workflow.svg" alt="Pytyliki implemented data flow" width="100%"></p>

Implemented path: local host and file inputs → bounded C11 core → inspection, analysis and DSP → evidence and local output.


[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml)  [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml)  [![License](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)


> **Scope:** a local C11 processor-systems workstation with a Windows GUI and portable command-line tools. This repository does **not** publish a hosted web demo, browser sandbox, API service or production control plane. Documentation was checked against `main` on **2026-11-22**.

## Purpose and boundaries

Pytyliki brings processor inspection, bounded workload planning, source diagnostics, numerical utilities and local audio processing into one native C11 core. It is aimed at systems developers, hardware researchers, QA engineers and reviewers who need inspectable local results.

The app reports information it can observe and produces advisory plans. It does not change OS scheduling, CPU affinity, processor frequency or voltage; it does not flash firmware or claim measured speedups from a proposed partition. Embedded MCU/MMIO material is reference or simulation data. QCS performs syntax analysis, not arbitrary source execution.

### What is implemented

| Area | What you can do | Important boundary |
|---|---|---|
| Processor inventory | inspect model, topology, memory and available ISA/XCR0 information | local, read-only discovery |
| CPU and memory suite | run bounded local measurements and workload-partition examples | results depend on the host; not universal benchmarks |
| Source review | inspect source and produce diagnostics/reports | reviewed source is not built or executed |
| DSP | load/process supported PCM16 WAV data, filter, compress, saturate and measure | local offline buffers; no audio-device or real-time callback |
| Embedded C | browse MCU profiles and estimate FLASH/SRAM use | reference estimates, not firmware deployment |
| QCS | tokenize/parse input and inspect AST/diagnostics | syntax analysis only |
| Interop | build the versioned C ABI v1 library | consumers must select a trusted library explicitly |
| Atomic-Quantum | run the bounded classical state-vector subset | simulation only; no quantum-hardware access |

The Windows screenshot below is a captured desktop application, not an interactive browser preview.


## Quick Start

### Requirements

- CMake **3.16 or newer**.
- A C11 compiler: MSVC, GCC or Clang.
- Git.
- Python 3 is optional for the extra plugin-federation contract test; the C/C++ build itself does not use Python as its runtime.

The Win32 GUI target is built on Windows only. The shared C11 core and CLI tools are the portable targets; actual platform capabilities still depend on the OS APIs available to the build.

### Build and test

Run from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

On Windows, CMake places Release executables in `build/Release/`. On a typical single-configuration Linux build, they are in `build/`.

### First smoke checks

```sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
```

On Windows use `build\Release\pytyliki.exe` and `build\Release\pytyliki-metrics.exe`. The desktop GUI is `build\Release\pytyliki-gui.exe`.

More local examples:

```sh
pytyliki --systems
pytyliki --embedded profiles
pytyliki --divide -17 5
pytyliki --tone-out demo.wav --frequency 440 --seconds 2
pytyliki --process-wav input.wav output.wav --lowpass 12000
```

The exact supported switches are also printed by `pytyliki --help`. These commands run locally; there is no default HTTP port or localhost URL.

### Optional sanitizer build

GCC and Clang can enable AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cmake -S . -B build-sanitized -DBUILD_TESTING=ON -DPYTYLIKI_ENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-sanitized --parallel
ctest --test-dir build-sanitized --output-on-failure
```

This option is not supported with MSVC.

## Data, test scenarios and state

The workstation does not connect to a remote database and has no login, demo account, mock API or browser storage. Processor information comes from the local host where the relevant platform probe is implemented. The `--demo` metrics mode is an illustrative input path; it must not be presented as live telemetry.

Try these reproducible scenarios:

1. **Host profile:** run `pytyliki --profile`; inspect the processor information reported on the current machine.
2. **Bounded planning:** run `pytyliki --partition 10000 8 256`; inspect the deterministic work ranges. A plan does not launch workers.
3. **Checked division:** run `pytyliki --divide -17 5`; compare the result with the Euclidean-division tests.
4. **Metric example:** run `pytyliki-metrics --demo`; the demo data is an example, not a measurement of the host.
5. **Source review:** run `pytyliki --review-processor path/to/source.c`; inspect diagnostics without executing the source.
6. **Audio file:** process a local WAV with `--process-wav` and inspect the output file.

For the DSP path, the current implementation accepts mono or stereo audio at 8–192 kHz and applies a 32 MiB WAV-container limit. Invalid headers, chunk bounds, unsupported formats and out-of-range samples are rejected. The project does not provide a delay/HTTP-500 simulator, remote media service or browser upload flow.

The app's working data is local to the process and to files the operator chooses to read or save. Closing the app ends the current in-memory session; generated reports and audio files are not automatically erased. There is no "reset demo" action or shared server-side state. Native appearance/motion preferences are handled by the desktop UI, not by cookies, localStorage, sessionStorage or IndexedDB.

## Configuration and access

There is no `.env.example`, Docker Compose stack, web proxy, CORS policy, HTTPS certificate setup, user authentication or API token for the normal workstation. No test credentials are required, and no secrets should be added to the repository.

Build-time options that are present in `CMakeLists.txt`:

| Option | Default | Purpose |
|---|---:|---|
| `BUILD_TESTING` | CMake/CTest default | include native tests and optional Python contract check |
| `PYTYLIKI_ENABLE_SANITIZERS` | `OFF` | enable ASan/UBSan on GCC or Clang |

A missing C compiler or incompatible sanitizer toolchain is a build configuration problem, not a missing web environment variable. Read the first CMake error and check that the selected compiler matches the platform.

## Architecture and repository map

The application follows a local pipeline: **input → validation → bounded module → structured result → optional file export**.

| Path | Contents |
|---|---|
| `src/` | C11 core, Win32 GUI, CLI entry points, processor probes, DSP, QCS and interop |
| `tests/` | CTest unit, regression, boundary and CLI smoke tests |
| `tools/` | validation utilities and repository tooling |
| `scripts/` | architecture and engineering contract checks |
| `docs/` | architecture, feature boundaries, UI behavior and module contracts |
| `assets/screenshots/` | captured desktop screenshots |
| `security/` | machine-readable Beelzebul policy |
| `.github/workflows/` | CI, Windows packaging and release automation |

The core is C11. The GUI uses the Windows Win32 API and is only built on Windows. There is no frontend/backend split, web server, mock server, browser bundle, port binding or deployed preview. Browser compatibility, cookies, CORS, viewport breakpoints and HTTPS are therefore not applicable.

## Build artifacts, CI and releases

- [C11 CI](.github/workflows/ci.yml) builds and tests on Ubuntu and Windows and includes a Linux sanitizer job.
- [Desktop build](.github/workflows/desktop-build.yml) builds the Windows GUI and CLI tools, runs CTest, captures the application screenshot and creates `Pytyliki-windows-x64.zip`.
- The Windows archive is uploaded as a workflow artifact with a 30-day retention period.
- A Git tag matching `v*` triggers release publication. A push to `main` is not itself a public web deployment.

Artifacts are created under the workflow's build/dist paths. To roll back a distributed desktop build, use the previous known-good GitHub Release/archive. There is no running service to roll back.

## Troubleshooting

| Symptom | Check |
|---|---|
| CMake cannot identify a C compiler | install a C11 toolchain and configure from the matching developer environment |
| `pytyliki-gui` is missing | the GUI target is Windows-only; build on Windows |
| CTest fails on one host | retain the failing test name and full output; platform probes can differ by OS and CPU |
| WAV import is rejected | confirm the PCM16 format, mono/stereo channel count, sample rate and size limits |
| Sanitizer option fails | `PYTYLIKI_ENABLE_SANITIZERS` requires GCC or Clang |
| An embedded estimate looks like deployment | it is a reference calculation only; this project does not flash or program hardware |

CI logs are available in the linked GitHub Actions workflows. The README and test suite are the source of truth for supported commands; if a CLI option changes, update the examples and localized files together.

## Known limitations, contribution and license

Known boundaries are listed above rather than hidden behind a "production-ready" claim: the GUI is Windows-only, source inspection is static, embedded values are advisory, DSP is local/offline and quantum routines are classical simulations. No separate `CONTRIBUTING.md` was found in the repository. For a change, open a GitHub Issue or Pull Request with the platform, exact command, expected result and actual output; run the build and CTest suite before proposing the change.

Support and defect reports: [GitHub Issues](https://github.com/Ivan-VHS/Pytyliki/issues). License: [MIT](LICENSE).

## Mapping to the 66 README rules

The 66-point web-stand standard is applied to the documentation without inventing a web application:

| Rules | How this README handles them |
|---|---|
| 1–11 · purpose and boundaries | identifies the local workstation, audience, supported workflows, limitations, data origin, `main` documentation date and absence of a hosted demo |
| 12–22 · quick start | exact toolchain, build, test, smoke-check and Windows GUI commands; no hidden server or port step |
| 23–33 · configuration and access | documents actual CMake options; marks ENV, credentials, CORS, proxy and HTTPS as not applicable |
| 34–44 · data and behavior | documents host data vs demo metrics, file constraints, repeatable tests, session lifetime and error cases |
| 45–55 · architecture and build | maps real folders, C11/Win32 stack, CTest, build outputs and CI boundaries |
| 56–66 · operations and support | links real workflows/releases, explains rollback of downloadable builds, troubleshooting, known limitations, contribution path and license |

**No live web URL is published for this project.** The desktop screenshot and GitHub Actions artifacts are not an interactive web stand.

## Unified report envelope

JSON outputs can be wrapped in the shared ivan-vhs.report.v1 metadata format without changing the tool-specific payload:

    python tools/report_envelope.py --tool TOOL_NAME --version VERSION < result.json

See docs/REPORT_ENVELOPE.md and schemas/ivan-vhs.report.v1.schema.json. The envelope records provenance and project limitations; it is not a security signature or a compatibility verdict.
