"""Read-only Linux host sensor snapshot with explicit source provenance."""
from __future__ import annotations

import argparse
import json
import platform
from pathlib import Path
from typing import Any

SCHEMA = "pytyliki.sensor-snapshot.v1"
MAX_SENSOR_FILES = 256
MAX_TEXT_BYTES = 4096


def _read_text(path: Path) -> str | None:
    try:
        if path.is_symlink() or not path.is_file() or path.stat().st_size > MAX_TEXT_BYTES:
            return None
        return path.read_text(encoding="utf-8", errors="replace").strip()
    except OSError:
        return None


def _integer(path: Path) -> int | None:
    value = _read_text(path)
    if value is None:
        return None
    try:
        parsed = int(value, 10)
    except ValueError:
        return None
    return parsed if parsed >= 0 else None


def _metric(name: str, value: float | int | str, unit: str, source: str, quality: str = "observed") -> dict[str, Any]:
    return {"name": name, "value": value, "unit": unit, "source": source, "quality": quality}


def collect_snapshot(sys_root: str | Path = "/sys", proc_root: str | Path = "/proc",
                     *, system: str | None = None) -> dict[str, Any]:
    """Collect available Linux sysfs/procfs facts; missing sensors remain absent."""
    system_name = system or platform.system()
    metrics: list[dict[str, Any]] = []
    if system_name != "Linux":
        return {"schema": SCHEMA, "platform": system_name, "status": "unsupported",
                "metrics": [], "notes": ["This adapter reads Linux sysfs and procfs only."]}

    sys_root = Path(sys_root)
    proc_root = Path(proc_root)
    cpuinfo = _read_text(proc_root / "cpuinfo")
    if cpuinfo:
        model = next((line.split(":", 1)[1].strip() for line in cpuinfo.splitlines()
                      if line.lower().startswith(("model name", "hardware", "processor")) and ":" in line), None)
        if model:
            metrics.append(_metric("cpu.model", model, "text", "/proc/cpuinfo"))

    frequency_values = []
    cpu_root = sys_root / "devices/system/cpu"
    try:
        cpu_dirs = sorted((item for item in cpu_root.glob("cpu[0-9]*") if item.is_dir() and not item.is_symlink()),
                          key=lambda item: item.name)[:128]
    except OSError:
        cpu_dirs = []
    for cpu_dir in cpu_dirs:
        freq_path = cpu_dir / "cpufreq/scaling_cur_freq"
        freq_khz = _integer(freq_path)
        if freq_khz is not None:
            frequency_values.append(freq_khz / 1000.0)
            metrics.append(_metric(f"{cpu_dir.name}.frequency", round(freq_khz / 1000.0, 3),
                                   "MHz", str(freq_path.relative_to(sys_root))))
        governor = _read_text(cpu_dir / "cpufreq/scaling_governor")
        if governor:
            metrics.append(_metric(f"{cpu_dir.name}.governor", governor, "text",
                                   str((cpu_dir / "cpufreq/scaling_governor").relative_to(sys_root))))

    thermal_root = sys_root / "class/thermal"
    try:
        zones = sorted((item for item in thermal_root.glob("thermal_zone*") if item.is_dir() and not item.is_symlink()),
                       key=lambda item: item.name)[:64]
    except OSError:
        zones = []
    for zone in zones:
        temperature = _integer(zone / "temp")
        if temperature is None:
            continue
        label = _read_text(zone / "type") or zone.name
        metrics.append(_metric(f"thermal.{label}", round(temperature / 1000.0, 3),
                               "°C", str((zone / "temp").relative_to(sys_root))))

    powercap_root = sys_root / "class/powercap"
    try:
        energy_files = sorted((item for item in powercap_root.glob("**/energy_uj")
                               if item.is_file() and not item.is_symlink()), key=lambda item: str(item))[:64]
    except OSError:
        energy_files = []
    for path in energy_files:
        energy = _integer(path)
        if energy is not None:
            metrics.append(_metric(f"energy.{path.parent.name}", energy, "µJ",
                                   str(path.relative_to(sys_root))))

    status = "available" if metrics else "no-sensors"
    notes = []
    if not frequency_values:
        notes.append("CPU frequency counters were not exposed by this kernel or host policy.")
    if not zones:
        notes.append("No readable thermal zones were exposed.")
    if not energy_files:
        notes.append("No readable powercap energy counters were exposed.")
    return {"schema": SCHEMA, "platform": "Linux", "status": status, "metrics": metrics, "notes": notes}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Capture read-only Linux sensor data.")
    parser.add_argument("--sys-root", type=Path, default=Path("/sys"))
    parser.add_argument("--proc-root", type=Path, default=Path("/proc"))
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    report = collect_snapshot(args.sys_root, args.proc_root)
    content = json.dumps(report, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    if args.output:
        args.output.write_text(content, encoding="utf-8", newline="\n")
    else:
        print(content, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
