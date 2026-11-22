# Copyright (c) 2026 Ivan-VHS
# SPDX-License-Identifier: MIT
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import unittest
from pytyliki_watch import (_linux_cpu_counters, _linux_load_average, _linux_memory,
                            analyze_samples, calculate_cpu_percent, parse_linux_snapshot)

class PytylikiWatchTests(unittest.TestCase):
    def test_cpu_counters_include_iowait_as_idle(self):
        total, idle = _linux_cpu_counters("cpu  100 20 30 400 50 4 5 6 7 8\n")
        self.assertEqual(total, 630)
        self.assertEqual(idle, 450)

    def test_cpu_percent_uses_counter_deltas(self):
        self.assertEqual(calculate_cpu_percent((1000, 600), (1100, 650)), 50.0)
        self.assertIsNone(calculate_cpu_percent(None, (1100, 650)))
        self.assertIsNone(calculate_cpu_percent((100, 50), (100, 50)))

    def test_memory_parser_uses_available_and_swap(self):
        values = "MemTotal: 8192 kB\nMemFree: 1024 kB\nMemAvailable: 2048 kB\nSwapTotal: 4096 kB\nSwapFree: 3072 kB\n"
        self.assertEqual(_linux_memory(values), (8 * 1024 * 1024, 2 * 1024 * 1024, 4 * 1024 * 1024, 3 * 1024 * 1024))

    def test_snapshot_reports_interrupts_processes_and_load(self):
        stat = "cpu 100 0 20 300 0 0 0 0 0 0\nintr 123456 1 2\n"
        memory = "MemTotal: 4096 kB\nMemAvailable: 2048 kB\nSwapTotal: 0 kB\nSwapFree: 0 kB\n"
        sample = parse_linux_snapshot(stat, memory, process_count=12, loadavg_text="1.25 0.80 0.60 2/100 500")
        self.assertEqual(sample["interrupts"], 123456)
        self.assertEqual(sample["process_count"], 12)
        self.assertEqual(sample["memory_available_bytes"], 2 * 1024 * 1024)
        self.assertEqual(sample["load_average"], (1.25, 0.8, 0.6))
        self.assertIsNone(sample["cpu_percent"])

    def test_load_average_rejects_incomplete_input(self):
        self.assertIsNone(_linux_load_average("broken 1"))

    def test_analysis_builds_threshold_alerts_and_percentiles(self):
        rows = [
            {"timestamp": 1, "cpu_percent": 20, "memory_percent": 50, "swap_percent": 0},
            {"timestamp": 2, "cpu_percent": 95, "memory_percent": 92, "swap_percent": 40},
            {"timestamp": 3, "cpu_percent": 80, "memory_percent": 70, "swap_percent": 10},
        ]
        result = analyze_samples(rows)
        self.assertEqual(result["schema"], "pytyliki.telemetry-analysis.v1")
        self.assertEqual(result["alert_count"], 3)
        self.assertEqual(result["metrics"]["cpu_percent"]["maximum"], 95)
        self.assertEqual(result["metrics"]["cpu_percent"]["p95"], 95)

    def test_analysis_accepts_custom_policy(self):
        result = analyze_samples(
            [{"timestamp": 5, "cpu_percent": 55, "memory_percent": 60, "swap_percent": 0}],
            {"cpu_percent": 50},
        )
        self.assertEqual(result["alert_count"], 1)
        self.assertEqual(result["alerts"][0]["metric"], "cpu_percent")

if __name__ == "__main__":
    unittest.main()
