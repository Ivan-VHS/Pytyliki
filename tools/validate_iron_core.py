#!/usr/bin/env python3
"""Validate the Pytyliki IRON CORE v1 contract without third-party dependencies."""

from __future__ import annotations

import json
import sys
from collections import Counter
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "iron" / "iron-core.v1.json"
ALLOWED_STATUS = {"implemented", "partial", "planned", "open_pr", "research"}
ALLOWED_OWNER = {"pytyliki", "reaper", "duck-os", "board-backend", "joint"}
EXPECTED_MODES = {
    "inspect", "plan", "simulate", "verify", "target", "privileged", "physical_design"
}
EXPECTED_RULE_IDS = [f"IRON-{index:03d}" for index in range(1, 101)]
EXPECTED_MATURITY_IDS = [f"IRON-{index}" for index in range(10)]


def validate_data(data: dict[str, Any], root: Path = ROOT) -> list[str]:
    errors: list[str] = []

    if data.get("schema") != "pytyliki.iron-core.v1":
        errors.append("schema must be pytyliki.iron-core.v1")

    project = data.get("project", {})
    if project.get("id") != "pytyliki":
        errors.append("project.id must be pytyliki")
    if project.get("repository") != "Ivan-VHS/Pytyliki":
        errors.append("project.repository must be Ivan-VHS/Pytyliki")

    baseline = data.get("baseline", {})
    if baseline.get("rule_count") != 100:
        errors.append("baseline.rule_count must be 100")
    if baseline.get("default_status") != "planned":
        errors.append("baseline.default_status must be planned")

    access = data.get("access_model", {})
    if access.get("default_mode") != "inspect":
        errors.append("access_model.default_mode must be inspect")
    if access.get("deny_by_default") is not True:
        errors.append("access_model.deny_by_default must be true")
    for key in (
        "physical_writes_default_allowed",
        "hidden_network_allowed",
        "automatic_driver_or_firmware_install",
    ):
        if access.get(key) is not False:
            errors.append(f"access_model.{key} must be false")
    modes = access.get("modes", [])
    mode_ids = [item.get("id") for item in modes]
    if set(mode_ids) != EXPECTED_MODES or len(mode_ids) != len(EXPECTED_MODES):
        errors.append("access_model.modes must contain each of the seven unique access modes")

    modules = data.get("modules", [])
    module_ids = [item.get("id") for item in modules]
    if not modules or len(module_ids) != len(set(module_ids)):
        errors.append("modules must be non-empty and have unique IDs")
    module_id_set = set(module_ids)
    for module in modules:
        if module.get("status") not in ALLOWED_STATUS:
            errors.append(f"{module.get('id')}: invalid module status")
        if module.get("owner") not in ALLOWED_OWNER:
            errors.append(f"{module.get('id')}: invalid module owner")

    maturity = data.get("maturity_levels", [])
    maturity_ids = [item.get("id") for item in maturity]
    if maturity_ids != EXPECTED_MATURITY_IDS:
        errors.append("maturity_levels must contain the ordered IRON-0 through IRON-9 gates")

    open_work = data.get("open_work", [])
    pr_ids = [item.get("pr") for item in open_work]
    if len(pr_ids) != len(set(pr_ids)):
        errors.append("open_work contains duplicate PR numbers")
    pr_by_id = {item.get("pr"): item for item in open_work}

    rules = data.get("rules", [])
    rule_ids = [item.get("id") for item in rules]
    if rule_ids != EXPECTED_RULE_IDS:
        errors.append("rules must contain exactly IRON-001 through IRON-100 in order")

    for rule in rules:
        rule_id = rule.get("id", "<missing-id>")
        if rule.get("status") not in ALLOWED_STATUS:
            errors.append(f"{rule_id}: invalid status")
        if rule.get("owner") not in ALLOWED_OWNER:
            errors.append(f"{rule_id}: invalid owner")
        if rule.get("module") not in module_id_set:
            errors.append(f"{rule_id}: references unknown module {rule.get('module')}")
        if not isinstance(rule.get("requirement"), str) or len(rule["requirement"].strip()) < 20:
            errors.append(f"{rule_id}: requirement is missing or too short")
        if not isinstance(rule.get("acceptance"), str) or len(rule["acceptance"].strip()) < 20:
            errors.append(f"{rule_id}: acceptance criterion is missing or too short")

        evidence = rule.get("evidence", [])
        if rule.get("status") in {"implemented", "partial"} and not evidence:
            errors.append(f"{rule_id}: implemented/partial status requires evidence paths")
        for evidence_path in evidence:
            candidate = root / evidence_path
            if not candidate.is_file():
                errors.append(f"{rule_id}: evidence path does not exist: {evidence_path}")

        tracking_prs = rule.get("tracking_prs", [])
        if rule.get("status") == "open_pr" and not tracking_prs:
            errors.append(f"{rule_id}: open_pr status requires tracking_prs")
        for pr_id in tracking_prs:
            if pr_id not in pr_by_id:
                errors.append(f"{rule_id}: references unknown tracking PR #{pr_id}")

    for module in modules:
        module_id = module.get("id", "<missing-module>")
        for evidence_path in module.get("evidence", []):
            if not (root / evidence_path).is_file():
                errors.append(f"{module_id}: evidence path does not exist: {evidence_path}")
        for pr_id in module.get("tracking_prs", []):
            if pr_id not in pr_by_id:
                errors.append(f"{module_id}: references unknown tracking PR #{pr_id}")

    for pr in open_work:
        pr_id = pr.get("pr")
        expected_url = f"https://github.com/Ivan-VHS/Pytyliki/pull/{pr_id}"
        if pr.get("url") != expected_url:
            errors.append(f"PR #{pr_id}: URL does not match repository and PR number")
        if pr.get("state") not in {"open", "open-draft"}:
            errors.append(f"PR #{pr_id}: state must preserve open/draft distinction")

    evidence_modes = data.get("evidence_modes", [])
    required_evidence_modes = {
        "live_os_observation",
        "bounded_host_measurement",
        "static_analysis",
        "reference_fixture",
        "deterministic_simulation",
        "hardware_in_loop",
        "physical_signoff",
    }
    if set(evidence_modes) != required_evidence_modes:
        errors.append("evidence_modes must contain the seven defined evidence modes")

    safety = data.get("safety_invariants", [])
    if len(safety) < 10 or len(safety) != len(set(safety)):
        errors.append("safety_invariants must contain at least ten unique rules")

    return errors


def validate_contract(root: Path = ROOT) -> tuple[list[str], dict[str, Any] | None]:
    contract_path = root / "iron" / "iron-core.v1.json"
    try:
        data = json.loads(contract_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"cannot read {contract_path}: {error}"], None
    if not isinstance(data, dict):
        return ["contract root must be a JSON object"], None
    return validate_data(data, root), data


def main() -> int:
    errors, data = validate_contract()
    if errors:
        print("IRON CORE contract: FAIL")
        for error in errors:
            print(f"- {error}")
        return 1

    assert data is not None
    counts = Counter(rule["status"] for rule in data["rules"])
    count_text = ", ".join(f"{name}={counts.get(name, 0)}" for name in sorted(ALLOWED_STATUS))
    print(
        "IRON CORE contract: PASS | "
        f"rules={len(data['rules'])} | modules={len(data['modules'])} | {count_text}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
