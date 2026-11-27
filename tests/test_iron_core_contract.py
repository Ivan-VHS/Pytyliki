"""Contract-level regression tests for Pytyliki IRON CORE."""

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

from validate_iron_core import validate_contract, validate_data  # noqa: E402


class IronCoreContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        errors, data = validate_contract(ROOT)
        if errors:
            raise AssertionError("\n".join(errors))
        assert data is not None
        cls.data = data

    def test_exactly_one_hundred_ordered_rules(self) -> None:
        ids = [rule["id"] for rule in self.data["rules"]]
        self.assertEqual(ids, [f"IRON-{number:03d}" for number in range(1, 101)])

    def test_rule_and_module_references_are_unique_and_valid(self) -> None:
        module_ids = [module["id"] for module in self.data["modules"]]
        self.assertEqual(len(module_ids), len(set(module_ids)))
        known_modules = set(module_ids)
        self.assertTrue(all(rule["module"] in known_modules for rule in self.data["rules"]))

    def test_every_rule_has_a_testable_acceptance_condition(self) -> None:
        for rule in self.data["rules"]:
            with self.subTest(rule=rule["id"]):
                self.assertGreaterEqual(len(rule["requirement"]), 20)
                self.assertGreaterEqual(len(rule["acceptance"]), 20)

    def test_open_pr_is_not_mislabeled_as_merged_implementation(self) -> None:
        pr_ids = {item["pr"] for item in self.data["open_work"]}
        for rule in self.data["rules"]:
            if rule["status"] == "open_pr":
                self.assertTrue(rule["tracking_prs"])
                self.assertTrue(set(rule["tracking_prs"]).issubset(pr_ids))

    def test_partial_and_implemented_statuses_have_existing_evidence(self) -> None:
        errors = validate_data(copy.deepcopy(self.data), ROOT)
        self.assertEqual(errors, [])
        for rule in self.data["rules"]:
            if rule["status"] in {"partial", "implemented"}:
                self.assertTrue(rule["evidence"])

    def test_privileged_and_physical_writes_are_denied_by_default(self) -> None:
        model = self.data["access_model"]
        self.assertEqual(model["default_mode"], "inspect")
        self.assertTrue(model["deny_by_default"])
        self.assertFalse(model["physical_writes_default_allowed"])
        self.assertFalse(model["hidden_network_allowed"])
        self.assertFalse(model["automatic_driver_or_firmware_install"])

    def test_unknown_module_is_rejected(self) -> None:
        broken = copy.deepcopy(self.data)
        broken["rules"][0]["module"] = "pyty.unknown"
        self.assertTrue(any("unknown module" in error for error in validate_data(broken, ROOT)))

    def test_open_pr_without_tracking_is_rejected(self) -> None:
        broken = copy.deepcopy(self.data)
        broken["rules"][0]["status"] = "open_pr"
        broken["rules"][0]["tracking_prs"] = []
        self.assertTrue(any("open_pr status requires" in error for error in validate_data(broken, ROOT)))

    def test_maturity_ladders_are_complete(self) -> None:
        ids = [item["id"] for item in self.data["maturity_levels"]]
        self.assertEqual(ids, [f"IRON-{number}" for number in range(10)])

    def test_low_level_contract_is_reused_as_the_target_boundary(self) -> None:
        related = self.data["related_contracts"]["low_level_contract"]
        self.assertEqual(related["schema"], "pytyliki.low-level-rules.v1")
        self.assertEqual(related["status"], "merged_in_main")
        self.assertEqual(related["rule_count"], 100)
        self.assertTrue((ROOT / related["path"]).is_file())
        self.assertTrue(all((ROOT / path).is_file() for path in related["implementation"]))

    def test_crosswalk_references_existing_iron_and_pll_rules(self) -> None:
        related = self.data["related_contracts"]
        iron_ids = {rule["id"] for rule in self.data["rules"]}
        low_level = json.loads((ROOT / related["low_level_contract"]["path"]).read_text(encoding="utf-8"))
        pll_ids = {rule["id"] for rule in low_level["rules"]}
        self.assertGreaterEqual(len(related["coverage_crosswalk"]), 20)
        for mapping in related["coverage_crosswalk"]:
            self.assertIn(mapping["iron_rule"], iron_ids)
            self.assertTrue(set(mapping["low_level_rules"]).issubset(pll_ids))


if __name__ == "__main__":
    unittest.main()
