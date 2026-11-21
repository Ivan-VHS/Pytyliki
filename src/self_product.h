#ifndef PYTYLIKI_SELF_PRODUCT_H
#define PYTYLIKI_SELF_PRODUCT_H
#include <stddef.h>
#include <stdint.h>
#define PL_SELF_PRODUCT_MAX_DIM 32u
#define PL_SELF_PRODUCT_MAX_DEPTH 8u
typedef struct {
    size_t dimension;
    unsigned layers_completed;
    size_t saturated_cells;
    uint64_t diagonal_sum;
    int saturated;
} pl_self_product_report;
/* Computes M(0)=input and M(k+1)=saturating_u64(M(k) x M(k)).
 * The output is input^(2^depth). Input/output may alias. No heap allocation,
 * I/O, hardware access, or source mutation. */
int pl_self_product_u64(const uint64_t *matrix, size_t dimension,
                        unsigned depth, uint64_t *output,
                        pl_self_product_report *report);
#endif
