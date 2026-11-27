#!/usr/bin/env python3
"""Validate the versioned Pytyliki Processor IDE contract."""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "contracts" / "processor-ide.v1.json"
EXPECTED_RULE_IDS = [f"PIDE-{number:03d}" for number in range(1, 101)]
EXPECTED_WORKSPACES = 20
EXPECTED_MODES = {
    "inspect", "plan", "static", "simulate", "debug", "hardware_in_loop", "privileged"
}
ALLOWED_STATUS = {"partial", "planned", "research", "implemented"}


def validate_data(data: dict[str, Any], root: Path = ROOT) -> list[str]:
    errors: list[str] = []

    if data.get("schema") != "pytyliki.processor-ide.v1":
        errors.append("schema must be pytyliki.processor-ide.v1")

    project = data.get("project", {})
    if project.get("id") != "processor-ide" or project.get("repository") != "Ivan-VHS/Pytyliki":
        errors.append("project identity must be processor-ide in Ivan-VHS/Pytyliki")

    baseline = data.get("baseline", {})
    if baseline.get("rule_count") != 100 or baseline.get("default_status") != "planned":
        errors.append("baseline must declare 100 requirements and planned default status")

    related = data.get("related_contracts", {})
    workspace_registry = related.get("system_workspace_registry", {})
    if workspace_registry.get("existing_count") != EXPECTED_WORKSPACES:
        errors.append("system workspace registry must reuse the existing 20-domain registry")
    for key in ("header", "source", "tests"):
        path = workspace_registry.get(key)
        if not isinstance(path, str) or not (root / path).is_file():
            errors.append(f"workspace registry {key} path does not exist: {path}")

    native_session = related.get("native_session_foundation", {})
    for key in ("header", "source", "tests", "build"):
        path = native_session.get(key)
        if not isinstance(path, str) or not (root / path).is_file():
            errors.append(f"native session {key} path does not exist: {path}")
    if native_session.get("api") != "pl_ide_session_init":
        errors.append("native session foundation must expose pl_ide_session_init")

    for key in ("iron_core", "low_level_contract"):
        path = related.get(key)
        if not isinstance(path, str) or not (root / path).is_file():
            errors.append(f"related contract path does not exist: {path}")

    workspaces = data.get("workspaces", [])
    workspace_ids = [item.get("id") for item in workspaces]
    if len(workspaces) != EXPECTED_WORKSPACES or len(workspace_ids) != len(set(workspace_ids)):
        errors.append("workspaces must contain exactly 20 unique IDs")
    for workspace in workspaces:
        if workspace.get("registry_status") != "implemented":
            errors.append(f"{workspace.get('id')}: existing registry status must remain implemented")
        if workspace.get("ide_workspace_status") not in {"planned", "partial", "implemented"}:
            errors.append(f"{workspace.get('id')}: invalid IDE workspace status")
        if workspace.get("source_registry") != workspace_registry.get("source"):
            errors.append(f"{workspace.get('id')}: must reuse the canonical enhancement registry")

    perspectives = data.get("perspectives", [])
    perspective_ids = [item.get("id") for item in perspectives]
    if len(perspectives) != EXPECTED_WORKSPACES or len(perspective_ids) != len(set(perspective_ids)):
        errors.append("perspectives must contain exactly 20 unique named engineering spaces")
    for perspective in perspectives:
        perspective_id = perspective.get("id", "<unknown>")
        if perspective.get("workspace") not in set(workspace_ids):
            errors.append(f"{perspective_id}: unknown workspace {perspective.get('workspace')}")
        if perspective.get("scope_kind") != "workspace_perspective":
            errors.append(f"{perspective_id}: must be a workspace perspective")
        if perspective.get("authority") != "no_privilege_elevation":
            errors.append(f"{perspective_id}: perspectives must not grant privilege elevation")

    shell = data.get("shell", {})
    if shell.get("default_mode") != "inspect":
        errors.append("Processor IDE default mode must be inspect")
    pane_ids = [item.get("id") for item in shell.get("panes", [])]
    if len(pane_ids) != len(set(pane_ids)) or len(pane_ids) != 10:
        errors.append("shell must define exactly ten unique panes")

    target_model = data.get("target_model", {})
    mode_ids = [item.get("id") for item in target_model.get("modes", [])]
    if set(mode_ids) != EXPECTED_MODES or len(mode_ids) != len(EXPECTED_MODES):
        errors.append("target_model.modes must define all seven unique execution modes")

    palette = data.get("signal_palette", {})
    palette_states = palette.get("states", [])
    expected_signal_states = {"high", "low", "floating", "contended", "unknown"}
    signal_ids = [item.get("id") for item in palette_states]
    if set(signal_ids) != expected_signal_states or len(signal_ids) != len(expected_signal_states):
        errors.append("signal_palette must define high, low, floating, contended and unknown states")
    if palette.get("scope") != "waveform_and_electrical_signal_views_only":
        errors.append("signal palette must remain scoped to signal views")
    if palette.get("theme_independent") is not True:
        errors.append("signal palette must be independent from the global IDE theme")
    if not isinstance(palette.get("accessibility"), str) or len(palette["accessibility"]) < 40:
        errors.append("signal palette must include non-color accessibility guidance")

    evidence = data.get("evidence", {})
    if len(evidence.get("modes", [])) < 8:
        errors.append("evidence must distinguish at least eight evidence modes")
    required_evidence = set(evidence.get("required_fields", []))
    for field in ("target_id", "evidence_mode", "clock_source", "unit", "uncertainty", "limitations"):
        if field not in required_evidence:
            errors.append(f"evidence.required_fields must include {field}")

    safety = data.get("safety", {})
    for field in (
        "deny_by_default",
        "no_implicit_hardware_access",
        "no_hidden_tool_invocation",
        "no_automatic_driver_firmware_install",
        "no_unreviewed_mmio_or_privileged_write",
        "no_secret_logging",
        "no_silent_source_rewrite",
    ):
        if safety.get(field) is not True:
            errors.append(f"safety.{field} must be true")

    rules = data.get("rules", [])
    rule_ids = [item.get("id") for item in rules]
    if rule_ids != EXPECTED_RULE_IDS:
        errors.append("rules must contain exactly PIDE-001 through PIDE-100 in order")

    known_workspaces = set(workspace_ids)
    for rule in rules:
        rule_id = rule.get("id", "<missing-id>")
        if rule.get("group") not in {"IDE_CORE", "SILICON_DEPTH"}:
            errors.append(f"{rule_id}: invalid group")
        if rule.get("workspace") not in known_workspaces:
            errors.append(f"{rule_id}: unknown workspace {rule.get('workspace')}")
        if rule.get("status") not in ALLOWED_STATUS:
            errors.append(f"{rule_id}: invalid status {rule.get('status')}")
        if not isinstance(rule.get("requirement"), str) or len(rule["requirement"].strip()) < 20:
            errors.append(f"{rule_id}: requirement is missing or too short")
        if not isinstance(rule.get("acceptance"), str) or len(rule["acceptance"].strip()) < 20:
            errors.append(f"{rule_id}: acceptance condition is missing or too short")

        rule_evidence = rule.get("evidence", [])
        if rule.get("status") in {"partial", "implemented"} and not rule_evidence:
            errors.append(f"{rule_id}: partial/implemented status requires evidence")
        for path in rule_evidence:
            if not (root / path).is_file():
                errors.append(f"{rule_id}: evidence path does not exist: {path}")

    if sum(1 for rule in rules if rule.get("group") == "IDE_CORE") != 50:
        errors.append("IDE_CORE group must contain 50 rules")
    if sum(1 for rule in rules if rule.get("group") == "SILICON_DEPTH") != 50:
        errors.append("SILICON_DEPTH group must contain 50 rules")

    panes = {item.get("id") for item in shell.get("panes", [])}
    required_panes = {
        "project_target", "source_ir", "assembly_image", "registers_flags",
        "memory_address", "pipeline_microarchitecture", "signals_timing",
        "trace_replay", "build_test", "diagnostics_diff"
    }
    if panes != required_panes:
        errors.append("shell panes must preserve the ten canonical Processor IDE surfaces")

    workstreams = data.get("integration_workstreams", [])
    pr_ids = [item.get("pr") for item in workstreams]
    if len(pr_ids) != len(set(pr_ids)) or len(pr_ids) < 8:
        errors.append("integration_workstreams must contain unique existing processor PR references")

    layers = data.get("implementation_layers", [])
    layer_ids = [item.get("id") for item in layers]
    if layer_ids != [f"IDE-{number}" for number in range(8)]:
        errors.append("implementation_layers must contain ordered IDE-0 through IDE-7 gates")

    adjustments = data.get("safety_adjustments", [])
    if len(adjustments) < 5 or len(adjustments) != len(set(adjustments)):
        errors.append("safety_adjustments must explain the non-literal/target-specific rules")

    return errors


def validate_contract(root: Path = ROOT) -> tuple[list[str], dict[str, Any] | None]:
    path = root / "contracts" / "processor-ide.v1.json"
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"cannot read {path}: {error}"], None
    if not isinstance(data, dict):
        return ["contract root must be a JSON object"], None
    return validate_data(data, root), data


def main() -> int:
    errors, data = validate_contract()
    if errors:
        print("Processor IDE contract: FAIL")
        for error in errors:
            print(f"- {error}")
        return 1
    assert data is not None
    counts = {
        status: sum(1 for rule in data["rules"] if rule["status"] == status)
        for status in sorted(ALLOWED_STATUS)
    }
    print(
        "Processor IDE contract: PASS | "
        f"rules={len(data['rules'])} | workspaces={len(data['workspaces'])} | "
        f"partial={counts['partial']} | planned={counts['planned']} | "
        f"research={counts['research']} | implemented={counts['implemented']}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
