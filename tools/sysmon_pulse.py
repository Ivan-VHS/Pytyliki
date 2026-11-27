"""Small Linux /proc monitor for CPU, memory, swap and interrupt activity.

The monitor is read-only and uses only the Python standard library.  It does
not need root privileges; inaccessible optional interrupt data is reported as
empty rather than guessed.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import sys
import time
from typing import Any

CPU_TICKS = tuple[int, int]
MEMORY_FIELDS = ("MemTotal", "MemAvailable", "SwapTotal", "SwapFree")


def parse_cpu_ticks(text: str) -> CPU_TICKS:
    """Return aggregate (total, idle) jiffies from the first /proc/stat cpu row."""
    for line in text.splitlines():
        fields = line.split()
        if fields and fields[0] == "cpu":
            try:
                values = [int(value) for value in fields[1:9]]
            except ValueError as error:
                raise ValueError("invalid numeric field in /proc/stat cpu row") from error
            if len(values) < 4 or any(value < 0 for value in values):
                raise ValueError("/proc/stat cpu row is incomplete or negative")
            total = sum(values)
            idle = values[3] + (values[4] if len(values) > 4 else 0)
            return total, idle
    raise ValueError("/proc/stat does not contain an aggregate cpu row")


def cpu_usage_percent(previous: CPU_TICKS, current: CPU_TICKS) -> float | None:
    total_delta = current[0] - previous[0]
    idle_delta = current[1] - previous[1]
    if total_delta <= 0:
        return None
    busy_delta = max(0, total_delta - max(0, idle_delta))
    return min(100.0, max(0.0, busy_delta * 100.0 / total_delta))


def parse_meminfo(text: str) -> dict[str, int]:
    """Read the byte-valued fields needed for RAM and swap pressure."""
    values: dict[str, int] = {}
    wanted = set(MEMORY_FIELDS)
    for line in text.splitlines():
        key, separator, rest = line.partition(":")
        if not separator or key not in wanted:
            continue
        parts = rest.split()
        if not parts:
            raise ValueError(f"missing value for {key} in /proc/meminfo")
        try:
            amount = int(parts[0])
        except ValueError as error:
            raise ValueError(f"invalid value for {key} in /proc/meminfo") from error
        if amount < 0:
            raise ValueError(f"negative value for {key} in /proc/meminfo")
        unit = parts[1].lower() if len(parts) > 1 else "b"
        multiplier = 1024 if unit == "kb" else 1
        if unit not in {"kb", "b"}:
            raise ValueError(f"unsupported unit for {key}: {unit}")
        values[key] = amount * multiplier
    missing = wanted.difference(values)
    if missing:
        raise ValueError("missing /proc/meminfo fields: " + ", ".join(sorted(missing)))
    if values["MemAvailable"] > values["MemTotal"] or values["SwapFree"] > values["SwapTotal"]:
        raise ValueError("/proc/meminfo free/available values exceed their totals")
    return values


def parse_interrupts(text: str) -> dict[str, int]:
    """Return interrupt totals by label, ignoring the non-numeric description."""
    result: dict[str, int] = {}
    for line in text.splitlines():
        if ":" not in line:
            continue
        label, remainder = line.split(":", 1)
        label = label.strip()
        if not label:
            continue
        counts: list[int] = []
        for field in remainder.split():
            try:
                counts.append(int(field))
            except ValueError:
                break
        if counts:
            result[label] = sum(counts)
    return result


def collect_snapshot(proc_root: Path | str = Path("/proc")) -> dict[str, Any]:
    root = Path(proc_root)
    cpu = parse_cpu_ticks((root / "stat").read_text(encoding="ascii"))
    memory = parse_meminfo((root / "meminfo").read_text(encoding="ascii"))
    try:
        interrupts = parse_interrupts((root / "interrupts").read_text(encoding="ascii"))
    except (OSError, UnicodeError):
        interrupts = {}
    return {
        "schema": "pytyliki.sysmon-pulse.v1",
        "cpu_ticks": list(cpu),
        "memory_bytes": memory,
        "interrupts": interrupts,
        "interrupt_total": sum(interrupts.values()),
    }


def _percent(used: int, total: int) -> float:
    return 0.0 if total <= 0 else min(100.0, max(0.0, used * 100.0 / total))


def _paint(value: str, percentage: float | None, color: bool) -> str:
    if not color or percentage is None:
        return value
    code = "32" if percentage < 65.0 else "33" if percentage < 85.0 else "31"
    return f"\033[{code}m{value}\033[0m"


def render_snapshot(
    snapshot: dict[str, Any],
    previous: dict[str, Any] | None = None,
    *,
    color: bool = False,
) -> str:
    memory = snapshot["memory_bytes"]
    ram_used = memory["MemTotal"] - memory["MemAvailable"]
    swap_used = memory["SwapTotal"] - memory["SwapFree"]
    cpu = None
    interrupt_delta = None
    if previous is not None:
        cpu = cpu_usage_percent(tuple(previous["cpu_ticks"]), tuple(snapshot["cpu_ticks"]))
        interrupt_delta = max(0, snapshot["interrupt_total"] - previous["interrupt_total"])

    cpu_text = "warming up" if cpu is None else f"{cpu:5.1f}%"
    ram_pct = _percent(ram_used, memory["MemTotal"])
    swap_pct = _percent(swap_used, memory["SwapTotal"])
    ram_text = f"{ram_used / (1024 ** 3):.2f}/{memory['MemTotal'] / (1024 ** 3):.2f} GiB ({ram_pct:.1f}%)"
    swap_text = f"{swap_used / (1024 ** 3):.2f}/{memory['SwapTotal'] / (1024 ** 3):.2f} GiB ({swap_pct:.1f}%)"
    irq_text = f"{snapshot['interrupt_total']:,}"
    if interrupt_delta is not None:
        irq_text += f" (+{interrupt_delta:,} since last sample)"

    rows = [
        "sysmon-pulse · read-only /proc monitor",
        f"CPU busy:      {_paint(cpu_text, cpu, color)}",
        f"RAM used:      {_paint(ram_text, ram_pct, color)}",
        f"Swap used:     {_paint(swap_text, swap_pct, color)}",
        f"Interrupts:    {irq_text}",
    ]
    top = sorted(snapshot["interrupts"].items(), key=lambda item: (-item[1], item[0]))[:5]
    if top:
        rows.append("Top IRQs:      " + "  ".join(f"{name}={count:,}" for name, count in top))
    else:
        rows.append("Top IRQs:      unavailable")
    return "\n".join(rows)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Monitor CPU, RAM, swap and Linux interrupts.")
    parser.add_argument("--interval", type=float, default=1.0, help="sample interval in seconds (default: 1)")
    parser.add_argument("--once", action="store_true", help="print one snapshot without CPU delta")
    parser.add_argument("--json", action="store_true", help="print the raw snapshot as JSON")
    parser.add_argument("--no-color", action="store_true", help="disable ANSI threshold colours")
    parser.add_argument("--proc-root", type=Path, default=Path("/proc"), help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    if args.interval < 0.1:
        parser.error("--interval must be at least 0.1 seconds")

    if sys.platform != "linux" and args.proc_root == Path("/proc"):
        parser.error("sysmon-pulse requires Linux /proc; use --proc-root only with a compatible proc fixture")

    previous = None
    try:
        while True:
            snapshot = collect_snapshot(args.proc_root)
            if args.json:
                print(json.dumps(snapshot, sort_keys=True))
            else:
                use_color = not args.no_color and sys.stdout.isatty()
                print(render_snapshot(snapshot, previous, color=use_color), flush=True)
            if args.once:
                return 0
            previous = snapshot
            time.sleep(args.interval)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"sysmon-pulse: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
