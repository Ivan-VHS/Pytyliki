# Compute kernel

| Area | Contract |
|---|---|
| Views | typed, strided, bounded, zero-copy |
| Slices | offset/length checks with overflow protection |
| Arena | caller-owned storage, alignment, marks/rewind |
| Pool | fixed blocks; no hidden allocation |
| Metrics | norms, distances, dot, projection, orthogonality |
| Statistics | mean, variance, RMS, covariance |
| ABI | stable C11 interfaces; versioned extensions |

Views never free backing storage. Arena reset is explicit. Pool free rejects foreign pointers/double-free. Metric APIs validate dimensions and finite values; they do not access hardware.
