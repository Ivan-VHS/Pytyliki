"""Regression tests for roadmap governance and CI scale validators."""
from __future__ import annotations
import copy
import unittest
from tools.validate_delivery_contracts import validate_governance, validate_scale, validate_workflow_scale

REPOSITORY = "Ivan-VHS/Pytyliki"
GOVERNANCE = {"schema":"anatidae.roadmap-governance.v1","version":1,"repository":"Ivan-VHS/Pytyliki","owner":"Ivan-VHS","change_control":{"changes_are_versioned":true,"semantic_status_changes_require_pull_request":true,"required_ci_before_merge":true,"expected_head_sha_required_for_branch_updates":true,"rollback_plan_required_for_behavior_changes":true},"status_rules":{"active_requires_existing_evidence":true,"active_requires_regression_verification":true,"extend_requires_explicit_gap":true,"extend_requires_next_acceptance_gate":true,"planned_requires_blocker":true,"planned_requires_exit_criteria":true,"status_claims_must_not_exceed_runtime_evidence":true},"priorities":{"P0":{"meaning":"Safety, data integrity, compatibility break or release blocker","review":"Maintainer review required before merge"},"P1":{"meaning":"Core workflow, correctness, maintainability or measurable capability gap","review":"Peer/maintainer review required before merge"},"P2":{"meaning":"Bounded improvement with a supported workaround","review":"Normal review and documented acceptance evidence"}},"release_controls":{"production_deployment_requires_protected_environment":true,"release_publication_requires_human_approval":true,"release_signing_requires_verified_identity":true,"untrusted_driver_or_firmware_execution_forbidden":true,"destructive_cleanup_requires_human_approval":true}}
SCALE = {"schema":"anatidae.ci-scale-policy.v1","version":1,"repository":"Ivan-VHS/Pytyliki","global_limits":{"max_jobs_per_workflow":4,"max_job_timeout_minutes":45,"max_parallel_matrix":3},"workflows":{".github/workflows/ci.yml":{"max_jobs":3,"max_job_timeout_minutes":35,"max_parallel_matrix":3},".github/workflows/quality-repeatability-native.yml":{"max_jobs":1,"max_job_timeout_minutes":15,"max_parallel_matrix":1}},"rules":{"workflow_concurrency_required":true,"workflow_cancellation_policy_must_be_explicit":true,"each_runner_job_requires_timeout":true,"matrix_jobs_require_max_parallel":true,"required_platform_jobs_must_not_be_removed":true}}

class DeliveryContractTests(unittest.TestCase):
    def test_governance_policy_is_valid(self):
        self.assertEqual(validate_governance(GOVERNANCE, REPOSITORY), [])

    def test_disabled_status_guard_is_rejected(self):
        candidate = copy.deepcopy(GOVERNANCE)
        candidate["status_rules"]["active_requires_existing_evidence"] = False
        self.assertTrue(validate_governance(candidate, REPOSITORY))

    def test_missing_priority_is_rejected(self):
        candidate = copy.deepcopy(GOVERNANCE)
        candidate["priorities"].pop("P2")
        self.assertTrue(validate_governance(candidate, REPOSITORY))

    def test_wrong_repository_is_rejected(self):
        self.assertTrue(validate_governance(GOVERNANCE, "other/project"))

    def test_scale_policy_is_valid_against_repository_contract(self):
        self.assertEqual(validate_scale(SCALE, REPOSITORY), [])

    def test_matrix_requires_bounded_parallelism_and_timeout(self):
        workflow = """jobs:
  test:
    runs-on: ubuntu-latest
    timeout-minutes: 10
    strategy:
      matrix:
        python: [3.10, 3.11]
    steps: []
"""
        errors = validate_workflow_scale(workflow, "test.yml", {
            "max_jobs": 1, "max_job_timeout_minutes": 20, "max_parallel_matrix": 2
        })
        self.assertTrue(any("concurrency" in e for e in errors))
        self.assertTrue(any("max-parallel" in e for e in errors))

    def test_timeout_over_budget_is_rejected(self):
        workflow = """concurrency:
  group: test-${{ github.ref }}
  cancel-in-progress: true
jobs:
  build:
    runs-on: ubuntu-latest
    timeout-minutes: 40
    steps: []
"""
        errors = validate_workflow_scale(workflow, "test.yml", {
            "max_jobs": 1, "max_job_timeout_minutes": 30, "max_parallel_matrix": 1
        })
        self.assertTrue(any("exceeds" in e for e in errors))

if __name__ == "__main__":
    unittest.main()
