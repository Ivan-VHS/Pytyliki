#!/usr/bin/env python3
"""Validate the Pytyliki Omega-Null research profile contract."""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PROFILE_PATH = ROOT / "omega-null" / "omega-null.v0.json"
EXPECTED_MARKERS = 20
EXPECTED_OPERATIONS = 12
EXPECTED_FRAMEWORKS = {
    "aegis-shield", "chrono-weave", "matter-forge", "mind-link"
}
EXPECTED_MODES = {
    "inspect", "plan", "simulate", "verify", "target", "privileged", "physical_design"
}


def validate_data(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    if data.get("schema") != "pytyliki.omega-null.v0":
        errors.append("schema must be pytyliki.omega-null.v0")

    profile = data.get("profile", {})
    required_profile = {
        "id": "omega-null",
        "display_name": "Omega-Null",
        "parent": "pytyliki.iron-core",
        "maturity": "research_profile",
        "compiler_owner": "reaper",
        "domain_owner": "pytyliki",
    }
    for key, expected in required_profile.items():
        if profile.get(key) != expected:
            errors.append(f"profile.{key} must be {expected!r}")
    if profile.get("grammar_status") == "implemented":
        errors.append("Omega-Null grammar cannot be marked implemented by the research profile alone")

    ownership = data.get("ownership", {})
    for owner in ("reaper", "pytyliki", "duck_os", "board_and_lab_backends"):
        if not isinstance(ownership.get(owner), str) or len(ownership[owner]) < 40:
            errors.append(f"ownership.{owner} must declare a substantive boundary")

    layers = data.get("semantic_layers", [])
    layer_ids = [item.get("id") for item in layers]
    if layer_ids != ["O0", "O1", "O2", "O3", "O4"]:
        errors.append("semantic_layers must contain O0 through O4 in order")

    markers = data.get("marker_semantics", [])
    marker_names = [item.get("marker") for item in markers]
    normalized_names = [item.get("normalized") for item in markers]
    if len(markers) != EXPECTED_MARKERS:
        errors.append(f"marker_semantics must contain exactly {EXPECTED_MARKERS} markers")
    if len(marker_names) != len(set(marker_names)):
        errors.append("marker spellings must be unique")
    if len(normalized_names) != len(set(normalized_names)):
        errors.append("normalized marker operation names must be unique")
    for item in markers:
        for field in ("marker", "normalized", "intent", "semantics", "forbidden_interpretation"):
            if not isinstance(item.get(field), str) or not item[field].strip():
                errors.append(f"marker entry {item.get('normalized', '<unknown>')} has empty {field}")
        if item.get("normalized") == "reality.mutate":
            errors.append("marker surface must not expose a reality mutation operation")

    operations = data.get("operation_catalog", [])
    operation_ids = [item.get("id") for item in operations]
    if len(operations) != EXPECTED_OPERATIONS:
        errors.append(f"operation_catalog must contain exactly {EXPECTED_OPERATIONS} operations")
    if len(operation_ids) != len(set(operation_ids)):
        errors.append("operation IDs must be unique")

    operation_map = {item.get("id"): item for item in operations}
    for item in operations:
        operation_id = item.get("id", "<unknown>")
        if not item.get("target"):
            errors.append(f"{operation_id}: target must be explicit")
        if not item.get("mode"):
            errors.append(f"{operation_id}: execution mode must be explicit")
        if not item.get("effect"):
            errors.append(f"{operation_id}: effect declaration must be explicit")
        if item.get("status") == "implemented":
            errors.append(f"{operation_id}: research catalogue cannot assert backend implementation")
        if "target" in item.get("mode", []) and item.get("required_capability") in (None, "", "none"):
            errors.append(f"{operation_id}: target mode requires an explicit capability")
        if not isinstance(item.get("operator_approval_required"), bool):
            errors.append(f"{operation_id}: operator_approval_required must be boolean")
        constraints = " ".join(item.get("constraints", [])).lower()
        if not constraints:
            errors.append(f"{operation_id}: constraints are required")

    measurement = operation_map.get("quantum.measure", {})
    measurement_constraints = " ".join(measurement.get("constraints", [])).lower()
    if "force a chosen outcome" not in measurement_constraints:
        errors.append("quantum.measure must explicitly prohibit forced measurement outcomes")
    if "measurement" not in measurement.get("effect", []):
        errors.append("quantum.measure must declare its measurement effect")

    field = operation_map.get("field.solve", {})
    if "spacetime" not in " ".join(field.get("constraints", [])).lower():
        errors.append("field.solve must state that it does not alter physical spacetime")

    chrono = operation_map.get("chrono.replay", {})
    chrono_constraints = " ".join(chrono.get("constraints", [])).lower()
    if "reverse external physical events" not in chrono_constraints or "earlier real time" not in chrono_constraints:
        errors.append("chrono.replay must distinguish logical replay from physical time reversal")

    fabrication = operation_map.get("fabrication.plan", {})
    fabrication_constraints = " ".join(fabrication.get("constraints", [])).lower()
    if "feedstock" not in fabrication_constraints or "operator approval" not in fabrication_constraints:
        errors.append("fabrication.plan must require physical feedstock and operator approval")

    bci = operation_map.get("bci.capture", {})
    bci_constraints = " ".join(bci.get("constraints", [])).lower()
    if "consent" not in bci_constraints:
        errors.append("bci.capture must require informed user consent")

    research_domains = data.get("research_domains", [])
    if len(research_domains) < 9:
        errors.append("research_domains must map at least nine exploratory domains")
    domain_ids = [item.get("id") for item in research_domains]
    if len(domain_ids) != len(set(domain_ids)):
        errors.append("research domain IDs must be unique")
    allowed_modules = {
        "pyty.quantum", "pyty.bci", "pyty.material", "pyty.field",
        "pyty.time", "pyty.power", "pyty.experiment", "pyty.sim"
    }
    for item in research_domains:
        if item.get("module") not in allowed_modules:
            errors.append(f"research domain {item.get('id')} references an unknown Omega module")

    practices = data.get("engineering_practices", {})
    gates = practices.get("verification_gates", [])
    gate_ids = [item.get("id") for item in gates]
    if gate_ids != ["syntax_type_effect", "model_invariants", "physical_target_qualification"]:
        errors.append("engineering_practices must define the three ordered verification gates")
    for key in ("resource_control", "state_backup", "forecast_policy"):
        if not isinstance(practices.get(key), str) or len(practices[key]) < 30:
            errors.append(f"engineering_practices.{key} must be explicit")

    frameworks = data.get("frameworks", [])
    framework_ids = [item.get("id") for item in frameworks]
    if set(framework_ids) != EXPECTED_FRAMEWORKS or len(framework_ids) != len(EXPECTED_FRAMEWORKS):
        errors.append("frameworks must contain exactly Aegis-Shield, Chrono-Weave, Matter-Forge and Mind-Link")

    claim_translation = data.get("claim_translation", [])
    if len(claim_translation) < 9:
        errors.append("claim_translation must cover at least nine physical/reality claims")
    if any(not item.get("translation") for item in claim_translation):
        errors.append("every claim must have a concrete engineering translation")

    evidence = data.get("evidence_policy", {})
    evidence_modes = evidence.get("modes", [])
    if len(evidence_modes) != len(set(evidence_modes)) or len(evidence_modes) < 8:
        errors.append("evidence_policy.modes must provide at least eight unique evidence modes")
    required_fields = evidence.get("required_fields", [])
    for required in ("evidence_mode", "source_revision", "target_or_model_id", "uncertainty", "limitations"):
        if required not in required_fields:
            errors.append(f"evidence_policy.required_fields must include {required}")

    access = data.get("access_model", {})
    if access.get("default_mode") != "inspect":
        errors.append("default mode must be inspect")
    for key in (
        "deny_by_default",
        "no_implicit_hardware_access",
        "no_hidden_network",
        "no_automatic_firmware_or_driver_install",
        "no_reality_mutation_api",
    ):
        if access.get(key) is not True:
            errors.append(f"access_model.{key} must be true")

    return errors


def validate_profile(path: Path = PROFILE_PATH) -> tuple[list[str], dict[str, Any] | None]:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return [f"cannot read {path}: {error}"], None
    if not isinstance(data, dict):
        return ["profile root must be a JSON object"], None
    return validate_data(data), data


def main() -> int:
    errors, data = validate_profile()
    if errors:
        print("Omega-Null profile: FAIL")
        for error in errors:
            print(f"- {error}")
        return 1
    assert data is not None
    print(
        "Omega-Null profile: PASS | "
        f"markers={len(data['marker_semantics'])} | "
        f"operations={len(data['operation_catalog'])} | "
        f"frameworks={len(data['frameworks'])} | "
        f"claims={len(data['claim_translation'])}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
