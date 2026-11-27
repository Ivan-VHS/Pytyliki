#include "hardware_syntax.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_source(const char *path) {
    FILE *file = fopen(path, "rb");
    long length;
    size_t count;
    char *source;

    if (file == NULL) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    length = ftell(file);
    if (length < 0 || (unsigned long)length > PL_HW_SYNTAX_MAX_SOURCE_BYTES ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    source = (char *)malloc((size_t)length + 1u);
    if (source == NULL) {
        fclose(file);
        return NULL;
    }
    count = fread(source, 1u, (size_t)length, file);
    if (ferror(file) || count != (size_t)length ||
        memchr(source, '\0', count) != NULL) {
        free(source);
        fclose(file);
        return NULL;
    }
    source[count] = '\0';
    fclose(file);
    return source;
}

static void usage(FILE *stream) {
    fputs(
        "Pytyliki hardware syntax frontend (structural validation only)\n"
        "Usage:\n"
        "  pytyliki-hw-syntax --version\n"
        "  pytyliki-hw-syntax --catalog [--json]\n"
        "  pytyliki-hw-syntax --check FILE\n"
        "  pytyliki-hw-syntax --ast-json FILE\n",
        stream
    );
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pytyliki hardware syntax frontend 0.1.0 (structural subset)");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--catalog") == 0) {
        const pl_hw_construct_info *items = NULL;
        size_t count = pl_hw_syntax_catalog(&items);
        size_t index;
        for (index = 0u; index < count; ++index)
            printf("%-14s %-16s %s\n", items[index].keyword,
                   items[index].family, items[index].execution_model);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "--catalog") == 0 &&
        strcmp(argv[2], "--json") == 0)
        return pl_hw_syntax_write_catalog_json(stdout) ? 0 : 1;

    if (argc != 3 ||
        (strcmp(argv[1], "--check") != 0 &&
         strcmp(argv[1], "--ast-json") != 0)) {
        usage(stderr);
        return 2;
    }

    char *source = read_source(argv[2]);
    if (source == NULL) {
        fprintf(stderr, "pytyliki-hw-syntax: cannot read %s (maximum: %u bytes; embedded NUL rejected)\n",
                argv[2], PL_HW_SYNTAX_MAX_SOURCE_BYTES);
        return 2;
    }

    pl_hw_construct_node node;
    pl_hw_syntax_error error;
    if (!pl_hw_syntax_parse_one(source, &node, &error)) {
        fprintf(stderr, "%s: structural syntax error at byte %zu: %s\n",
                argv[2], error.offset, error.message);
        free(source);
        return 1;
    }

    int result = 0;
    if (strcmp(argv[1], "--ast-json") == 0) {
        if (!pl_hw_syntax_write_json(&node, stdout)) {
            fputs("pytyliki-hw-syntax: AST serialization failed\n", stderr);
            result = 1;
        }
    } else {
        printf("Pytyliki construct: structurally valid (%s, name=%s; not compiled or executed)\n",
               pl_hw_construct_name(node.kind), node.name);
    }

    free(source);
    return result;
}
