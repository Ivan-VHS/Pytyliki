"""Contract regression tests for the Omega-Null research profile."""

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

from validate_omega_null import validate_data, validate_profile  # noqa: E402


class OmegaNullContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        errors, data = validate_profile(ROOT / "omega-null" / "omega-null.v0.json")
        if errors:
            raise AssertionError("\n".join(errors))
        assert data is not None
        cls.data = data

    def test_twenty_source_markers_are_preserved_and_normalized(self) -> None:
        self.assertEqual(len(self.data["marker_semantics"]), 20)
        self.assertEqual(len({item["marker"] for item in self.data["marker_semantics"]}), 20)
        self.assertEqual(len({item["normalized"] for item in self.data["marker_semantics"]}), 20)

    def test_operation_catalog_is_explicit_and_not_marked_shipped(self) -> None:
        self.assertEqual(len(self.data["operation_catalog"]), 12)
        for operation in self.data["operation_catalog"]:
            with self.subTest(operation=operation["id"]):
                self.assertTrue(operation["target"])
                self.assertTrue(operation["effect"])
                self.assertTrue(operation["constraints"])
                self.assertNotEqual(operation["status"], "implemented")

    def test_quantum_measurement_does_not_force_outcomes(self) -> None:
        op = next(item for item in self.data["operation_catalog"] if item["id"] == "quantum.measure")
        self.assertIn("measurement", op["effect"])
        self.assertIn(
            "force a chosen outcome",
            " ".join(op["constraints"]).lower(),
        )

    def test_time_replay_is_not_physical_time_travel(self) -> None:
        op = next(item for item in self.data["operation_catalog"] if item["id"] == "chrono.replay")
        constraints = " ".join(op["constraints"]).lower()
        self.assertIn("reverse external physical events", constraints)
        self.assertIn("earlier real time", constraints)

    def test_fabrication_requires_real_process_inputs(self) -> None:
        op = next(item for item in self.data["operation_catalog"] if item["id"] == "fabrication.plan")
        text = " ".join(op["constraints"]).lower()
        self.assertIn("feedstock", text)
        self.assertIn("operator approval", text)

    def test_bci_requires_consent(self) -> None:
        op = next(item for item in self.data["operation_catalog"] if item["id"] == "bci.capture")
        self.assertIn("consent", " ".join(op["constraints"]).lower())

    def test_target_operations_require_named_capabilities(self) -> None:
        for operation in self.data["operation_catalog"]:
            with self.subTest(operation=operation["id"]):
                if "target" in operation["mode"]:
                    self.assertNotEqual(operation["required_capability"], "none")

    def test_fabrication_requires_operator_approval(self) -> None:
        operation = next(item for item in self.data["operation_catalog"] if item["id"] == "fabrication.plan")
        self.assertTrue(operation["operator_approval_required"])

    def test_all_four_frameworks_are_mapped(self) -> None:
        self.assertEqual(
            {item["id"] for item in self.data["frameworks"]},
            {"aegis-shield", "chrono-weave", "matter-forge", "mind-link"},
        )

    def test_experimental_domains_map_to_named_modules(self) -> None:
        self.assertGreaterEqual(len(self.data["research_domains"]), 9)
        for domain in self.data["research_domains"]:
            self.assertTrue(domain["module"].startswith("pyty."))

    def test_three_verification_gates_are_ordered(self) -> None:
        ids = [item["id"] for item in self.data["engineering_practices"]["verification_gates"]]
        self.assertEqual(ids, [
            "syntax_type_effect",
            "model_invariants",
            "physical_target_qualification",
        ])

    def test_no_physical_reality_mutation_api(self) -> None:
        access = self.data["access_model"]
        self.assertTrue(access["deny_by_default"])
        self.assertTrue(access["no_implicit_hardware_access"])
        self.assertTrue(access["no_reality_mutation_api"])

    def test_missing_quantum_safety_constraint_is_rejected(self) -> None:
        broken = copy.deepcopy(self.data)
        op = next(item for item in broken["operation_catalog"] if item["id"] == "quantum.measure")
        op["constraints"] = ["Return one result."]
        self.assertTrue(any("forced measurement outcomes" in error for error in validate_data(broken)))


if __name__ == "__main__":
    unittest.main()
