"""Evidence-based gaming-session analysis using Pytyliki Watch samples."""
from __future__ import annotations

import math
from typing import Any, Iterable

SCHEMA = "pytyliki.gaming-performance-assessment.v1"
MAX_SAMPLES = 10000
MAX_FRAME_SAMPLES = 2_000_000
METRICS = ("cpu_percent", "memory_percent", "swap_percent")
DEFAULT_LIMITS = {"cpu_percent": 90.0, "memory_percent": 90.0, "swap_percent": 30.0}


class GamingAssessmentError(ValueError):
    """Raised when telemetry is invalid, incomplete or outside safety bounds."""


def _finite_percent(value: Any, field: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise GamingAssessmentError(f"{field} must be a finite percentage")
    number = float(value)
    if not math.isfinite(number) or not 0.0 <= number <= 100.0:
        raise GamingAssessmentError(f"{field} must be between 0 and 100")
    return number


def _finite_frame_time(value: Any, index: int) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise GamingAssessmentError(f"frame_times_ms[{index}] must be a finite number")
    number = float(value)
    if not math.isfinite(number) or not 0.05 <= number <= 10000.0:
        raise GamingAssessmentError(f"frame_times_ms[{index}] must be between 0.05 and 10000 ms")
    return number


def _nearest_rank(values: list[float], fraction: float) -> float:
    ordered = sorted(values)
    index = max(0, math.ceil(fraction * len(ordered)) - 1)
    return ordered[index]


def _metric_summary(values: list[float], limit: float) -> dict[str, Any]:
    if not values:
        return {
            "sample_count": 0, "minimum": None, "median": None,
            "p95": None, "maximum": None, "threshold_percent": limit,
        }
    ordered = sorted(values)
    middle = len(ordered) // 2
    median = ordered[middle] if len(ordered) % 2 else (ordered[middle - 1] + ordered[middle]) / 2
    return {
        "sample_count": len(values),
        "minimum": round(ordered[0], 2),
        "median": round(median, 2),
        "p95": round(_nearest_rank(ordered, 0.95), 2),
        "maximum": round(ordered[-1], 2),
        "threshold_percent": limit,
    }


def _frame_time_summary(samples: list[Any] | None) -> dict[str, Any] | None:
    if samples is None:
        return None
    if not isinstance(samples, list) or not samples or len(samples) > MAX_FRAME_SAMPLES:
        raise GamingAssessmentError(
            f"frame_times_ms must contain 1..{MAX_FRAME_SAMPLES} measurements"
        )
    values = [_finite_frame_time(value, index) for index, value in enumerate(samples)]
    ordered = sorted(values)
    slowest_one = ordered[-max(1, math.ceil(len(ordered) * 0.01)):]
    slowest_point_one = (
        ordered[-max(1, math.ceil(len(ordered) * 0.001)):]
        if len(ordered) >= 1000 else []
    )
    mean = math.fsum(values) / len(values)
    return {
        "sample_count": len(values),
        "mean_frametime_ms": round(mean, 4),
        "p95_frametime_ms": round(_nearest_rank(ordered, 0.95), 4),
        "p99_frametime_ms": round(_nearest_rank(ordered, 0.99), 4),
        "average_fps": round(1000.0 / mean, 2),
        "one_percent_low_fps": round(1000.0 / (math.fsum(slowest_one) / len(slowest_one)), 2),
        "point_one_percent_low_fps": (
            round(1000.0 / (math.fsum(slowest_point_one) / len(slowest_point_one)), 2)
            if slowest_point_one else None
        ),
        "point_one_percent_low_available": bool(slowest_point_one),
        "definition": "Low FPS is the inverse mean frametime of the slowest frame percentile.",
    }


def assess_game_session(
    samples: Iterable[dict[str, Any]],
    *,
    game_id: str,
    frame_times_ms: list[Any] | None = None,
    thresholds: dict[str, Any] | None = None,
) -> dict[str, Any]:
    """Summarize actual whole-host samples and optional measured frame times."""
    if not isinstance(game_id, str) or not game_id.strip() or len(game_id) > 128 or "\x00" in game_id:
        raise GamingAssessmentError("game_id must be non-empty text of at most 128 characters")
    rows = list(samples)
    if not rows or len(rows) > MAX_SAMPLES:
        raise GamingAssessmentError(f"samples must contain 1..{MAX_SAMPLES} records")
    limits = dict(DEFAULT_LIMITS)
    if thresholds is not None:
        if not isinstance(thresholds, dict) or set(thresholds) - set(METRICS):
            raise GamingAssessmentError("thresholds may only contain CPU, memory and swap percentages")
        for name, value in thresholds.items():
            limits[name] = _finite_percent(value, f"thresholds.{name}")

    columns: dict[str, list[float]] = {name: [] for name in METRICS}
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            raise GamingAssessmentError(f"samples[{index}] must be an object")
        for name in METRICS:
            value = row.get(name)
            if value is not None:
                columns[name].append(_finite_percent(value, f"samples[{index}].{name}"))

    metrics = {name: _metric_summary(columns[name], limits[name]) for name in METRICS}
    pressure = []
    recommendations = []
    for name, title in (
        ("cpu_percent", "CPU load"),
        ("memory_percent", "physical-memory use"),
        ("swap_percent", "swap use"),
    ):
        summary = metrics[name]
        p95 = summary["p95"]
        if p95 is not None and p95 >= limits[name]:
            pressure.append({
                "metric": name,
                "p95": p95,
                "threshold": limits[name],
                "severity": "warning",
            })
            recommendations.append(f"Review {title}; keep measurements system-wide and identify the actual process before changing policy.")

    frame_summary = _frame_time_summary(frame_times_ms)
    if frame_summary is not None and frame_summary["sample_count"] < 1000:
        recommendations.append("Capture a longer repeatable run before interpreting 0.1% low FPS.")
    if frame_summary is None:
        recommendations.append("No frame-time capture was supplied; FPS and frame pacing remain unknown.")

    return {
        "schema": SCHEMA,
        "game_id": game_id.strip(),
        "sample_count": len(rows),
        "metrics": metrics,
        "frame_time": frame_summary,
        "pressure_events": pressure,
        "recommendations": recommendations,
        "source": "pytyliki_watch_measured_samples",
        "scope": "whole_host",
        "per_process_attribution": False,
        "hardware_tuning_performed": False,
        "host_settings_changed": False,
        "execution_performed": False,
    }


def to_duckos_capability_evidence(report: dict[str, Any]) -> dict[str, Any]:
    """Translate measured Pytyliki counters into DuckOS evidence, not control claims."""
    if not isinstance(report, dict) or report.get("schema") != SCHEMA:
        raise GamingAssessmentError(f"expected a {SCHEMA} report")
    if report.get("host_settings_changed") is not False or report.get("execution_performed") is not False:
        raise GamingAssessmentError("Pytyliki evidence must be read-only")
    metrics = report.get("metrics")
    if not isinstance(metrics, dict):
        raise GamingAssessmentError("report.metrics must be an object")

    def available(metric_name: str) -> bool:
        record = metrics.get(metric_name)
        if not isinstance(record, dict):
            raise GamingAssessmentError(f"report.metrics.{metric_name} must be an object")
        count = record.get("sample_count")
        if isinstance(count, bool) or not isinstance(count, int) or count < 0:
            raise GamingAssessmentError(f"report.metrics.{metric_name}.sample_count is invalid")
        return count > 0

    capabilities = {
        "cpu_telemetry": available("cpu_percent"),
        "memory_pressure": available("memory_percent"),
        "swap_metrics": available("swap_percent"),
        "frame_time": report.get("frame_time") is not None,
    }
    evidence = {
        name: {
            "available": present,
            "provider": "Pytyliki Watch",
            "version": "measured",
            "reason": None if present else "no measured samples were supplied",
        }
        for name, present in capabilities.items()
    }
    if capabilities["frame_time"]:
        frame_time = report["frame_time"]
        frame_count = frame_time.get("sample_count") if isinstance(frame_time, dict) else None
        if isinstance(frame_count, bool) or not isinstance(frame_count, int) or frame_count <= 0:
            raise GamingAssessmentError("report.frame_time must contain a positive measured sample count")
    return {
        "schema": "duckos.gaming-provider-evidence.v1",
        "source_schema": SCHEMA,
        "capabilities": evidence,
        "evidence_scope": "whole_host",
        "per_process_attribution": False,
        "system_mutations_performed": False,
    }
