#include "interop.h"
#if defined(PYTYLIKI_WITH_REAPER)
#include "reaper/ffi.h"
#endif
#include "metric_geometry.h"
#include "metric_registry.h"
#include "embedded.h"
#include <stddef.h>
#include <ctype.h>
#include <string.h>

typedef struct { const char *id, *name, *domain, *service, *capabilities; } plugin_entry;
static const plugin_entry PLUGIN_CATALOG[] = {
    {"operandu", "Operandu", "Data & logic", "inspect_value", "value.validation;schema.inspection;expression.metadata"},
    {"dropaul", "Dropaul", "Actions & workflows", "plan_workflow", "workflow.graph;dependency.ordering;dry_run"},
    {"zdoro", "Zdoro", "Integrity & diagnostics", "inspect_workspace", "workspace.inspection;integrity.summary"},
    {"pyemont", "Pyemont", "Development", "inspect_project", "source.inventory;module.discovery;project.manifest"},
    {"dilly", "Dilly", "Interface & experience", "lint_design_tokens", "design.token.lint;label.inventory;theme.contract"},
    {"baplap", "Baplap", "Data infrastructure", "index_files", "file.catalog;content.fingerprint;metadata.index"},
    {"sretuo", "Sretuo", "Quality engineering", "plan_tests", "test.discovery;coverage.targets;test.plan"},
    {"kali", "Kali", "Security", "scan_security", "secret.pattern.scan;unsafe.api.inventory;static.findings"},
    {"zalam", "Zalam", "Connectivity", "inspect_contracts", "contract.validation;integration.inventory;schema.checks"},
    {"liur", "Liur", "Runtime resources", "measure_resources", "source.size.metrics;resource.budget;runtime.metadata"},
    {"keel-koh", "Keel Koh", "Knowledge", "index_knowledge", "documentation.index;heading.extraction;link.inventory"},
    {"vorof", "Vorof", "Reverse engineering", "inspect_structure", "ast.inventory;symbol.discovery;dependency.outline"},
    {"vprof", "Vprof", "Profiling & metrics", "profile_source", "ast.metrics;complexity.estimates;module.comparison"},
    {"profe", "Profe", "Engineering standards", "check_standards", "source.conventions;documentation.checks;policy.inventory"},
    {"prossee", "Prossee", "Project planning", "plan_project", "task.graph;milestone.planning;critical.path.estimate"},
    {"vprossee", "Vprossee", "Verification & validation", "verify_evidence", "evidence.matrix;acceptance.checks;verification.report"},
    {"professiya", "Professiya", "Learning", "learning_paths", "learning.paths;topic.map;progress.checklist"},
    {"profevo", "Profevo", "Evolution", "plan_evolution", "deprecation.inventory;migration.plan;change.impact"}
};

static size_t plugin_catalog_count(void) {
    return sizeof(PLUGIN_CATALOG) / sizeof(PLUGIN_CATALOG[0]);
}

static int plugin_name_equal(const char *left, const char *right) {
    if (left == NULL || right == NULL) return 0;
    while (*left != '\0' && *right != '\0') {
        unsigned char a = (unsigned char)*left++;
        unsigned char b = (unsigned char)*right++;
        if (a == '_' || a == ' ') a = '-';
        if (b == '_' || b == ' ') b = '-';
        if ((unsigned char)tolower(a) != (unsigned char)tolower(b)) return 0;
    }
    return *left == '\0' && *right == '\0';
}

static const plugin_entry *plugin_catalog_at(size_t index) {
    return index < plugin_catalog_count() ? &PLUGIN_CATALOG[index] : NULL;
}

static const plugin_entry *plugin_catalog_find(const char *name) {
    size_t i;
    if (name == NULL || name[0] == '\0') return NULL;
    for (i = 0u; i < plugin_catalog_count(); ++i)
        if (plugin_name_equal(name, PLUGIN_CATALOG[i].id) || plugin_name_equal(name, PLUGIN_CATALOG[i].name))
            return &PLUGIN_CATALOG[i];
    return NULL;
}
uint32_t pytyliki_abi_version(void) { return PYTYLIKI_ABI_VERSION; }

int32_t pytyliki_v1_reaper_available(void) {
#if defined(PYTYLIKI_WITH_REAPER)
    return reaper_abi_version() == REAPER_ABI_VERSION ? 1 : 0;
#else
    return 0;
#endif
}

uint32_t pytyliki_v1_reaper_abi_version(void) {
#if defined(PYTYLIKI_WITH_REAPER)
    return reaper_abi_version();
#else
    return 0u;
#endif
}

int32_t pytyliki_v1_reaper_eval_i64(
    const char *source, uint64_t source_size, int64_t *result,
    char *diagnostic, uint64_t diagnostic_capacity
) {
#if defined(PYTYLIKI_WITH_REAPER)
    if (reaper_abi_version() != REAPER_ABI_VERSION)
        return PYTYLIKI_ABI_UNSUPPORTED_ABI;
    return reaper_v1_eval_i64(source, source_size, result,
                              diagnostic, diagnostic_capacity);
#else
    (void)source;
    (void)source_size;
    (void)result;
    (void)diagnostic;
    (void)diagnostic_capacity;
    return PYTYLIKI_ABI_BACKEND_UNAVAILABLE;
#endif
}

int32_t pytyliki_v1_reaper_selfhood_lookup(const char *identity, uint64_t *ordinal) {
#if defined(PYTYLIKI_WITH_REAPER)
    if (reaper_abi_version() != REAPER_ABI_VERSION)
        return PYTYLIKI_ABI_UNSUPPORTED_ABI;
    return reaper_v1_selfhood_lookup(identity, ordinal);
#else
    (void)identity;
    (void)ordinal;
    return PYTYLIKI_ABI_BACKEND_UNAVAILABLE;
#endif
}
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

static int has_terminator(const char *value, size_t capacity) {
    return value != NULL && memchr(value, '\0', capacity) != NULL;
}

static void copy_embedded_text(char *out, size_t capacity, const char *value) {
    size_t length;
    if (out == NULL || capacity == 0u || value == NULL) return;
    length = strlen(value);
    if (length >= capacity) length = capacity - 1u;
    memcpy(out, value, length);
    out[length] = '\0';
}



int32_t pytyliki_v1_metric_registry_search(
    const pytyliki_metric_query_v1 *query,
    pytyliki_metric_operation_v1 *results,
    uint64_t result_capacity,
    pytyliki_metric_search_result_v1 *out
) {
    size_t offset = 0u, limit = 0u, capacity = 0u;
    size_t i, available;
    pl_metric_search_result_t native_result = {0u, 0u};
    pl_metric_search_result_t item_result = {0u, 0u};
    pl_metric_query_t native_query;
    pl_metric_query_t item_query;
    const pl_metric_operation_t *item = NULL;

    if (query == NULL || out == NULL
        || (results == NULL && result_capacity != 0u)
        || !has_terminator(query->query, sizeof(query->query))
        || !has_terminator(query->category, sizeof(query->category))
        || count_to_size(query->offset, &offset) != PYTYLIKI_ABI_OK
        || count_to_size(query->limit, &limit) != PYTYLIKI_ABI_OK
        || count_to_size(result_capacity, &capacity) != PYTYLIKI_ABI_OK
        || limit == 0u || limit > PL_METRIC_QUERY_MAX_PAGE) {
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    }

    if (capacity > limit) capacity = limit;
    native_query.query = query->query;
    native_query.category = query->category;
    native_query.required_capabilities = query->required_capabilities;
    native_query.excluded_capabilities = query->excluded_capabilities;
    native_query.required_type_mask = query->required_type_mask;
    native_query.excluded_type_mask = query->excluded_type_mask;
    native_query.offset = offset;
    native_query.limit = limit;

    /* Count first, then fetch only the caller-owned page one row at a time.
     * This keeps the native stack independent of the maximum page size. */
    if (pl_metric_registry_search_ex(
            &native_query, NULL, 0u, &native_result
        ) != PL_METRIC_QUERY_OK) {
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    }

    out->total_matches = (uint64_t)native_result.total_matches;
    out->written = 0u;
    available = native_result.total_matches > offset
        ? native_result.total_matches - offset : 0u;
    if (capacity > available) capacity = available;

    item_query = native_query;
    item_query.limit = 1u;
    for (i = 0u; i < capacity; ++i) {
        item_query.offset = offset + i;
        item = NULL;
        if (pl_metric_registry_search_ex(
                &item_query, &item, 1u, &item_result
            ) != PL_METRIC_QUERY_OK || item_result.written != 1u || item == NULL) {
            return PYTYLIKI_ABI_INVALID_ARGUMENT;
        }
        memset(&results[i], 0, sizeof(results[i]));
        results[i].id = item->id;
        copy_embedded_text(results[i].name, sizeof(results[i].name), item->name);
        copy_embedded_text(results[i].category, sizeof(results[i].category), item->category);
        copy_embedded_text(results[i].description, sizeof(results[i].description), item->description);
        copy_embedded_text(results[i].complexity, sizeof(results[i].complexity), item->complexity);
        results[i].type_mask = item->type_mask;
        results[i].capabilities = item->capabilities;
        ++out->written;
    }
    return PYTYLIKI_ABI_OK;
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

static void copy_plugin_descriptor(pytyliki_plugin_descriptor_v1 *out,
                                   const plugin_entry *plugin) {
    memset(out, 0, sizeof(*out));
    copy_embedded_text(out->id, sizeof(out->id), plugin->id);
    copy_embedded_text(out->name, sizeof(out->name), plugin->name);
    copy_embedded_text(out->domain, sizeof(out->domain), plugin->domain);
    copy_embedded_text(out->service, sizeof(out->service), plugin->service);
    copy_embedded_text(out->capabilities, sizeof(out->capabilities), plugin->capabilities);
}

int32_t pytyliki_v1_plugin_count(uint64_t *out_count) {
    if (out_count == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
    *out_count = (uint64_t)plugin_catalog_count();
    return PYTYLIKI_ABI_OK;
}

int32_t pytyliki_v1_plugin_at(uint64_t index, pytyliki_plugin_descriptor_v1 *out) {
    size_t position = 0u;
    const plugin_entry *plugin;
    if (out == NULL || count_to_size(index, &position) != PYTYLIKI_ABI_OK)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    plugin = plugin_catalog_at(position);
    if (plugin == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
    copy_plugin_descriptor(out, plugin);
    return PYTYLIKI_ABI_OK;
}

int32_t pytyliki_v1_plugin_find(const char *id_or_name, pytyliki_plugin_descriptor_v1 *out) {
    const plugin_entry *plugin;
    if (id_or_name == NULL || id_or_name[0] == '\0' || out == NULL)
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    plugin = plugin_catalog_find(id_or_name);
    if (plugin == NULL) return PYTYLIKI_ABI_INVALID_ARGUMENT;
    copy_plugin_descriptor(out, plugin);
    return PYTYLIKI_ABI_OK;
}
