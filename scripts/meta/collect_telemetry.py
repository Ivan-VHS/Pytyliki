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
    args = parser.parse_args()
    record = {
        "schema_version": 1,
        "shard": args.shard,
        "status": args.status,
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
