# Automatic multiply and compute operations

Pytyliki exposes a bounded C11 compute API in `src/compute_ops.h` and
`src/compute_ops.c`. It turns the requested operation vocabulary into
concrete, testable primitives rather than implicit or background execution.

| Requested capability | Implementation |
|---|---|
| Automatic Multiply / Product | Checked scalar multiplication and aggregate product |
| Compute / Calculate | Finite-checked scalar and vector calculations |
| Scale / Amplify | Vector scaling by a finite factor |
| Compound / Expand | Element-wise combination and bounded vector products |
| Accumulate / Iterate | Prefix accumulation over a numeric sequence |
| Cross-multiply | Outer product of two vectors |
| Exponentiate | Checked real power operation |
| Factor | Bounded prime factorization (inputs up to 10^12) |
| Combine / Fuse | Element-wise sum and full linear convolution |
| Aggregate | Count, sum, product, mean, minimum and maximum |
| Pair / Couple | Bounded Cartesian pairing of two vectors |
| Convolve | Full one-dimensional discrete convolution |
| Cartesianize | Ordered Cartesian product represented as parallel pair arrays |

Every vector API validates finite inputs, enforces `PL_COMPUTE_MAX_ITEMS`,
checks output capacity, and rejects non-finite or out-of-range results.
The factorizer returns prime factors with multiplicity in ascending order and
rejects zero or values above `PL_COMPUTE_MAX_FACTOR_INPUT`.

Build and test with:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
