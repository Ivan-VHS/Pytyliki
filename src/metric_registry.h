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
#endif
