# Vertical Self-Product

Pytyliki implements vertical self-multiplication as repeated squaring of a
bounded architecture or subsystem interaction matrix.

For an input matrix M, each layer calculates M(0)=M and
M(k+1)=M(k)×M(k). After d layers, the result is M^(2^d). Callers decide what
rows and columns represent; a numeric relationship is not proof of real
hardware compatibility.

## Runtime contract

- C11 API: pl_self_product_u64 in src/self_product.h.
- Matrix dimensions: 1–32; vertical depth: 1–8.
- Values use uint64_t. Multiplication and accumulation saturate at UINT64_MAX.
- Input and output buffers are caller-owned and may alias.
- Two fixed-size stack buffers are used; no heap allocation, I/O, hardware
  probing, firmware access, or source rewriting occurs.
- The report includes completed layers, saturated cells, and the final
  diagonal sum; saturation is explicitly marked.

## Verification

tests/test_self_product.c checks known Fibonacci-matrix powers, overflow
saturation, and invalid bounds. CMake exposes the test as
self-product-matrix-tests.
