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


static void copy_embedded_text(char *out, size_t capacity, const char *value) {
    size_t length;
    if (out == NULL || capacity == 0u || value == NULL) return;
    length = strlen(value);
    if (length >= capacity) length = capacity - 1u;
    memcpy(out, value, length);
    out[length] = '\0';
}

int32_t pytyliki_v1_embedded_profile_count(uint64_t *out_count) {
    if (out_count == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
    *out_count = (uint64_t)pl_embedded_profile_count();
    return PYTYLIKI_ABI_OK;
}

int32_t pytyliki_v1_embedded_profile_at(
    uint64_t index, pytyliki_embedded_profile_v1 *out
) {
    size_t position = 0u;
    const pl_embedded_profile *profile;
    if (out == NULL || count_to_size(index, &position) != PYTYLIKI_ABI_OK)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    profile = pl_embedded_profile_at(position);
    if (profile == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    copy_embedded_text(out->id, sizeof(out->id), profile->id);
    copy_embedded_text(out->architecture, sizeof(out->architecture), profile->architecture);
    copy_embedded_text(out->compiler, sizeof(out->compiler), profile->compiler);
    copy_embedded_text(out->compiler_flags, sizeof(out->compiler_flags), profile->compiler_flags);
    out->clock_hz = profile->clock_hz;
    out->flash_base = profile->flash.base;
    out->flash_size_bytes = profile->flash.size_bytes;
    out->sram_base = profile->sram.base;
    out->sram_size_bytes = profile->sram.size_bytes;
    out->stack_reserve_bytes = profile->stack_reserve_bytes;
    out->is_reference_profile = profile->is_reference_profile ? 1 : 0;
    return PYTYLIKI_ABI_OK;
}

int32_t pytyliki_v1_embedded_budget_evaluate(
    const char *profile_id,
    const pytyliki_embedded_sections_v1 *sections,
    pytyliki_embedded_budget_v1 *out
) {
    const pl_embedded_profile *profile;
    pl_embedded_sections native_sections;
    pl_embedded_memory_budget budget;
    if (profile_id == NULL || profile_id[0] == '\0' || sections == NULL || out == NULL)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    profile = pl_embedded_profile_find(profile_id);
    if (profile == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
    native_sections.text_bytes = sections->text_bytes;
    native_sections.rodata_bytes = sections->rodata_bytes;
    native_sections.data_bytes = sections->data_bytes;
    native_sections.bss_bytes = sections->bss_bytes;
    native_sections.noinit_bytes = sections->noinit_bytes;
    native_sections.stack_reserve_bytes = sections->stack_reserve_bytes;
    if (pl_embedded_budget_evaluate(profile, &native_sections, &budget) != 0)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    out->flash_used_bytes = budget.flash_used_bytes;
    out->sram_used_bytes = budget.sram_used_bytes;
    out->flash_total_bytes = budget.flash_total_bytes;
    out->sram_total_bytes = budget.sram_total_bytes;
    out->flash_percent = budget.flash_percent;
    out->sram_percent = budget.sram_percent;
    out->flash_fits = budget.flash_fits ? 1 : 0;
    out->sram_fits = budget.sram_fits ? 1 : 0;
    return PYTYLIKI_ABI_OK;
}


#ifdef PYTYLIKI_HAS_REAPER
#include "reaper/ffi.h"
#endif

int32_t pytyliki_v1_reaper_available(int32_t *out_available) {
    if (out_available == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
#ifdef PYTYLIKI_HAS_REAPER
    *out_available = 1;
#else
    *out_available = 0;
#endif
    return PYTYLIKI_ABI_OK;
}

int32_t pytyliki_v1_reaper_eval_i64(
    const char *source, uint64_t source_length, int64_t *out,
    char *error, uint64_t error_capacity
) {
    if (source == NULL || out == NULL || (error != NULL && error_capacity == 0u))
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
#ifdef PYTYLIKI_HAS_REAPER
    {
        char native_error[256] = {0};
        int32_t status = reaper_v1_eval_i64(
            source, source_length, out, native_error, (uint64_t)sizeof native_error);
        if (status != REAPER_ABI_OK) {
            if (error != NULL && error_capacity > 0u) {
                size_t limit = error_capacity > (uint64_t)SIZE_MAX
                    ? SIZE_MAX : (size_t)error_capacity;
                (void)snprintf(error, limit, "Reaper ABI v1: %s", native_error);
            }
            return PYTYLIKI_ABI_REAPER_ERROR;
        }
        if (error != NULL && error_capacity > 0u) error[0] = '\0';
        return PYTYLIKI_ABI_OK;
    }
#else
    (void)source_length;
    if (error != NULL && error_capacity > 0u) {
        size_t limit = error_capacity > (uint64_t)SIZE_MAX
            ? SIZE_MAX : (size_t)error_capacity;
        (void)snprintf(error, limit, "%s",
                       "Reaper runtime unavailable; configure REAPER_SOURCE_DIR");
    }
    return PYTYLIKI_ABI_BACKEND_UNAVAILABLE;
#endif
}

int32_t pytyliki_v1_reaper_selfhood_lookup(
    const char *identity, uint64_t *ordinal
) {
    if (identity == NULL || ordinal == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
#ifdef PYTYLIKI_HAS_REAPER
    {
        int32_t status = reaper_v1_selfhood_lookup(identity, ordinal);
        if (status == REAPER_ABI_NOT_FOUND) return PYTYLIKI_ABI_NOT_FOUND;
        return status == REAPER_ABI_OK ? PYTYLIKI_ABI_OK : PYTYLIKI_ABI_REAPER_ERROR;
    }
#else
    return PYTYLIKI_ABI_BACKEND_UNAVAILABLE;
#endif
}
