#!/usr/bin/env python3
"""Aggregate CI outcomes deterministically; do not use opaque classifiers."""
from __future__ import annotations
import argparse
import hashlib
import json
import math
from collections import Counter
from pathlib import Path

def load_records(root: Path) -> list[dict]:
    records = []
    for path in sorted(root.rglob("*.json")):
        try:
            value = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            continue
        if isinstance(value, dict) and value.get("schema_version") == 1 and "checks" in value:
            records.append(value)
    return records

def shannon_entropy(counts: Counter) -> float:
    total = sum(counts.values())
    if total == 0:
        return 0.0
    return -sum((count / total) * math.log2(count / total)
                for count in counts.values() if count)

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--json-output", default="telemetry-summary.json")
    parser.add_argument("--markdown-output", default="telemetry-summary.md")
    parser.add_argument("--github-output")
    args = parser.parse_args()
    records = load_records(Path(args.input))
    outcomes = Counter(record.get("result", "unknown") for record in records)
    failed = Counter(check for record in records for check in record.get("failed_checks", []))
    platforms = sorted({str(record.get("platform", "unknown")) for record in records})
    packed = json.dumps(records, sort_keys=True, separators=(",", ":")).encode("utf-8")
    summary = {
        "schema_version": 1,
        "records": len(records),
        "outcomes": dict(sorted(outcomes.items())),
        "failed_checks": dict(sorted(failed.items())),
        "platforms": platforms,
        "failure_entropy_bits": round(shannon_entropy(outcomes), 6),
        "digest_sha256": hashlib.sha256(packed).hexdigest(),
    }
    Path(args.json_output).parent.mkdir(parents=True, exist_ok=True)
    Path(args.json_output).write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n",
                                      encoding="utf-8")
    lines = [
        "# CI telemetry summary", "",
        "- Records: **{}**".format(summary["records"]),
        "- Outcomes: " + json.dumps(summary["outcomes"], sort_keys=True),
        "- Platforms: " + (", ".join(platforms) if platforms else "none"),
        "- Failure entropy: **{} bits**".format(summary["failure_entropy_bits"]),
        "- SHA-256: {}".format(summary["digest_sha256"]), "", "## Failed checks", ""
    ]
    lines.extend("- {}: {}".format(name, count) for name, count in sorted(failed.items()))
    if not failed:
        lines.append("- No failed checks recorded.")
    Path(args.markdown_output).write_text("\n".join(lines) + "\n", encoding="utf-8")
    if args.github_output:
        with Path(args.github_output).open("a", encoding="utf-8") as output:
            output.write("digest={}\n".format(summary["digest_sha256"]))
            output.write("records={}\n".format(summary["records"]))
            output.write("failures={}\n".format(sum(failed.values())))
    print("Aggregated {} telemetry record(s)".format(len(records)))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
