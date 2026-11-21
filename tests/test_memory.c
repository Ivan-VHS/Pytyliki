#include "memory.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int main(void){
 double values[]={1,2,3,4,5},value=0;pl_view_t view,slice,ro;
 assert(pl_view_init(&view,values,5,PL_TYPE_F64,sizeof(double),sizeof(values),0)==PL_STATUS_OK);
 assert(pl_view_slice(&view,1,3,&slice)==PL_STATUS_OK);
 assert(pl_view_read_f64(&slice,2,&value)==PL_STATUS_OK&&value==4);
 assert(pl_view_write_f64(&slice,0,9)==PL_STATUS_OK&&values[1]==9);
 assert(pl_view_init(&ro,values,5,PL_TYPE_F64,sizeof(double),sizeof(values),PL_VIEW_READ_ONLY)==PL_STATUS_OK);
 assert(pl_view_write_f64(&ro,0,0)==PL_STATUS_READ_ONLY);
 assert(pl_view_read_f64(&view,5,&value)==PL_STATUS_OUT_OF_RANGE);
 assert(pl_view_init(&view,values,5,PL_TYPE_F64,0,sizeof(values),0)==PL_STATUS_INVALID_ARGUMENT);
 assert(pl_view_init(&view,values,5,PL_TYPE_F64,sizeof(double),sizeof(double),0)==PL_STATUS_OUT_OF_RANGE);
 assert(pl_view_init(&view,NULL,1,PL_TYPE_F64,sizeof(double),sizeof(double),0)==PL_STATUS_INVALID_ARGUMENT);

 unsigned char scratch[160];pl_arena_t arena;
 assert(pl_arena_init(&arena,scratch+1,sizeof(scratch)-1)==PL_STATUS_OK);
 void *a=pl_arena_alloc(&arena,13,8),*b=pl_arena_alloc(&arena,16,16);
 assert(a!=NULL&&b!=NULL&&pl_memory_is_aligned(a,8)&&pl_memory_is_aligned(b,16));
 pl_arena_mark_t mark;assert(pl_arena_mark(&arena,&mark)==PL_STATUS_OK);
 void *c=pl_arena_alloc(&arena,24,32);assert(c!=NULL&&pl_memory_is_aligned(c,32));
 pl_arena_stats_t stats;assert(pl_arena_get_stats(&arena,&stats)==PL_STATUS_OK);
 assert(stats.capacity==sizeof(scratch)-1&&stats.used>mark.used);
 assert(stats.peak_used==stats.used&&stats.allocation_count==3);
 assert(pl_arena_rewind(&arena,mark,1)==PL_STATUS_OK);
 assert(arena.used==mark.used&&arena.allocation_count==mark.allocation_count);
 assert(((unsigned char *)c)[0]==0);
 pl_arena_mark_t invalid_mark={SIZE_MAX,0};
 assert(pl_arena_rewind(&arena,invalid_mark,0)==PL_STATUS_OUT_OF_RANGE);
 assert(pl_arena_alloc(&arena,256,8)==NULL);
 pl_arena_reset(&arena);
 assert(arena.used==0&&arena.allocation_count==0);
 assert(pl_arena_get_stats(&arena,&stats)==PL_STATUS_OK&&stats.peak_used>0);
 pl_arena_destroy(&arena);

 pl_arena_t owned;assert(pl_arena_create(&owned,256,NULL)==PL_STATUS_OK);
 assert(pl_arena_alloc(&owned,32,16)!=NULL);pl_arena_destroy(&owned);assert(owned.memory==NULL);
 unsigned char bitmap[4];pl_pool_t pool;
 unsigned char *pool_storage=(unsigned char *)pl_aligned_alloc(16u,512u);
 assert(pool_storage!=NULL);
 assert(pl_pool_init(&pool,pool_storage,512u,bitmap,sizeof(bitmap),24u,16u)==PL_STATUS_OK);
 void *p1=pl_pool_alloc(&pool),*p2=pl_pool_alloc(&pool);
 assert(p1!=NULL&&p2!=NULL&&p1!=p2&&pool.in_use==2);
 assert(pl_pool_free(&pool,p1)==PL_STATUS_OK&&pool.in_use==1);
 assert(pl_pool_free(&pool,p1)==PL_STATUS_INVALID_ARGUMENT);
 assert(pl_pool_free(&pool,(unsigned char *)p2+1)==PL_STATUS_OUT_OF_RANGE);
 assert(pl_pool_free(&pool,p2)==PL_STATUS_OK&&pool.in_use==0);
 void *blocks[16];
 for(size_t i=0u;i<16u;i++){blocks[i]=pl_pool_alloc(&pool);assert(blocks[i]!=NULL);}
 assert(pl_pool_alloc(&pool)==NULL&&pool.in_use==16u);
 assert(pl_pool_free(&pool,blocks[8])==PL_STATUS_OK);
 assert(pl_pool_alloc(&pool)==blocks[8]&&pool.in_use==16u);
 for(size_t i=0u;i<16u;i++)assert(pl_pool_free(&pool,blocks[i])==PL_STATUS_OK);
 assert(pool.in_use==0u);
 pl_aligned_free(pool_storage);
 pl_chunk_t chunk;assert(pl_view_init(&view,values,5,PL_TYPE_F64,sizeof(double),sizeof(values),0)==PL_STATUS_OK);
 assert(pl_chunk_init(&chunk,&view,4096,7)==PL_STATUS_OK&&chunk.source_offset==4096&&chunk.sequence==7);
 void *aligned=pl_aligned_alloc(64,100);assert(aligned!=NULL&&pl_memory_is_aligned(aligned,64));pl_aligned_free(aligned);
 return 0;
}
