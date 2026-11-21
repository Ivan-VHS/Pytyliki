# Safety architecture

| Boundary | Rule |
|---|---|
| Hardware | discovery only; no privileged writes |
| Source code | inspect; never execute |
| Inputs | validate sizes, paths and ranges |
| Resources | explicit memory/workload caps |
| Errors | fail closed; preserve unknown |
| Promotion | human review and required checks |
