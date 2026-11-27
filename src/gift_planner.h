#ifndef PYTYLIKI_GIFT_PLANNER_H
#define PYTYLIKI_GIFT_PLANNER_H

#include <stddef.h>
#include <stdint.h>

#define PL_GIFT_MAX_RECIPIENTS 256u
#define PL_GIFT_MAX_ITEMS 256u
#define PL_GIFT_MAX_EDGES 65536u

typedef struct {
    uint32_t price;
    uint32_t stock;
    uint32_t preference;
} pl_gift_candy_item;

typedef struct {
    size_t item_index;
    uint32_t quantity;
} pl_gift_candy_line;

typedef struct {
    size_t sender_index;
    size_t recipient_index;
    size_t greeting_index;
} pl_gift_greeting_edge;

/* Integer money units are selected by the caller (e.g. UAH or kopiykas).
 * Equal splitting gives indivisible remainder units to lower indexes first. */
int pl_gift_split_evenly(uint32_t total, size_t parts,
                         uint32_t *shares, size_t capacity);
int pl_gift_split_weighted(uint32_t total, const uint32_t *weights,
                           size_t count, uint32_t *shares, size_t capacity);
int pl_gift_percent(uint32_t total, uint32_t percent, uint32_t *amount);

/* Exact n-choose-k; returns -1 for invalid input or uint64 overflow. */
int pl_gift_count_combinations(uint32_t n, uint32_t k, uint64_t *out);

/* Greedy basket: descending preference/price ratio, stable input-order ties.
 * This is an explainable heuristic, not a claim of global knapsack optimality. */
int pl_gift_build_candy_basket(uint32_t budget,
                               const pl_gift_candy_item *items, size_t item_count,
                               pl_gift_candy_line *lines, size_t line_capacity,
                               size_t *line_count, uint32_t *spent);

/* Complete directed sender-to-recipient bipartite graph. Greeting templates
 * rotate deterministically by (sender + recipient) modulo greeting_count. */
int pl_gift_build_greeting_graph(size_t sender_count, size_t recipient_count,
                                 size_t greeting_count,
                                 pl_gift_greeting_edge *edges, size_t capacity,
                                 size_t *edge_count);

#endif
