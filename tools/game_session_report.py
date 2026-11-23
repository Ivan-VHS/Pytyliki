"""Bounded analysis of whole-host telemetry around legacy game sessions.

The report consumes snapshots produced by Pytyliki Watch. It deliberately
labels the measurements as system-wide; it does not pretend to attribute
CPU, RAM, interrupts or page-file pressure to one game process.
"""
from __future__ import annotations

import argparse
import json
import math
import statistics
import sys
from pathlib import Path
from typing import Any

SCHEMA = "pytyliki.game-session-report.v1"
MAX_SAMPLES = 10000
METRICS = ("cpu_percent", "memory_percent", "swap_percent")

def _number(value: Any, name: str, *, optional: bool = False) -> float | None:
    if value is None and optional:
        return None
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{name} must be a finite number")
    number = float(value)
    if not math.isfinite(number) or number < 0 or number > 100:
        raise ValueError(f"{name} must be between 0 and 100")
    return number

def _percentile(values: list[float], percentile: float) -> float | None:
    if not values:
        return None
    ordered = sorted(values)
    index = max(0, math.ceil(percentile * len(ordered)) - 1)
    return round(ordered[index], 2)

def build_game_session_report(
    game_id: str,
    samples: list[dict[str, Any]],
    thresholds: dict[str, float] | None = None,
) -> dict[str, Any]:
    if not isinstance(game_id, str) or not game_id.strip() or len(game_id) > 128:
        raise ValueError("game_id must be a non-empty string of at most 128 characters")
    if not isinstance(samples, list) or not samples or len(samples) > MAX_SAMPLES:
        raise ValueError(f"samples must contain between 1 and {MAX_SAMPLES} records")
    policy = {"cpu_percent": 90.0, "memory_percent": 90.0, "swap_percent": 30.0}
    if thresholds is not None:
        if not isinstance(thresholds, dict):
            raise ValueError("thresholds must be an object")
        for metric, value in thresholds.items():
            if metric not in METRICS:
                raise ValueError(f"unsupported threshold metric: {metric}")
            policy[metric] = _number(value, f"thresholds.{metric}") or 0.0

    normalized: list[dict[str, Any]] = []
    for index, sample in enumerate(samples):
        if not isinstance(sample, dict):
            raise ValueError(f"samples[{index}] must be an object")
        row: dict[str, Any] = {}
        for metric in METRICS:
            row[metric] = _number(sample.get(metric), f"samples[{index}].{metric}", optional=True)
        timestamp = sample.get("timestamp")
        if timestamp is not None:
            if isinstance(timestamp, bool) or not isinstance(timestamp, (int, float)) or not math.isfinite(float(timestamp)):
                raise ValueError(f"samples[{index}].timestamp must be finite")
            row["timestamp"] = float(timestamp)
        available = sample.get("memory_available_bytes")
        total = sample.get("memory_total_bytes")
        for field, value in (("memory_available_bytes", available), ("memory_total_bytes", total)):
            if value is not None and (isinstance(value, bool) or not isinstance(value, int) or value < 0):
                raise ValueError(f"samples[{index}].{field} must be a non-negative integer")
        if available is not None and total is not None and available > total:
            raise ValueError(f"samples[{index}] available memory cannot exceed total memory")
        row["memory_available_bytes"] = available
        row["memory_total_bytes"] = total
        normalized.append(row)

    metrics: dict[str, Any] = {}
    alerts: list[dict[str, Any]] = []
    for metric in METRICS:
        values = [row[metric] for row in normalized if row[metric] is not None]
        metrics[metric] = {
            "sample_count": len(values),
            "minimum": round(min(values), 2) if values else None,
            "median": round(statistics.median(values), 2) if values else None,
            "p95": _percentile(values, 0.95),
            "maximum": round(max(values), 2) if values else None,
            "threshold_percent": policy[metric],
        }
        exceeded = sum(value >= policy[metric] for value in values)
        if exceeded:
            alerts.append({
                "metric": metric,
                "threshold_percent": policy[metric],
                "samples_over_threshold": exceeded,
                "severity": "warning" if exceeded / max(1, len(values)) >= 0.25 else "info",
            })

    timestamps = [row["timestamp"] for row in normalized if "timestamp" in row]
    memory_values = [row["memory_available_bytes"] for row in normalized if row["memory_available_bytes"] is not None]
    return {
        "schema": SCHEMA,
        "game_id": game_id.strip(),
        "sample_count": len(normalized),
        "duration_seconds": round(max(timestamps) - min(timestamps), 3) if len(timestamps) > 1 else None,
        "metrics": metrics,
        "minimum_available_memory_bytes": min(memory_values) if memory_values else None,
        "alerts": alerts,
        "alert_count": len(alerts),
        "source": "pytyliki_watch_system_counters",
        "scope": "whole_host",
        "per_process_attribution": False,
        "execution_performed": False,
        "host_settings_changed": False,
    }

def analyze_file(path: str | Path, game_id: str, thresholds: dict[str, float] | None = None) -> dict[str, Any]:
    source = Path(path).expanduser()
    if source.is_symlink() or not source.is_file():
        raise ValueError("samples path must be a regular, non-symlink file")
    if source.stat().st_size > 8 * 1024 * 1024:
        raise ValueError("samples file exceeds the 8 MiB input limit")
    raw = source.read_text(encoding="utf-8")
    try:
        payload = json.loads(raw)
    except json.JSONDecodeError:
        # Pytyliki Watch's jsonl output is one independent sample per line.
        payload = []
        for line_number, line in enumerate(raw.splitlines(), start=1):
            if not line.strip():
                continue
            try:
                payload.append(json.loads(line))
            except json.JSONDecodeError as error:
                raise ValueError(f"invalid JSONL record on line {line_number}: {error.msg}") from error
    if isinstance(payload, dict):
        payload = payload.get("samples")
    return build_game_session_report(game_id, payload, thresholds)

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Analyze Pytyliki Watch telemetry around a game session.")
    parser.add_argument("samples", type=Path, help="JSON array, object with a samples array, or Pytyliki Watch JSONL file")
    parser.add_argument("--game-id", required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    try:
        report = analyze_file(args.samples, args.game_id)
        encoded = json.dumps(report, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
        if args.output:
            if args.output.resolve() == args.samples.resolve():
                raise ValueError("report output must not overwrite the samples input")
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(encoded, encoding="utf-8", newline="\n")
        else:
            print(encoded, end="")
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"pytyliki-game-session: {error}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())
