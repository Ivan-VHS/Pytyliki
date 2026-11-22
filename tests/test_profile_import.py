from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.profile_import import ProfileError, parse_collapsed, parse_speedscope, render_folded, render_svg


class ProfileImportTests(unittest.TestCase):
    def test_collapsed_profiles_aggregate_weights_and_render(self):
        report = parse_collapsed("main;worker;parse 3\nmain;worker;parse 2\nmain;worker;write 1\n")
        self.assertEqual(report["total_weight"], 6.0)
        self.assertEqual(report["sample_count"], 2)
        folded = render_folded(report)
        self.assertIn("main;worker;parse 5", folded)
        svg = render_svg(report, width=640)
        self.assertIn("<svg", svg)
        self.assertIn("parse", svg)

    def test_speedscope_sampled_json(self):
        document = {
            "$schema": "https://www.speedscope.app/file-format-schema.json",
            "shared": {"frames": [{"name": "main"}, {"name": "compute"}]},
            "profiles": [{"type": "sampled", "samples": [[0, 1], [0, 1], [0]], "weights": [2, 3, 1]}],
        }
        report = parse_speedscope(document)
        self.assertEqual(report["total_weight"], 6.0)
        self.assertEqual(report["frames"][0]["name"], "main")
        self.assertEqual(report["frames"][0]["total_weight"], 6.0)

    def test_rejects_evented_or_invalid_profile(self):
        with self.assertRaises(ProfileError):
            parse_speedscope({"shared": {"frames": []}, "profiles": [{"type": "evented", "events": []}]})
        with self.assertRaises(ProfileError):
            parse_collapsed("main;worker not-a-number")

    def test_limits_and_unsafe_frame_names(self):
        with self.assertRaises(ProfileError):
            parse_collapsed("main;bad\x00frame 1")
        with self.assertRaises(ProfileError):
            parse_speedscope({"shared": {"frames": [{"name": "main"}]},
                              "profiles": [{"type": "sampled", "samples": [[4]], "weights": [1]}]})

    def test_render_svg_escapes_frame_names(self):
        report = parse_collapsed("main;<script>alert 1")
        svg = render_svg(report)
        self.assertNotIn("<script>", svg)
        self.assertIn("&lt;script&gt;", svg)


if __name__ == "__main__":
    unittest.main()
