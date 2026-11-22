"""Regression tests for the local Pytyliki benchmark archive utility."""

import json
from pathlib import Path
import tempfile
import unittest

from tools.benchmark_archive import (
    ARCHIVE_SCHEMA,
    ReportError,
    compare_reports,
    load_report,
    save_archive,
)


def make_report(integer_mean: float, memory_mean: float = 4.0,
                matrix_mean: float = 8.0, cpu: str = "Example CPU") -> dict:
    return {
        "schema": "pytyliki.report.v1",
        "profile": {
            "architecture": "x86_64",
            "vendor": "Example",
            "cpu": cpu,
        },
        "suite": {
            "schema": "pytyliki.suite.v1",
            "repeat": 3,
            "warmup_runs": 1,
            "measured_runs": 2,
            "runs": [
                {"run": 1, "warmup": True},
                {"run": 2, "warmup": False},
                {"run": 3, "warmup": False},
            ],
            "statistics": {
                "integer_ms": {"mean": integer_mean, "median": integer_mean},
                "memory_ms": {"mean": memory_mean, "median": memory_mean},
                "matrix_ms": {"mean": matrix_mean, "median": matrix_mean},
            },
        },
    }


class BenchmarkArchiveTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def write_report(self, name: str, report: dict) -> Path:
        path = self.root / name
        path.write_text(json.dumps(report), encoding="utf-8")
        return path

    def test_archive_and_compare(self) -> None:
        baseline = self.write_report("baseline.json", make_report(12.0))
        candidate = self.write_report("candidate.json", make_report(9.0))
        archive_path = self.root / "archives" / "baseline.archive.json"

        archive = save_archive(baseline, archive_path, "known-good")
        self.assertEqual(archive["schema"], ARCHIVE_SCHEMA)
        self.assertEqual(archive["label"], "known-good")
        self.assertEqual(load_report(archive_path)[0]["suite"]["measured_runs"], 2)

        result = compare_reports(archive_path, candidate)
        self.assertTrue(result["same_host_profile"])
        self.assertTrue(result["metrics"]["integer_ms"]["faster"])
        self.assertEqual(result["metrics"]["integer_ms"]["change_percent"], -25.0)

    def test_different_host_is_not_presented_as_same_machine(self) -> None:
        baseline = self.write_report("old.json", make_report(12.0, cpu="CPU A"))
        candidate = self.write_report("new.json", make_report(9.0, cpu="CPU B"))
        self.assertFalse(compare_reports(baseline, candidate)["same_host_profile"])

    def test_rejects_non_suite_and_inconsistent_measurement_count(self) -> None:
        invalid = make_report(12.0)
        invalid["suite"]["measured_runs"] = 4
        path = self.write_report("invalid.json", invalid)
        with self.assertRaises(ReportError):
            load_report(path)

    def test_refuses_to_overwrite_an_archive_by_default(self) -> None:
        source = self.write_report("source.json", make_report(12.0))
        destination = self.root / "archive.json"
        save_archive(source, destination)
        with self.assertRaises(ReportError):
            save_archive(source, destination)


if __name__ == "__main__":
    unittest.main()
