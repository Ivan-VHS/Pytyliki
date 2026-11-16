# Architecture and safety contract

## Components
The application is split into a platform-neutral C11 core, a portable CLI, and an optional Win32 presentation layer. The core exposes four operations: CPU profile, bounded benchmark, static source scan, and JSON export.

## Data flow
1. CLI or GUI requests a CPU profile.
2. The benchmark performs a fixed-count integer workload with a hard upper bound.
3. The source scanner reads a caller-selected file in text mode and checks lines for a small list of review patterns. It does not parse, compile, link, load, or execute source.
4. JSON export writes a report to a caller-selected path.

## Safety boundaries
- No privileged elevation or kernel driver.
- No BIOS/UEFI, microcode, firmware, voltage, frequency, or power-limit writes.
- No automatic source rewriting or patch application.
- No remote code execution or automatic downloading of code.
- Benchmark cap: 500,000,000 iterations.
- Findings are heuristic review prompts, not proof of a vulnerability.
- The JSON report contains diagnostic metadata only.

## Limitations
CPU brand/model marketing strings are not yet decoded; the tool reports vendor and family/model identifiers. The scanner is intentionally heuristic and currently examines one file per invocation. These limits are explicit so output is not misrepresented as a complete hardware optimization engine.
