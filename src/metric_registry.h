#ifndef PYTYLIKI_METRIC_REGISTRY_H
#define PYTYLIKI_METRIC_REGISTRY_H
#include <stddef.h>
#include <stdint.h>
typedef struct {uint32_t id;const char *name;const char *category;const char *terms;const char *description;const char *complexity;uint32_t type_mask;uint32_t capabilities;} pl_metric_operation_t;
enum {PL_METRIC_CAP_STREAMING=1u<<0,PL_METRIC_CAP_IN_PLACE=1u<<1,PL_METRIC_CAP_DETERMINISTIC=1u<<2,PL_METRIC_CAP_ALLOCATES=1u<<3};
size_t pl_metric_registry_count(void);
const pl_metric_operation_t *pl_metric_registry_at(size_t index);
const pl_metric_operation_t *pl_metric_registry_find_id(uint32_t id);
size_t pl_metric_registry_search(const char *query,const char *category,uint32_t required_capabilities,const pl_metric_operation_t **results,size_t result_capacity);

/* Typed query contract inspired by jOOQ-style explicit predicates and page
 * limits. A zero type mask means no type constraint. limit is bounded to
 * 4096 entries; result_capacity is an independent caller-owned buffer bound. */
#define PL_METRIC_QUERY_MAX_PAGE 4096u
#define PL_METRIC_QUERY_MAX_TEXT 256u
typedef struct {
    const char *query;
    const char *category;
    uint32_t required_capabilities;
    uint32_t excluded_capabilities;
    uint32_t required_type_mask;
    uint32_t excluded_type_mask;
    size_t offset;
    size_t limit;
} pl_metric_query_t;

typedef struct {
    size_t total_matches;
    size_t written;
} pl_metric_search_result_t;

enum {
    PL_METRIC_QUERY_OK = 0,
    PL_METRIC_QUERY_INVALID = -1
};

int pl_metric_registry_search_ex(
    const pl_metric_query_t *query,
    const pl_metric_operation_t **results,
    size_t result_capacity,
    pl_metric_search_result_t *out_result
);
#endif
