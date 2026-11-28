#ifndef PYTYLIKI_HARDWARE_SYNTAX_H
#define PYTYLIKI_HARDWARE_SYNTAX_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_HW_SYNTAX_MAX_SOURCE_BYTES (1024u * 1024u)
#define PL_HW_SYNTAX_MAX_NAME 96u
#define PL_HW_SYNTAX_CONSTRUCT_COUNT 21u
#define PL_HW_SYNTAX_SCHEMA "pytyliki.hardware-construct-ast.v1"

typedef enum {
    PL_HW_CONSTRUCT_IRQ_TABLE = 0,
    PL_HW_CONSTRUCT_STREAM,
    PL_HW_CONSTRUCT_STACK_PROC,
    PL_HW_CONSTRUCT_FSM,
    PL_HW_CONSTRUCT_CONTRACT,
    PL_HW_CONSTRUCT_ASSEMBLY_SECTION,
    PL_HW_CONSTRUCT_REACT,
    PL_HW_CONSTRUCT_MATRIX,
    PL_HW_CONSTRUCT_FUNCTION,
    PL_HW_CONSTRUCT_SCHEDULE,
    PL_HW_CONSTRUCT_RULE,
    PL_HW_CONSTRUCT_ATOMIC_TX,
    PL_HW_CONSTRUCT_DAG,
    PL_HW_CONSTRUCT_MACRO,
    PL_HW_CONSTRUCT_HOT_PATCH,
    PL_HW_CONSTRUCT_PROBE,
    PL_HW_CONSTRUCT_DEVICE,
    PL_HW_CONSTRUCT_INVARIANT,
    PL_HW_CONSTRUCT_MEMORY_MAP,
    PL_HW_CONSTRUCT_TRACE_SCRIPT,
    PL_HW_CONSTRUCT_PHYSICAL_MODEL
} pl_hw_construct_kind;

typedef enum {
    PL_HW_BODY_BLOCK = 0,
    PL_HW_BODY_PIPELINE,
    PL_HW_BODY_EXPRESSION,
    PL_HW_BODY_ASSEMBLY
} pl_hw_body_kind;

typedef struct {
    pl_hw_construct_kind kind;
    const char *keyword;
    const char *family;
    const char *execution_model;
    const char *safety_notes;
} pl_hw_construct_info;

typedef struct {
    pl_hw_construct_kind kind;
    pl_hw_body_kind body_kind;
    char name[PL_HW_SYNTAX_MAX_NAME];
    size_t source_offset;
    size_t source_length;
    size_t body_offset;
    size_t body_length;
} pl_hw_construct_node;

typedef struct {
    char message[192];
    size_t offset;
} pl_hw_syntax_error;

/* Structural parsing only; no name resolution, type checking or execution. */
int pl_hw_syntax_parse_one(const char *source,
                           pl_hw_construct_node *out,
                           pl_hw_syntax_error *error);

size_t pl_hw_syntax_catalog(const pl_hw_construct_info **out);
const char *pl_hw_construct_name(pl_hw_construct_kind kind);
const char *pl_hw_body_kind_name(pl_hw_body_kind kind);
int pl_hw_syntax_write_json(const pl_hw_construct_node *node, FILE *output);
int pl_hw_syntax_write_catalog_json(FILE *output);

#ifdef __cplusplus
}
#endif
#endif
