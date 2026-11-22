#!/usr/bin/env python3
"""Archive and compare Pytyliki benchmark reports without changing the measured host."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import sys
import tempfile
from typing import Any

REPORT_SCHEMA = "pytyliki.report.v1"
SUITE_SCHEMA = "pytyliki.suite.v1"
ARCHIVE_SCHEMA = "pytyliki.benchmark-archive.v1"
MAX_REPORT_BYTES = 16 * 1024 * 1024
METRICS = ("integer_ms", "memory_ms", "matrix_ms")


class ReportError(ValueError):
    """Raised when a benchmark report is invalid or incomplete."""


def _read_json(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        size = path.stat().st_size
        if size > MAX_REPORT_BYTES:
            raise ReportError(f"{path} exceeds the {MAX_REPORT_BYTES}-byte report limit")
        raw = path.read_bytes()
    except OSError as error:
        raise ReportError(f"cannot read {path}: {error}") from error
    try:
        value = json.loads(raw.decode("utf-8-sig"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise ReportError(f"{path} is not valid UTF-8 JSON: {error}") from error
    if not isinstance(value, dict):
        raise ReportError(f"{path} must contain a JSON object")
    return value, raw


def _finite_positive(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ReportError(f"{label} must be a number")
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise ReportError(f"{label} must be finite and greater than zero")
    return number


def _unwrap_report(value: dict[str, Any], label: str) -> dict[str, Any]:
    if value.get("schema") == ARCHIVE_SCHEMA:
        nested = value.get("report")
        if not isinstance(nested, dict):
            raise ReportError(f"{label} has no embedded report")
        value = nested
    if value.get("schema") != REPORT_SCHEMA:
        raise ReportError(f"{label} must use schema {REPORT_SCHEMA!r}")

    suite = value.get("suite")
    if not isinstance(suite, dict) or suite.get("schema") != SUITE_SCHEMA:
        raise ReportError(f"{label} does not contain a {SUITE_SCHEMA!r} suite")
    measured = suite.get("measured_runs")
    runs = suite.get("runs")
    if isinstance(measured, bool) or not isinstance(measured, int) or measured < 1:
        raise ReportError(f"{label} must contain at least one measured run")
    if not isinstance(runs, list) or len(runs) < measured:
        raise ReportError(f"{label} contains an incomplete run list")
    if sum(1 for run in runs if isinstance(run, dict) and run.get("warmup") is False) != measured:
        raise ReportError(f"{label} measured_runs does not match the run records")

    statistics = suite.get("statistics")
    if not isinstance(statistics, dict):
        raise ReportError(f"{label} has no benchmark statistics")
    for metric in METRICS:
        group = statistics.get(metric)
        if not isinstance(group, dict):
            raise ReportError(f"{label} is missing statistics for {metric}")
        _finite_positive(group.get("mean"), f"{label} {metric}.mean")
        _finite_positive(group.get("median"), f"{label} {metric}.median")
    return value


def load_report(path: str | Path) -> tuple[dict[str, Any], bytes]:
    """Load either a raw Pytyliki report or a previously saved archive."""
    source = Path(path).expanduser()
    value, raw = _read_json(source)
    return _unwrap_report(value, str(source)), raw


def _write_json(path: Path, value: dict[str, Any], overwrite: bool) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = (json.dumps(value, ensure_ascii=False, indent=2, sort_keys=True) + "\n").encode("utf-8")
    if overwrite:
        temporary_name = None
        try:
            with tempfile.NamedTemporaryFile("wb", dir=path.parent, prefix=f".{path.name}.",
                                             suffix=".tmp", delete=False) as stream:
                temporary_name = stream.name
                stream.write(payload)
                stream.flush()
                os.fsync(stream.fileno())
            os.replace(temporary_name, path)
        except OSError as error:
            if temporary_name:
                try:
                    os.unlink(temporary_name)
                except OSError:
                    pass
            raise ReportError(f"cannot write {path}: {error}") from error
        return

    try:
        with path.open("xb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
    except FileExistsError as error:
        raise ReportError(f"{path} already exists; pass --overwrite to replace it") from error
    except OSError as error:
        raise ReportError(f"cannot write {path}: {error}") from error


def save_archive(input_path: str | Path, output_path: str | Path, label: str = "",
                 overwrite: bool = False) -> dict[str, Any]:
    report, raw = load_report(input_path)
    label = label.strip()
    if len(label) > 80:
        raise ReportError("archive label must be 80 characters or fewer")
    archive = {
        "schema": ARCHIVE_SCHEMA,
        "created_utc": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "label": label,
        "source_sha256": hashlib.sha256(raw).hexdigest(),
        "report": report,
    }
    _write_json(Path(output_path).expanduser(), archive, overwrite)
    return archive


def compare_reports(baseline_path: str | Path, candidate_path: str | Path) -> dict[str, Any]:
    baseline, _ = load_report(baseline_path)
    candidate, _ = load_report(candidate_path)
    base_profile = baseline.get("profile") if isinstance(baseline.get("profile"), dict) else {}
    next_profile = candidate.get("profile") if isinstance(candidate.get("profile"), dict) else {}

    def identity(profile: dict[str, Any]) -> tuple[str, str, str]:
        return tuple(str(profile.get(key, "")).strip().casefold()
                     for key in ("architecture", "vendor", "cpu"))

    same_host = bool(identity(base_profile) == identity(next_profile) and any(identity(base_profile)))
    base_suite = baseline["suite"]
    next_suite = candidate["suite"]
    results = {}
    for metric in METRICS:
        before = _finite_positive(base_suite["statistics"][metric]["mean"], f"baseline {metric}")
        after = _finite_positive(next_suite["statistics"][metric]["mean"], f"candidate {metric}")
        delta = ((after - before) / before) * 100.0
        results[metric] = {
            "baseline_mean_ms": before,
            "candidate_mean_ms": after,
            "change_percent": round(delta, 3),
            "faster": after < before,
        }

    return {
        "schema": "pytyliki.benchmark-comparison.v1",
        "same_host_profile": same_host,
        "interpretation": (
            "Same reported CPU profile; still compare power mode, thermal state and background load."
            if same_host else
            "Host identity is missing or differs; treat absolute timing differences as cross-host observations."
        ),
        "lower_time_is_better": True,
        "metrics": results,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Archive and compare local Pytyliki benchmark reports.")
    commands = parser.add_subparsers(dest="command", required=True)

    save = commands.add_parser("save", help="save a validated report as an immutable archive")
    save.add_argument("input", type=Path, help="JSON produced by pytyliki --suite --json")
    save.add_argument("output", type=Path, help="destination archive JSON")
    save.add_argument("--label", default="", help="optional label (up to 80 characters)")
    save.add_argument("--overwrite", action="store_true", help="replace an existing archive")

    compare = commands.add_parser("compare", help="compare two raw reports or saved archives")
    compare.add_argument("baseline", type=Path)
    compare.add_argument("candidate", type=Path)

    args = parser.parse_args(argv)
    try:
        if args.command == "save":
            result = save_archive(args.input, args.output, args.label, args.overwrite)
            print(json.dumps({
                "schema": result["schema"],
                "output": str(args.output),
                "label": result["label"],
                "source_sha256": result["source_sha256"],
            }, ensure_ascii=False, indent=2))
        else:
            print(json.dumps(compare_reports(args.baseline, args.candidate),
                             ensure_ascii=False, indent=2))
    except ReportError as error:
        print(f"benchmark archive: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
