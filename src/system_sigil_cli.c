#include "system_sigil.h"

#include <stdio.h>
#include <string.h>

static void usage(FILE *stream) {
    fputs(
        "Pytyliki SystemArray sigil registry (metadata-only)\n"
        "Usage:\n"
        "  pytyliki-system-sigil --version\n"
        "  pytyliki-system-sigil --catalog\n"
        "  pytyliki-system-sigil --catalog-json\n"
        "  pytyliki-system-sigil --check TEXT\n"
        "  pytyliki-system-sigil --ast-json TEXT\n",
        stream
    );
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pytyliki SystemArray sigil registry 0.1.0 (metadata-only)");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--catalog-json") == 0)
        return pl_system_sigil_write_catalog_json(stdout) ? 0 : 1;
    if (argc == 2 && strcmp(argv[1], "--catalog") == 0) {
        const pl_system_sigil_info *items = NULL;
        size_t count = pl_system_sigil_catalog(&items);
        size_t index;
        for (index = 0u; index < count; ++index) {
            printf("%-6s %-34s %-18s %s\n",
                   items[index].token, items[index].semantic_id,
                   items[index].effect_class, items[index].scalar_signature);
        }
        return 0;
    }
    if (argc != 3 ||
        (strcmp(argv[1], "--check") != 0 &&
         strcmp(argv[1], "--ast-json") != 0)) {
        usage(stderr);
        return 2;
    }

    {
        pl_system_sigil_directive directive;
        pl_system_array_error error;
        if (!pl_system_sigil_parse(argv[2], &directive, &error)) {
            fprintf(stderr, "system-sigil syntax error at byte %zu: %s\n",
                    error.offset, error.message);
            return 1;
        }
        if (strcmp(argv[1], "--ast-json") == 0)
            return pl_system_sigil_write_json(&directive, stdout) ? 0 : 1;
        puts("Pytyliki system sigil: valid; overload resolution pending; not executed");
    }
    return 0;
}
