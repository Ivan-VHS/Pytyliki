#include "gift_planner.h"

#include <stdint.h>

static uint64_t gcd_u64(uint64_t a, uint64_t b) {
    while (b != 0u) {
        const uint64_t remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int pl_gift_split_evenly(uint32_t total, size_t parts,
                         uint32_t *shares, size_t capacity) {
    if (parts == 0u || parts > PL_GIFT_MAX_RECIPIENTS ||
        shares == NULL || capacity < parts) return -1;
    const uint32_t base = total / (uint32_t)parts;
    const uint32_t remainder = total % (uint32_t)parts;
    for (size_t i = 0u; i < parts; ++i)
        shares[i] = base + (i < (size_t)remainder ? 1u : 0u);
    return 0;
}

int pl_gift_split_weighted(uint32_t total, const uint32_t *weights,
                           size_t count, uint32_t *shares, size_t capacity) {
    if (weights == NULL || shares == NULL || count == 0u ||
        count > PL_GIFT_MAX_RECIPIENTS || capacity < count) return -1;
    uint64_t weight_sum = 0u;
    for (size_t i = 0u; i < count; ++i) weight_sum += weights[i];
    if (weight_sum == 0u) return -1;

    uint64_t remainders[PL_GIFT_MAX_RECIPIENTS] = {0u};
    unsigned char awarded[PL_GIFT_MAX_RECIPIENTS] = {0u};
    uint64_t assigned = 0u;
    for (size_t i = 0u; i < count; ++i) {
        const uint64_t product = (uint64_t)total * (uint64_t)weights[i];
        shares[i] = (uint32_t)(product / weight_sum);
        remainders[i] = product % weight_sum;
        assigned += shares[i];
    }
    uint64_t left = (uint64_t)total - assigned;
    while (left > 0u) {
        size_t best = count;
        for (size_t i = 0u; i < count; ++i) {
            if (awarded[i] != 0u) continue;
            if (best == count || remainders[i] > remainders[best]) best = i;
        }
        if (best == count) return -1;
        ++shares[best];
        awarded[best] = 1u;
        --left;
    }
    return 0;
}

int pl_gift_percent(uint32_t total, uint32_t percent, uint32_t *amount) {
    if (amount == NULL || percent > 100u) return -1;
    *amount = (uint32_t)(((uint64_t)total * percent) / 100u);
    return 0;
}

int pl_gift_count_combinations(uint32_t n, uint32_t k, uint64_t *out) {
    if (out == NULL || k > n || n > 1000000u) return -1;
    if (k > n - k) k = n - k;
    uint64_t result = 1u;
    for (uint32_t i = 1u; i <= k; ++i) {
        uint64_t numerator = (uint64_t)n - k + i;
        uint64_t denominator = i;
        uint64_t common = gcd_u64(numerator, denominator);
        numerator /= common;
        denominator /= common;
        common = gcd_u64(result, denominator);
        result /= common;
        denominator /= common;
        if (denominator != 1u || (numerator != 0u && result > UINT64_MAX / numerator))
            return -1;
        result *= numerator;
    }
    *out = result;
    return 0;
}

static int better_ratio(const pl_gift_candy_item *left, size_t left_index,
                        const pl_gift_candy_item *right, size_t right_index) {
    const uint64_t lhs = (uint64_t)left->preference * right->price;
    const uint64_t rhs = (uint64_t)right->preference * left->price;
    if (lhs != rhs) return lhs > rhs;
    return left_index < right_index;
}

int pl_gift_build_candy_basket(uint32_t budget,
                               const pl_gift_candy_item *items, size_t item_count,
                               pl_gift_candy_line *lines, size_t line_capacity,
                               size_t *line_count, uint32_t *spent) {
    if (items == NULL || lines == NULL || line_count == NULL || spent == NULL ||
        item_count == 0u || item_count > PL_GIFT_MAX_ITEMS ||
        line_capacity < item_count) return -1;
    size_t order[PL_GIFT_MAX_ITEMS];
    for (size_t i = 0u; i < item_count; ++i) {
        if (items[i].price == 0u) return -1;
        order[i] = i;
    }
    for (size_t i = 1u; i < item_count; ++i) {
        const size_t current = order[i];
        size_t j = i;
        while (j > 0u && better_ratio(&items[current], current,
                                      &items[order[j - 1u]], order[j - 1u])) {
            order[j] = order[j - 1u];
            --j;
        }
        order[j] = current;
    }
    uint32_t remaining = budget;
    size_t used = 0u;
    for (size_t i = 0u; i < item_count; ++i) {
        const size_t index = order[i];
        if (items[index].stock == 0u || remaining < items[index].price) continue;
        uint32_t quantity = remaining / items[index].price;
        if (quantity > items[index].stock) quantity = items[index].stock;
        if (quantity == 0u) continue;
        lines[used].item_index = index;
        lines[used].quantity = quantity;
        ++used;
        remaining -= quantity * items[index].price;
    }
    *line_count = used;
    *spent = budget - remaining;
    return 0;
}

int pl_gift_build_greeting_graph(size_t sender_count, size_t recipient_count,
                                 size_t greeting_count,
                                 pl_gift_greeting_edge *edges, size_t capacity,
                                 size_t *edge_count) {
    if (sender_count == 0u || recipient_count == 0u || greeting_count == 0u ||
        edges == NULL || edge_count == NULL ||
        sender_count > PL_GIFT_MAX_RECIPIENTS ||
        recipient_count > PL_GIFT_MAX_RECIPIENTS ||
        sender_count > PL_GIFT_MAX_EDGES / recipient_count) return -1;
    const size_t required = sender_count * recipient_count;
    if (required > capacity || required > PL_GIFT_MAX_EDGES) return -1;
    size_t at = 0u;
    for (size_t sender = 0u; sender < sender_count; ++sender) {
        for (size_t recipient = 0u; recipient < recipient_count; ++recipient) {
            edges[at].sender_index = sender;
            edges[at].recipient_index = recipient;
            edges[at].greeting_index = (sender + recipient) % greeting_count;
            ++at;
        }
    }
    *edge_count = required;
    return 0;
}
