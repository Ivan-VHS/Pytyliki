#include "system_array.h"

#include <stdio.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

typedef struct {
    const char *text;
    unsigned id;
} marker_entry;

typedef struct {
    const char *text;
    pl_system_array_alias alias;
} alias_entry;

/* Longest markers come first so overlapping framing prefixes are unambiguous. */
static const marker_entry prefix_markers[] = {
    {"{]+/*+?$", 1u},
    {"{]+/*", 2u},
    {"{]?$+", 15u},
    {"{?$+*", 10u},
    {"*?{/+", 11u},
    {"{/*+?$", 12u},
    {"{+/*?", 7u},
    {"[/+*]", 9u},
    {"$/*+}", 14u},
    {"*]+$", 6u},
    {"$+?/", 5u},
    {"?+$}", 8u},
    {"*/+?", 3u},
    {"{]/*", 4u},
    {"]+?*/", 13u}
};

static const marker_entry suffix_markers[] = {
    {"^>==!", 13u},
    {"!!==", 4u},
    {"^^>>", 5u},
    {">>^^", 6u},
    {"!^>", 7u},
    {"^!>", 8u},
    {">!^", 9u},
    {"!==", 2u},
    {">^=", 10u},
    {"^>>", 11u},
    {">>^", 12u},
    {"^>", 1u},
    {">^", 3u}
};

static const alias_entry aliases[] = {
    {"system(array)", PL_SYSTEM_ARRAY_ALIAS_PAREN},
    {"system_array", PL_SYSTEM_ARRAY_ALIAS_SNAKE},
    {"systemArray", PL_SYSTEM_ARRAY_ALIAS_CAMEL},
    {"system-array", PL_SYSTEM_ARRAY_ALIAS_KEBAB},
    {"system.array", PL_SYSTEM_ARRAY_ALIAS_DOTTED},
    {"system::array", PL_SYSTEM_ARRAY_ALIAS_SCOPED},
    {"system/array", PL_SYSTEM_ARRAY_ALIAS_SLASHED},
    {"system\\array", PL_SYSTEM_ARRAY_ALIAS_BACKSLASHED},
    {"system[array]", PL_SYSTEM_ARRAY_ALIAS_BRACKETED},
    {"system", PL_SYSTEM_ARRAY_ALIAS_BARE}
};

static const char *const alias_names[PL_SYSTEM_ARRAY_ALIAS_COUNT] = {
    "paren", "snake", "camel", "kebab", "dotted", "scoped",
    "slashed", "backslashed", "bracketed", "bare"
};

static int ascii_lower(unsigned char value) {
    if (value >= (unsigned char)'A' && value <= (unsigned char)'Z')
        return (int)(value + ((unsigned char)'a' - (unsigned char)'A'));
    return (int)value;
}

static int ascii_equal_n(const char *left, const char *right, size_t length) {
    size_t index;
    for (index = 0u; index < length; ++index) {
        if (left[index] == '\0' || right[index] == '\0' ||
            ascii_lower((unsigned char)left[index]) !=
                ascii_lower((unsigned char)right[index]))
            return 0;
    }
    return 1;
}

static int is_space_ascii(char value) {
    return value == ' ' || value == '\t' || value == '\r' || value == '\n';
}

static size_t skip_space(const char *source, size_t offset) {
    while (source[offset] != '\0' && is_space_ascii(source[offset])) ++offset;
    return offset;
}

static int set_error(pl_system_array_error *error, size_t offset,
                     const char *message) {
    if (error != NULL) {
        (void)snprintf(error->message, sizeof(error->message), "%s", message);
        error->offset = offset;
    }
    return 0;
}

static const marker_entry *match_marker(const char *source, size_t offset,
                                        const marker_entry *entries, size_t count,
                                        size_t *matched_length) {
    size_t index;
    for (index = 0u; index < count; ++index) {
        size_t length = strlen(entries[index].text);
        if (strncmp(source + offset, entries[index].text, length) == 0) {
            *matched_length = length;
            return &entries[index];
        }
    }
    *matched_length = 0u;
    return NULL;
}

static int is_boundary(char value) {
    return value == '\0' || is_space_ascii(value) || value == ';';
}

int pl_system_array_parse(const char *source,
                          pl_system_array_directive *out,
                          pl_system_array_error *error) {
    size_t length;
    size_t offset = 0u;
    size_t marker_length = 0u;
    size_t spelling_length = 0u;
    size_t index;
    const marker_entry *marker;
    const alias_entry *alias = NULL;

    if (error != NULL) memset(error, 0, sizeof(*error));
    if (source == NULL || out == NULL)
        return set_error(error, 0u, "source and output are required");

    length = 0u;
    while (length <= PL_SYSTEM_ARRAY_MAX_INPUT && source[length] != '\0')
        ++length;
    if (length == 0u)
        return set_error(error, 0u, "empty system-array directive");
    if (length > PL_SYSTEM_ARRAY_MAX_INPUT)
        return set_error(error, PL_SYSTEM_ARRAY_MAX_INPUT,
                         "system-array directive exceeds the input limit");

    memset(out, 0, sizeof(*out));
    offset = skip_space(source, offset);

    marker = match_marker(source, offset, prefix_markers,
                          ARRAY_COUNT(prefix_markers), &marker_length);
    if (marker != NULL) {
        out->prefix_marker = marker->id;
        offset += marker_length;
        offset = skip_space(source, offset);
    }

    for (index = 0u; index < ARRAY_COUNT(aliases); ++index) {
        size_t candidate_length = strlen(aliases[index].text);
        char next;
        if (candidate_length <= spelling_length ||
            !ascii_equal_n(source + offset, aliases[index].text, candidate_length))
            continue;

        next = source[offset + candidate_length];
        if (!is_boundary(next) && next != '^' && next != '!' && next != '>')
            continue;

        alias = &aliases[index];
        spelling_length = candidate_length;
    }

    if (alias == NULL) {
        return set_error(error, offset,
                         "expected system(array) or a documented system-array alias");
    }

    if (spelling_length >= sizeof(out->spelling))
        return set_error(error, offset, "system-array spelling is too long");

    memcpy(out->spelling, source + offset, spelling_length);
    out->spelling[spelling_length] = '\0';
    out->alias = alias->alias;
    out->source_offset = offset;
    out->source_length = spelling_length;
    out->shorthand = alias->alias == PL_SYSTEM_ARRAY_ALIAS_BARE;
    offset += spelling_length;
    offset = skip_space(source, offset);

    marker = match_marker(source, offset, suffix_markers,
                          ARRAY_COUNT(suffix_markers), &marker_length);
    if (marker != NULL) {
        out->suffix_marker = marker->id;
        offset += marker_length;
        offset = skip_space(source, offset);
    }

    if (source[offset] == ';') {
        ++offset;
        offset = skip_space(source, offset);
    }

    if (source[offset] != '\0')
        return set_error(error, offset, "unexpected trailing input after system-array directive");

    /* Shorthand is intentionally accepted only as a framed legacy spelling. */
    if (out->shorthand && out->prefix_marker == 0u && out->suffix_marker == 0u)
        return set_error(error, out->source_offset,
                         "bare system shorthand requires an explicit framing marker");

    return 1;
}

const char *pl_system_array_alias_name(pl_system_array_alias alias) {
    if ((unsigned)alias >= (unsigned)PL_SYSTEM_ARRAY_ALIAS_COUNT) return "invalid";
    return alias_names[(unsigned)alias];
}

const char *pl_system_array_prefix_marker(unsigned marker_id) {
    size_t index;
    if (marker_id == 0u) return "";
    for (index = 0u; index < ARRAY_COUNT(prefix_markers); ++index)
        if (prefix_markers[index].id == marker_id) return prefix_markers[index].text;
    return "invalid";
}

const char *pl_system_array_suffix_marker(unsigned marker_id) {
    size_t index;
    if (marker_id == 0u) return "";
    for (index = 0u; index < ARRAY_COUNT(suffix_markers); ++index)
        if (suffix_markers[index].id == marker_id) return suffix_markers[index].text;
    return "invalid";
}

const char *pl_system_array_canonical_name(void) {
    return PL_SYSTEM_ARRAY_CANONICAL;
}

int pl_system_array_write_json(const pl_system_array_directive *directive,
                               FILE *output) {
    if (directive == NULL || output == NULL) return 0;
    if (fprintf(output,
                "{\"schema\":\"%s\",\"kind\":\"system.array\","
                "\"canonical\":\"%s\",\"spelling\":\"%s\","
                "\"alias\":\"%s\",\"prefix_marker\":\"%s\","
                "\"suffix_marker\":\"%s\",\"shorthand\":%s,"
                "\"source_execution_performed\":false}\n",
                PL_SYSTEM_ARRAY_SCHEMA, PL_SYSTEM_ARRAY_CANONICAL,
                directive->spelling,
                pl_system_array_alias_name(directive->alias),
                pl_system_array_prefix_marker(directive->prefix_marker),
                pl_system_array_suffix_marker(directive->suffix_marker),
                directive->shorthand ? "true" : "false") < 0)
        return 0;
    return 1;
}
