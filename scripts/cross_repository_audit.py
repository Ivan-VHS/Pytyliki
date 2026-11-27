#!/usr/bin/env python3
"""Audit the four-project federation with one central, redaction-safe scanner."""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
from typing import Any

EXPECTED = {
    "Pytyliki": "pytyliki",
    "Reaper": "reaper",
    "OPlague": "oplague",
    "VentuSample": "ventusample",
}
SHARED_SECURITY_FILES = ("SECURITY.md", "security/beelzebul.v1.json")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_module(path: Path, name: str) -> Any:
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load central assurance module: {path.name}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _project_identity(root: Path, name: str, project_id: str, problems: list[str]) -> None:
    contract_path = root / "project-contract.json"
    if not contract_path.is_file():
        problems.append(f"{name}: project-contract.json is missing")
        return
    try:
        contract = json.loads(contract_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        problems.append(f"{name}: project contract is unreadable or invalid JSON")
        return

    project = contract.get("project")
    if not isinstance(project, dict):
        problems.append(f"{name}: project contract has no project object")
        return
    if project.get("id") != project_id:
        problems.append(f"{name}: project identifier does not match the federation registry")
    if project.get("repository") != f"Ivan-VHS/{name}":
        problems.append(f"{name}: repository identity does not match the federation registry")


def audit(roots: dict[str, Path]) -> dict[str, Any]:
    problems: list[str] = []
    if set(roots) != set(EXPECTED):
        problems.append("exactly the four registered repositories must be supplied")

    # Use the current Pytyliki checkout for pull-request audits, not a second
    # checkout of main that would hide the changes under review.
    central_root = Path(__file__).resolve().parents[1]
    if central_root.name == "Pytyliki":
        roots = dict(roots)
        roots["Pytyliki"] = central_root

    try:
        scanner = load_module(
            central_root / "scripts" / "security_audit.py",
            "federated_security_audit",
        )
        dependency = load_module(
            central_root / "scripts" / "dependency_audit.py",
            "federated_dependency_audit",
        )
    except Exception as error:
        return {
            "schema": "cross-repository-security-assessment.v3",
            "repositories_checked": [],
            "expected_repositories": sorted(EXPECTED),
            "shared_security_files_checked": list(SHARED_SECURITY_FILES),
            "status": "fail",
            "problems": [f"central assurance tools could not be loaded ({type(error).__name__})"],
            "score": 0,
            "score_components": {},
            "aggregate_findings": {},
            "source_audits": {},
            "dependency_audits": {},
            "limitations": "The central scanner could not be loaded; no security score is inferred.",
        }

    canonical_root = roots.get("Pytyliki")
    canonical_hashes: dict[str, str] = {}
    if canonical_root is None or not canonical_root.is_dir():
        problems.append("Pytyliki: repository checkout is missing")
    else:
        for relative in SHARED_SECURITY_FILES:
            candidate = canonical_root / relative
            if not candidate.is_file():
                problems.append(f"Pytyliki: shared security source is missing: {relative}")
            else:
                canonical_hashes[relative] = digest(candidate)

    source_reports: dict[str, dict[str, Any]] = {}
    dependency_reports: dict[str, dict[str, Any]] = {}
    checked: list[str] = []
    for name, project_id in EXPECTED.items():
        root = roots.get(name)
        if root is None or not root.is_dir():
            problems.append(f"{name}: repository checkout is missing")
            continue

        _project_identity(root, name, project_id, problems)

        security_doc = root / "SECURITY.md"
        policy_file = root / "security" / "beelzebul.v1.json"
        codeql = root / ".github" / "workflows" / "codeql.yml"
        for path, label in (
            (security_doc, "SECURITY.md"),
            (policy_file, "security/beelzebul.v1.json"),
            (codeql, ".github/workflows/codeql.yml"),
        ):
            if not path.is_file():
                problems.append(f"{name}: required security control is missing: {label}")

        for relative, expected_hash in canonical_hashes.items():
            candidate = root / relative
            if candidate.is_file() and digest(candidate) != expected_hash:
                problems.append(f"{name}: shared security source differs: {relative}")

        try:
            source_reports[name] = scanner.audit(root)
            dependency_reports[name] = dependency.audit(root)
            checked.append(name)
        except Exception as error:
            problems.append(f"{name}: central source/dependency audit failed ({type(error).__name__})")

    combined = {
        "critical_secrets": sum(
            int(report.get("counts", {}).get("critical_secrets", 0))
            for report in source_reports.values()
        ),
        "privacy": sum(
            int(report.get("counts", {}).get("privacy", 0))
            for report in source_reports.values()
        ),
        "source_risks": sum(
            int(report.get("counts", {}).get("source_risks", 0))
            for report in source_reports.values()
        ),
    }
    if combined["critical_secrets"]:
        problems.append("one or more repositories contain high-confidence secret findings")

    parity_score = max(0, 100 - min(100, 15 * len(problems)))
    source_penalty = min(
        100,
        combined["critical_secrets"] * 30
        + combined["privacy"] * 3
        + combined["source_risks"],
    )
    source_score = max(0, 100 - source_penalty)
    dependency_scores = [
        int(report.get("pin_hygiene_score", 0))
        for report in dependency_reports.values()
    ]
    dependency_score = (
        round(sum(dependency_scores) / len(dependency_scores))
        if dependency_scores else 0
    )
    overall_score = round(
        parity_score * 0.50 + source_score * 0.35 + dependency_score * 0.15
    )

    return {
        "schema": "cross-repository-security-assessment.v3",
        "repositories_checked": sorted(checked),
        "expected_repositories": sorted(EXPECTED),
        "shared_security_files_checked": list(SHARED_SECURITY_FILES),
        "status": "pass" if not problems else "fail",
        "problems": problems,
        "score": overall_score,
        "score_components": {
            "repository_security_contracts": {"weight": 50, "score": parity_score},
            "source_and_privacy_scan": {"weight": 35, "score": source_score},
            "dependency_pin_hygiene": {"weight": 15, "score": dependency_score},
        },
        "aggregate_findings": combined,
        "source_audits": source_reports,
        "dependency_audits": dependency_reports,
        "limitations": (
            "One centrally maintained redaction-safe scanner and dependency inventory are "
            "applied to the four checked-out source trees. Each repository retains its own "
            "project contract and security policy. Shared Beelzebul policy files are checked "
            "for parity. Static matching and offline metadata do not prove absence of vulnerabilities."
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", action="append", required=True, metavar="NAME=PATH")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    roots: dict[str, Path] = {}
    for item in args.repo:
        if "=" not in item:
            parser.error("--repo must use NAME=PATH")
        name, path = item.split("=", 1)
        roots[name] = Path(path).resolve()
    report = audit(roots)
    rendered = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0 if report["status"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
