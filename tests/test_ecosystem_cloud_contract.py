import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ADAPTER = json.loads(
    (ROOT / "federation" / "ecosystem-cloud-adapter.v1.json").read_text(encoding="utf-8")
)


class EcosystemCloudAdapterContractTests(unittest.TestCase):
    def test_contract_identity_and_project_scope(self):
        self.assertEqual(ADAPTER["schema"], "ivan-vhs.ecosystem-cloud-adapter.v1")
        self.assertEqual(ADAPTER["version"], 1)
        self.assertEqual(ADAPTER["project"], "pytyliki")
        self.assertEqual(ADAPTER["project_key"], "pytyliki")
        self.assertEqual(ADAPTER["shared_contract"], "ivan-vhs.ecosystem-cloud.v1")
        self.assertEqual(ADAPTER["status"], "contract-only")

    def test_network_and_telemetry_are_opt_in(self):
        self.assertEqual(ADAPTER["runtime"]["default_network_mode"], "disabled")
        self.assertTrue(ADAPTER["runtime"]["user_opt_in_required"])
        self.assertFalse(ADAPTER["runtime"]["core_requires_network"])
        self.assertEqual(ADAPTER["runtime"]["supabase_transport"], "not-implemented")
        self.assertEqual(ADAPTER["privacy"]["telemetry"], "disabled-by-default")
        self.assertEqual(ADAPTER["privacy"]["device_fingerprint"], "never-derived-or-exported")

    def test_raw_device_identifiers_are_forbidden(self):
        denied = set(ADAPTER["denylist"])
        for value in (
            "raw-device-serial-numbers",
            "raw-pnp-instance-ids",
            "mac-addresses",
            "motherboard-serials",
            "raw-hardware-identifiers",
            "full-hardware-inventory",
            "access-tokens",
            "service-role-keys",
        ):
            with self.subTest(value=value):
                self.assertIn(value, denied)
        self.assertTrue(ADAPTER["synchronization"]["local_source_of_truth"])
        self.assertTrue(ADAPTER["synchronization"]["upload_requires_user_action"])


if __name__ == "__main__":
    unittest.main()
