#include "interop.h"
#include "metric_geometry.h"
#include "embedded.h"
#include <stddef.h>
#include <string.h>
uint32_t pytyliki_abi_version(void) { return PYTYLIKI_ABI_VERSION; }
static int count_to_size(uint64_t count, size_t *out) {
    if (out == NULL || count > (uint64_t)SIZE_MAX) return PYTYLIKI_ABI_INVALID_ARGUMENT;
    *out = (size_t)count; return PYTYLIKI_ABI_OK;
}
int32_t pytyliki_v1_dot_f64(const double *a,const double *b,uint64_t n,double *out) {
    size_t count=0u;
    if(out==NULL||count_to_size(n,&count)!=PYTYLIKI_ABI_OK)return PYTYLIKI_ABI_INVALID_ARGUMENT;
    return pl_metric_dot(a,b,count,out)==0?PYTYLIKI_ABI_OK:PYTYLIKI_ABI_INVALID_ARGUMENT;
}
int32_t pytyliki_v1_norm_f64(const double *v,uint64_t n,double *out) {
    size_t count=0u;
    if(out==NULL||count_to_size(n,&count)!=PYTYLIKI_ABI_OK)return PYTYLIKI_ABI_INVALID_ARGUMENT;
    return pl_metric_norm(v,count,out)==0?PYTYLIKI_ABI_OK:PYTYLIKI_ABI_INVALID_ARGUMENT;
}
int32_t pytyliki_v1_distance_f64(const double *a,const double *b,uint64_t n,double *out) {
    size_t count=0u;
    if(out==NULL||count_to_size(n,&count)!=PYTYLIKI_ABI_OK)return PYTYLIKI_ABI_INVALID_ARGUMENT;
    return pl_metric_distance(a,b,count,out)==0?PYTYLIKI_ABI_OK:PYTYLIKI_ABI_INVALID_ARGUMENT;
}
int32_t pytyliki_v1_orthogonal_f64(const double *a,const double *b,uint64_t n,double tolerance,int32_t *out) {
    size_t count=0u; int result=0;
    if(out==NULL||count_to_size(n,&count)!=PYTYLIKI_ABI_OK)return PYTYLIKI_ABI_INVALID_ARGUMENT;
    if(pl_metric_is_orthogonal(a,b,count,tolerance,&result)!=0)return PYTYLIKI_ABI_INVALID_ARGUMENT;
    *out=result?1:0; return PYTYLIKI_ABI_OK;
}

static void copy_field(char *out, size_t capacity, const char *value) {
    size_t length;
    if (out == NULL || capacity == 0u || value == NULL) return;
    length = strlen(value);
    if (length >= capacity) length = capacity - 1u;
    memcpy(out, value, length);
    out[length] = '\0';
}
uint64_t pytyliki_v1_embedded_target_count(void) {
    return (uint64_t)pl_embedded_target_count();
}
int32_t pytyliki_v1_embedded_target_at(uint64_t index, pytyliki_embedded_target_v1 *out) {
    size_t position = 0u;
    const pl_embedded_target *target = NULL;
    if (out == NULL || count_to_size(index, &position) != PYTYLIKI_ABI_OK)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    if (pl_embedded_target_at(position, &target) != 0 || target == NULL)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    copy_field(out->id, sizeof(out->id), target->id);
    copy_field(out->architecture, sizeof(out->architecture), target->architecture);
    copy_field(out->abi, sizeof(out->abi), target->abi);
    copy_field(out->compiler, sizeof(out->compiler), target->compiler);
    copy_field(out->description, sizeof(out->description), target->description);
    out->word_bits = target->word_bits;
    out->capabilities = target->capabilities;
    return PYTYLIKI_ABI_OK;
}
int32_t pytyliki_v1_embedded_inspect_source(
    const char *source, uint64_t length, pytyliki_embedded_source_report_v1 *out
) {
    size_t source_length = 0u;
    pl_embedded_source_report report;
    if (source == NULL || out == NULL || count_to_size(length, &source_length) != PYTYLIKI_ABI_OK)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    if (pl_embedded_inspect_source(source, source_length, &report) != 0)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    out->source_bytes = (uint64_t)report.source_bytes;
    out->line_count = (uint64_t)report.line_count;
    out->include_count = (uint64_t)report.include_count;
    out->heap_calls = (uint64_t)report.heap_calls;
    out->blocking_calls = (uint64_t)report.blocking_calls;
    out->register_markers = (uint64_t)report.register_markers;
    out->host_io_calls = (uint64_t)report.host_io_calls;
    out->warnings = report.warnings;
    return PYTYLIKI_ABI_OK;
}
