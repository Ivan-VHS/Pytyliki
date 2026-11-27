#ifndef PYTYLIKI_IRON_SYNTAX_H
#define PYTYLIKI_IRON_SYNTAX_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_IRON_SYNTAX_SCHEMA "pytyliki.iron-syntax-ast.v1"
#define PL_IRON_SYNTAX_VERSION "1.0.0"
#define PL_IRON_SYNTAX_MAX_SOURCE_BYTES (64u * 1024u)
#define PL_IRON_SYNTAX_MAX_RECORDS 4096u
#define PL_IRON_SYNTAX_RECORD_COUNT 20u

typedef enum {
    PL_IRON_BEGIN_IRON = 0,
    PL_IRON_CORE,
    PL_IRON_SYSTEM,
    PL_IRON_STOP_OPTION,
    PL_IRON_LEVEL,
    PL_IRON_SELECTIVE,
    PL_IRON_OPERATION,
    PL_IRON_MASSIVE,
    PL_IRON_START,
    PL_IRON_NEW_WORK,
    PL_IRON_CONCRETE,
    PL_IRON_UTILITY,
    PL_IRON_UNIFY,
    PL_IRON_SETUP_BRACHPASS,
    PL_IRON_HYPER_TOURNAMENT,
    PL_IRON_BASED_RULES,
    PL_IRON_LEVEL_JUMP_TO_RUN,
    PL_IRON_TRUE_CONNECTIVE,
    PL_IRON_VERISON,
    PL_IRON_VERIFICATED
} pl_iron_syntax_kind;

typedef struct {
    pl_iron_syntax_kind kind;
    const char *name;
    const char *phrase;
    const char *concept;
    const char *marker;
    const char *family;
} pl_iron_syntax_info;

typedef struct {
    pl_iron_syntax_kind kind;
    size_t source_offset;
    size_t source_length;
    size_t line;
    size_t column;
} pl_iron_syntax_node;

typedef struct {
    pl_iron_syntax_node *nodes;
    size_t length;
    size_t source_length;
} pl_iron_syntax_program;

typedef struct {
    char code[24];
    char message[192];
    size_t offset;
    size_t line;
    size_t column;
} pl_iron_syntax_error;

/* Exact, line-oriented syntax validation. No source is executed. */
int pl_iron_syntax_parse(const char *source,
                         pl_iron_syntax_program *out,
                         pl_iron_syntax_node *storage,
                         size_t storage_capacity,
                         pl_iron_syntax_error *error);

size_t pl_iron_syntax_catalog(const pl_iron_syntax_info **out);
const pl_iron_syntax_info *pl_iron_syntax_get(pl_iron_syntax_kind kind);
int pl_iron_syntax_write_json(const pl_iron_syntax_program *program,
                              FILE *output);
int pl_iron_syntax_write_catalog_json(FILE *output);

#ifdef __cplusplus
}
#endif
#endif
