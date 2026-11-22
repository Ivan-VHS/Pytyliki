# Copyright (c) 2026 Ivan-VHS
# SPDX-License-Identifier: MIT
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import unittest
from pytyliki_watch import _linux_cpu_counters, _linux_memory, calculate_cpu_percent, parse_linux_snapshot

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

    def test_snapshot_reports_interrupts_and_process_count(self):
        stat = "cpu 100 0 20 300 0 0 0 0 0 0\nintr 123456 1 2\n"
        memory = "MemTotal: 4096 kB\nMemAvailable: 2048 kB\nSwapTotal: 0 kB\nSwapFree: 0 kB\n"
        sample = parse_linux_snapshot(stat, memory, process_count=12)
        self.assertEqual(sample["interrupts"], 123456)
        self.assertEqual(sample["process_count"], 12)
        self.assertEqual(sample["memory_available_bytes"], 2 * 1024 * 1024)
        self.assertIsNone(sample["cpu_percent"])

if __name__ == "__main__":
    unittest.main()
