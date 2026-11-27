#include "semantic_hardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_source(const char *path) {
    FILE *file = fopen(path, "rb");
    long length;
    char *source;
    size_t count;
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        (unsigned long)length > PL_SH_MAX_SOURCE_BYTES || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file); return NULL;
    }
    source = (char *)malloc((size_t)length + 1u);
    if (!source) { fclose(file); return NULL; }
    count = fread(source, 1u, (size_t)length, file);
    if (ferror(file) || count != (size_t)length || memchr(source, '\0', count) != NULL) {
        free(source); fclose(file); return NULL;
    }
    source[count] = '\0'; fclose(file); return source;
}

static void usage(FILE *stream) {
    fputs("Pytyliki Semantic Hardware Compiler v1\n"
          "Usage: pytyliki-semantic --check|--ir-json|--reaper-ir|--emit-c SOURCE.shw\n"
          "The source subset is bounded and declarative; MMIO nodes are represented, never executed.\n", stream);
}

int main(int argc, char **argv) {
    char *source;
    pl_sh_module module;
    pl_sh_error error;
    int ok;
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pytyliki Semantic Hardware Compiler 1.0.0"); return 0;
    }
    if (argc != 3) { usage(stderr); return 2; }
    source = read_source(argv[2]);
    if (!source) {
        fprintf(stderr, "pytyliki-semantic: cannot read %s (64 KiB limit; embedded NUL rejected)\n", argv[2]);
        return 2;
    }
    if (!pl_sh_compile(source, argv[2], &module, &error)) {
        fprintf(stderr, "%s:%u:%u: %s: %s\n", argv[2], error.line, error.column, error.code, error.message);
        free(source); return 1;
    }
    if (!strcmp(argv[1], "--check")) {
        printf("Semantic hardware module valid: %s; values=%zu symbols=%zu effects=0x%08x\n",
               module.module_name, module.value_count, module.symbol_count, module.inferred_effects);
        ok = 1;
    } else if (!strcmp(argv[1], "--ir-json")) ok = pl_sh_write_ir_json(&module, stdout);
    else if (!strcmp(argv[1], "--reaper-ir")) ok = pl_sh_write_reaper_ir_json(&module, stdout);
    else if (!strcmp(argv[1], "--emit-c")) {
        ok = pl_sh_write_c17(&module, stdout, &error);
        if (!ok) fprintf(stderr, "%s:%u:%u: %s: %s\n", argv[2], error.line, error.column, error.code, error.message);
    } else { usage(stderr); free(source); return 2; }
    free(source);
    return ok ? 0 : 1;
}
