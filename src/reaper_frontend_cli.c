#include "reaper/ast.h"
#include "reaper/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOURCE_LIMIT (2u * 1024u * 1024u)

static char *read_source(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return NULL; }
    long length = ftell(file);
    if (length < 0 || (unsigned long)length > SOURCE_LIMIT ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    char *source = (char *)malloc((size_t)length + 1u);
    if (!source) { fclose(file); return NULL; }
    size_t read_count = fread(source, 1u, (size_t)length, file);
    int failed = ferror(file) || read_count != (size_t)length;
    fclose(file);
    if (failed || memchr(source, '\0', read_count) != NULL) {
        free(source);
        return NULL;
    }
    source[read_count] = '\0';
    return source;
}

static void usage(FILE *stream) {
    fputs(
        "Pytyliki Reaper frontend (native C)\n"
        "Usage:\n"
        "  pytyliki-reaper --version\n"
        "  pytyliki-reaper --tokens SOURCE.rpr\n"
        "  pytyliki-reaper --ast-json EXPRESSION.rpr\n"
        "  pytyliki-reaper --check-expression EXPRESSION.rpr\n"
        "\n"
        "This build tokenizes Reaper source and parses a single expression.\n"
        "It does not execute complete .rpr programs.\n",
        stream
    );
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pytyliki Reaper frontend 0.1.0 (vendored Reaper C17 core, C11 build)");
        return 0;
    }
    if (argc != 3) {
        usage(stderr);
        return 2;
    }

    int show_tokens = strcmp(argv[1], "--tokens") == 0;
    int show_ast = strcmp(argv[1], "--ast-json") == 0;
    int check_expression = strcmp(argv[1], "--check-expression") == 0;
    if (!show_tokens && !show_ast && !check_expression) {
        usage(stderr);
        return 2;
    }

    char *source = read_source(argv[2]);
    if (!source) {
        fprintf(stderr, "pytyliki-reaper: cannot read %s (limit: %u bytes; embedded NUL is rejected)\n",
                argv[2], SOURCE_LIMIT);
        return 2;
    }

    ReaperTokenArray tokens = {0};
    ReaperLexError lex_error = {{0}, 0u, 0u};
    if (!reaper_lex(source, &tokens, &lex_error)) {
        fprintf(stderr, "%s:%zu:%zu: lexer error: %s\n",
                argv[2], lex_error.line, lex_error.column, lex_error.message);
        free(source);
        return 1;
    }

    int result = 0;
    if (show_tokens) {
        for (size_t i = 0; i < tokens.length; ++i) {
            const ReaperToken *token = &tokens.items[i];
            printf("%zu:%zu %-18s %s\n", token->line, token->column,
                   reaper_token_kind_name(token->kind),
                   reaper_token_canonical_text(token));
        }
    } else {
        ReaperAst ast;
        ReaperParseError parse_error = {{0}, 0u, 0u};
        if (!reaper_parse_expression(&tokens, &ast, &parse_error)) {
            fprintf(stderr, "%s:%zu:%zu: parser error: %s\n",
                    argv[2], parse_error.line, parse_error.column, parse_error.message);
            result = 1;
        } else {
            if (show_ast) {
                if (!reaper_ast_write_json(&ast, stdout)) {
                    fputs("pytyliki-reaper: AST serialization failed\n", stderr);
                    result = 1;
                }
            } else {
                puts("Reaper expression: valid (native lexer + expression parser; not executed)");
            }
            reaper_ast_free(&ast);
        }
    }

    reaper_tokens_free(&tokens);
    free(source);
    return result;
}
