"""Regression tests for the Processor IDE machine-readable contract."""

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

from validate_processor_ide import validate_contract, validate_data  # noqa: E402


class ProcessorIDEContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        errors, data = validate_contract(ROOT)
        if errors:
            raise AssertionError("\n".join(errors))
        assert data is not None
        cls.data = data

    def test_contract_has_two_ordered_fifty_rule_groups(self) -> None:
        rules = self.data["rules"]
        self.assertEqual(len(rules), 100)
        self.assertEqual([rule["id"] for rule in rules], [f"PIDE-{n:03d}" for n in range(1, 101)])
        self.assertEqual(sum(rule["group"] == "IDE_CORE" for rule in rules), 50)
        self.assertEqual(sum(rule["group"] == "SILICON_DEPTH" for rule in rules), 50)

    def test_reuses_the_existing_twenty_workspace_registry(self) -> None:
        registry = self.data["related_contracts"]["system_workspace_registry"]
        self.assertEqual(registry["existing_count"], 20)
        for key in ("header", "source", "tests"):
            self.assertTrue((ROOT / registry[key]).is_file())
        self.assertEqual(len(self.data["workspaces"]), 20)
        self.assertEqual(len({workspace["id"] for workspace in self.data["workspaces"]}), 20)

    def test_native_target_session_foundation_is_present(self) -> None:
        native = self.data["related_contracts"]["native_session_foundation"]
        self.assertEqual(native["api"], "pl_ide_session_init")
        for key in ("header", "source", "tests", "build"):
            self.assertTrue((ROOT / native[key]).is_file())

    def test_every_rule_uses_a_known_workspace(self) -> None:
        workspaces = {item["id"] for item in self.data["workspaces"]}
        for rule in self.data["rules"]:
            with self.subTest(rule=rule["id"]):
                self.assertIn(rule["workspace"], workspaces)

    def test_twenty_named_engineering_perspectives_are_preserved(self) -> None:
        perspectives = self.data["perspectives"]
        self.assertEqual(len(perspectives), 20)
        self.assertEqual(len({item["id"] for item in perspectives}), 20)
        workspaces = {item["id"] for item in self.data["workspaces"]}
        for perspective in perspectives:
            with self.subTest(perspective=perspective["id"]):
                self.assertIn(perspective["workspace"], workspaces)
                self.assertEqual(perspective["authority"], "no_privilege_elevation")
                self.assertEqual(perspective["scope_kind"], "workspace_perspective")

    def test_partial_status_is_backed_by_existing_evidence(self) -> None:
        errors = validate_data(copy.deepcopy(self.data), ROOT)
        self.assertEqual(errors, [])
        for rule in self.data["rules"]:
            if rule["status"] in {"partial", "implemented"}:
                self.assertTrue(rule["evidence"])

    def test_default_mode_is_inspect_and_hardware_access_is_deny_by_default(self) -> None:
        self.assertEqual(self.data["shell"]["default_mode"], "inspect")
        safety = self.data["safety"]
        self.assertTrue(safety["deny_by_default"])
        self.assertTrue(safety["no_implicit_hardware_access"])
        self.assertTrue(safety["no_unreviewed_mmio_or_privileged_write"])

    def test_machine_facts_are_target_specific(self) -> None:
        adjustments = " ".join(self.data["safety_adjustments"]).lower()
        self.assertIn("endianness", adjustments)
        self.assertIn("cache geometry", adjustments)
        self.assertIn("instruction length", adjustments)
        self.assertIn("privilege", adjustments)

    def test_signal_palette_preserves_electrical_states(self) -> None:
        palette = self.data["signal_palette"]
        states = {item["id"]: item for item in palette["states"]}
        self.assertEqual(set(states), {"high", "low", "floating", "contended", "unknown"})
        self.assertEqual(states["high"]["default_color"], "#E53935")
        self.assertEqual(states["low"]["default_color"], "#070707")
        self.assertEqual(states["floating"]["default_color"], "#2878D0")
        self.assertEqual(palette["theme_independent"], True)
        self.assertTrue(palette["accessibility"])

    def test_exact_cycles_require_a_declared_model(self) -> None:
        rule = next(item for item in self.data["rules"] if item["id"] == "PIDE-090")
        self.assertEqual(rule["status"], "planned")
        self.assertIn("declared microarchitecture", rule["requirement"].lower())

    def test_unknown_workspace_is_rejected(self) -> None:
        broken = copy.deepcopy(self.data)
        broken["rules"][0]["workspace"] = "unlisted-workspace"
        self.assertTrue(any("unknown workspace" in error for error in validate_data(broken, ROOT)))

    def test_missing_evidence_for_partial_rule_is_rejected(self) -> None:
        broken = copy.deepcopy(self.data)
        partial = next(item for item in broken["rules"] if item["status"] == "partial")
        partial["evidence"] = []
        self.assertTrue(any("requires evidence" in error for error in validate_data(broken, ROOT)))

    def test_silent_hardware_mutation_is_rejected(self) -> None:
        broken = copy.deepcopy(self.data)
        broken["safety"]["no_implicit_hardware_access"] = False
        self.assertTrue(any("no_implicit_hardware_access" in error for error in validate_data(broken, ROOT)))


if __name__ == "__main__":
    unittest.main()
