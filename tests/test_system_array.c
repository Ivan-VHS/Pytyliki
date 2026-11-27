#include "system_array.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_documented_spellings(void) {
    static const char *const cases[] = {
        "{]+/*+?$ system(array) ^>",
        "{]+/* System(Array) ^>",
        "{]+/* SYSTEM(ARRAY) !==",
        "*/+? system_array !==",
        "{]/* systemArray >^",
        "$+?/ SystemArray >^",
        "*]+$ SYSTEM_ARRAY >^",
        "{+/*? system-array >^",
        "?+$} system.array !!==",
        "[/+*] system::array ^^>>",
        "{?$+* system/array >>^^",
        "*?{/+ system\\array ^>>",
        "*?{/+ system[array] >>^",
        "{/*+?$ system >>^",
        "]+?*/ system >^=",
        "$/*+} system(array) !^>",
        "{]?$+ System(Array) ^!>",
        "{]?$+ SYSTEM(ARRAY) >!^",
        "{]?$+ system_array ^>==!",
        "systemArray ^>==!"
    };
    size_t index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        pl_system_array_directive directive;
        pl_system_array_error error;
        assert(pl_system_array_parse(cases[index], &directive, &error));
        assert(strcmp(pl_system_array_canonical_name(), "system(array)") == 0);
        assert(directive.source_length == strlen(directive.spelling));
    }
}

static void test_prefix_marker_identity(void) {
    static const struct {
        const char *prefix;
        unsigned marker_id;
    } cases[] = {
        {"{]+/*+?$", 1u}, {"{]+/*", 2u}, {"*/+?", 3u},
        {"{]/*", 4u}, {"$+?/", 5u}, {"*]+$", 6u},
        {"{+/*?", 7u}, {"?+$}", 8u}, {"[/+*]", 9u},
        {"{?$+*", 10u}, {"*?{/+", 11u}, {"{/*+?$", 12u},
        {"]+?*/", 13u}, {"$/*+}", 14u}, {"{]?$+", 15u}
    };
    size_t index;
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        char source[80];
        pl_system_array_directive directive;
        pl_system_array_error error;
        (void)snprintf(source, sizeof(source), "%s system(array) ^>", cases[index].prefix);
        assert(pl_system_array_parse(source, &directive, &error));
        assert(directive.prefix_marker == cases[index].marker_id);
        assert(strcmp(pl_system_array_prefix_marker(directive.prefix_marker),
                      cases[index].prefix) == 0);
    }
}

static void test_suffix_marker_identity(void) {
    static const struct {
        const char *suffix;
        unsigned marker_id;
    } cases[] = {
        {"^>", 1u}, {"!==", 2u}, {">^", 3u}, {"!!==", 4u},
        {"^^>>", 5u}, {">>^^", 6u}, {"!^>", 7u}, {"^!>", 8u},
        {">!^", 9u}, {">^=", 10u}, {"^>>", 11u}, {">>^", 12u},
        {"^>==!", 13u}
    };
    size_t index;
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        char source[64];
        pl_system_array_directive directive;
        pl_system_array_error error;
        (void)snprintf(source, sizeof(source), "system(array) %s", cases[index].suffix);
        assert(pl_system_array_parse(source, &directive, &error));
        assert(directive.suffix_marker == cases[index].marker_id);
        assert(strcmp(pl_system_array_suffix_marker(directive.suffix_marker),
                      cases[index].suffix) == 0);
    }
}

static void test_alias_normalization(void) {
    static const char *const aliases[] = {
        "system(array)", "System(Array)", "SYSTEM(ARRAY)",
        "system_array", "systemArray", "SYSTEM_ARRAY", "system-array",
        "system.array", "system::array", "system/array", "system\\array",
        "system[array]"
    };
    size_t index;
    for (index = 0u; index < sizeof(aliases) / sizeof(aliases[0]); ++index) {
        pl_system_array_directive directive;
        pl_system_array_error error;
        assert(pl_system_array_parse(aliases[index], &directive, &error));
        assert(!directive.shorthand);
        assert(directive.prefix_marker == 0u);
        assert(directive.suffix_marker == 0u);
    }
}

static void test_rejects_unknown_or_ambiguous_input(void) {
    static const char *const invalid[] = {
        "system(vector)",
        "system_array_extra",
        "prefix system(array)",
        "system(array) nonsense",
        "system(array) ^??",
        "system",
        "{unknown} system(array) ^>",
        "system(array) { capacity: 64; }"
    };
    size_t index;
    for (index = 0u; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        pl_system_array_directive directive;
        pl_system_array_error error;
        assert(!pl_system_array_parse(invalid[index], &directive, &error));
        assert(error.message[0] != '\0');
    }
}

static void test_framed_bare_shorthand(void) {
    pl_system_array_directive directive;
    pl_system_array_error error;
    assert(pl_system_array_parse("{/*+?$ system >>^", &directive, &error));
    assert(directive.shorthand);
    assert(strcmp(directive.spelling, "system") == 0);
    assert(strcmp(pl_system_array_prefix_marker(directive.prefix_marker), "{/*+?$") == 0);
}

static void test_bounds_and_null_arguments(void) {
    pl_system_array_directive directive;
    pl_system_array_error error;
    char oversized[PL_SYSTEM_ARRAY_MAX_INPUT + 2u];
    memset(oversized, 'x', sizeof(oversized));
    oversized[sizeof(oversized) - 1u] = '\0';
    assert(!pl_system_array_parse(oversized, &directive, &error));
    assert(strstr(error.message, "input limit") != NULL);
    assert(!pl_system_array_parse(NULL, &directive, &error));
    assert(!pl_system_array_parse("system(array)", NULL, &error));
}

static void test_json_contract(void) {
    pl_system_array_directive directive;
    pl_system_array_error error;
    FILE *stream = tmpfile();
    char buffer[512];
    size_t count;
    assert(stream != NULL);
    assert(pl_system_array_parse("{]+/* systemArray >^", &directive, &error));
    assert(pl_system_array_write_json(&directive, stream));
    rewind(stream);
    count = fread(buffer, 1u, sizeof(buffer) - 1u, stream);
    buffer[count] = '\0';
    assert(strstr(buffer, "pytyliki.system-array-ast.v1") != NULL);
    assert(strstr(buffer, "\"kind\":\"system.array\"") != NULL);
    assert(strstr(buffer, "\"source_execution_performed\":false") != NULL);
    fclose(stream);
}

int main(void) {
    test_documented_spellings();
    test_prefix_marker_identity();
    test_suffix_marker_identity();
    test_alias_normalization();
    test_rejects_unknown_or_ambiguous_input();
    test_framed_bare_shorthand();
    test_bounds_and_null_arguments();
    test_json_contract();
    puts("Pytyliki system-array syntax tests passed");
    return 0;
}
