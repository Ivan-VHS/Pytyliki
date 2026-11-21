#include "qcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_source(const char *path) {
    FILE *file = fopen(path, "rb");
    long length;
    char *buffer;
    size_t read_count;
    if (file == NULL) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) { fclose(file); return NULL; }
    buffer = (char *)malloc((size_t)length + 1u);
    if (buffer == NULL) { fclose(file); return NULL; }
    read_count = fread(buffer, 1u, (size_t)length, file);
    if (read_count != (size_t)length || ferror(file)) { free(buffer); fclose(file); return NULL; }
    buffer[read_count] = '\0';
    fclose(file);
    return buffer;
}

static void usage(FILE *f) {
    fprintf(f, "Pytyliki QCS 1.0 — native frame-aware syntax frontend\n"
               "Usage: pytyliki-qcs check|tokens|ast <source.qcs>\n");
}

int main(int argc, char **argv) {
    const char *mode, *path;
    char *source;
    pl_qcs_error error = {{0}, 0u, 0u};
    if (argc != 3) { usage(stderr); return 2; }
    mode = argv[1]; path = argv[2];
    source = read_source(path);
    if (source == NULL) { fprintf(stderr, "pytyliki-qcs: cannot read %s\n", path); return 2; }

    if (strcmp(mode, "tokens") == 0) {
        pl_qcs_tokens tokens = {0};
        size_t i;
        if (!pl_qcs_lex(source, &tokens, &error)) {
            fprintf(stderr, "%s:%zu:%zu: %s\n", path, error.line, error.column, error.message);
            free(source); return 1;
        }
        for (i = 0; i < tokens.length; ++i)
            printf("%zu:%zu %-12s %s\n", tokens.items[i].line, tokens.items[i].column,
                   pl_qcs_token_kind_name(tokens.items[i].kind), tokens.items[i].text);
        pl_qcs_tokens_free(&tokens);
        free(source);
        return 0;
    }

    if (strcmp(mode, "check") == 0 || strcmp(mode, "ast") == 0) {
        pl_qcs_ast ast = {0};
        int ok = pl_qcs_parse_source(source, &ast, &error);
        free(source);
        if (!ok) {
            fprintf(stderr, "%s:%zu:%zu: QCS error: %s\n", path, error.line, error.column, error.message);
            return 1;
        }
        if (strcmp(mode, "ast") == 0) {
            if (!pl_qcs_ast_write_json(&ast, stdout)) {
                fprintf(stderr, "pytyliki-qcs: cannot serialize AST\n");
                pl_qcs_ast_free(&ast);
                return 1;
            }
        } else {
            printf("QCS valid: %s; frames=%zu functions=%zu flow-ops=%zu domains=%zu\n",
                   path, ast.frame_count, ast.function_count, ast.flow_count, pl_qcs_domain_count());
        }
        pl_qcs_ast_free(&ast);
        return 0;
    }

    free(source);
    usage(stderr);
    return 2;
}
