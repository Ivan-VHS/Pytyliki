#include "iron_syntax.h"
#include <stdlib.h>
#include <string.h>

static int read_input(const char *path, char *buffer, size_t capacity) {
    FILE *stream = strcmp(path, "-") == 0 ? stdin : fopen(path, "rb");
    size_t count;
    int close_stream = stream != stdin;
    if (stream == NULL) return 0;
    count = fread(buffer, 1u, capacity - 1u, stream);
    if (ferror(stream) || count == capacity - 1u) {
        if (close_stream) (void)fclose(stream);
        return 0;
    }
    buffer[count] = '\0';
    if (close_stream && fclose(stream) != 0) return 0;
    return 1;
}

static void usage(FILE *stream) {
    fputs("Pytyliki Iron Syntax v1 (syntax-only)\n"
          "Usage:\n"
          "  pytyliki-iron --version\n"
          "  pytyliki-iron --catalog\n"
          "  pytyliki-iron --check FILE|-\n"
          "  pytyliki-iron --ast-json FILE|-\n", stream);
}

int main(int argc, char **argv) {
    char *source;
    pl_iron_syntax_node *nodes;
    pl_iron_syntax_program program;
    pl_iron_syntax_error error;
    int result = 0;

    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pytyliki Iron Syntax " PL_IRON_SYNTAX_VERSION " (syntax-only)");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--catalog") == 0)
        return pl_iron_syntax_write_catalog_json(stdout) ? 0 : 1;
    if (argc != 3 || (strcmp(argv[1], "--check") != 0 &&
                      strcmp(argv[1], "--ast-json") != 0)) {
        usage(stderr);
        return 2;
    }

    source = (char *)malloc(PL_IRON_SYNTAX_MAX_SOURCE_BYTES + 2u);
    nodes = (pl_iron_syntax_node *)calloc(PL_IRON_SYNTAX_MAX_RECORDS, sizeof(*nodes));
    if (source == NULL || nodes == NULL) {
        fputs("unable to allocate bounded Iron parser buffers\n", stderr);
        free(source);
        free(nodes);
        return 2;
    }
    if (!read_input(argv[2], source, PL_IRON_SYNTAX_MAX_SOURCE_BYTES + 2u)) {
        fputs("unable to read input or input exceeds 64 KiB\n", stderr);
        result = 2;
        goto cleanup;
    }
    if (!pl_iron_syntax_parse(source, &program, nodes, PL_IRON_SYNTAX_MAX_RECORDS, &error)) {
        fprintf(stderr, "%s at %zu:%zu (byte %zu): %s\n",
                error.code, error.line, error.column, error.offset, error.message);
        result = 1;
        goto cleanup;
    }
    if (strcmp(argv[1], "--ast-json") == 0)
        result = pl_iron_syntax_write_json(&program, stdout) ? 0 : 1;
    else
        printf("Iron syntax valid: %zu records (syntax checked; not executed)\n", program.length);

cleanup:
    free(nodes);
    free(source);
    return result;
}
