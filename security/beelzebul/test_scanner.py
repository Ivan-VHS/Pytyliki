"""Regression, privacy and deterministic fuzz tests for Beelzebul."""
from __future__ import annotations
import json, random, unittest
from pathlib import Path
from scanner import correlate, inspect_file, score_findings, validate_policy, workflow_has_unscoped_write

class ScannerRegressionTests(unittest.TestCase):
    def test_secret_is_detected_but_never_echoed(self):
        fake="ghp_" + ("A"*40)
        findings=inspect_file("src/config.py",('ACCESS_TOKEN = "'+fake+'"\n').encode(),2097152)
        encoded=json.dumps(findings)
        self.assertTrue(any(x["rule_id"]=="A01" for x in findings))
        self.assertNotIn(fake,encoded)
        self.assertTrue(all("sample" not in x for x in findings))
    def test_invisible_controls_and_conflict_markers(self):
        source="value = 1\n"+"name = 'x'\u202e\n"+"<<<<<<< branch\n"
        ids={x["rule_id"] for x in inspect_file("src/module.py",source.encode("utf-8"),2097152)}
        self.assertIn("A05",ids);self.assertIn("A08",ids)
    def test_unsafe_process_sink_is_classified(self):
        findings=inspect_file("src/runner.py",b"subprocess.run(command, shell=True)\n",2097152)
        self.assertIn("M02",{x["rule_id"] for x in findings})
    def test_oversize_guard_is_bounded(self):
        self.assertEqual(inspect_file("src/blob.py",b"x"*128,64)[0]["rule_id"],"A10")
    def test_seeded_fuzz_inputs_do_not_crash_parser(self):
        rng=random.Random(20261121)
        alphabet=b"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789{}[]()'\\\";:=_+-*/\\\\ \t\n"
        for index in range(300):
            payload=bytes(rng.choice(alphabet) for _ in range(rng.randrange(0,2048)))
            inspect_file(f"fuzz/{index}.py",payload,4096)
    def test_risk_score_is_stable(self):
        findings=[{"severity":"high"},{"severity":"medium"},{"severity":"low"}]
        self.assertEqual(score_findings(findings,{}),score_findings(findings,{}))
        self.assertEqual(score_findings(findings,{})["risk_score"],18)
    def test_correlations_ignore_scanner_and_policy_metadata(self):
        findings = []
        sources = {
            "security/beelzebul/scanner.py": (
                "password token sha1 "
                "logger.info(value)"
            ),
            ".beelzebul/policy.v1.json": '{"password": "token", "sha1": "auth"}',
        }
        correlate(findings, sources, set(sources))
        self.assertFalse([item for item in findings if item["rule_id"].startswith("Q")])

    def test_secret_log_correlation_requires_same_line_evidence(self):
        unrelated = []
        correlate(
            unrelated,
            {"src/module.py": "token = read_token()\nlogger.info('request complete')"},
        )
        self.assertFalse([item for item in unrelated if item["rule_id"] == "Q05"])

        direct = []
        source = "logger" + ".info('token=%s', token)"
        correlate(direct, {"src/module.py": source})
        self.assertTrue([item for item in direct if item["rule_id"] == "Q05"])

    def test_repository_correlation_uses_only_changed_files(self):
        findings = [
            {"rule_id": "M01", "path": "legacy/a.py", "severity": "high"},
            {"rule_id": "M01", "path": "legacy/b.py", "severity": "high"},
            {"rule_id": "M01", "path": "legacy/c.py", "severity": "high"},
            {"rule_id": "M01", "path": "legacy/d.py", "severity": "high"},
        ]
        correlate(findings, {}, set())
        self.assertFalse([item for item in findings if item["rule_id"] == "Q14"])

    def test_release_write_permission_is_tag_gated_and_job_scoped(self):
        safe_release = """permissions:
  contents: read
jobs:
  build-windows:
    runs-on: windows-latest
  publish-release:
    if: startsWith(github.ref, 'refs/tags/')
    needs: build-windows
    permissions:
      contents: write
"""
        broad_release = safe_release.replace(
            "if: startsWith(github.ref, 'refs/tags/')",
            "if: always()",
        )
        writable_build = safe_release.replace(
            "  build-windows:\n    runs-on: windows-latest",
            "  build-windows:\n    permissions:\n      contents: write\n    runs-on: windows-latest",
        )
        global_write = safe_release.replace(
            "  contents: read",
            "  contents: write",
        )
        self.assertFalse(workflow_has_unscoped_write(safe_release))
        self.assertTrue(workflow_has_unscoped_write(broad_release))
        self.assertTrue(workflow_has_unscoped_write(writable_build))
        self.assertTrue(workflow_has_unscoped_write(global_write))

    def test_auto_merge_write_permission_requires_independent_review(self):
        safe_auto_merge = """permissions:
  contents: read
jobs:
  validate:
    permissions:
      contents: read
  sanitizer:
    permissions:
      contents: read
  auto-merge:
    if: github.event_name != 'workflow_dispatch' && github.event.pull_request.draft == false && github.event.pull_request.head.repo.full_name == github.repository
    needs: [validate, sanitizer]
    permissions:
      actions: read
      contents: write
      pull-requests: write
    steps:
      - name: Wait for every pull-request workflow on the validated head
        env:
          EXPECTED_HEAD_SHA: sha
      - name: Confirm validated PR head is unchanged
      - name: Require independent current-head approval and resolved review threads
        env:
          REVIEW_STATE: missing-current-head-human-approval
        run: |
          reviewThreads(first: 100) { nodes { isResolved } }
      - name: Enable squash auto-merge
"""
        unsafe_auto_merge = safe_auto_merge.replace(
            "Require independent current-head approval and resolved review threads",
            "Approve without reviewer checks",
        )
        self.assertFalse(workflow_has_unscoped_write(safe_auto_merge))
        self.assertTrue(workflow_has_unscoped_write(unsafe_auto_merge))

    def test_policy_contract_has_66_unique_rules(self):
        findings=[];policy=validate_policy(Path(__file__).resolve().parents[2],findings)
        self.assertEqual(policy.get("codex"),"BEELZEBUL-66")
        self.assertFalse([x for x in findings if x["rule_id"]=="G01"])
if __name__=="__main__":unittest.main()
