#!/usr/bin/env python3
# Copyright (c) 2026 Ivan-VHS
# SPDX-License-Identifier: MIT
"""Pytyliki Watch: a read-only host telemetry and trend-analysis workbench."""
from __future__ import annotations
import argparse, csv, ctypes, hashlib, json, os, platform, statistics, sys, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterable

DEFAULT_THRESHOLDS = {"cpu_percent": 90.0, "memory_percent": 90.0, "swap_percent": 30.0}

def _linux_cpu_counters(stat_text: str) -> tuple[int, int]:
    line = next((line for line in stat_text.splitlines() if line.startswith("cpu ")), None)
    if line is None:
        raise ValueError("/proc/stat does not contain the aggregate cpu row")
    values = [int(value) for value in line.split()[1:]]
    if len(values) < 4:
        raise ValueError("aggregate cpu row has too few counters")
    return sum(values), values[3] + (values[4] if len(values) > 4 else 0)

def _linux_memory(meminfo_text: str) -> tuple[int, int, int, int]:
    values: dict[str, int] = {}
    for line in meminfo_text.splitlines():
        fields = line.split()
        if len(fields) >= 2 and fields[0].endswith(":"):
            try:
                values[fields[0][:-1]] = int(fields[1]) * 1024
            except ValueError:
                continue
    total = values.get("MemTotal", 0)
    available = values.get("MemAvailable", values.get("MemFree", 0))
    swap_total, swap_free = values.get("SwapTotal", 0), values.get("SwapFree", 0)
    if total <= 0:
        raise ValueError("/proc/meminfo does not report a valid MemTotal")
    return total, min(max(available, 0), total), swap_total, min(max(swap_free, 0), swap_total)

def _linux_interrupts(stat_text: str) -> int | None:
    for line in stat_text.splitlines():
        if line.startswith("intr "):
            fields = line.split()
            try:
                return int(fields[1])
            except (IndexError, ValueError):
                return None
    return None

def _linux_load_average(loadavg_text: str) -> tuple[float, float, float] | None:
    try:
        values = tuple(float(value) for value in loadavg_text.split()[:3])
    except ValueError:
        return None
    return values if len(values) == 3 else None

def calculate_cpu_percent(previous: tuple[int, int] | None, current: tuple[int, int]) -> float | None:
    if previous is None:
        return None
    total_delta, idle_delta = current[0] - previous[0], current[1] - previous[1]
    if total_delta <= 0 or idle_delta < 0:
        return None
    return round(min(100.0, max(0, total_delta - idle_delta) * 100.0 / total_delta), 1)

def parse_linux_snapshot(stat_text: str, meminfo_text: str, process_count: int | None = None,
                         loadavg_text: str | None = None) -> dict[str, Any]:
    counters = _linux_cpu_counters(stat_text)
    total, available, swap_total, swap_free = _linux_memory(meminfo_text)
    return {
        "platform": "linux", "cpu_model": platform.processor() or platform.machine() or "unknown",
        "logical_processors": os.cpu_count() or 0, "cpu_counters": counters, "cpu_percent": None,
        "memory_total_bytes": total, "memory_available_bytes": available,
        "swap_total_bytes": swap_total, "swap_free_bytes": swap_free,
        "interrupts": _linux_interrupts(stat_text), "process_count": process_count,
        "load_average": _linux_load_average(loadavg_text or ""),
    }

def _read_linux_snapshot() -> dict[str, Any]:
    stat_text = Path("/proc/stat").read_text(encoding="ascii", errors="replace")
    meminfo_text = Path("/proc/meminfo").read_text(encoding="ascii", errors="replace")
    try:
        process_count = sum(name.isdigit() for name in os.listdir("/proc"))
    except OSError:
        process_count = None
    try:
        loadavg_text = Path("/proc/loadavg").read_text(encoding="ascii", errors="replace")
    except OSError:
        loadavg_text = ""
    return parse_linux_snapshot(stat_text, meminfo_text, process_count, loadavg_text)

def _windows_snapshot() -> dict[str, Any]:
    class FILETIME(ctypes.Structure):
        _fields_ = [("low", ctypes.c_uint32), ("high", ctypes.c_uint32)]
        def as_int(self) -> int:
            return (self.high << 32) | self.low
    class MEMORYSTATUSEX(ctypes.Structure):
        _fields_ = [
            ("dwLength", ctypes.c_uint32), ("dwMemoryLoad", ctypes.c_uint32),
            ("ullTotalPhys", ctypes.c_uint64), ("ullAvailPhys", ctypes.c_uint64),
            ("ullTotalPageFile", ctypes.c_uint64), ("ullAvailPageFile", ctypes.c_uint64),
            ("ullTotalVirtual", ctypes.c_uint64), ("ullAvailVirtual", ctypes.c_uint64),
            ("ullAvailExtendedVirtual", ctypes.c_uint64),
        ]
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    idle, kernel, user = FILETIME(), FILETIME(), FILETIME()
    if not kernel32.GetSystemTimes(ctypes.byref(idle), ctypes.byref(kernel), ctypes.byref(user)):
        raise ctypes.WinError(ctypes.get_last_error())
    memory = MEMORYSTATUSEX()
    memory.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
    if not kernel32.GlobalMemoryStatusEx(ctypes.byref(memory)):
        raise ctypes.WinError(ctypes.get_last_error())
    page_total = int(memory.ullTotalPageFile)
    swap_total = max(0, page_total - int(memory.ullTotalPhys))
    page_free = max(0, int(memory.ullAvailPageFile) - int(memory.ullAvailPhys))
    return {
        "platform": "windows", "cpu_model": platform.processor() or platform.machine() or "unknown",
        "logical_processors": os.cpu_count() or 0,
        "cpu_counters": (kernel.as_int() + user.as_int(), idle.as_int()), "cpu_percent": None,
        "memory_total_bytes": int(memory.ullTotalPhys), "memory_available_bytes": int(memory.ullAvailPhys),
        "swap_total_bytes": swap_total, "swap_free_bytes": min(page_free, swap_total),
        "interrupts": None, "process_count": None, "load_average": None,
    }

def host_profile() -> dict[str, Any]:
    identity = "|".join((platform.system(), platform.release(), platform.machine(),
                        platform.processor(), str(os.cpu_count() or 0)))
    return {
        "platform": platform.system().lower(), "platform_release": platform.release(),
        "architecture": platform.machine(), "cpu_model": platform.processor() or platform.machine() or "unknown",
        "logical_processors": os.cpu_count() or 0,
        "profile_id": hashlib.sha256(identity.encode("utf-8", errors="replace")).hexdigest()[:16],
    }

def collect_snapshot(previous: dict[str, Any] | None = None) -> dict[str, Any]:
    if sys.platform.startswith("linux"):
        sample = _read_linux_snapshot()
    elif sys.platform == "win32":
        sample = _windows_snapshot()
    else:
        raise OSError(f"read-only host counters are not implemented for {sys.platform!r}")
    old = previous.get("cpu_counters") if previous else None
    sample["cpu_percent"] = calculate_cpu_percent(old, sample["cpu_counters"])
    sample["timestamp"] = round(time.time(), 3)
    sample["memory_used_bytes"] = sample["memory_total_bytes"] - sample["memory_available_bytes"]
    sample["memory_percent"] = round(sample["memory_used_bytes"] * 100.0 / sample["memory_total_bytes"], 1)
    sample["swap_used_bytes"] = sample["swap_total_bytes"] - sample["swap_free_bytes"]
    sample["swap_percent"] = round(sample["swap_used_bytes"] * 100.0 / sample["swap_total_bytes"], 1) if sample["swap_total_bytes"] else 0.0
    return sample

def _percentile(values: list[float], fraction: float) -> float | None:
    if not values:
        return None
    ordered = sorted(values)
    index = min(len(ordered) - 1, max(0, round((len(ordered) - 1) * fraction)))
    return round(ordered[index], 2)

def analyze_samples(samples: Iterable[dict[str, Any]], thresholds: dict[str, float] | None = None) -> dict[str, Any]:
    limits = dict(DEFAULT_THRESHOLDS)
    if thresholds:
        limits.update(thresholds)
    rows = list(samples)
    metrics: dict[str, Any] = {}
    alerts: list[dict[str, Any]] = []
    for metric, limit in limits.items():
        values = [float(row[metric]) for row in rows if row.get(metric) is not None]
        metrics[metric] = {
            "samples": len(values), "minimum": round(min(values), 2) if values else None,
            "average": round(statistics.fmean(values), 2) if values else None,
            "maximum": round(max(values), 2) if values else None, "p95": _percentile(values, 0.95),
            "threshold": limit,
        }
        for row in rows:
            value = row.get(metric)
            if value is not None and float(value) >= limit:
                alerts.append({"timestamp": row.get("timestamp"), "metric": metric, "value": value,
                               "threshold": limit, "severity": "warning"})
    return {"schema": "pytyliki.telemetry-analysis.v1", "sample_count": len(rows),
            "metrics": metrics, "alerts": alerts, "alert_count": len(alerts), "thresholds": limits}

def _display_row(sample: dict[str, Any]) -> dict[str, Any]:
    return {key: value for key, value in sample.items() if key != "cpu_counters"}

def _write_rows(path: Path, rows: list[dict[str, Any]], output_format: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if output_format == "jsonl":
        path.write_text("".join(json.dumps(row, ensure_ascii=False, sort_keys=True) + "\n" for row in rows), encoding="utf-8")
    else:
        with path.open("w", encoding="utf-8", newline="") as stream:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)

def run(interval: float, count: int, output_format: str, output_path: str | None,
        report_path: str | None = None, thresholds: dict[str, float] | None = None) -> int:
    previous: dict[str, Any] | None = None
    rows: list[dict[str, Any]] = []
    for _ in range(count):
        sample = collect_snapshot(previous)
        previous = sample
        row = _display_row(sample)
        rows.append(row)
        if output_format == "jsonl":
            print(json.dumps(row, ensure_ascii=False, sort_keys=True))
        elif output_format == "csv":
            if len(rows) == 1:
                writer = csv.DictWriter(sys.stdout, fieldnames=list(row))
                writer.writeheader()
            writer.writerow(row)
        else:
            cpu = "n/a" if row["cpu_percent"] is None else f'{row["cpu_percent"]:.1f}%'
            process_count = row["process_count"] if row["process_count"] is not None else "n/a"
            print(f'{datetime.fromtimestamp(row["timestamp"], timezone.utc).isoformat()}  '
                  f'CPU {cpu:>6}  RAM {row["memory_percent"]:5.1f}%  '
                  f'SWAP {row["swap_percent"]:5.1f}%  PROCS {process_count}')
        if len(rows) < count:
            time.sleep(interval)
    if output_path:
        _write_rows(Path(output_path), rows, output_format)
    analysis = analyze_samples(rows, thresholds)
    report = {
        **analysis, "generated_at": datetime.now(timezone.utc).isoformat(),
        "source": "local_read_only_counters", "host": host_profile(),
        "sample_interval_seconds": interval,
        "limitations": [
            "CPU utilization requires at least two samples.",
            "Hardware sensors, per-process resource attribution and fan controls are outside this monitor.",
            "Windows interrupt totals and process count are not collected by this version.",
        ],
    }
    if report_path:
        target = Path(report_path)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if analysis["alert_count"]:
        print(f'Pytyliki analysis: {analysis["alert_count"]} threshold warning(s); report schema {analysis["schema"]}', file=sys.stderr)
    return 0

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Read-only CPU, memory, swap and host telemetry workbench")
    parser.add_argument("--interval", type=float, default=1.0, help="seconds between samples (0.1–60)")
    parser.add_argument("--count", type=int, default=2, help="number of samples (1–10000)")
    parser.add_argument("--format", choices=("jsonl", "csv", "table"), default="table")
    parser.add_argument("--output", help="save raw samples to this file")
    parser.add_argument("--report", help="save an aggregate JSON analysis report")
    parser.add_argument("--cpu-limit", type=float, default=90.0, help="CPU warning threshold, percent")
    parser.add_argument("--memory-limit", type=float, default=90.0, help="memory warning threshold, percent")
    parser.add_argument("--swap-limit", type=float, default=30.0, help="swap warning threshold, percent")
    args = parser.parse_args(argv)
    if not 0.1 <= args.interval <= 60:
        parser.error("--interval must be between 0.1 and 60 seconds")
    if not 1 <= args.count <= 10000:
        parser.error("--count must be between 1 and 10000")
    thresholds = {"cpu_percent": args.cpu_limit, "memory_percent": args.memory_limit, "swap_percent": args.swap_limit}
    if any(not 0 <= value <= 100 for value in thresholds.values()):
        parser.error("all thresholds must be between 0 and 100 percent")
    try:
        return run(args.interval, args.count, args.format, args.output, args.report, thresholds)
    except (OSError, ValueError) as exc:
        print(f"pytyliki-watch: {exc}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())
