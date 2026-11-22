#!/usr/bin/env python3
"""Compare repeatable Pytyliki benchmark reports without external dependencies."""
from __future__ import annotations

import argparse
import json
import math
import statistics
import sys
from pathlib import Path
from typing import Any

METRICS = ("integer_ms", "memory_ms", "matrix_ms")
SCHEMA = "pytyliki.suite.v1"


def _load_suite(path: str | Path) -> dict[str, Any]:
    try:
        payload = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ValueError(f"cannot read benchmark report {path}: {error}") from error
    if not isinstance(payload, dict):
        raise ValueError(f"{path}: report root must be a JSON object")
    suite = payload.get("suite", payload)
    if not isinstance(suite, dict) or suite.get("schema") != SCHEMA:
        raise ValueError(f"{path}: expected {SCHEMA}")
    runs = suite.get("runs")
    if not isinstance(runs, list) or not runs:
        raise ValueError(f"{path}: suite must contain a non-empty runs array")
    return suite


def _measured_values(suite: dict[str, Any], metric: str) -> list[float]:
    values: list[float] = []
    for index, run in enumerate(suite["runs"]):
        if not isinstance(run, dict):
            raise ValueError(f"runs[{index}] must be an object")
        if run.get("warmup") is True:
            continue
        if metric not in run:
            continue
        value = run[metric]
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ValueError(f"runs[{index}].{metric} must be a number")
        number = float(value)
        if not math.isfinite(number) or number < 0:
            raise ValueError(f"runs[{index}].{metric} must be finite and non-negative")
        values.append(number)
    return values


def compare_suites(baseline: dict[str, Any], candidate: dict[str, Any],
                   regression_threshold_percent: float = 5.0) -> dict[str, Any]:
    """Compare median measured milliseconds; negative change means faster."""
    if not math.isfinite(regression_threshold_percent) or regression_threshold_percent < 0:
        raise ValueError("regression threshold must be finite and non-negative")
    if baseline.get("schema") != SCHEMA or candidate.get("schema") != SCHEMA:
        raise ValueError(f"both inputs must use {SCHEMA}")
    comparisons: dict[str, Any] = {}
    for metric in METRICS:
        before = _measured_values(baseline, metric)
        after = _measured_values(candidate, metric)
        if not before or not after:
            continue
        baseline_median = statistics.median(before)
        candidate_median = statistics.median(after)
        if baseline_median == 0:
            change = 0.0 if candidate_median == 0 else None
            status = "not_comparable_zero_baseline" if change is None else "unchanged"
        else:
            change = (candidate_median - baseline_median) / baseline_median * 100.0
            status = (
                "regression" if change > regression_threshold_percent
                else "improvement" if change < -regression_threshold_percent
                else "within_threshold"
            )
        comparisons[metric] = {
            "unit": "ms",
            "baseline_median": baseline_median,
            "candidate_median": candidate_median,
            "change_percent": change,
            "status": status,
            "baseline_samples": len(before),
            "candidate_samples": len(after),
        }
    if not comparisons:
        raise ValueError("reports contain no comparable measured benchmark metrics")
    return {
        "schema": "pytyliki.benchmark-comparison.v1",
        "method": "median of non-warmup runs",
        "regression_threshold_percent": regression_threshold_percent,
        "baseline_repeat": baseline.get("repeat"),
        "candidate_repeat": candidate.get("repeat"),
        "metrics": comparisons,
        "semantics": (
            "Lower elapsed time is better. Compare runs on the same hardware, "
            "power policy, operating system and workload settings. This report "
            "does not normalize across different machines."
        ),
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Compare two Pytyliki --suite --json reports.")
    parser.add_argument("baseline", help="baseline JSON report")
    parser.add_argument("candidate", help="candidate JSON report")
    parser.add_argument("--threshold", type=float, default=5.0,
                        help="percentage change required to flag improvement/regression (default: 5)")
    parser.add_argument("--json", action="store_true", help="print the complete comparison as JSON")
    args = parser.parse_args(argv)
    try:
        result = compare_suites(_load_suite(args.baseline), _load_suite(args.candidate), args.threshold)
    except (ValueError, TypeError) as error:
        print(f"pytyliki-compare: {error}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(result, indent=2, ensure_ascii=False, allow_nan=False))
        return 0
    print("Pytyliki benchmark comparison — median measured time")
    for name, item in result["metrics"].items():
        change = item["change_percent"]
        change_text = "n/a" if change is None else f"{change:+.2f}%"
        print(f"{name:12} {item['baseline_median']:10.3f} -> {item['candidate_median']:10.3f} ms"
              f"   {change_text:>9}   {item['status']}")
    print("Use identical hardware and workload settings for meaningful comparisons.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
