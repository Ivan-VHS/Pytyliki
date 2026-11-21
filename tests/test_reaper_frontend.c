#include "reaper/ast.h"
#include "reaper/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_unicode_identifier_and_alias(void) {
    const char *source = "пусть значение := 7\n";
    ReaperTokenArray tokens = {0};
    ReaperLexError error = {{0}, 0u, 0u};
    assert(reaper_lex(source, &tokens, &error));
    assert(tokens.length >= 5u);
    assert(tokens.items[0].kind == REAPER_TOK_KEYWORD);
    assert(strcmp(reaper_token_canonical_text(&tokens.items[0]), "make") == 0);
    assert(tokens.items[1].kind == REAPER_TOK_IDENTIFIER);
    assert(strcmp(tokens.items[1].text, "значение") == 0);
    assert(strcmp(tokens.items[2].text, ":=") == 0);
    reaper_tokens_free(&tokens);
}

static void test_expression_precedence(void) {
    const char *source = "2 + 3 * 4";
    ReaperTokenArray tokens = {0};
    ReaperLexError lex_error = {{0}, 0u, 0u};
    assert(reaper_lex(source, &tokens, &lex_error));

    ReaperAst ast;
    ReaperParseError parse_error = {{0}, 0u, 0u};
    assert(reaper_parse_expression(&tokens, &ast, &parse_error));
    const ReaperAstNode *root = &ast.items[ast.root];
    assert(root->kind == REAPER_AST_BINARY);
    assert(strcmp(root->text, "+") == 0);
    assert(root->right != REAPER_AST_NO_NODE);
    assert(ast.items[root->right].kind == REAPER_AST_BINARY);
    assert(strcmp(ast.items[root->right].text, "*") == 0);

    reaper_ast_free(&ast);
    reaper_tokens_free(&tokens);
}

static void test_malformed_expression_is_rejected(void) {
    ReaperTokenArray tokens = {0};
    ReaperLexError lex_error = {{0}, 0u, 0u};
    assert(reaper_lex("1 +", &tokens, &lex_error));

    ReaperAst ast;
    ReaperParseError parse_error = {{0}, 0u, 0u};
    assert(!reaper_parse_expression(&tokens, &ast, &parse_error));
    assert(parse_error.line == 1u);
    assert(parse_error.column >= 3u);
    reaper_tokens_free(&tokens);
}

int main(void) {
    test_unicode_identifier_and_alias();
    test_expression_precedence();
    test_malformed_expression_is_rejected();
    puts("Reaper native frontend tests passed");
    return 0;
}
