"""Cross-repository syntax conformance runner for Reaper.

Runs only the syntax validator in an explicitly selected Reaper checkout.
Fixture source is never executed. All child processes use an argv vector,
a bounded timeout and shell=False.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable

SCHEMA = "pytyliki.reaper-syntax-conformance.v1"
REPORT_SCHEMA = "pytyliki.reaper-syntax-conformance-report.v1"
SUPPORTED_SYNTAXES = {"reaper", "python", "pipeline", "lisp", "forth", "brainfuck", "befunge93"}
MAX_MANIFEST_BYTES = 1024 * 1024
MAX_SOURCE_BYTES = 1024 * 1024
MAX_CASES = 100
DEFAULT_TIMEOUT_SECONDS = 20
ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST = ROOT / "fixtures" / "multisyntax" / "v1" / "manifest.json"
ID_PATTERN = re.compile(r"^[a-z0-9]+(?:-[a-z0-9]+)*$")


class ConformanceError(ValueError):
    """Invalid manifest, fixture or Reaper checkout."""


def _unique_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ConformanceError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _safe_file(base: Path, relative_text: Any, label: str, max_bytes: int) -> Path:
    if not isinstance(relative_text, str) or not relative_text.strip():
        raise ConformanceError(f"{label} must be a non-empty relative path")
    relative = Path(relative_text)
    if relative.is_absolute() or ".." in relative.parts:
        raise ConformanceError(f"{label} must not be absolute or contain '..'")
    candidate = base
    for part in relative.parts:
        candidate = candidate / part
        if candidate.is_symlink():
            raise ConformanceError(f"{label} must not traverse symbolic links")
    try:
        resolved = candidate.resolve(strict=True)
    except OSError as error:
        raise ConformanceError(f"{label} does not exist: {error}") from error
    try:
        resolved.relative_to(base.resolve(strict=True))
    except ValueError as error:
        raise ConformanceError(f"{label} escapes its fixture directory") from error
    if not resolved.is_file():
        raise ConformanceError(f"{label} must point to a regular file")
    if resolved.stat().st_size > max_bytes:
        raise ConformanceError(f"{label} exceeds the {max_bytes}-byte limit")
    return resolved


def load_manifest(path: str | Path = DEFAULT_MANIFEST) -> tuple[Path, list[dict[str, Any]]]:
    manifest_path = Path(path).expanduser()
    if manifest_path.is_symlink() or not manifest_path.is_file():
        raise ConformanceError("manifest must be a regular, non-symlink file")
    try:
        if manifest_path.stat().st_size > MAX_MANIFEST_BYTES:
            raise ConformanceError("manifest exceeds the 1 MiB input limit")
        payload = json.loads(
            manifest_path.read_text(encoding="utf-8"),
            object_pairs_hook=_unique_object,
            parse_constant=lambda value: (_ for _ in ()).throw(
                ConformanceError(f"non-finite JSON value: {value}")
            ),
        )
    except OSError as error:
        raise ConformanceError(f"cannot read manifest: {error}") from error
    except json.JSONDecodeError as error:
        raise ConformanceError(f"manifest is not valid JSON: {error}") from error

    if not isinstance(payload, dict) or set(payload) != {"schema", "cases"}:
        raise ConformanceError("manifest root must contain only schema and cases")
    if payload["schema"] != SCHEMA:
        raise ConformanceError(f"manifest schema must be {SCHEMA}")
    cases = payload["cases"]
    if not isinstance(cases, list) or not cases or len(cases) > MAX_CASES:
        raise ConformanceError(f"cases must contain 1..{MAX_CASES} entries")

    base = manifest_path.parent.resolve(strict=True)
    seen: set[str] = set()
    validated: list[dict[str, Any]] = []
    for index, case in enumerate(cases):
        label = f"cases[{index}]"
        required = {"id", "syntax", "source", "expected_valid"}
        if not isinstance(case, dict) or set(case) - (required | {"polyglot"}) or not required.issubset(case):
            raise ConformanceError(f"{label} must contain id, syntax, source, expected_valid and optional polyglot")
        identifier = case["id"]
        if not isinstance(identifier, str) or not ID_PATTERN.fullmatch(identifier):
            raise ConformanceError(f"{label}.id must be a canonical lowercase identifier")
        if identifier in seen:
            raise ConformanceError(f"duplicate conformance case id: {identifier}")
        seen.add(identifier)
        syntax = case["syntax"]
        if not isinstance(syntax, str) or syntax not in SUPPORTED_SYNTAXES:
            raise ConformanceError(f"{label}.syntax is not supported")
        if type(case["expected_valid"]) is not bool:
            raise ConformanceError(f"{label}.expected_valid must be a boolean")
        polyglot = case.get("polyglot", False)
        if type(polyglot) is not bool:
            raise ConformanceError(f"{label}.polyglot must be a boolean")
        source = _safe_file(base, case["source"], f"{label}.source", MAX_SOURCE_BYTES)
        validated.append({
            "id": identifier,
            "syntax": syntax,
            "source": source,
            "expected_valid": case["expected_valid"],
            "polyglot": polyglot,
        })
    return base, validated


def _reaper_root(path: str | Path) -> Path:
    candidate = Path(path).expanduser()
    if candidate.is_symlink():
        raise ConformanceError("Reaper root must not be a symbolic link")
    try:
        root = candidate.resolve(strict=True)
    except OSError as error:
        raise ConformanceError(f"cannot open Reaper checkout: {error}") from error
    if not (root / "pyproject.toml").is_file() or not (root / "reaper" / "multisyntax.py").is_file():
        raise ConformanceError("Reaper root must contain pyproject.toml and reaper/multisyntax.py")
    return root


def run_conformance(
    reaper_root: str | Path,
    manifest_path: str | Path = DEFAULT_MANIFEST,
    *,
    python_executable: str | None = None,
    timeout_seconds: int = DEFAULT_TIMEOUT_SECONDS,
    runner: Callable[..., Any] = subprocess.run,
) -> dict[str, Any]:
    if isinstance(timeout_seconds, bool) or not isinstance(timeout_seconds, int) or not 1 <= timeout_seconds <= 120:
        raise ConformanceError("timeout_seconds must be between 1 and 120")
    root = _reaper_root(reaper_root)
    _base, cases = load_manifest(manifest_path)
    executable = python_executable or sys.executable
    results: list[dict[str, Any]] = []

    for case in cases:
        command = [
            executable, "-m", "reaper", "syntax",
            "--check", "--json", "--syntax", case["syntax"],
        ]
        if case["polyglot"]:
            command.append("--polyglot")
        command.append(str(case["source"]))
        try:
            completed = runner(
                command,
                cwd=root,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
                timeout=timeout_seconds,
                shell=False,
                check=False,
            )
            payload = None
            if completed.stdout and completed.stdout.strip():
                try:
                    payload = json.loads(completed.stdout)
                except json.JSONDecodeError:
                    payload = None
            actual_valid = completed.returncode == 0 and isinstance(payload, dict) and payload.get("valid") is True
            matched = actual_valid is case["expected_valid"]
            results.append({
                "id": case["id"],
                "syntax": case["syntax"],
                "expected_valid": case["expected_valid"],
                "actual_valid": actual_valid,
                "matched": matched,
                "exit_code": completed.returncode,
                "stderr": (completed.stderr or "")[:4096],
                "validation_only": True,
            })
        except subprocess.TimeoutExpired:
            results.append({
                "id": case["id"],
                "syntax": case["syntax"],
                "expected_valid": case["expected_valid"],
                "actual_valid": False,
                "matched": False,
                "exit_code": None,
                "stderr": f"syntax validation exceeded {timeout_seconds} seconds",
                "validation_only": True,
            })
        except OSError as error:
            results.append({
                "id": case["id"],
                "syntax": case["syntax"],
                "expected_valid": case["expected_valid"],
                "actual_valid": False,
                "matched": False,
                "exit_code": None,
                "stderr": f"cannot start Reaper validator: {error}",
                "validation_only": True,
            })

    passed = sum(item["matched"] for item in results)
    return {
        "schema": REPORT_SCHEMA,
        "status": "passed" if passed == len(results) else "failed",
        "reaper_root": str(root),
        "case_count": len(results),
        "passed": passed,
        "failed": len(results) - passed,
        "cases": results,
        "source_execution_performed": False,
        "shell_used": False,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reaper-root", type=Path, required=True, help="explicit local Reaper checkout")
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT_SECONDS)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    try:
        report = run_conformance(
            args.reaper_root,
            args.manifest,
            timeout_seconds=args.timeout,
        )
        if args.json:
            print(json.dumps(report, ensure_ascii=False, indent=2))
        else:
            for case in report["cases"]:
                state = "PASS" if case["matched"] else "FAIL"
                print(f"{state}  {case['id']}  {case['syntax']}")
                if not case["matched"] and case["stderr"]:
                    print(f"      {case['stderr']}")
            print(f"Reaper syntax conformance: {report['passed']}/{report['case_count']} passed")
        return 0 if report["status"] == "passed" else 1
    except (ConformanceError, OSError, ValueError) as error:
        print(f"pytyliki-syntax-conformance: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
