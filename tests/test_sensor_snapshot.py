from __future__ import annotations

from pathlib import Path
import tempfile
import unittest

from tools.sensor_snapshot import collect_snapshot


class SensorSnapshotTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.sys = self.root / "sys"
        self.proc = self.root / "proc"
        (self.proc).mkdir()
        (self.proc / "cpuinfo").write_text("model name : Example CPU\n", encoding="utf-8")

    def test_collects_only_present_sources_with_provenance(self):
        cpu = self.sys / "devices/system/cpu/cpu0/cpufreq"
        cpu.mkdir(parents=True)
        (cpu / "scaling_cur_freq").write_text("3200000", encoding="ascii")
        (cpu / "scaling_governor").write_text("performance", encoding="ascii")
        zone = self.sys / "class/thermal/thermal_zone0"
        zone.mkdir(parents=True)
        (zone / "temp").write_text("47500", encoding="ascii")
        (zone / "type").write_text("package", encoding="ascii")
        report = collect_snapshot(self.sys, self.proc, system="Linux")
        self.assertEqual(report["status"], "available")
        metrics = {item["name"]: item for item in report["metrics"]}
        self.assertEqual(metrics["cpu0.frequency"]["value"], 3200.0)
        self.assertEqual(metrics["thermal.package"]["value"], 47.5)
        self.assertEqual(metrics["cpu0.governor"]["value"], "performance")
        self.assertEqual(metrics["thermal.package"]["quality"], "observed")

    def test_missing_sensors_are_not_fabricated(self):
        report = collect_snapshot(self.sys, self.proc, system="Linux")
        self.assertEqual(report["status"], "available")
        self.assertFalse(any(item["name"].startswith("thermal.") for item in report["metrics"]))
        self.assertIn("No readable thermal zones were exposed.", report["notes"])

    def test_non_linux_adapter_reports_unsupported(self):
        report = collect_snapshot(self.sys, self.proc, system="Windows")
        self.assertEqual(report["status"], "unsupported")
        self.assertEqual(report["metrics"], [])


if __name__ == "__main__":
    unittest.main()
