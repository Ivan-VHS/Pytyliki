from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from tools.sysmon_pulse import (
    collect_snapshot,
    cpu_usage_percent,
    parse_cpu_ticks,
    parse_interrupts,
    parse_meminfo,
    render_snapshot,
)


class SysmonPulseTests(unittest.TestCase):
    def test_cpu_ticks_and_busy_delta(self):
        before = parse_cpu_ticks("cpu  100 2 30 60 8 4 3 1\ncpu0 10 0 3 6\n")
        after = parse_cpu_ticks("cpu  160 2 45 90 10 5 5 3\n")
        self.assertEqual(before, (208, 68))
        self.assertAlmostEqual(cpu_usage_percent(before, after), 55.0)

    def test_cpu_delta_without_elapsed_ticks_is_unknown(self):
        self.assertIsNone(cpu_usage_percent((10, 4), (10, 4)))

    def test_meminfo_uses_bytes_and_validates_totals(self):
        values = parse_meminfo(
            "MemTotal:  1024 kB\nMemAvailable: 256 kB\n"
            "SwapTotal: 512 kB\nSwapFree: 128 kB\n"
        )
        self.assertEqual(values["MemTotal"], 1024 * 1024)
        self.assertEqual(values["MemAvailable"], 256 * 1024)
        with self.assertRaises(ValueError):
            parse_meminfo(
                "MemTotal: 1 kB\nMemAvailable: 2 kB\n"
                "SwapTotal: 1 kB\nSwapFree: 0 kB\n"
            )

    def test_interrupt_counts_ignore_description(self):
        counts = parse_interrupts(
            "           CPU0       CPU1\n"
            "  0:         10         12  IO-APIC   2-edge timer\n"
            "NMI:          2          3  Non-maskable interrupts\n"
        )
        self.assertEqual(counts, {"0": 22, "NMI": 5})

    def test_snapshot_can_be_collected_from_a_fixture(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "stat").write_text("cpu 100 0 20 80 0 0 0 0\n", encoding="ascii")
            (root / "meminfo").write_text(
                "MemTotal: 4096 kB\nMemAvailable: 1024 kB\n"
                "SwapTotal: 2048 kB\nSwapFree: 1024 kB\n",
                encoding="ascii",
            )
            (root / "interrupts").write_text(
                "           CPU0\n  1: 12 IO-APIC timer\n", encoding="ascii"
            )
            snapshot = collect_snapshot(root)
        self.assertEqual(snapshot["schema"], "pytyliki.sysmon-pulse.v1")
        self.assertEqual(snapshot["interrupt_total"], 12)
        self.assertEqual(snapshot["memory_bytes"]["MemTotal"], 4096 * 1024)

    def test_text_output_has_threshold_values_without_ansi_by_default(self):
        snapshot = {
            "cpu_ticks": [200, 100],
            "memory_bytes": {
                "MemTotal": 1000, "MemAvailable": 200,
                "SwapTotal": 500, "SwapFree": 250,
            },
            "interrupts": {"0": 100},
            "interrupt_total": 100,
        }
        rendered = render_snapshot(snapshot, color=False)
        self.assertIn("RAM used:", rendered)
        self.assertIn("Swap used:", rendered)
        self.assertIn("Interrupts:", rendered)
        self.assertNotIn("\033[", rendered)


if __name__ == "__main__":
    unittest.main()
