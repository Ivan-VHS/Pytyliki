#ifndef PYTYLIKI_SYSTEM_ARRAY_H
#define PYTYLIKI_SYSTEM_ARRAY_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SYSTEM_ARRAY_MAX_INPUT 512u
#define PL_SYSTEM_ARRAY_CANONICAL "system(array)"
#define PL_SYSTEM_ARRAY_SCHEMA "pytyliki.system-array-ast.v1"

typedef enum {
    PL_SYSTEM_ARRAY_ALIAS_PAREN = 0,
    PL_SYSTEM_ARRAY_ALIAS_SNAKE,
    PL_SYSTEM_ARRAY_ALIAS_CAMEL,
    PL_SYSTEM_ARRAY_ALIAS_KEBAB,
    PL_SYSTEM_ARRAY_ALIAS_DOTTED,
    PL_SYSTEM_ARRAY_ALIAS_SCOPED,
    PL_SYSTEM_ARRAY_ALIAS_SLASHED,
    PL_SYSTEM_ARRAY_ALIAS_BACKSLASHED,
    PL_SYSTEM_ARRAY_ALIAS_BRACKETED,
    PL_SYSTEM_ARRAY_ALIAS_BARE,
    PL_SYSTEM_ARRAY_ALIAS_COUNT
} pl_system_array_alias;

typedef struct {
    pl_system_array_alias alias;
    unsigned prefix_marker;
    unsigned suffix_marker;
    size_t source_offset;
    size_t source_length;
    int shorthand;
    char spelling[32];
} pl_system_array_directive;

typedef struct {
    char message[160];
    size_t offset;
} pl_system_array_error;

/* Parse one framed system-array directive. This validates syntax only. */
int pl_system_array_parse(const char *source,
                          pl_system_array_directive *out,
                          pl_system_array_error *error);

const char *pl_system_array_alias_name(pl_system_array_alias alias);
const char *pl_system_array_prefix_marker(unsigned marker_id);
const char *pl_system_array_suffix_marker(unsigned marker_id);
const char *pl_system_array_canonical_name(void);

/* Write one stable JSON AST record. Returns 1 on success, 0 on I/O failure. */
int pl_system_array_write_json(const pl_system_array_directive *directive,
                               FILE *output);

#ifdef __cplusplus
}
#endif
#endif
