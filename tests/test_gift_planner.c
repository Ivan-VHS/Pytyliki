#include "gift_planner.h"

#include <assert.h>
#include <stdint.h>

int main(void) {
    uint32_t shares[8] = {0u};
    assert(pl_gift_split_evenly(500u, 5u, shares, 8u) == 0);
    for (size_t i = 0u; i < 5u; ++i) assert(shares[i] == 100u);
    assert(pl_gift_split_evenly(10u, 3u, shares, 8u) == 0);
    assert(shares[0] == 4u && shares[1] == 3u && shares[2] == 3u);

    const uint32_t weights[] = {2u, 1u, 1u};
    assert(pl_gift_split_weighted(500u, weights, 3u, shares, 8u) == 0);
    assert(shares[0] == 250u && shares[1] == 125u && shares[2] == 125u);
    const uint32_t equal_weights[] = {1u, 1u, 1u};
    assert(pl_gift_split_weighted(10u, equal_weights, 3u, shares, 8u) == 0);
    assert(shares[0] == 4u && shares[1] == 3u && shares[2] == 3u);

    uint32_t amount = 0u;
    assert(pl_gift_percent(500u, 25u, &amount) == 0 && amount == 125u);
    assert(pl_gift_percent(500u, 101u, &amount) == -1);
    uint64_t combinations = 0u;
    assert(pl_gift_count_combinations(5u, 2u, &combinations) == 0 && combinations == 10u);
    assert(pl_gift_count_combinations(5u, 0u, &combinations) == 0 && combinations == 1u);
    assert(pl_gift_count_combinations(4u, 5u, &combinations) == -1);

    const pl_gift_candy_item candy[] = {
        {40u, 10u, 5u}, {25u, 10u, 4u}, {80u, 2u, 5u}
    };
    pl_gift_candy_line lines[3];
    size_t line_count = 0u;
    uint32_t spent = 0u;
    assert(pl_gift_build_candy_basket(500u, candy, 3u, lines, 3u, &line_count, &spent) == 0);
    assert(line_count > 0u && spent <= 500u);
    assert(spent + (500u - spent) == 500u);
    for (size_t i = 0u; i < line_count; ++i)
        assert(lines[i].item_index < 3u && lines[i].quantity > 0u);

    pl_gift_greeting_edge edges[8];
    size_t edge_count = 0u;
    assert(pl_gift_build_greeting_graph(2u, 3u, 4u, edges, 8u, &edge_count) == 0);
    assert(edge_count == 6u);
    assert(edges[0].sender_index == 0u && edges[0].recipient_index == 0u);
    assert(edges[5].sender_index == 1u && edges[5].recipient_index == 2u);
    assert(edges[5].greeting_index == 3u);
    assert(pl_gift_build_greeting_graph(2u, 3u, 0u, edges, 8u, &edge_count) == -1);
    return 0;
}
