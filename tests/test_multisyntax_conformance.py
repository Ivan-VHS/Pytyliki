from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from tools.multisyntax_conformance import (
    ConformanceError, load_manifest, run_conformance,
)


class MultiSyntaxConformanceTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.fixtures = self.root / "fixtures"
        self.fixtures.mkdir()
        self.source = self.fixtures / "sample.py"
        self.source.write_text("print(42)\n", encoding="utf-8")
        self.manifest = self.fixtures / "manifest.json"
        self.reaper = self.root / "reaper"
        (self.reaper / "reaper").mkdir(parents=True)
        (self.reaper / "pyproject.toml").write_text("[project]\nname='reaper'\n", encoding="utf-8")
        (self.reaper / "reaper" / "multisyntax.py").write_text("REGISTRY = True\n", encoding="utf-8")

    def write_manifest(self, cases):
        payload = {
            "schema": "pytyliki.reaper-syntax-conformance.v1",
            "cases": cases,
        }
        self.manifest.write_text(json.dumps(payload), encoding="utf-8")

    def test_loads_and_confines_fixture_paths(self):
        self.write_manifest([{
            "id": "python-basic", "syntax": "python",
            "source": "sample.py", "expected_valid": True,
        }])
        base, cases = load_manifest(self.manifest)
        self.assertEqual(base, self.fixtures.resolve())
        self.assertEqual(cases[0]["source"], self.source.resolve())

    def test_rejects_path_traversal_unknown_syntax_and_duplicates(self):
        for case in (
            {"id": "escape", "syntax": "python", "source": "../outside.py", "expected_valid": True},
            {"id": "unknown", "syntax": "powershell", "source": "sample.py", "expected_valid": True},
            {"id": "bad", "syntax": "python", "source": "sample.py", "expected_valid": "yes"},
        ):
            self.write_manifest([case])
            with self.assertRaises(ConformanceError):
                load_manifest(self.manifest)
        duplicate = {
            "id": "same", "syntax": "python",
            "source": "sample.py", "expected_valid": True,
        }
        self.write_manifest([duplicate, duplicate])
        with self.assertRaisesRegex(ConformanceError, "duplicate"):
            load_manifest(self.manifest)

    def test_rejects_symbolic_link_fixture(self):
        link = self.fixtures / "linked.py"
        link.symlink_to(self.source)
        self.write_manifest([{
            "id": "linked", "syntax": "python",
            "source": "linked.py", "expected_valid": True,
        }])
        with self.assertRaisesRegex(ConformanceError, "symbolic links"):
            load_manifest(self.manifest)

    def test_runs_validator_without_shell_and_checks_expected_invalid_cases(self):
        self.write_manifest([
            {"id": "valid", "syntax": "python", "source": "sample.py", "expected_valid": True},
            {"id": "invalid", "syntax": "brainfuck", "source": "sample.py", "expected_valid": False},
        ])
        calls = []
        responses = [
            subprocess.CompletedProcess([], 0, '{"valid": true}', ""),
            subprocess.CompletedProcess([], 1, "", "unmatched bracket"),
        ]

        def runner(command, **kwargs):
            calls.append((command, kwargs))
            return responses.pop(0)

        report = run_conformance(
            self.reaper, self.manifest, python_executable="python-test", runner=runner
        )
        self.assertEqual(report["status"], "passed")
        self.assertEqual(report["passed"], 2)
        self.assertFalse(report["source_execution_performed"])
        self.assertFalse(calls[0][1]["shell"])
        self.assertEqual(calls[0][1]["cwd"], self.reaper.resolve())
        self.assertIn("--check", calls[0][0])
        self.assertIn("--json", calls[0][0])

    def test_timeout_is_reported_as_a_failed_case(self):
        self.write_manifest([{
            "id": "slow", "syntax": "python",
            "source": "sample.py", "expected_valid": True,
        }])
        def runner(*args, **kwargs):
            raise subprocess.TimeoutExpired(cmd=args[0], timeout=kwargs["timeout"])
        report = run_conformance(self.reaper, self.manifest, runner=runner)
        self.assertEqual(report["failed"], 1)
        self.assertIn("exceeded", report["cases"][0]["stderr"])

    def test_requires_an_explicit_reaper_checkout(self):
        with self.assertRaises(ConformanceError):
            run_conformance(self.root / "not-reaper", self.manifest)


if __name__ == "__main__":
    unittest.main()
