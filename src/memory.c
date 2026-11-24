#include "memory.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined(_MSC_VER)
#include <malloc.h>
#endif
static int power_of_two(size_t v){return v!=0u&&(v&(v-1u))==0u;}
static int add_overflow_size(size_t a,size_t b,size_t *out){if(a>SIZE_MAX-b)return 1;*out=a+b;return 0;}
static int mul_overflow_size(size_t a,size_t b,size_t *out){if(a!=0u&&b>SIZE_MAX/a)return 1;*out=a*b;return 0;}
static int align_up_size(size_t v,size_t a,size_t *out){
 size_t rem,pad;if(!power_of_two(a))return 0;rem=v&(a-1u);pad=rem==0u?0u:a-rem;
 return !add_overflow_size(v,pad,out);
}
static void *default_allocate(void *u,size_t n){(void)u;return malloc(n);}
static void default_deallocate(void *u,void *p){(void)u;free(p);}
size_t pl_data_type_size(pl_data_type_t t){
 switch(t){
 case PL_TYPE_U8:case PL_TYPE_I8:case PL_TYPE_BYTES:return 1u;
 case PL_TYPE_U16:case PL_TYPE_I16:return 2u;
 case PL_TYPE_U32:case PL_TYPE_I32:case PL_TYPE_F32:return 4u;
 case PL_TYPE_U64:case PL_TYPE_I64:case PL_TYPE_F64:return 8u;
 default:return 0u;
 }}
const char *pl_status_string(pl_status_t s){
 switch(s){
 case PL_STATUS_OK:return "success";case PL_STATUS_INVALID_ARGUMENT:return "invalid argument";
 case PL_STATUS_OUT_OF_RANGE:return "out of range";case PL_STATUS_OVERFLOW:return "size overflow";
 case PL_STATUS_OUT_OF_MEMORY:return "out of memory";case PL_STATUS_TYPE_MISMATCH:return "type mismatch";
 case PL_STATUS_READ_ONLY:return "view is read-only";case PL_STATUS_DIMENSION_MISMATCH:return "dimension mismatch";
 case PL_STATUS_DEGENERATE:return "degenerate input";default:return "unknown status";
 }}
int pl_memory_is_aligned(const void *p,size_t a){
 if(p==NULL||!power_of_two(a))return 0;return ((uintptr_t)p&(uintptr_t)(a-1u))==0u;
}
void *pl_aligned_alloc(size_t a,size_t n){
 size_t rounded;if(!power_of_two(a)||a<sizeof(void *)||n==0u||!align_up_size(n,a,&rounded))return NULL;
#if defined(_MSC_VER)
 return _aligned_malloc(rounded,a);
#else
 return aligned_alloc(a,rounded);
#endif
}
void pl_aligned_free(void *p){
#if defined(_MSC_VER)
 _aligned_free(p);
#else
 free(p);
#endif
}
pl_status_t pl_view_init(pl_view_t *out,void *data,size_t count,pl_data_type_t type,
                         size_t stride,size_t extent,uint32_t flags){
 size_t elem=pl_data_type_size(type),offset,required;
 if(out==NULL||elem==0u||stride==0u||stride<elem||
    (flags&~(PL_VIEW_READ_ONLY|PL_VIEW_OWNS_DATA))!=0u)return PL_STATUS_INVALID_ARGUMENT;
 if(count!=0u&&data==NULL)return PL_STATUS_INVALID_ARGUMENT;
 if(count==0u){if(extent!=0u)return PL_STATUS_INVALID_ARGUMENT;}
 else{
  if((uintmax_t)extent>(uintmax_t)(UINTPTR_MAX-(uintptr_t)data))return PL_STATUS_OVERFLOW;
  if(mul_overflow_size(count-1u,stride,&offset)||add_overflow_size(offset,elem,&required))return PL_STATUS_OVERFLOW;
  if(required>extent)return PL_STATUS_OUT_OF_RANGE;
 }
 out->data=data;out->count=count;out->elem_size=elem;out->stride_bytes=stride;
 out->extent_bytes=extent;out->type=type;out->flags=flags;return PL_STATUS_OK;
}
pl_status_t pl_view_validate(const pl_view_t *v){
 pl_view_t tmp;if(v==NULL)return PL_STATUS_INVALID_ARGUMENT;
 return pl_view_init(&tmp,v->data,v->count,v->type,v->stride_bytes,v->extent_bytes,v->flags);
}
pl_status_t pl_view_slice(const pl_view_t *src,size_t first,size_t count,pl_view_t *out){
 size_t offset;pl_status_t rc=pl_view_validate(src);if(rc!=PL_STATUS_OK)return rc;
 if(out==NULL)return PL_STATUS_INVALID_ARGUMENT;
 if(first>src->count||count>src->count-first)return PL_STATUS_OUT_OF_RANGE;
 if(count==0u)return pl_view_init(out,NULL,0u,src->type,src->stride_bytes,0u,src->flags&PL_VIEW_READ_ONLY);
 if(mul_overflow_size(first,src->stride_bytes,&offset))return PL_STATUS_OVERFLOW;
 if(offset>src->extent_bytes)return PL_STATUS_OUT_OF_RANGE;
 return pl_view_init(out,(unsigned char *)src->data+offset,count,src->type,src->stride_bytes,
                     src->extent_bytes-offset,src->flags&PL_VIEW_READ_ONLY);
}
pl_status_t pl_view_read_f64(const pl_view_t *v,size_t index,double *out){
 size_t offset;pl_status_t rc=pl_view_validate(v);if(rc!=PL_STATUS_OK)return rc;
 if(out==NULL)return PL_STATUS_INVALID_ARGUMENT;
 if(v->type!=PL_TYPE_F64)return PL_STATUS_TYPE_MISMATCH;
 if(index>=v->count)return PL_STATUS_OUT_OF_RANGE;
 if(mul_overflow_size(index,v->stride_bytes,&offset))return PL_STATUS_OVERFLOW;
 memcpy(out,(const unsigned char *)v->data+offset,sizeof(*out));return PL_STATUS_OK;
}
pl_status_t pl_view_write_f64(pl_view_t *v,size_t index,double value){
 size_t offset;pl_status_t rc=pl_view_validate(v);if(rc!=PL_STATUS_OK)return rc;
 if((v->flags&PL_VIEW_READ_ONLY)!=0u)return PL_STATUS_READ_ONLY;
 if(v->type!=PL_TYPE_F64)return PL_STATUS_TYPE_MISMATCH;
 if(index>=v->count)return PL_STATUS_OUT_OF_RANGE;
 if(mul_overflow_size(index,v->stride_bytes,&offset))return PL_STATUS_OVERFLOW;
 memcpy((unsigned char *)v->data+offset,&value,sizeof(value));return PL_STATUS_OK;
}
pl_status_t pl_chunk_init(pl_chunk_t *out,const pl_view_t *v,uint64_t offset,uint64_t seq){
 pl_status_t rc;if(out==NULL)return PL_STATUS_INVALID_ARGUMENT;rc=pl_view_validate(v);if(rc!=PL_STATUS_OK)return rc;
 out->view=*v;out->source_offset=offset;out->sequence=seq;return PL_STATUS_OK;
}
pl_status_t pl_arena_init(pl_arena_t *a,void *storage,size_t capacity){
 if(a==NULL||(capacity!=0u&&storage==NULL))return PL_STATUS_INVALID_ARGUMENT;
 if(capacity!=0u&&(uintmax_t)capacity>(uintmax_t)(UINTPTR_MAX-(uintptr_t)storage))
  return PL_STATUS_OVERFLOW;
 memset(a,0,sizeof(*a));a->memory=(unsigned char *)storage;a->capacity=capacity;return PL_STATUS_OK;
}
pl_status_t pl_arena_create(pl_arena_t *a,size_t capacity,const pl_allocator_t *allocator){
 pl_allocator_t chosen;void *memory;if(a==NULL||capacity==0u)return PL_STATUS_INVALID_ARGUMENT;
 if(allocator!=NULL){if(allocator->allocate==NULL||allocator->deallocate==NULL)return PL_STATUS_INVALID_ARGUMENT;chosen=*allocator;}
 else{chosen.user_data=NULL;chosen.allocate=default_allocate;chosen.deallocate=default_deallocate;}
 memory=chosen.allocate(chosen.user_data,capacity);if(memory==NULL)return PL_STATUS_OUT_OF_MEMORY;
 memset(a,0,sizeof(*a));a->memory=(unsigned char *)memory;a->capacity=capacity;a->allocator=chosen;a->owns_memory=1u;
 return PL_STATUS_OK;
}
void *pl_arena_alloc(pl_arena_t *a,size_t size,size_t alignment){
 uintptr_t base,current,mask;size_t padding,remaining,aligned,end;
 if(a==NULL||a->memory==NULL||size==0u||!power_of_two(alignment)||a->used>a->capacity)return NULL;
 if(alignment-1u>(size_t)UINTPTR_MAX)return NULL;
 base=(uintptr_t)a->memory;
 if(a->used>(size_t)(UINTPTR_MAX-base))return NULL;
 current=base+(uintptr_t)a->used;mask=(uintptr_t)(alignment-1u);
 padding=(size_t)(((uintptr_t)0u-current)&mask);
 remaining=a->capacity-a->used;
 if(padding>remaining)return NULL;
 if(size>remaining-padding)return NULL;
 aligned=a->used+padding;end=aligned+size;
 a->used=end;
 if(a->used>a->peak_used)a->peak_used=a->used;
 if(a->allocation_count<SIZE_MAX)++a->allocation_count;
 return a->memory+aligned;
}
pl_status_t pl_arena_mark(const pl_arena_t *a,pl_arena_mark_t *out){
 if(a==NULL||out==NULL||a->used>a->capacity||(a->capacity!=0u&&a->memory==NULL))return PL_STATUS_INVALID_ARGUMENT;
 out->used=a->used;out->allocation_count=a->allocation_count;return PL_STATUS_OK;
}
pl_status_t pl_arena_rewind(pl_arena_t *a,pl_arena_mark_t mark,int clear_released){
 if(a==NULL||a->used>a->capacity||(a->capacity!=0u&&a->memory==NULL))return PL_STATUS_INVALID_ARGUMENT;
 if(mark.used>a->used||mark.allocation_count>a->allocation_count)return PL_STATUS_OUT_OF_RANGE;
 if(clear_released&&mark.used<a->used)memset(a->memory+mark.used,0,a->used-mark.used);
 a->used=mark.used;a->allocation_count=mark.allocation_count;return PL_STATUS_OK;
}
pl_status_t pl_arena_get_stats(const pl_arena_t *a,pl_arena_stats_t *out){
 if(a==NULL||out==NULL||a->used>a->capacity||(a->capacity!=0u&&a->memory==NULL))return PL_STATUS_INVALID_ARGUMENT;
 out->capacity=a->capacity;out->used=a->used;out->available=a->capacity-a->used;
 out->peak_used=a->peak_used;out->allocation_count=a->allocation_count;return PL_STATUS_OK;
}
void pl_arena_reset(pl_arena_t *a){if(a!=NULL){a->used=0u;a->allocation_count=0u;}}
void pl_arena_destroy(pl_arena_t *a){
 if(a==NULL)return;if(a->owns_memory&&a->allocator.deallocate!=NULL)a->allocator.deallocate(a->allocator.user_data,a->memory);
 memset(a,0,sizeof(*a));
}
pl_status_t pl_pool_init(pl_pool_t *p,void *storage,size_t storage_bytes,void *bitmap,
                         size_t bitmap_bytes,size_t block_size,size_t alignment){
 size_t stride,blocks,need;
 if(p==NULL||storage==NULL||bitmap==NULL||block_size==0u||!power_of_two(alignment)||
    !pl_memory_is_aligned(storage,alignment))return PL_STATUS_INVALID_ARGUMENT;
 if(!align_up_size(block_size,alignment,&stride))return PL_STATUS_OVERFLOW;
 blocks=storage_bytes/stride;if(blocks==0u)return PL_STATUS_OUT_OF_RANGE;
 need=blocks/8u+(blocks%8u!=0u?1u:0u);if(bitmap_bytes<need)return PL_STATUS_OUT_OF_RANGE;
 /*
  * The bitmap is mutable allocator metadata. If it overlaps the pool's data
  * region, initialization or later allocation can silently corrupt payloads.
  * Check integer address ranges before touching either caller-owned region.
  */
 {
  uintptr_t storage_begin=(uintptr_t)storage,bitmap_begin=(uintptr_t)bitmap;
  uintptr_t storage_end,bitmap_end;
  if(storage_bytes>(size_t)(UINTPTR_MAX-storage_begin) ||
     need>(size_t)(UINTPTR_MAX-bitmap_begin))return PL_STATUS_OVERFLOW;
  storage_end=storage_begin+(uintptr_t)storage_bytes;
  bitmap_end=bitmap_begin+(uintptr_t)need;
  if(storage_begin<bitmap_end && bitmap_begin<storage_end)
   return PL_STATUS_INVALID_ARGUMENT;
 }
 memset(p,0,sizeof(*p));memset(bitmap,0,need);p->storage=(unsigned char *)storage;
 p->bitmap=(unsigned char *)bitmap;p->storage_bytes=storage_bytes;p->bitmap_bytes=bitmap_bytes;
 p->block_size=block_size;p->stride_bytes=stride;p->block_count=blocks;return PL_STATUS_OK;
}
static int pool_state_valid(const pl_pool_t *p){
 size_t required_bitmap;
 uintptr_t storage_begin,bitmap_begin,storage_end,bitmap_end;
 if(p==NULL||p->storage==NULL||p->bitmap==NULL||p->block_size==0u||
    p->stride_bytes<p->block_size||p->stride_bytes==0u||p->block_count==0u||
    p->block_count>p->storage_bytes/p->stride_bytes||p->in_use>p->block_count)
  return 0;
 required_bitmap=p->block_count/8u+(p->block_count%8u!=0u?1u:0u);
 if(required_bitmap==0u||required_bitmap>p->bitmap_bytes)return 0;
 storage_begin=(uintptr_t)p->storage;
 bitmap_begin=(uintptr_t)p->bitmap;
 if(p->storage_bytes>(size_t)(UINTPTR_MAX-storage_begin)||
    required_bitmap>(size_t)(UINTPTR_MAX-bitmap_begin))return 0;
 storage_end=storage_begin+(uintptr_t)p->storage_bytes;
 bitmap_end=bitmap_begin+(uintptr_t)required_bitmap;
 return !(storage_begin<bitmap_end&&bitmap_begin<storage_end);
}
void *pl_pool_alloc(pl_pool_t *p){
 size_t bitmap_bytes;
 if(!pool_state_valid(p)||p->in_use>=p->block_count)return NULL;
 bitmap_bytes=p->block_count/8u+(p->block_count%8u!=0u?1u:0u);
 for(size_t byte=0u;byte<bitmap_bytes;byte++){
  unsigned char occupied=p->bitmap[byte];
  size_t first=byte*8u,remaining=p->block_count-first;
  size_t bits=remaining<8u?remaining:8u;
  if(occupied==0xFFu)continue;
  for(size_t bit=0u;bit<bits;bit++){
   unsigned char mask=(unsigned char)(1u<<bit);
   if((occupied&mask)==0u){
    size_t index=first+bit;
    p->bitmap[byte]=(unsigned char)(occupied|mask);
    ++p->in_use;
    return p->storage+index*p->stride_bytes;
   }
  }
 }
 return NULL;
}
pl_status_t pl_pool_free(pl_pool_t *p,void *block){
 uintptr_t base,address,delta;size_t index;unsigned char mask;
 if(block==NULL)return PL_STATUS_INVALID_ARGUMENT;
 if(!pool_state_valid(p)||p->in_use==0u)return PL_STATUS_INVALID_ARGUMENT;
 base=(uintptr_t)p->storage;address=(uintptr_t)block;if(address<base)return PL_STATUS_OUT_OF_RANGE;
 delta=address-base;if(delta>=p->storage_bytes||delta%p->stride_bytes!=0u)return PL_STATUS_OUT_OF_RANGE;
 index=(size_t)(delta/p->stride_bytes);if(index>=p->block_count)return PL_STATUS_OUT_OF_RANGE;
 mask=(unsigned char)(1u<<(index&7u));if((p->bitmap[index>>3u]&mask)==0u)return PL_STATUS_INVALID_ARGUMENT;
 p->bitmap[index>>3u]=(unsigned char)(p->bitmap[index>>3u]&(unsigned char)~mask);--p->in_use;return PL_STATUS_OK;
}
