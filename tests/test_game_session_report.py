from __future__ import annotations

import unittest
import json
import tempfile
from pathlib import Path

from tools.game_session_report import analyze_file, build_game_session_report


class GameSessionReportTests(unittest.TestCase):
    def test_summarizes_host_load_and_pagefile_pressure(self):
        report = build_game_session_report("quake-era-title", [
            {"timestamp": 1, "cpu_percent": 20, "memory_percent": 55, "swap_percent": 0,
             "memory_total_bytes": 1000, "memory_available_bytes": 450},
            {"timestamp": 2, "cpu_percent": 96, "memory_percent": 93, "swap_percent": 35,
             "memory_total_bytes": 1000, "memory_available_bytes": 70},
            {"timestamp": 3, "cpu_percent": 84, "memory_percent": 88, "swap_percent": 12,
             "memory_total_bytes": 1000, "memory_available_bytes": 120},
        ])
        self.assertEqual(report["schema"], "pytyliki.game-session-report.v1")
        self.assertEqual(report["metrics"]["cpu_percent"]["maximum"], 96.0)
        self.assertEqual(report["minimum_available_memory_bytes"], 70)
        self.assertEqual(report["duration_seconds"], 2.0)
        self.assertEqual(report["scope"], "whole_host")
        self.assertFalse(report["per_process_attribution"])
        self.assertEqual(report["alert_count"], 3)

    def test_accepts_missing_first_cpu_sample(self):
        report = build_game_session_report("sample", [
            {"timestamp": 1, "cpu_percent": None, "memory_percent": 20, "swap_percent": 0},
            {"timestamp": 2, "cpu_percent": 45, "memory_percent": 25, "swap_percent": 0},
        ])
        self.assertEqual(report["metrics"]["cpu_percent"]["sample_count"], 1)
        self.assertEqual(report["metrics"]["cpu_percent"]["maximum"], 45.0)

    def test_custom_threshold_changes_alerts(self):
        report = build_game_session_report(
            "sample",
            [{"cpu_percent": 65, "memory_percent": 40, "swap_percent": 0}],
            {"cpu_percent": 60},
        )
        self.assertEqual(report["alert_count"], 1)
        self.assertEqual(report["alerts"][0]["metric"], "cpu_percent")

    def test_reads_pytyliki_watch_jsonl_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            source = Path(temporary) / "session.jsonl"
            rows = [
                {"timestamp": 1, "cpu_percent": 20, "memory_percent": 40, "swap_percent": 0},
                {"timestamp": 2, "cpu_percent": 80, "memory_percent": 70, "swap_percent": 10},
            ]
            source.write_text("\n".join(json.dumps(row) for row in rows) + "\n", encoding="utf-8")
            report = analyze_file(source, "sample-game")
            single = Path(temporary) / "single.jsonl"
            single.write_text(json.dumps(rows[0]) + "\n", encoding="utf-8")
            single_report = analyze_file(single, "single-sample")
        self.assertEqual(report["sample_count"], 2)
        self.assertEqual(report["duration_seconds"], 1.0)
        self.assertEqual(single_report["sample_count"], 1)

    def test_rejects_unbounded_or_invalid_samples(self):
        with self.assertRaises(ValueError):
            build_game_session_report("sample", [])
        with self.assertRaises(ValueError):
            build_game_session_report("sample", [{"cpu_percent": 120, "memory_percent": 10, "swap_percent": 0}])
        with self.assertRaises(ValueError):
            build_game_session_report("sample", [{"cpu_percent": 20, "memory_percent": 10, "swap_percent": 0,
                                                  "memory_total_bytes": 4, "memory_available_bytes": 5}])


if __name__ == "__main__":
    unittest.main()
