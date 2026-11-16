#!/usr/bin/env python3
"""Create a human-reviewed proposal; never mutates verification rules."""
from __future__ import annotations
import argparse
import json
from pathlib import Path

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--summary", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    summary = json.loads(Path(args.summary).read_text(encoding="utf-8"))
    failures = summary.get("failed_checks", {})
    lines = [
        "# Verification evolution proposal", "",
        "Deterministic proposal generated from observed CI outcomes.",
        "This tool does not modify source code, workflow files, or merge policy.", "",
        "- Telemetry records: {}".format(summary.get("records", 0)),
        "- Failure entropy: {} bits".format(summary.get("failure_entropy_bits", 0)),
        "- Telemetry digest: {}".format(summary.get("digest_sha256", "unknown")), "",
        "## Findings", ""
    ]
    if not failures:
        lines.append("- No failed checks were recorded; retain the current verification gates.")
    else:
        lines.extend("- {} failed {} time(s); inspect raw logs before changing a gate.".format(k, v)
                     for k, v in sorted(failures.items()))
    lines.extend([
        "", "## Recommended review sequence", "",
        "1. Classify each failure as a product regression, runner/infrastructure fault, or stale rule.",
        "2. Reproduce the failure in a clean runner before proposing any configuration change.",
        "3. Add or update a regression test for the observed failure mode.",
        "4. Submit a separate branch and pull request; require passing CI and human review.",
        "5. Merge only through the repository's normal protected-branch process.", "",
        "## Safety boundary", "",
        "No LLM-generated patch is applied automatically. No failed test is suppressed automatically.",
        "No pull request is auto-approved or auto-merged."
    ])
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("Proposal written: {}".format(output))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
