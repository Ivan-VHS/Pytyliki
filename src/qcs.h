#ifndef PYTYLIKI_QCS_H
#define PYTYLIKI_QCS_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PL_QCS_EOF = 0,
    PL_QCS_NEWLINE,
    PL_QCS_IDENTIFIER,
    PL_QCS_KEYWORD,
    PL_QCS_FLOW,
    PL_QCS_NUMBER,
    PL_QCS_DURATION,
    PL_QCS_STRING,
    PL_QCS_FRAME_OPEN,
    PL_QCS_FRAME_CLOSE,
    PL_QCS_OPERATOR,
    PL_QCS_PUNCTUATION
} pl_qcs_token_kind;

typedef struct {
    pl_qcs_token_kind kind;
    char *text;
    size_t line;
    size_t column;
} pl_qcs_token;

typedef struct {
    pl_qcs_token *items;
    size_t length;
    size_t capacity;
} pl_qcs_tokens;

typedef struct {
    char message[256];
    size_t line;
    size_t column;
} pl_qcs_error;

typedef enum {
    PL_QCS_NODE_MODULE = 1,
    PL_QCS_NODE_FRAME,
    PL_QCS_NODE_FUNCTION,
    PL_QCS_NODE_FLOW
} pl_qcs_node_kind;

typedef struct {
    pl_qcs_node_kind kind;
    char *name;
    size_t parent;
    size_t line;
    size_t column;
} pl_qcs_node;

typedef struct {
    pl_qcs_node *items;
    size_t length;
    size_t capacity;
    size_t root;
    size_t frame_count;
    size_t function_count;
    size_t flow_count;
} pl_qcs_ast;

int pl_qcs_lex(const char *source, pl_qcs_tokens *out, pl_qcs_error *error);
void pl_qcs_tokens_free(pl_qcs_tokens *tokens);
const char *pl_qcs_token_kind_name(pl_qcs_token_kind kind);

int pl_qcs_parse(const pl_qcs_tokens *tokens, pl_qcs_ast *out, pl_qcs_error *error);
int pl_qcs_parse_source(const char *source, pl_qcs_ast *out, pl_qcs_error *error);
void pl_qcs_ast_free(pl_qcs_ast *ast);
const char *pl_qcs_node_kind_name(pl_qcs_node_kind kind);
size_t pl_qcs_domain_count(void);
int pl_qcs_domain_known(const char *name);
int pl_qcs_ast_write_json(const pl_qcs_ast *ast, FILE *output);

#ifdef __cplusplus
}
#endif
#endif
