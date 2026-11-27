from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest

SCRIPT= Path(__file__).resolve().parents[1] / "tools" / "compare_suite_runs.py"
SPEC = importlib.util.spec_from_file_location("compare_suite_runs", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


def make_report(values, *, profile=None, workload=None, warmup=None, integrity=True):
    profile = profile or {
        "architecture": "x86_64", "cpu": $", "vendor": "Example Vendor",
        "logical_processors": 8,
    }
    workload = workload or {"iterations": 12000000, "memory_mib": 8, "passes": 4}
    runs = []
    if warmup is not None:
        runs.append({"warmup": True, "integrity_ok": True, **workload,
                     "integer_ms": warmup, "memory_ms": warmup, "matrix_ms": warmup})
    for i, value in enumerate(values):
        runs.append({"warmup": False, "integrity_ok": integrity, **workload,
                     "integer_ms": value, "memory_ms": value * 2, "matrix_ms": value * 3})
    return {"schema": "pytyliki.report.v1", "profile": profile,
            "suite": {"schema": "pytyliki.suite.v1", "repeat": len(runs),
                      "measured_runs": len(values), "runs": runs}}


class SuiteComparisonTests(unittest.TestCase):
    def test_detects_improvement_beyond_noise(self):
        result = module.compare_reports(make_report([99, 100, 101, 102]), make_report([89, 90, 91, 92]))
        self.assertEqual(result["outcome"], "improvement")
        self.assertEqual(result["metrics"]["integer_ms"]["status"], "improved")
        self.assertAlmostEqual(result["metrics"]["integer_ms"]["baseline"]["median_ms"], 100.5)

    def test_marks_small_change_within_noise(self):
        result = module.compare_reports(make_report([99, 100, 101]), make_report([99.8, 100.3, 100.8]))
        self.assertEqual(result["metrics"]["integer_ms"]["status"], "within_noise")

    def test_excludes_warmup_runs(self):
        result = module.compare_reports(make_report([99, 100, 101], warmup=9999), make_report([89, 90, 91], warmup=1))
        self.assertEqual(result["metrics"]["integer_ms"]["baseline"]["median_ms"], 100)
        self.assertEqual(result["metrics"]["integer_ms"]["baseline"]["samples"], 3)

    def test_reports_host_mismatch_as_not_comparable(self):
        other = {"architecture": "arm64", "cpu": "Other CPU", "vendor": "Other", "logical_processors": 8}
        result = module.compare_reports(make_report([1, 2]), make_report([1, 2], profile=other))
        self.assertFalse(result["comparable"])
        self.assertEqual(result["outcome"], "not_comparable")

    def test_reports_workload_mismatch_as_not_comparable(self):
        other = {"iterations": 100, "memory_mib": 8, "passes": 4}
        result = module.compare_reports(make_report([1, 2]), make_report([1, 2], workload=other))
        self.assertFalse(result["comparable"])

    def test_formats_host_mismatch_without_crashing(self):
        other = {"architecture": "arm64", "cpu": "Other CPU", "vendor": "Other", "logical_processors": 8}
        result = module.compare_reports(make_report([1, 2]), make_report([1, 2], profile=other))
        summary = module.format_summary(result)
        self.assertIn("not_comparable", summary)
        self.assertIn("processor identity differs", summary)

    def test_rejects_failed_integrity_check(self):
        with self.assertRaises(ValueError):
            module.compare_reports(make_report([1, 2]), make_report([1, 2], integrity=False))


    def test_requires_two_measured_runs(self):
        with self.assertRaises(ValueError):
            module.compare_reports(make_report([1]), make_report([1, 2]))

    def test_rejects_non_finite_measurement(self):
        report = make_report([1, 2])
        report["suite"]["runs"][0]["integer_ms"] = float("nan")
        with self.assertRaises(ValueError):
            module.compare_reports(report, make_report([1, 2]))

    def test_rejects_invalid_noise_band(self):
        with self.assertRaises(ValueError):
            module.compare_reports(make_report([1, 2]), make_report([1, 2]), 101)


if __name__ == "__main__":
    unittest.main()
