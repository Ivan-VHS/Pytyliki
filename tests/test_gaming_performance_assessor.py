from __future__ import annotations

import unittest

from tools.gaming_performance_assessor import (
    GamingAssessmentError,
    assess_game_session,
)


class GamingPerformanceAssessorTests(unittest.TestCase):
    def test_summarizes_measured_host_pressure_without_per_process_claims(self):
        report = assess_game_session(
            [
                {"cpu_percent": 20, "memory_percent": 55, "swap_percent": 0},
                {"cpu_percent": 96, "memory_percent": 93, "swap_percent": 35},
                {"cpu_percent": 84, "memory_percent": 88, "swap_percent": 12},
            ],
            game_id="sample-game",
        )
        self.assertEqual(report["schema"], "pytyliki.gaming-performance-assessment.v1")
        self.assertEqual(report["metrics"]["cpu_percent"]["maximum"], 96.0)
        self.assertEqual(report["metrics"]["memory_percent"]["p95"], 93.0)
        self.assertEqual(report["scope"], "whole_host")
        self.assertFalse(report["per_process_attribution"])
        self.assertFalse(report["host_settings_changed"])

    def test_computes_frame_time_and_low_fps_from_measured_values(self):
        report = assess_game_session(
            [{"cpu_percent": 45, "memory_percent": 60, "swap_percent": 0}],
            game_id="sample",
            frame_times_ms=[10.0] * 99 + [50.0],
        )
        frame = report["frame_time"]
        self.assertEqual(frame["sample_count"], 100)
        self.assertEqual(frame["average_fps"], 96.15)
        self.assertIsNone(frame["point_one_percent_low_fps"])
        self.assertFalse(frame["point_one_percent_low_available"])

    def test_0_1_low_requires_at_least_one_thousand_frames(self):
        report = assess_game_session(
            [{"cpu_percent": 10, "memory_percent": 20, "swap_percent": 0}],
            game_id="sample",
            frame_times_ms=[10.0] * 999 + [50.0],
        )
        self.assertEqual(report["frame_time"]["sample_count"], 1000)
        self.assertTrue(report["frame_time"]["point_one_percent_low_available"])

    def test_missing_metrics_remain_unknown(self):
        report = assess_game_session([{}], game_id="sample")
        self.assertIsNone(report["metrics"]["cpu_percent"]["p95"])
        self.assertIsNone(report["frame_time"])
        self.assertTrue(any("remain unknown" in item for item in report["recommendations"]))

    def test_rejects_bad_values_and_unbounded_input(self):
        with self.assertRaises(GamingAssessmentError):
            assess_game_session([{"cpu_percent": 101}], game_id="sample")
        with self.assertRaises(GamingAssessmentError):
            assess_game_session([{"memory_percent": float("nan")}], game_id="sample")
        with self.assertRaises(GamingAssessmentError):
            assess_game_session([{}], game_id="sample", frame_times_ms=[0.001])
        with self.assertRaises(GamingAssessmentError):
            assess_game_session([], game_id="sample")
        with self.assertRaises(GamingAssessmentError):
            assess_game_session([{}], game_id="sample", thresholds={"fan_rpm": 1000})


if __name__ == "__main__":
    unittest.main()
