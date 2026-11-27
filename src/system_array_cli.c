#include "system_array.h"

#include <stdio.h>
#include <string.h>

static void usage(FILE *stream) {
    fputs(
        "Pytyliki system-array syntax frontend\n"
        "Usage:\n"
        "  pytyliki-system-array --version\n"
        "  pytyliki-system-array --check TEXT\n"
        "  pytyliki-system-array --normalize TEXT\n"
        "  pytyliki-system-array --json TEXT\n"
        "\n"
        "The frontend validates one directive; it never accesses hardware.\n",
        stream
    );
}

int main(int argc, char **argv) {
    pl_system_array_directive directive;
    pl_system_array_error error;

    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pytyliki system-array frontend 1.0.0 (syntax-only)");
        return 0;
    }

    if (argc != 3 ||
        (strcmp(argv[1], "--check") != 0 &&
         strcmp(argv[1], "--normalize") != 0 &&
         strcmp(argv[1], "--json") != 0)) {
        usage(stderr);
        return 2;
    }

    if (!pl_system_array_parse(argv[2], &directive, &error)) {
        fprintf(stderr, "system-array syntax error at byte %zu: %s\n",
                error.offset, error.message);
        return 1;
    }

    if (strcmp(argv[1], "--json") == 0) {
        return pl_system_array_write_json(&directive, stdout) ? 0 : 1;
    }

    if (strcmp(argv[1], "--normalize") == 0) {
        puts(pl_system_array_canonical_name());
        return 0;
    }

    puts("system-array directive: valid (syntax checked; not executed)");
    return 0;
}
