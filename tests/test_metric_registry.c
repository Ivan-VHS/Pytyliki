#include "metric_registry.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    const pl_metric_operation_t *results[4] = {0};
    pl_metric_search_result_t page = {0u, 0u};
    pl_metric_query_t query = {
        "vector",
        "vector-algebra",
        PL_METRIC_CAP_STREAMING,
        PL_METRIC_CAP_ALLOCATES,
        1u << 0,
        0u,
        1u,
        2u
    };

    assert(pl_metric_registry_count() == 16u);
    assert(pl_metric_registry_search_ex(&query, results, 4u, &page) == PL_METRIC_QUERY_OK);
    assert(page.total_matches == 5u);
    assert(page.written == 2u);
    assert(results[0] != NULL && strcmp(results[0]->name, "vector.norm.l2") == 0);
    assert(results[1] != NULL && strcmp(results[1]->name, "vector.norm.l1") == 0);

    query.offset = 0u;
    query.limit = 1u;
    assert(pl_metric_registry_search_ex(&query, results, 4u, &page) == PL_METRIC_QUERY_OK);
    assert(page.total_matches == 5u && page.written == 1u);
    assert(strcmp(results[0]->name, "vector.dot") == 0);

    query.required_type_mask = 1u << 1;
    query.excluded_type_mask = 1u << 0;
    query.category = "memory";
    query.query = NULL;
    query.required_capabilities = PL_METRIC_CAP_IN_PLACE;
    query.excluded_capabilities = 0u;
    query.limit = 8u;
    assert(pl_metric_registry_search_ex(&query, results, 4u, &page) == PL_METRIC_QUERY_OK);
    assert(page.total_matches == 2u && page.written == 2u);
    assert(strcmp(results[0]->name, "memory.arena.alloc") == 0);
    assert(strcmp(results[1]->name, "memory.pool.alloc") == 0);

    query.required_capabilities = PL_METRIC_CAP_STREAMING;
    query.excluded_capabilities = PL_METRIC_CAP_STREAMING;
    assert(pl_metric_registry_search_ex(&query, results, 4u, &page) == PL_METRIC_QUERY_INVALID);

    query.required_capabilities = 0u;
    query.excluded_capabilities = 0u;
    {
        char oversized[PL_METRIC_QUERY_MAX_TEXT + 2u];
        memset(oversized, 'x', sizeof(oversized));
        oversized[sizeof(oversized) - 1u] = '\0';
        query.query = oversized;
        assert(pl_metric_registry_search_ex(&query, results, 4u, &page) == PL_METRIC_QUERY_INVALID);
    }

    query.query = "vector";
    query.limit = PL_METRIC_QUERY_MAX_PAGE + 1u;
    assert(pl_metric_registry_search_ex(&query, results, 4u, &page) == PL_METRIC_QUERY_INVALID);

    assert(pl_metric_registry_search("variance", NULL, 0u, results, 4u) == 1u);
    assert(strcmp(results[0]->name, "statistics.mean_variance") == 0);
    return 0;
}
