#ifndef PYTYLIKI_MEMORY_H
#define PYTYLIKI_MEMORY_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PL_MEMORY_ABI_VERSION 2u
typedef enum {
 PL_STATUS_OK=0, PL_STATUS_INVALID_ARGUMENT=1, PL_STATUS_OUT_OF_RANGE=2,
 PL_STATUS_OVERFLOW=3, PL_STATUS_OUT_OF_MEMORY=4, PL_STATUS_TYPE_MISMATCH=5,
 PL_STATUS_READ_ONLY=6, PL_STATUS_DIMENSION_MISMATCH=7, PL_STATUS_DEGENERATE=8
} pl_status_t;
typedef enum {
 PL_TYPE_U8=1, PL_TYPE_I8, PL_TYPE_U16, PL_TYPE_I16, PL_TYPE_U32, PL_TYPE_I32,
 PL_TYPE_U64, PL_TYPE_I64, PL_TYPE_F32, PL_TYPE_F64, PL_TYPE_BYTES
} pl_data_type_t;
enum { PL_VIEW_READ_ONLY=1u<<0, PL_VIEW_OWNS_DATA=1u<<1 };
typedef struct {
 void *data; size_t count; size_t elem_size; size_t stride_bytes; size_t extent_bytes;
 pl_data_type_t type; uint32_t flags;
} pl_view_t;
typedef struct {
 void *user_data;
 void *(*allocate)(void *user_data,size_t size);
 void (*deallocate)(void *user_data,void *pointer);
} pl_allocator_t;
typedef struct {
 unsigned char *memory; size_t capacity; size_t used;
 size_t peak_used; size_t allocation_count;
 pl_allocator_t allocator; uint32_t owns_memory;
} pl_arena_t;
typedef struct { size_t used; size_t allocation_count; } pl_arena_mark_t;
typedef struct {
 size_t capacity; size_t used; size_t available;
 size_t peak_used; size_t allocation_count;
} pl_arena_stats_t;
typedef struct {
 unsigned char *storage; unsigned char *bitmap;
 size_t storage_bytes; size_t bitmap_bytes; size_t block_size;
 size_t stride_bytes; size_t block_count; size_t in_use;
} pl_pool_t;
typedef struct { pl_view_t view; uint64_t source_offset; uint64_t sequence; } pl_chunk_t;
size_t pl_data_type_size(pl_data_type_t type);
const char *pl_status_string(pl_status_t status);
int pl_memory_is_aligned(const void *pointer,size_t alignment);
void *pl_aligned_alloc(size_t alignment,size_t size);
void pl_aligned_free(void *pointer);
pl_status_t pl_view_init(pl_view_t *out,void *data,size_t count,pl_data_type_t type,
                         size_t stride_bytes,size_t extent_bytes,uint32_t flags);
pl_status_t pl_view_validate(const pl_view_t *view);
pl_status_t pl_view_slice(const pl_view_t *source,size_t first,size_t count,pl_view_t *out);
pl_status_t pl_view_read_f64(const pl_view_t *view,size_t index,double *out);
pl_status_t pl_view_write_f64(pl_view_t *view,size_t index,double value);
pl_status_t pl_chunk_init(pl_chunk_t *out,const pl_view_t *view,uint64_t source_offset,uint64_t sequence);
pl_status_t pl_arena_init(pl_arena_t *arena,void *storage,size_t capacity);
pl_status_t pl_arena_create(pl_arena_t *arena,size_t capacity,const pl_allocator_t *allocator);
void *pl_arena_alloc(pl_arena_t *arena,size_t size,size_t alignment);
pl_status_t pl_arena_mark(const pl_arena_t *arena,pl_arena_mark_t *out);
pl_status_t pl_arena_rewind(pl_arena_t *arena,pl_arena_mark_t mark,int clear_released);
pl_status_t pl_arena_get_stats(const pl_arena_t *arena,pl_arena_stats_t *out);
void pl_arena_reset(pl_arena_t *arena);
void pl_arena_destroy(pl_arena_t *arena);
pl_status_t pl_pool_init(pl_pool_t *pool,void *storage,size_t storage_bytes,void *bitmap,
                         size_t bitmap_bytes,size_t block_size,size_t alignment);
void *pl_pool_alloc(pl_pool_t *pool);
pl_status_t pl_pool_free(pl_pool_t *pool,void *block);
#ifdef __cplusplus
}
#endif
#endif
