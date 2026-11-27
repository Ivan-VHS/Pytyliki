#include "processor_languages_cli.h"

#include "processor_languages.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_language_usage(void) {
    puts("Usage:");
    puts("  pytyliki --language catalog");
    puts("  pytyliki --language show ID_OR_NAME");
    puts("  pytyliki --language detect FILE");
    puts("  pytyliki --language inspect ID_OR_NAME FILE");
    puts("  pytyliki --language target ID_OR_NAME ARCH");
    puts("  pytyliki --language plan ID_OR_NAME ARCH SOURCE OUTPUT");
    puts("The workbench inspects source and prepares argument vectors; it never launches a toolchain.");
}

static int read_source_file(const char *path, char **contents, size_t *length) {
    FILE *input;
    long file_size;
    char *buffer;
    size_t bytes_read;
    int close_status;

    if (path == NULL || contents == NULL || length == NULL) return -1;
    *contents = NULL;
    *length = 0u;
    input = fopen(path, "rb");
    if (input == NULL) return -1;
    if (fseek(input, 0L, SEEK_END) != 0 || (file_size = ftell(input)) < 0L ||
        (unsigned long)file_size > (unsigned long)PL_LANGUAGE_SOURCE_MAX ||
        fseek(input, 0L, SEEK_SET) != 0) {
        fclose(input);
        return -1;
    }
    buffer = (char *)malloc((size_t)file_size + 1u);
    if (buffer == NULL) {
        fclose(input);
        return -1;
    }
    bytes_read = fread(buffer, 1u, (size_t)file_size, input);
    close_status = fclose(input);
    if (bytes_read != (size_t)file_size || close_status != 0) {
        free(buffer);
        return -1;
    }
    if (memchr(buffer, '\0', bytes_read) != NULL) {
        free(buffer);
        return -1;
    }
    buffer[bytes_read] = '\0';
    *contents = buffer;
    *length = bytes_read;
    return 0;
}

static void print_profile(const pl_language_profile *profile) {
    printf("id: %s\nname: %s\nkind: %s\nfamily: %s\nextension: %s\n"
           "toolchain: %s\ntarget profile: %s\n",
           profile->id, profile->name, pl_language_kind_name(profile->kind),
           profile->family, profile->extension, profile->toolchain,
           profile->target_summary);
    printf("build route: %s\n", profile->route == PL_LANGUAGE_ROUTE_NONE
           ? "reference-only" : "argument-plan available");
    printf("scope: %s\n", profile->support_note);
}

static int print_catalog(void) {
    const pl_language_profile *items = NULL;
    const size_t count = pl_language_catalog(&items);
    if (count != PL_LANGUAGE_COUNT || items == NULL) return 1;
    printf("Pytyliki processor language workbench: %u unique profiles\n",
           (unsigned)count);
    puts("ID                         KIND                 NAME");
    puts("--------------------------------------------------------------------------");
    for (size_t i = 0u; i < count; ++i) {
        printf("%-26s %-20s %s\n", items[i].id,
               pl_language_kind_name(items[i].kind), items[i].name);
    }
    puts("Profiles are metadata and native analysis routes, not 66 bundled compilers.");
    return 0;
}

static int command_detect(const char *path) {
    char *source = NULL;
    size_t length = 0u;
    const pl_language_profile *profile = NULL;
    int result;

    if (read_source_file(path, &source, &length) != 0) {
        fprintf(stderr, "Cannot read source file within the 1 MiB workbench limit: %s\n", path);
        return 2;
    }
    result = pl_language_detect(path, source, length, &profile);
    free(source);
    if (result == 1) {
        printf("No registered language profile matched: %s\n", path);
        return 1;
    }
    if (result != 0 || profile == NULL) {
        fprintf(stderr, "Language detection failed or was ambiguous: %s\n", path);
        return 2;
    }
    printf("%s -> %s (%s)\n", path, profile->name, profile->id);
    return 0;
}

static int command_inspect(const char *id, const char *path) {
    char *source = NULL;
    size_t length = 0u;
    pl_language_report report = {0};
    int result;

    if (read_source_file(path, &source, &length) != 0) {
        fprintf(stderr, "Cannot read source file within the 1 MiB workbench limit: %s\n", path);
        return 2;
    }
    result = pl_language_inspect(id, source, length, &report);
    free(source);
    if (result != 0) {
        fprintf(stderr, "Structural inspection failed (code %d, byte offset %u).\n",
                result, (unsigned)report.error_offset);
        return 3;
    }
    printf("Source structure: PASS\nbytes: %u\nlines: %u\ntokens: %u\n"
           "comments: %u\nstrings: %u\nmaximum delimiter depth: %u\n",
           (unsigned)report.source_bytes, (unsigned)report.line_count,
           (unsigned)report.token_count, (unsigned)report.comment_count,
           (unsigned)report.string_count, (unsigned)report.max_delimiter_depth);
    puts("This is a bounded structural scan, not a full language parser or compiler.");
    return 0;
}

static int command_target(const char *id, const char *target) {
    int compatible = 0;
    if (pl_language_check_target(id, target, &compatible) != 0) {
        fprintf(stderr, "Unknown language profile or target: %s / %s\n", id, target);
        return 2;
    }
    printf("%s on %s: %s\n", id, target,
           compatible ? "compatible profile" : "unsupported target");
    return compatible ? 0 : 3;
}

static int command_plan(const char *id, const char *target,
                        const char *source, const char *output) {
    pl_language_build_plan plan;
    int result = pl_language_build_plan_create(id, target, source, output, &plan);
    if (result != 0) {
        fprintf(stderr, "Build plan unavailable or invalid (code %d).\n", result);
        return result == -4 ? 3 : 2;
    }
    puts("Build plan only; process execution: disabled; hardware writes: none");
    printf("executable: %s\n", plan.executable);
    for (size_t i = 0u; i < plan.argument_count; ++i)
        printf("argv[%u]: %s\n", (unsigned)i, plan.arguments[i]);
    puts("The arguments are separate values, not a shell command string.");
    return 0;
}

int pl_language_cli(int argc, char **argv) {
    const char *operation;
    if (argc < 2 || argv == NULL) {
        print_language_usage();
        return 2;
    }
    operation = argv[1];
    if (strcmp(operation, "catalog") == 0 && argc == 2) return print_catalog();
    if (strcmp(operation, "show") == 0 && argc == 3) {
        const pl_language_profile *profile = pl_language_find(argv[2]);
        if (profile == NULL) {
            fprintf(stderr, "Unknown language profile: %s\n", argv[2]);
            return 2;
        }
        print_profile(profile);
        return 0;
    }
    if (strcmp(operation, "detect") == 0 && argc == 3)
        return command_detect(argv[2]);
    if (strcmp(operation, "inspect") == 0 && argc == 4)
        return command_inspect(argv[2], argv[3]);
    if (strcmp(operation, "target") == 0 && argc == 4)
        return command_target(argv[2], argv[3]);
    if (strcmp(operation, "plan") == 0 && argc == 6)
        return command_plan(argv[2], argv[3], argv[4], argv[5]);
    print_language_usage();
    return 2;
}
