#include "metric_registry.h"
#include <ctype.h>
#define F64 (1u<<0)
#define VIEW (1u<<1)
#define BASE (PL_METRIC_CAP_STREAMING|PL_METRIC_CAP_DETERMINISTIC)
static const pl_metric_operation_t operations[]={
 {1001u,"vector.dot","vector-algebra","dot product scalar product inner product","Compute the scalar product of two finite vectors.","O(n) time, O(1) extra",F64,BASE},
 {1002u,"vector.norm.l2","vector-algebra","euclidean norm l2 length magnitude","Stable Euclidean norm of a vector.","O(n) time, O(1) extra",F64,BASE},
 {1003u,"vector.norm.l1","vector-algebra","manhattan norm l1 taxicab","Sum of absolute component values.","O(n) time, O(1) extra",F64,BASE},
 {1004u,"vector.norm.linf","vector-algebra","chebyshev norm maximum norm","Maximum absolute component value.","O(n) time, O(1) extra",F64,BASE},
 {1010u,"vector.distance.euclidean","metric-space","euclidean distance l2 distance","Euclidean distance between two vectors.","O(n) time, O(1) extra",F64,BASE},
 {1011u,"vector.distance.squared","metric-space","squared euclidean distance ranking","Squared Euclidean distance without square root.","O(n) time, O(1) extra",F64,BASE},
 {1012u,"vector.distance.weighted","metric-space","weighted euclidean distance diagonal metric","Weighted Euclidean distance with non-negative coordinate weights.","O(n) time, O(1) extra",F64,BASE},
 {1020u,"vector.project","vector-algebra","projection orthogonal component decomposition","Project a vector onto a non-zero direction.","O(n) time, O(n) output",F64,BASE},
 {1021u,"vector.cosine","metric-space","cosine similarity angle orientation","Cosine similarity independent of vector magnitude.","O(n) time, O(1) extra",F64,BASE},
 {1030u,"matrix.quadratic_form","metric-space","quadratic form xmx mahalanobis anisotropic","Evaluate x transpose M x for a dense matrix.","O(n^2) time, O(1) extra",F64,BASE},
 {1040u,"statistics.mean_variance","statistics","mean variance standard deviation dispersion","Welford mean and population or sample variance.","O(n) time, O(1) extra",F64,BASE},
 {1041u,"statistics.rms","statistics","rms root mean square energy","Root mean square of a finite sequence.","O(n) time, O(1) extra",F64,BASE},
 {1042u,"statistics.covariance","statistics","covariance correlation co-movement","Population or sample covariance of paired observations.","O(n) time, O(1) extra",F64,BASE},
 {1050u,"buffer.view.slice","memory","view slice window stride zero-copy","Create a bounded non-owning slice over caller memory.","O(1) time, O(1) extra",VIEW,BASE},
 {1051u,"memory.arena.alloc","memory","arena allocator scratch temporary workspace","Allocate aligned temporary memory from a caller-owned arena.","O(1) time, O(1) extra",VIEW,BASE|PL_METRIC_CAP_IN_PLACE},
 {1052u,"memory.pool.alloc","memory","pool fixed block allocator","Allocate fixed-size blocks from a caller-provided pool.","O(n) worst case, O(1) metadata per block",VIEW,BASE|PL_METRIC_CAP_IN_PLACE}
};
static int contains_ci(const char *text,const char *needle){
 if(needle==NULL||needle[0]=='\0')return 1;if(text==NULL)return 0;
 for(;*text;++text){const char *a=text,*b=needle;
  while(*a&&*b&&tolower((unsigned char)*a)==tolower((unsigned char)*b)){++a;++b;}
  if(*b=='\0')return 1;
 }return 0;
}
size_t pl_metric_registry_count(void){return sizeof(operations)/sizeof(operations[0]);}
const pl_metric_operation_t *pl_metric_registry_at(size_t i){return i<pl_metric_registry_count()?&operations[i]:NULL;}
const pl_metric_operation_t *pl_metric_registry_find_id(uint32_t id){
 for(size_t i=0u;i<pl_metric_registry_count();++i)if(operations[i].id==id)return &operations[i];return NULL;
}

static size_t bounded_length(const char *text, size_t maximum) {
    size_t length = 0u;
    if (text == NULL) return 0u;
    while (length <= maximum && text[length] != '\0') ++length;
    return length;
}

int pl_metric_registry_search_ex(
    const pl_metric_query_t *query,
    const pl_metric_operation_t **results,
    size_t result_capacity,
    pl_metric_search_result_t *out_result
) {
    size_t i;
    size_t total = 0u;
    size_t written = 0u;
    size_t page_limit;
    if (query == NULL || out_result == NULL
        || (results == NULL && result_capacity != 0u)
        || query->limit == 0u || query->limit > PL_METRIC_QUERY_MAX_PAGE
        || bounded_length(query->query, PL_METRIC_QUERY_MAX_TEXT) > PL_METRIC_QUERY_MAX_TEXT
        || bounded_length(query->category, PL_METRIC_QUERY_MAX_TEXT) > PL_METRIC_QUERY_MAX_TEXT
        || (query->required_capabilities & query->excluded_capabilities) != 0u
        || (query->required_type_mask & query->excluded_type_mask) != 0u) {
        return PL_METRIC_QUERY_INVALID;
    }

    page_limit = query->limit < result_capacity ? query->limit : result_capacity;
    out_result->total_matches = 0u;
    out_result->written = 0u;

    for (i = 0u; i < pl_metric_registry_count(); ++i) {
        const pl_metric_operation_t *op = &operations[i];
        const int text_match = query->query == NULL || query->query[0] == '\0'
            || contains_ci(op->name, query->query)
            || contains_ci(op->category, query->query)
            || contains_ci(op->terms, query->query)
            || contains_ci(op->description, query->query);
        const int category_match = query->category == NULL || query->category[0] == '\0'
            || contains_ci(op->category, query->category);
        const int capabilities_match =
            (op->capabilities & query->required_capabilities) == query->required_capabilities
            && (op->capabilities & query->excluded_capabilities) == 0u;
        const int type_match =
            (op->type_mask & query->required_type_mask) == query->required_type_mask
            && (op->type_mask & query->excluded_type_mask) == 0u;
        if (!text_match || !category_match || !capabilities_match || !type_match) continue;

        if (total >= query->offset && written < page_limit && results != NULL) {
            results[written++] = op;
        }
        ++total;
    }
    out_result->total_matches = total;
    out_result->written = written;
    return PL_METRIC_QUERY_OK;
}

size_t pl_metric_registry_search(const char *q,const char *category,uint32_t required,
                                 const pl_metric_operation_t **results,size_t cap) {
    pl_metric_query_t query = {
        q, category, required, 0u, 0u, 0u, 0u,
        cap == 0u ? 1u : cap
    };
    pl_metric_search_result_t result = {0u, 0u};
    if (cap > PL_METRIC_QUERY_MAX_PAGE) return 0u;
    if (pl_metric_registry_search_ex(&query, results, cap, &result) != PL_METRIC_QUERY_OK) {
        return 0u;
    }
    return result.total_matches;
}
