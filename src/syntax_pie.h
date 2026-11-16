#ifndef PYTYLIKI_SYNTAX_PIE_H
#define PYTYLIKI_SYNTAX_PIE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SYNTAX_PIE_LAYER_COUNT 20u

typedef enum {
    PL_SYNTAX_LEXEME = 0,
    PL_SYNTAX_TOKEN,
    PL_SYNTAX_WORD_FORM,
    PL_SYNTAX_COLLOCATION,
    PL_SYNTAX_SYNTAGMA,
    PL_SYNTAX_PHRASE,
    PL_SYNTAX_TURNOVER,
    PL_SYNTAX_CLAUSE,
    PL_SYNTAX_PREDICATIVE,
    PL_SYNTAX_SIMPLE_SENTENCE,
    PL_SYNTAX_COMPLEX_SENTENCE,
    PL_SYNTAX_BLOCK,
    PL_SYNTAX_NESTED,
    PL_SYNTAX_PARSE_TREE,
    PL_SYNTAX_AST,
    PL_SYNTAX_GRAMMAR_RULE,
    PL_SYNTAX_CFG,
    PL_SYNTAX_GRAPH,
    PL_SYNTAX_COMPOSITION,
    PL_SYNTAX_HIERARCHY
} pl_syntax_level;

typedef struct {
    unsigned id;
    const char *function_name;
    const char *room_name;
    pl_syntax_level level;
    const char *level_name;
    const char *operation;
} pl_syntax_pie_layer;

/* Static catalogue: returned strings remain valid for the process lifetime. */
size_t pl_syntax_pie_catalog(const pl_syntax_pie_layer **out);
int pl_syntax_pie_get(unsigned id, const pl_syntax_pie_layer **out);
int pl_syntax_pie_find_function(const char *name, const pl_syntax_pie_layer **out);
int pl_syntax_pie_find_room(const char *name, const pl_syntax_pie_layer **out);
int pl_syntax_pie_find_level(const char *name, const pl_syntax_pie_layer **out);
const char *pl_syntax_pie_level_name(pl_syntax_level level);

#ifdef __cplusplus
}
#endif
#endif
