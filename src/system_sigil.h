#ifndef PYTYLIKI_SYSTEM_SIGIL_H
#define PYTYLIKI_SYSTEM_SIGIL_H

#include "system_array.h"

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SYSTEM_SIGIL_COUNT 13u
#define PL_SYSTEM_SIGIL_SCHEMA "pytyliki.system-sigil-ast.v1"
#define PL_SYSTEM_SIGIL_CATALOG_SCHEMA "pytyliki.system-sigil-catalog.v1"
#define PL_SYSTEM_SIGIL_CANONICAL_SOURCE "system::array"
#define PL_SYSTEM_SIGIL_LEGACY_AST_NAME "system(array)"

typedef enum {
    PL_SYSTEM_SIGIL_ROTATE_LEFT = 1,
    PL_SYSTEM_SIGIL_NOT_EQUAL,
    PL_SYSTEM_SIGIL_ROTATE_RIGHT,
    PL_SYSTEM_SIGIL_PATTERN_NOT_EQUAL,
    PL_SYSTEM_SIGIL_VECTOR_LANE_SHIFT_LEFT,
    PL_SYSTEM_SIGIL_VECTOR_LANE_SHIFT_RIGHT,
    PL_SYSTEM_SIGIL_ANY_MASKED_BIT_SET,
    PL_SYSTEM_SIGIL_ALL_MASKED_BITS_MATCH,
    PL_SYSTEM_SIGIL_XOR,
    PL_SYSTEM_SIGIL_ROTATE_RIGHT_ASSIGN,
    PL_SYSTEM_SIGIL_ARITHMETIC_SHIFT_RIGHT,
    PL_SYSTEM_SIGIL_LOGICAL_SHIFT_RIGHT,
    PL_SYSTEM_SIGIL_LOGIC_EQUAL_WITH_VALIDITY
} pl_system_sigil_kind;

typedef struct {
    pl_system_sigil_kind kind;
    const char *token;
    const char *semantic_id;
    const char *scalar_signature;
    const char *system_array_signature;
    const char *effect_class;
    const char *applicable_domains;
    const char *safety_contract;
    int requires_active_mutation_transaction;
} pl_system_sigil_info;

typedef struct {
    pl_system_array_directive system_array;
    pl_system_sigil_kind kind;
} pl_system_sigil_directive;

/*
 * Parse one SystemArray directive whose suffix marker selects a registered
 * sigil. Parsing and catalog lookup are metadata-only: no overload resolution,
 * evaluation, memory access, register access, or mutation is performed.
 */
int pl_system_sigil_parse(const char *source,
                          pl_system_sigil_directive *out,
                          pl_system_array_error *error);

/* Return the immutable, versioned operator catalog. */
size_t pl_system_sigil_catalog(const pl_system_sigil_info **out);
const pl_system_sigil_info *pl_system_sigil_lookup(const char *token);
const pl_system_sigil_info *pl_system_sigil_info_for_kind(pl_system_sigil_kind kind);
const char *pl_system_sigil_kind_name(pl_system_sigil_kind kind);

/* Stable JSON serialization. Returns 1 on success, 0 on I/O or input error. */
int pl_system_sigil_write_json(const pl_system_sigil_directive *directive,
                               FILE *output);
int pl_system_sigil_write_catalog_json(FILE *output);

#ifdef __cplusplus
}
#endif
#endif
