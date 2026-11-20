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
size_t pl_metric_registry_search(const char *q,const char *category,uint32_t required,const pl_metric_operation_t **results,size_t cap){
 size_t matches=0u;
 for(size_t i=0u;i<pl_metric_registry_count();++i){const pl_metric_operation_t *op=&operations[i];
  int text_match=q==NULL||q[0]=='\0'||contains_ci(op->name,q)||contains_ci(op->category,q)||contains_ci(op->terms,q)||contains_ci(op->description,q);
  if(!text_match||(category!=NULL&&category[0]!='\0'&&!contains_ci(op->category,category))||(op->capabilities&required)!=required)continue;
  if(results!=NULL&&matches<cap)results[matches]=op;++matches;
 }
 return matches;
}
