#!/usr/bin/env python3
"""Write deterministic, secret-free CI telemetry."""
from __future__ import annotations
import argparse
import json
import os
import platform
from pathlib import Path

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--shard", required=True)
    parser.add_argument("--status", choices=("success", "failure", "cancelled"), required=True)
    parser.add_argument("--check", action="append", default=[])
    args = parser.parse_args()

    checks = {}
    for item in args.check:
        name, separator, outcome = item.partition("=")
        if separator and name:
            checks[name] = outcome or "unknown"
    failed = sorted(name for name, outcome in checks.items() if outcome != "success")
    record = {
        "schema_version": 1,
        "shard": args.shard,
        "result": args.status,
        "status": args.status,
        "checks": checks,
        "failed_checks": failed,
        "platform": platform.system().lower(),
        "machine": platform.machine().lower(),
        "commit": os.environ.get("GITHUB_SHA", "local"),
        "run_id": os.environ.get("GITHUB_RUN_ID", "local"),
    }
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(record, sort_keys=True, indent=2) + "\n", encoding="utf-8")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
