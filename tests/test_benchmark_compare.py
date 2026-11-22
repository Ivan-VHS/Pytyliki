"""Tests for repeatable Pytyliki benchmark comparison."""
import unittest

from scripts.compare_benchmarks import SCHEMA, compare_suites


def suite(runs):
    return {"schema": SCHEMA, "repeat": len(runs), "runs": runs}


class BenchmarkComparisonTests(unittest.TestCase):
    def test_ignores_warmup_and_compares_medians(self):
        baseline = suite([
            {"warmup": True, "integer_ms": 9999, "memory_ms": 9999, "matrix_ms": 9999},
            {"warmup": False, "integer_ms": 10, "memory_ms": 20, "matrix_ms": 30},
            {"warmup": False, "integer_ms": 12, "memory_ms": 22, "matrix_ms": 32},
        ])
        candidate = suite([
            {"warmup": False, "integer_ms": 9, "memory_ms": 24, "matrix_ms": 30},
            {"warmup": False, "integer_ms": 11, "memory_ms": 26, "matrix_ms": 34},
        ])
        result = compare_suites(baseline, candidate)
        metrics = result["metrics"]
        self.assertEqual(metrics["integer_ms"]["baseline_median"], 11)
        self.assertEqual(metrics["integer_ms"]["candidate_median"], 10)
        self.assertEqual(metrics["integer_ms"]["status"], "improvement")
        self.assertEqual(metrics["memory_ms"]["status"], "regression")
        self.assertEqual(metrics["matrix_ms"]["status"], "within_threshold")
        self.assertEqual(metrics["integer_ms"]["baseline_samples"], 2)

    def test_zero_baseline_is_not_divided(self):
        result = compare_suites(
            suite([{"warmup": False, "integer_ms": 0}]),
            suite([{"warmup": False, "integer_ms": 1}]),
        )
        metric = result["metrics"]["integer_ms"]
        self.assertIsNone(metric["change_percent"])
        self.assertEqual(metric["status"], "not_comparable_zero_baseline")

    def test_rejects_non_finite_and_negative_values(self):
        for value in (float("nan"), float("inf"), -1, True):
            with self.subTest(value=value):
                with self.assertRaises(ValueError):
                    compare_suites(
                        suite([{"warmup": False, "integer_ms": value}]),
                        suite([{"warmup": False, "integer_ms": 1}]),
                    )

    def test_rejects_unknown_schema(self):
        with self.assertRaises(ValueError):
            compare_suites({"schema": "other", "runs": []}, suite([{"integer_ms": 1}]))

    def test_requires_at_least_one_shared_metric(self):
        with self.assertRaisesRegex(ValueError, "no comparable"):
            compare_suites(
                suite([{"warmup": False, "unknown": 1}]),
                suite([{"warmup": False, "unknown": 2}]),
            )


if __name__ == "__main__":
    unittest.main()
