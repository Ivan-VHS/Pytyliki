#!/usr/bin/env python3
"""Compare two Pytyliki repeated-suite reports using robust statistics."""
from __future__ import annotations

import argparse
import json
import math
import statistics
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

METRICS = ("integer_ms", "memory_ms", "matrix_ms")
PROFILE_FIELDS = ("architecture", "cpu", "vendor", "logical_processors")
WORKLOAD_FIELDS = ("iterations", "memory_mib", "passes")


def _is_positive_integer(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool) and value > 0


def _read_report(path: Path) -> Dict[str, Any]:
    try:
        report = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"Cannot read report '{path}': {exc}") from exc
    if not isinstance(report, dict):
        raise ValueError(f"Report '{path}' must contain a JSON object")
    if report.get("schema") != "pytyliki.report.v1":
        raise ValueError(f"Report '{path}' is not pytyliki.report.v1")
    return report


def _validated_run_data(report: Dict[str, Any], label: str) -> Dict[str, Any]:
    profile = report.get("profile")
    if not isinstance(profile, dict):
        raise ValueError(f"{label}: processor profile is missing; capture with --profile")

    identity: Dict[str, Any] = {}
    for field in PROFILE_FIELDS:
        value = profile.get(field)
        if field == "logical_processors":
            if not _is_positive_integer(value):
                raise ValueError(f"{label}: profile.{field} must be a positive integer")
        elif not isinstance(value, str) or not value.strip():
            raise ValueError(f"{label}: profile.{field} must be a non-empty string")
        identity[field] = value

    suite = report.get("suite")
    if not isinstance(suite, dict) or suite.get("schema") != "pytyliki.suite.v1":
        raise ValueError(f"{label}: suite payload is missing or has an unsupported schema")

    repeat = suite.get("repeat")
    measured_runs = suite.get("measured_runs")
    runs = suite.get("runs")
    if not _is_positive_integer(repeat) or not isinstance(runs, list) or len(runs) != repeat:
        raise ValueError(f"{label}: suite.repeat must match the number of run records")
    if not _is_positive_integer(measured_runs):
        raise ValueError(f"{label}: suite.measured_runs must be a positive integer")

    samples = {metric: [] for metric in METRICS}
    workload: Optional[Dict[str, int]] = None
    actual_measured = 0

    for index, run in enumerate(runs, start=1):
        if not isinstance(run, dict) or not isinstance(run.get("warmup"), bool):
            raise ValueError(f"{label}: run {index} is malformed")
        if run["warmup"]:
            continue
        actual_measured += 1
        if run.get("integrity_ok") is not True:
            raise ValueError(f"{label}: run {index} failed its integrity check")

        current_workload: dict[str, int] = {}
        for field in WORKLOAD_FIELDS:
            value = run.get(field)
            if not _is_positive_integer(value):
                raise ValueError(f"{label}: run {index} has invalid {field}")
            current_workload[field] = value
        if workload is None:
            workload = current_workload
        elif workload != current_workload:
            raise ValueError(f"{label}: workload settings vary between measured runs")

        for metric in METRICS:
            value = run.get(metric)
            if isinstance(value, bool) or not isinstance(value, (int, float)):
                raise ValueError(f"{label}: run {index} has invalid {metric}")
            number = float(value)
            if not math.isfinite(number) or number <= 0.0:
                raise ValueError(f"{label}: run {index} has non-positive or non-finite {metric}")
            samples[metric].append(number)

    if actual_measured != measured_runs:
        raise ValueError(f"{label}: measured_runs does not match the non-warm-up records")
    if actual_measured < 2:
        raise ValueError(f"{label}: at least two measured runs are required for comparison")
    if workload is None:
        raise ValueError(f"{label}: no measured workload was found")

    return {"profile": identity, "workload": workload, "samples": samples}


def _median_absolute_deviation(values: List[float], center: float) -> float:
    return statistics.median(abs(value - center) for value in values)


def compare_reports(
    baseline: Dict[str, Any], candidate: Dict[str, Any], noise_percent: float = 1.0,
) -> Dict[str, Any]:
    """Compare reports; return an explicit not_comparable result on context mismatch."""
    if isinstance(noise_percent, bool) or not isinstance(noise_percent, (int, float)):
        raise ValueError("noise_percent must be a number")
    noise_percent = float(noise_percent)
    if not math.isfinite(noise_percent) or not 0.0 <= noise_percent <= 100.0:
        raise ValueError("noise_percent must be between 0 and 100")

    before = _validated_run_data(baseline, "baseline")
    after = _validated_run_data(candidate, "candidate")
    reasons = []
    if before["profile"] != after["profile"]:
        reasons.append("processor identity differs (architecture, CPU, vendor or logical processor count)")
    if before["workload"] != after["workload"]:
        reasons.append("benchmark workload settings differ")

    result: Dict[str, Any] = {
        "schema": "pytyliki.suite-comparison.v1",
        "comparable": not reasons,
        "outcome": "not_comparable" if reasons else "within_noise",
        "reasons": reasons,
        "noise_percent": noise_percent,
        "profile": before["profile"],
        "workload": before["workload"],
        "metrics": {},
        "method": (
            "Median and median absolute deviation (MAD) over measured runs; warm-up runs are excluded. "
            "The noise band is max(noise_percent of the baseline median, 3 times the sum of both MAD values)."
        ),
        "timing_note": "Lower time is better. Results are host-dependent and are not a guaranteed performance claim.",
    }
    if reasons:
        return result

    metric_statuses = []
    for metric in METRICS:
        base_values = before["samples"][metric]
        candidate_values = after["samples"][metric]
        base_median = statistics.median(base_values)
        candidate_median = statistics.median(candidate_values)
        base_mad = _median_absolute_deviation(base_values, base_median)
        candidate_mad = _median_absolute_deviation(candidate_values, candidate_median)
        delta_ms = candidate_median - base_median
        delta_percent = (delta_ms / base_median) * 100.0
        noise_band = max(base_median * noise_percent / 100.0, 3.0 * (base_mad + candidate_mad))

        if abs(delta_ms) <= noise_band:
            status = "within_noise"
        elif delta_ms < 0.0:
            status = "improved"
        else:
            status = "regressed"
        metric_statuses.append(status)
        result["metrics"][metric] = {
            "baseline": {"median_ms": round(base_median, 6), "mad_ms": round(base_mad, 6), "samples": len(base_values)},
            "candidate": {"median_ms": round(candidate_median, 6), "mad_ms": round(candidate_mad, 6), "samples": len(candidate_values)},
            "delta_ms": round(delta_ms, 6),
            "delta_percent": round(delta_percent, 4),
            "noise_band_ms": round(noise_band, 6),
            "status": status,
        }

    if "regressed" in metric_statuses:
        result["outcome"] = "regression"
    elif "improved" in metric_statuses:
        result["outcome"] = "improvement"
    else:
        result["outcome"] = "within_noise"
    return result


def format_summary(result: Dict[str, Any]) -> str:
    lines = [f"Pytyliki suite comparison: {result['outcome']}"]
    if not result["comparable"]:
        lines.extend(f"- Not comparable: {reason}" for reason in result["reasons"])
        return "\n".join(lines)
    for metric, values in result["metrics"].items():
        delta = values["delta_percent"]
        lines.append(
            f"- {metric}: {values['baseline']['median_ms']:.3f} → "
            f"{values['candidate']['median_ms']:.3f} ms ({delta:+.2f}%, {values['status']}; "
            f"MAD {values['baseline']['mad_ms']:.3f}/{values['candidate']['mad_ms']:.3f} ms)"
        )
    lines.append(result["timing_note"])
    return "\n".join(lines)


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path, help="baseline pytyliki.report.v1 JSON file")
    parser.add_argument("candidate", type=Path, help="candidate pytyliki.report.v1 JSON file")
    parser.add_argument("--noise-percent", type=float, default=1.0, help="minimum noise band as a percentage of baseline median (default: 1.0)")
    parser.add_argument("--json", dest="json_path", type=Path, help="write the machine-readable comparison report to this file")
    args = parser.parse_args(argv)

    try:
        baseline = _read_report(args.baseline)
        candidate = _read_report(args.candidate)
        result = compare_reports(baseline, candidate, args.noise_percent)
    except (OSError, ValueError) as exc:
        print(f"compare-suite-runs: {exc}", file=sys.stderr)
        return 2

    summary = format_summary(result)
    if args.json_path:
        try:
            args.json_path.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
        except OSError as exc:
            print(f"compare-suite-runs: cannot write '{args.json_path}': {exc}", file=sys.stderr)
            return 2
        print(f"Comparison report written: {args.json_path}")
    print(summary)
    return 0 if result["comparable"] else 3


if __name__ == "__main__":
    raise SystemExit(main())
