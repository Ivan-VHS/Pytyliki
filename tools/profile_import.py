"""Bounded import and deterministic rendering of sampling profiler data.

The importer reads collapsed stacks and the open Speedscope JSON format. It
never launches a profiler or executes code from a profile.
"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import math
from collections import defaultdict
from pathlib import Path
from typing import Any

SCHEMA = "pytyliki.profile.v1"
MAX_INPUT_BYTES = 32 * 1024 * 1024
MAX_SAMPLES = 1_000_000
MAX_FRAMES = 250_000
MAX_DEPTH = 256
MAX_FRAME_LENGTH = 512


class ProfileError(ValueError):
    """Raised when a profiler export exceeds the supported input contract."""


def _frame_name(value: Any) -> str:
    if not isinstance(value, str):
        raise ProfileError("frame names must be text")
    name = value.strip()
    if not name or len(name) > MAX_FRAME_LENGTH:
        raise ProfileError("frame names must contain 1..512 characters")
    if any(ord(char) < 32 for char in name):
        raise ProfileError("frame names cannot contain control characters")
    return name


def _weight(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ProfileError(f"{label} must be a finite positive number")
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise ProfileError(f"{label} must be a finite positive number")
    return number


def _aggregate(stacks: dict[tuple[str, ...], float], source_format: str) -> dict[str, Any]:
    if not stacks:
        raise ProfileError("profile contains no usable samples")
    total_weight = math.fsum(stacks.values())
    inclusive: dict[str, float] = defaultdict(float)
    self_weight: dict[str, float] = defaultdict(float)
    for stack, weight in stacks.items():
        for frame in stack:
            inclusive[frame] += weight
        self_weight[stack[-1]] += weight

    frames = [
        {
            "name": name,
            "total_weight": round(inclusive[name], 9),
            "self_weight": round(self_weight[name], 9),
        }
        for name in sorted(inclusive, key=lambda item: (-inclusive[item], item.casefold(), item))
    ]
    records = [
        {"frames": list(stack), "weight": round(weight, 9)}
        for stack, weight in sorted(stacks.items(), key=lambda item: (-item[1], item[0]))
    ]
    return {
        "schema": SCHEMA,
        "source_format": source_format,
        "sample_count": len(records),
        "frame_count": len(inclusive),
        "total_weight": round(total_weight, 9),
        "frames": frames,
        "stacks": records,
    }


def parse_collapsed(text: str) -> dict[str, Any]:
    if not isinstance(text, str):
        raise TypeError("collapsed profile must be text")
    stacks: dict[tuple[str, ...], float] = defaultdict(float)
    lines = text.splitlines()
    if len(lines) > MAX_SAMPLES:
        raise ProfileError("collapsed profile contains too many samples")
    frame_count = 0
    for line_number, line in enumerate(lines, 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        try:
            stack_text, raw_weight = line.rsplit(" ", 1)
        except ValueError as error:
            raise ProfileError(f"line {line_number}: expected 'frame;frame weight'") from error
        names = tuple(_frame_name(part) for part in stack_text.split(";"))
        if len(names) > MAX_DEPTH:
            raise ProfileError(f"line {line_number}: stack exceeds the {MAX_DEPTH}-frame limit")
        frame_count += len(names)
        if frame_count > MAX_FRAMES:
            raise ProfileError("profile exceeds the aggregate frame limit")
        try:
            weight = _weight(float(raw_weight), f"line {line_number} weight")
        except ValueError as error:
            raise ProfileError(f"line {line_number}: invalid weight") from error
        stacks[names] += weight
    return _aggregate(stacks, "collapsed")


def parse_speedscope(value: Any) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ProfileError("Speedscope document must be a JSON object")
    shared = value.get("shared")
    profiles = value.get("profiles")
    if not isinstance(shared, dict) or not isinstance(profiles, list):
        raise ProfileError("document must contain shared.frames and profiles")
    raw_frames = shared.get("frames")
    if not isinstance(raw_frames, list) or len(raw_frames) > MAX_FRAMES:
        raise ProfileError("shared.frames is missing or exceeds the frame limit")
    frames = [_frame_name(item.get("name") if isinstance(item, dict) else None) for item in raw_frames]
    stacks: dict[tuple[str, ...], float] = defaultdict(float)
    sample_total = 0

    for profile_index, profile in enumerate(profiles):
        if not isinstance(profile, dict) or profile.get("type") != "sampled":
            raise ProfileError("only Speedscope sampled profiles are supported")
        samples = profile.get("samples")
        weights = profile.get("weights")
        if not isinstance(samples, list):
            raise ProfileError(f"profile {profile_index}: samples must be an array")
        if weights is None:
            weights = [1.0] * len(samples)
        if not isinstance(weights, list) or len(weights) != len(samples):
            raise ProfileError(f"profile {profile_index}: weights must match the sample count")
        sample_total += len(samples)
        if sample_total > MAX_SAMPLES:
            raise ProfileError("Speedscope document exceeds the sample limit")
        for sample_index, (sample, raw_weight) in enumerate(zip(samples, weights)):
            if not isinstance(sample, list) or not sample or len(sample) > MAX_DEPTH:
                raise ProfileError(f"profile {profile_index}, sample {sample_index}: invalid stack")
            try:
                names = tuple(frames[index] for index in sample
                              if isinstance(index, int) and not isinstance(index, bool))
            except (IndexError, TypeError) as error:
                raise ProfileError("sample references an unknown frame") from error
            if len(names) != len(sample):
                raise ProfileError("sample frame indexes must be valid integers")
            weight = _weight(raw_weight, f"profile {profile_index} weight")
            stacks[names] += weight
    return _aggregate(stacks, "speedscope-sampled")


def load_profile(path: str | Path) -> dict[str, Any]:
    source = Path(path).expanduser()
    try:
        size = source.stat().st_size
        if size > MAX_INPUT_BYTES:
            raise ProfileError(f"profile exceeds the {MAX_INPUT_BYTES}-byte input limit")
        raw = source.read_bytes()
    except OSError as error:
        raise ProfileError(f"cannot read profile: {error}") from error
    if source.suffix.casefold() in {".collapsed", ".folded", ".txt"}:
        try:
            return parse_collapsed(raw.decode("utf-8-sig"))
        except UnicodeDecodeError as error:
            raise ProfileError("collapsed profile must be UTF-8 text") from error
    try:
        value = json.loads(raw.decode("utf-8-sig"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise ProfileError(f"profile is not valid UTF-8 JSON: {error}") from error
    if value.get("$schema") or "profiles" in value:
        return parse_speedscope(value)
    raise ProfileError("unsupported profile format")


def render_folded(report: dict[str, Any]) -> str:
    _validate_report(report)
    return "".join(
        ";".join(item["frames"]) + f" {item['weight']:g}\n"
        for item in report["stacks"]
    )


def _validate_report(report: Any) -> None:
    if not isinstance(report, dict) or report.get("schema") != SCHEMA:
        raise ProfileError("expected a pytyliki.profile.v1 report")
    if not isinstance(report.get("stacks"), list):
        raise ProfileError("report stacks must be an array")


def render_svg(report: dict[str, Any], *, width: int = 1200) -> str:
    _validate_report(report)
    if isinstance(width, bool) or not isinstance(width, int) or not 320 <= width <= 4096:
        raise ValueError("SVG width must be between 320 and 4096 pixels")
    root: dict[str, Any] = {"name": "all", "weight": 0.0, "children": {}}
    max_depth = 0
    for item in report["stacks"]:
        weight = _weight(item.get("weight"), "stack weight")
        frames = item.get("frames")
        if not isinstance(frames, list) or not frames or len(frames) > MAX_DEPTH:
            raise ProfileError("report contains an invalid stack")
        node = root
        node["weight"] += weight
        max_depth = max(max_depth, len(frames))
        for raw_name in frames:
            name = _frame_name(raw_name)
            child = node["children"].setdefault(name, {"name": name, "weight": 0.0, "children": {}})
            child["weight"] += weight
            node = child
    row_height = 22
    height = max(80, (max_depth + 1) * row_height + 24)
    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#101820"/>',
        '<g font-family="Segoe UI, sans-serif" font-size="11">',
    ]

    def draw(node: dict[str, Any], x: float, y: int, available: float) -> None:
        children = sorted(node["children"].values(), key=lambda item: (-item["weight"], item["name"].casefold()))
        cursor = x
        for child in children:
            child_width = available * child["weight"] / root["weight"] if root["weight"] else 0
            if child_width < 0.6:
                continue
            digest = hashlib.sha256(child["name"].encode("utf-8")).digest()
            color = f"#{80 + digest[0] % 100:02x}{85 + digest[1] % 90:02x}{95 + digest[2] % 80:02x}"
            label = html.escape(child["name"], quote=True)
            parts.append(f'<rect x="{cursor:.2f}" y="{y}" width="{max(0.5, child_width - 1):.2f}" height="20" fill="{color}" stroke="#101820"/>')
            if child_width >= 30:
                parts.append(f'<text x="{cursor + 4:.2f}" y="{y + 14}" fill="#101820">{label}</text>')
            draw(child, cursor, y + row_height, child_width)
            cursor += child_width

    draw(root, 0.0, 2, float(width))
    parts.extend(["</g></svg>"])
    return "".join(parts)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Import collapsed or Speedscope CPU profiles.")
    parser.add_argument("input", type=Path)
    parser.add_argument("--format", choices=("json", "folded", "svg"), default="json")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    try:
        report = load_profile(args.input)
        content = (
            json.dumps(report, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
            if args.format == "json"
            else render_folded(report) if args.format == "folded"
            else render_svg(report)
        )
        if args.output:
            args.output.write_text(content, encoding="utf-8", newline="\n")
        else:
            print(content, end="")
        return 0
    except (ProfileError, OSError, ValueError) as error:
        parser.exit(2, f"profile import failed: {error}\n")


if __name__ == "__main__":
    raise SystemExit(main())
