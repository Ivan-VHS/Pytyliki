import json
import tempfile
import unittest
from pathlib import Path
from scripts.meta.analyze_telemetry import load_records, shannon_entropy

class TelemetryTests(unittest.TestCase):
    def test_entropy_single_outcome(self):
        self.assertEqual(shannon_entropy({"success": 3}), 0.0)

    def test_load_records_ignores_invalid_json(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "valid.json").write_text(json.dumps({
                "schema_version": 1, "checks": {}, "result": "success"
            }), encoding="utf-8")
            (root / "invalid.json").write_text("{", encoding="utf-8")
            self.assertEqual(len(load_records(root)), 1)

if __name__ == "__main__":
    unittest.main()
