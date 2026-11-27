# Early Mathematics Numerical Workbench Profile v1

**Status:** proposed compute profile; no new public C ABI function is claimed as implemented.  
**Project boundary:** Pytyliki remains a C11 bounded compute, measurement and inspection toolkit. It is not a child-facing tutor, curriculum engine or learner analytics service.

## 1. Purpose

The DAK early-mathematics experience may need deterministic, reusable operations for finite counts, comparisons, simple measurements, geometric quantities and pattern summaries. Pytyliki is a suitable optional provider for bounded numerical work where a native C11 implementation is justified.

The profile translates mathematical concepts into safe compute requirements. It does not own activity wording, age recommendations, lesson sequencing or assessment.

## 2. Candidate operation groups

| Group | Candidate operations | Contract |
|---|---|---|
| Finite collections | length, count-by-key, histogram | caller-owned bounded views; no implicit allocation |
| Comparison | equality, ordering, absolute difference | domain and signedness explicit |
| Part–whole | checked sum, checked difference, quotient/remainder | overflow and negative-domain behavior explicit |
| Measurement | scale conversion and comparison | unit and dimension supplied by caller |
| Geometry | 2D distance, bounding extents, area for supported shapes | finite inputs and checked intermediates |
| Pattern summaries | period candidate, repeat count, growth difference | bounded scan; report ambiguity |
| Statistics | mean, range and distribution summaries | distinguish exact integer/rational from approximate floating point |

These are candidate extension points, not a statement that the corresponding public functions already exist.

## 3. Numeric representation

- Cardinal counts should use an explicitly sized unsigned integer type.
- Signed differences must use a signed domain with checked conversion.
- Fractions should use a numerator/denominator representation with denominator validation, sign normalization and an explicit overflow result.
- Measurement values must carry a unit identifier and dimension outside the raw scalar buffer.
- Floating-point routines may be used for geometry or visualization where approximation is acceptable, but their result must be labelled approximate and must never replace exact counting or fraction comparison.
- NaN, infinity, zero denominators, non-finite intermediates and integer overflow are rejected before output buffers are modified.

C11 portability is a constraint: do not assume a compiler-specific 128-bit integer is available. Implement checked multiplication/division using portable bounded algorithms or return overflow.

## 4. Memory and ABI rules

- All buffers are caller-owned, typed, strided and length-bounded.
- No hidden allocation, process launch, file access, hardware probing or network access is part of a numerical operation.
- Input dimensions and byte sizes are checked before dereference; offset calculations are overflow-safe.
- Failure leaves caller-owned output unchanged unless the function contract explicitly documents a transactional partial result.
- Public extensions use the existing versioned C ABI discipline and stable error/status reporting.
- Reaper may use the adapter only after explicit native FFI enablement and caller selection of a trusted absolute library path.
- DAK must not link Pytyliki into the OS kernel or depend on the native library to open a basic activity.

## 5. Correctness properties

1. Count and histogram totals are invariant under input permutation.
2. Reordering a sequence may change positional queries but not its length or histogram.
3. A checked sum either returns the exact representable value or an overflow status.
4. Quotient and remainder satisfy n = qk + r, with 0 ≤ r < k for positive k.
5. Unit conversion round trips are exact when the ratio is exact and the representation is sufficient.
6. Geometric distance is symmetric within the stated numeric tolerance; exact integer functions have no tolerance.
7. A pattern detector reports a candidate period, not a unique rule, unless uniqueness is established by additional declared constraints.
8. Empty input is handled explicitly and never dereferenced.

## 6. Test matrix

Before any new capability is advertised as implemented, add native CTest coverage for:

- zero, one and maximum representable counts;
- empty, single-element and maximum-length views;
- duplicate values and stable histograms;
- overflow at addition, multiplication, offset and buffer-size boundaries;
- denominator zero, negative denominator normalization and rational overflow;
- quotient/remainder identities and leftover reporting;
- exact and inexact unit conversions;
- NaN/Inf rejection before mutation;
- strided views, alignment, aliasing and buffer-boundary checks;
- deterministic repeated calls and no-allocation guarantees.

Differential tests may compare Pytyliki results with Reaper's independently tested semantic reference. A matching result is interoperability evidence, not proof that either component has complete educational functionality.

## 7. Privacy and operating limits

The numerical layer receives only the small input required by the operation. It must not receive names, birth dates, voice, photos, precise location, family relationships or a persistent learner identifier. No learner profile is needed to count objects or compare lengths.

The workbench remains local and deterministic. It must not infer ability, classify a child, rank learners or produce developmental diagnoses.
