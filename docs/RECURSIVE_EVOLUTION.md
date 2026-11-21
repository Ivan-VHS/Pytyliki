# Recursive evolution policy

| Stage | Required evidence |
|---|---|
| Observe | source, build, tests, docs, platform matrix |
| Diagnose | reproducible gap and root cause |
| Propose | bounded change, hypothesis, risks, rollback |
| Implement | dedicated branch; never push main |
| Verify | build, CTest, platform and regression checks |
| Review | PR diff, limitations, evidence |
| Promote | branch protection and required checks |

Automation may analyze and open PRs. A **human maintainer authorizes merge**. It must not bypass branch protection, weaken tests, fabricate evidence or self-approve.
