#!/usr/bin/env python3
"""Write a deterministic telemetry record for one CI matrix job."""
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--platform", required=True)
    for name in ("configure", "build", "test", "smoke", "python"):
        parser.add_argument("--" + name + "-status", default="unknown")
    args = parser.parse_args()
    checks = {name: getattr(args, name + "_status")
              for name in ("configure", "build", "test", "smoke", "python")}
    failed = [name for name, status in checks.items()
              if status not in {"success", "skipped"}]
    record = {
        "schema_version": 1,
        "run_id": os.getenv("GITHUB_RUN_ID", "local"),
        "run_attempt": os.getenv("GITHUB_RUN_ATTEMPT", "1"),
        "commit": os.getenv("GITHUB_SHA", "unknown"),
        "platform": args.platform,
        "checks": checks,
        "result": "failure" if failed else "success",
        "failed_checks": failed,
    }
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print("Telemetry written: {} ({})".format(output, record["result"]))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
