#ifndef PYTYLIKI_IRON_CORE_H
#define PYTYLIKI_IRON_CORE_H
#include <stddef.h>
#include <stdio.h>
#define PL_IRON_MAX_SOURCE_BYTES (1024u * 1024u)
#define PL_IRON_MAX_BLOCKS 64u
#define PL_IRON_MAX_NESTING 32u
#define PL_IRON_MAX_LABEL 96u
#define PL_IRON_MARKER_COUNT 20u
#define PL_IRON_AST_SCHEMA "pytyliki.iron-core-ast.v1"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
 PL_IRON_BEGIN=0, PL_IRON_CORE, PL_IRON_SYSTEM, PL_IRON_STOP, PL_IRON_LEVEL,
 PL_IRON_SELECTIVE, PL_IRON_OPERATION, PL_IRON_MASSIVE, PL_IRON_START,
 PL_IRON_NEW_WORK, PL_IRON_CONCRETE, PL_IRON_UTILITY, PL_IRON_UNIFY,
 PL_IRON_BRACHPASS, PL_IRON_TOURNAMENT, PL_IRON_CC_RULES,
 PL_IRON_LEVEL_JUMP, PL_IRON_TRUE_CONNECTIVE, PL_IRON_VERSION, PL_IRON_VERIFICATED
} pl_iron_marker_kind;
typedef struct {
 pl_iron_marker_kind kind;
 const char *token, *semantic_id, *category, *effect_class, *safety_contract;
 int opens_block;
} pl_iron_marker_info;
typedef struct {
 pl_iron_marker_kind kind;
 size_t start_offset, body_offset, body_length, end_offset, line;
 int closed;
} pl_iron_block;
typedef struct { char message[192]; size_t offset, line; } pl_iron_error;
typedef struct {
 size_t source_bytes, block_count, marker_counts[PL_IRON_MARKER_COUNT];
 size_t root_offset, root_line, version_count, policy_count;
 size_t verification_count, verification_after_root_count;
 char version[PL_IRON_MAX_LABEL], verification_label[PL_IRON_MAX_LABEL];
 int root_seen, source_seal_asserted, source_seal_is_ci_evidence;
 int semantic_validation_performed, execution_performed, hardware_access_performed;
 pl_iron_block blocks[PL_IRON_MAX_BLOCKS];
} pl_iron_program;
int pl_iron_parse(const char *source, pl_iron_program *out, pl_iron_error *error);
size_t pl_iron_catalog(const pl_iron_marker_info **out);
const pl_iron_marker_info *pl_iron_marker_for_kind(pl_iron_marker_kind kind);
int pl_iron_write_json(const pl_iron_program *program, FILE *output);
int pl_iron_write_catalog_json(FILE *output);
#ifdef __cplusplus
}
#endif
#endif
