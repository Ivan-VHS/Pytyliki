#include "hardware_syntax.h"

#include <stdio.h>
#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))
#define NO_OFFSET ((size_t)-1)

typedef struct {
    const char *keyword;
    pl_hw_construct_kind kind;
} keyword_entry;

static const pl_hw_construct_info construct_catalog[PL_HW_SYNTAX_CONSTRUCT_COUNT] = {
    {PL_HW_CONSTRUCT_IRQ_TABLE, "table", "declarative", "interrupt-dispatch table",
     "IRQ handlers must be bounded, reentrant where required, and explicitly declare interrupt-safe effects."},
    {PL_HW_CONSTRUCT_STREAM, "stream", "reactive", "typed dataflow pipeline",
     "Pipeline stages must declare effects, backpressure, capacity, loss policy and DMA ownership."},
    {PL_HW_CONSTRUCT_STACK_PROC, "stack_proc", "execution", "explicit operand-stack program",
     "Stack effects, underflow, overflow, instruction availability and clobbers require static validation."},
    {PL_HW_CONSTRUCT_FSM, "fsm", "execution", "finite-state machine",
     "Power, clock and voltage transitions require platform capabilities, bounds and thermal safeguards."},
    {PL_HW_CONSTRUCT_CONTRACT, "contract", "declarative", "hardware constraint contract",
     "A violation handler is not an automatic repair; address changes and device writes require separate authorization."},
    {PL_HW_CONSTRUCT_ASSEMBLY_SECTION, ".section", "machine", "target-specific assembly section",
     "Section attributes, relocations, instruction encoding, privilege and ABI constraints are target-specific."},
    {PL_HW_CONSTRUCT_REACT, "react", "reactive", "typed hardware event handler",
     "Event source availability, execution context, event loss and handler effects must be checked."},
    {PL_HW_CONSTRUCT_MATRIX, "matrix", "declarative", "finite mode-transition matrix",
     "Every cell must have a type-correct transition and explicit resource/effect requirements."},
    {PL_HW_CONSTRUCT_FUNCTION, "fn", "machine", "typed function or expression",
     "Function effects, recursion limits, stack usage, ABI and target features must be checked."},
    {PL_HW_CONSTRUCT_SCHEDULE, "schedule", "execution", "time- and core-aware schedule",
     "Hard real-time guarantees require a schedulability model, platform timing data and bounded tasks."},
    {PL_HW_CONSTRUCT_RULE, "rule", "reactive", "prioritized transformation rule",
     "Rule ordering, termination, conflicting writes and input trust boundaries must be explicit."},
    {PL_HW_CONSTRUCT_ATOMIC_TX, "atomic_tx", "execution", "transactional state update",
     "Software rollback is not hardware atomicity; commit, isolation and recovery guarantees must be defined."},
    {PL_HW_CONSTRUCT_DAG, "dag", "execution", "dependency graph",
     "Parallel execution is permitted only after data, ordering, resource and effect dependencies are verified."},
    {PL_HW_CONSTRUCT_MACRO, "macro_def", "machine", "typed hygienic macro",
     "Macro expansion must be bounded, hygienic, source-mapped and free of implicit privilege escalation."},
    {PL_HW_CONSTRUCT_HOT_PATCH, "hot_patch", "instrumentation", "live instrumentation update",
     "Quiescence, in-flight events, resource lifetime and rollback must be guaranteed before activation."},
    {PL_HW_CONSTRUCT_PROBE, "probe", "instrumentation", "performance-counter probe",
     "Counter availability, multiplexing, measurement overhead and zero-denominator cases must be handled."},
    {PL_HW_CONSTRUCT_DEVICE, "device", "reactive", "device event/state handler",
     "DMA descriptors, cache coherency, interrupt acknowledgement and device ownership require explicit protocols."},
    {PL_HW_CONSTRUCT_INVARIANT, "invariant", "declarative", "temporal hardware property",
     "Always/eventually properties require defined clocks, sampling semantics, bounds and verification scope."},
    {PL_HW_CONSTRUCT_MEMORY_MAP, "memory_map", "declarative", "target memory layout",
     "Mappings must be target-owned, non-overlapping, overflow-checked and permission-checked; declarations do not map memory."},
    {PL_HW_CONSTRUCT_TRACE_SCRIPT, "trace_script", "instrumentation", "debugger and trace script",
     "Debug attachment and memory/register inspection require explicit target selection and access permissions."}
};

static const keyword_entry keywords[PL_HW_SYNTAX_CONSTRUCT_COUNT] = {
    {"table", PL_HW_CONSTRUCT_IRQ_TABLE},
    {"stream", PL_HW_CONSTRUCT_STREAM},
    {"stack_proc", PL_HW_CONSTRUCT_STACK_PROC},
    {"fsm", PL_HW_CONSTRUCT_FSM},
    {"contract", PL_HW_CONSTRUCT_CONTRACT},
    {".section", PL_HW_CONSTRUCT_ASSEMBLY_SECTION},
    {"react", PL_HW_CONSTRUCT_REACT},
    {"matrix", PL_HW_CONSTRUCT_MATRIX},
    {"fn", PL_HW_CONSTRUCT_FUNCTION},
    {"schedule", PL_HW_CONSTRUCT_SCHEDULE},
    {"rule", PL_HW_CONSTRUCT_RULE},
    {"atomic_tx", PL_HW_CONSTRUCT_ATOMIC_TX},
    {"dag", PL_HW_CONSTRUCT_DAG},
    {"macro_def", PL_HW_CONSTRUCT_MACRO},
    {"hot_patch", PL_HW_CONSTRUCT_HOT_PATCH},
    {"probe", PL_HW_CONSTRUCT_PROBE},
    {"device", PL_HW_CONSTRUCT_DEVICE},
    {"invariant", PL_HW_CONSTRUCT_INVARIANT},
    {"memory_map", PL_HW_CONSTRUCT_MEMORY_MAP},
    {"trace_script", PL_HW_CONSTRUCT_TRACE_SCRIPT}
};

typedef struct {
    char values[256];
    size_t length;
} delimiter_stack;

static int is_space(char value) {
    return value == ' ' || value == '\t' || value == '\r' || value == '\n';
}

static int is_name_char(unsigned char value) {
    return (value >= (unsigned char)'a' && value <= (unsigned char)'z') ||
           (value >= (unsigned char)'A' && value <= (unsigned char)'Z') ||
           (value >= (unsigned char)'0' && value <= (unsigned char)'9') ||
           value == (unsigned char)'_' || value == (unsigned char)'.' ||
           value == (unsigned char)':' || value == (unsigned char)'-';
}

static int set_error(pl_hw_syntax_error *error, size_t offset,
                     const char *message) {
    if (error != NULL) {
        (void)snprintf(error->message, sizeof(error->message), "%s", message);
        error->offset = offset;
    }
    return 0;
}

static size_t skip_space(const char *source, size_t offset) {
    while (source[offset] != '\0' && is_space(source[offset])) ++offset;
    return offset;
}

static int skip_comment(const char *source, size_t *offset,
                        pl_hw_syntax_error *error) {
    size_t index = *offset;
    if (source[index] == '/' && source[index + 1u] == '/') {
        while (source[index] != '\0' && source[index] != '\n') ++index;
        *offset = index;
        return 1;
    }
    if (source[index] == '/' && source[index + 1u] == '*') {
        index += 2u;
        while (source[index] != '\0' &&
               !(source[index] == '*' && source[index + 1u] == '/'))
            ++index;
        if (source[index] == '\0')
            return set_error(error, *offset, "unterminated block comment");
        *offset = index + 2u;
        return 1;
    }
    return 0;
}

static size_t skip_trivia(const char *source, size_t offset,
                          pl_hw_syntax_error *error) {
    for (;;) {
        size_t before = offset;
        offset = skip_space(source, offset);
        if (source[offset] == '/' &&
            (source[offset + 1u] == '/' || source[offset + 1u] == '*')) {
            if (!skip_comment(source, &offset, error)) return NO_OFFSET;
            continue;
        }
        if (offset == before || source[offset] == '\0') return offset;
    }
}

static int parse_quoted(const char *source, size_t *offset,
                        pl_hw_syntax_error *error) {
    char quote = source[*offset];
    size_t index = *offset + 1u;
    while (source[index] != '\0') {
        if (source[index] == '\\') {
            if (source[index + 1u] == '\0')
                return set_error(error, index, "unterminated escape sequence");
            index += 2u;
            continue;
        }
        if (source[index] == quote) {
            *offset = index + 1u;
            return 1;
        }
        if (source[index] == '\n' && quote == '\'')
            return set_error(error, *offset, "character literal cannot span lines");
        ++index;
    }
    return set_error(error, *offset, "unterminated string or character literal");
}

static int push_delimiter(delimiter_stack *stack, char value,
                          size_t offset, pl_hw_syntax_error *error) {
    if (stack->length >= sizeof(stack->values))
        return set_error(error, offset, "delimiter nesting limit exceeded");
    stack->values[stack->length++] = value;
    return 1;
}

static int pop_delimiter(delimiter_stack *stack, char value,
                         size_t offset, pl_hw_syntax_error *error) {
    char expected;
    if (stack->length == 0u)
        return set_error(error, offset, "unmatched closing delimiter");
    expected = stack->values[stack->length - 1u];
    if ((value == ')' && expected != '(') ||
        (value == ']' && expected != '[') ||
        (value == '}' && expected != '{'))
        return set_error(error, offset, "mismatched closing delimiter");
    --stack->length;
    return 1;
}

static int has_top_level_assignment(const char *source, size_t start, size_t end) {
    delimiter_stack stack = {{0}, 0u};
    size_t index = start;

    while (index < end) {
        char value = source[index];
        if (value == '"' || value == '\'') {
            pl_hw_syntax_error ignored = {{0}, 0u};
            if (!parse_quoted(source, &index, &ignored)) return 0;
            continue;
        }
        if (value == '/' && index + 1u < end &&
            (source[index + 1u] == '/' || source[index + 1u] == '*')) {
            if (source[index + 1u] == '/') {
                while (index < end && source[index] != '\n') ++index;
            } else {
                index += 2u;
                while (index + 1u < end &&
                       !(source[index] == '*' && source[index + 1u] == '/'))
                    ++index;
                index += 2u;
            }
            continue;
        }
        if (value == '(' || value == '[') {
            if (stack.length < sizeof(stack.values))
                stack.values[stack.length++] = value;
            ++index;
            continue;
        }
        if (value == ')' || value == ']') {
            if (stack.length != 0u) --stack.length;
            ++index;
            continue;
        }
        if (value == '=' && stack.length == 0u) return 1;
        ++index;
    }
    return 0;
}

static int scan_to_body_or_semicolon(const char *source, size_t start,
                                     size_t *body_open, size_t *semicolon,
                                     pl_hw_syntax_error *error) {
    delimiter_stack stack = {{0}, 0u};
    size_t index = start;
    *body_open = NO_OFFSET;
    *semicolon = NO_OFFSET;

    while (source[index] != '\0') {
        char value = source[index];
        if (value == '"' || value == '\'') {
            if (!parse_quoted(source, &index, error)) return 0;
            continue;
        }
        if (value == '/' && (source[index + 1u] == '/' ||
                             source[index + 1u] == '*')) {
            if (!skip_comment(source, &index, error)) return 0;
            continue;
        }
        if (value == '(' || value == '[') {
            if (!push_delimiter(&stack, value, index, error)) return 0;
            ++index;
            continue;
        }
        if (value == ')' || value == ']') {
            if (!pop_delimiter(&stack, value, index, error)) return 0;
            ++index;
            continue;
        }
        if (stack.length == 0u && value == '{') {
            *body_open = index;
            return 1;
        }
        if (stack.length == 0u && value == ';') {
            *semicolon = index;
            return 1;
        }
        ++index;
    }
    if (stack.length != 0u)
        return set_error(error, index, "unclosed header delimiter");
    return 1;
}

static int scan_balanced_body(const char *source, size_t body_open,
                              size_t *body_close, pl_hw_syntax_error *error) {
    delimiter_stack stack = {{0}, 0u};
    size_t index = body_open;
    if (!push_delimiter(&stack, '{', index, error)) return 0;
    ++index;

    while (source[index] != '\0') {
        char value = source[index];
        if (value == '"' || value == '\'') {
            if (!parse_quoted(source, &index, error)) return 0;
            continue;
        }
        if (value == '/' && (source[index + 1u] == '/' ||
                             source[index + 1u] == '*')) {
            if (!skip_comment(source, &index, error)) return 0;
            continue;
        }
        if (value == '(' || value == '[' || value == '{') {
            if (!push_delimiter(&stack, value, index, error)) return 0;
            ++index;
            continue;
        }
        if (value == ')' || value == ']' || value == '}') {
            if (!pop_delimiter(&stack, value, index, error)) return 0;
            ++index;
            if (stack.length == 0u) {
                *body_close = index - 1u;
                return 1;
            }
            continue;
        }
        ++index;
    }
    return set_error(error, body_open, "unterminated construct body");
}

static int scan_to_semicolon(const char *source, size_t start,
                             size_t *semicolon, pl_hw_syntax_error *error) {
    delimiter_stack stack = {{0}, 0u};
    size_t index = start;
    *semicolon = NO_OFFSET;
    while (source[index] != '\0') {
        char value = source[index];
        if (value == '"' || value == '\'') {
            if (!parse_quoted(source, &index, error)) return 0;
            continue;
        }
        if (value == '/' && (source[index + 1u] == '/' ||
                             source[index + 1u] == '*')) {
            if (!skip_comment(source, &index, error)) return 0;
            continue;
        }
        if (value == '(' || value == '[' || value == '{') {
            if (!push_delimiter(&stack, value, index, error)) return 0;
            ++index;
            continue;
        }
        if (value == ')' || value == ']' || value == '}') {
            if (!pop_delimiter(&stack, value, index, error)) return 0;
            ++index;
            continue;
        }
        if (value == ';' && stack.length == 0u) {
            *semicolon = index;
            return 1;
        }
        ++index;
    }
    if (stack.length != 0u)
        return set_error(error, index, "unclosed expression delimiter");
    return set_error(error, index, "expected terminating semicolon");
}

static int find_keyword(const char *source, size_t offset,
                        pl_hw_construct_kind *kind, size_t *keyword_length) {
    size_t index;
    for (index = 0u; index < ARRAY_COUNT(keywords); ++index) {
        size_t length = strlen(keywords[index].keyword);
        if (strncmp(source + offset, keywords[index].keyword, length) == 0 &&
            (source[offset + length] == '\0' ||
             is_space(source[offset + length]))) {
            *kind = keywords[index].kind;
            *keyword_length = length;
            return 1;
        }
    }
    return 0;
}

static int copy_construct_name(const char *source, size_t *offset,
                               pl_hw_construct_kind kind, char *out,
                               size_t out_size, pl_hw_syntax_error *error) {
    size_t index = skip_space(source, *offset);
    size_t length = 0u;

    if (kind == PL_HW_CONSTRUCT_REACT) {
        if (strncmp(source + index, "on", 2u) == 0 &&
            is_space(source[index + 2u]))
            index = skip_space(source, index + 2u);
    }

    while (is_name_char((unsigned char)source[index + length])) ++length;
    if (length == 0u)
        return set_error(error, index, "construct name or event target is required");
    if (length >= out_size)
        return set_error(error, index, "construct name exceeds the supported limit");

    memcpy(out, source + index, length);
    out[length] = '\0';
    *offset = index + length;
    return 1;
}

static int validate_trailing(const char *source, size_t offset,
                             pl_hw_syntax_error *error) {
    offset = skip_trivia(source, offset, error);
    if (offset == NO_OFFSET) return 0;
    if (source[offset] == ';') {
        ++offset;
        offset = skip_trivia(source, offset, error);
        if (offset == NO_OFFSET) return 0;
    }
    if (source[offset] != '\0')
        return set_error(error, offset, "unexpected trailing tokens after construct");
    return 1;
}

const char *pl_hw_construct_name(pl_hw_construct_kind kind) {
    if ((unsigned)kind >= PL_HW_SYNTAX_CONSTRUCT_COUNT) return "invalid";
    return construct_catalog[(unsigned)kind].keyword;
}

const char *pl_hw_body_kind_name(pl_hw_body_kind kind) {
    switch (kind) {
    case PL_HW_BODY_BLOCK: return "block";
    case PL_HW_BODY_PIPELINE: return "pipeline";
    case PL_HW_BODY_EXPRESSION: return "expression";
    case PL_HW_BODY_ASSEMBLY: return "assembly";
    default: return "invalid";
    }
}

size_t pl_hw_syntax_catalog(const pl_hw_construct_info **out) {
    if (out == NULL) return 0u;
    *out = construct_catalog;
    return PL_HW_SYNTAX_CONSTRUCT_COUNT;
}

int pl_hw_syntax_parse_one(const char *source,
                           pl_hw_construct_node *out,
                           pl_hw_syntax_error *error) {
    size_t source_length = 0u;
    size_t offset;
    size_t keyword_length = 0u;
    size_t body_open = NO_OFFSET;
    size_t body_close = NO_OFFSET;
    size_t semicolon = NO_OFFSET;
    pl_hw_construct_kind kind;

    if (error != NULL) memset(error, 0, sizeof(*error));
    if (source == NULL || out == NULL)
        return set_error(error, 0u, "source and output are required");

    while (source_length <= PL_HW_SYNTAX_MAX_SOURCE_BYTES &&
           source[source_length] != '\0')
        ++source_length;
    if (source_length == 0u)
        return set_error(error, 0u, "empty construct source");
    if (source_length > PL_HW_SYNTAX_MAX_SOURCE_BYTES)
        return set_error(error, PL_HW_SYNTAX_MAX_SOURCE_BYTES,
                         "construct exceeds the one-megabyte source limit");

    memset(out, 0, sizeof(*out));
    out->source_length = source_length;
    offset = skip_trivia(source, 0u, error);
    if (offset == NO_OFFSET) return 0;
    out->source_offset = offset;

    if (!find_keyword(source, offset, &kind, &keyword_length))
        return set_error(error, offset, "unknown Pytyliki hardware construct");

    out->kind = kind;
    offset += keyword_length;
    offset = skip_space(source, offset);

    if (!copy_construct_name(source, &offset, kind, out->name,
                             sizeof(out->name), error))
        return 0;

    if (kind == PL_HW_CONSTRUCT_ASSEMBLY_SECTION) {
        delimiter_stack stack = {{0}, 0u};
        size_t index = offset;
        while (source[index] != '\0') {
            char value = source[index];
            if (value == '"' || value == '\'') {
                if (!parse_quoted(source, &index, error)) return 0;
                continue;
            }
            if (value == ';') {
                while (source[index] != '\0' && source[index] != '\n') ++index;
                continue;
            }
            if (value == '(' || value == '[') {
                if (!push_delimiter(&stack, value, index, error)) return 0;
                ++index;
                continue;
            }
            if (value == ')' || value == ']') {
                if (!pop_delimiter(&stack, value, index, error)) return 0;
                ++index;
                continue;
            }
            if (value == '/' && (source[index + 1u] == '/' ||
                                 source[index + 1u] == '*')) {
                if (!skip_comment(source, &index, error)) return 0;
                continue;
            }
            ++index;
        }
        if (stack.length != 0u)
            return set_error(error, source_length, "unclosed assembly operand delimiter");
        if (skip_trivia(source, offset, error) == source_length)
            return set_error(error, offset, "assembly section must contain instructions");
        out->body_kind = PL_HW_BODY_ASSEMBLY;
        out->body_offset = offset;
        out->body_length = source_length - offset;
        return 1;
    }

    if (!scan_to_body_or_semicolon(source, offset, &body_open, &semicolon, error))
        return 0;

    if (body_open != NO_OFFSET) {
        if (!scan_balanced_body(source, body_open, &body_close, error)) return 0;
        if (!validate_trailing(source, body_close + 1u, error)) return 0;
        out->body_kind = PL_HW_BODY_BLOCK;
        out->body_offset = body_open;
        out->body_length = body_close - body_open + 1u;
        if (kind == PL_HW_CONSTRUCT_FUNCTION &&
            has_top_level_assignment(source, offset, body_open))
            out->body_kind = PL_HW_BODY_EXPRESSION;
        return 1;
    }

    if (semicolon == NO_OFFSET)
        return set_error(error, source_length, "expected a construct body or terminating semicolon");

    if (kind != PL_HW_CONSTRUCT_STREAM && kind != PL_HW_CONSTRUCT_FUNCTION)
        return set_error(error, semicolon, "this construct requires a braced body");

    if (!validate_trailing(source, semicolon + 1u, error)) return 0;
    out->body_kind = kind == PL_HW_CONSTRUCT_STREAM ?
                     PL_HW_BODY_PIPELINE : PL_HW_BODY_EXPRESSION;
    out->body_offset = offset;
    out->body_length = semicolon - offset;
    if (out->body_length == 0u)
        return set_error(error, offset, "construct body cannot be empty");
    return 1;
}

static const pl_hw_construct_info *find_info(pl_hw_construct_kind kind) {
    if ((unsigned)kind >= PL_HW_SYNTAX_CONSTRUCT_COUNT) return NULL;
    return &construct_catalog[(unsigned)kind];
}

int pl_hw_syntax_write_json(const pl_hw_construct_node *node, FILE *output) {
    const pl_hw_construct_info *info;
    if (node == NULL || output == NULL) return 0;
    info = find_info(node->kind);
    if (info == NULL) return 0;
    return fprintf(output,
        "{\"schema\":\"%s\",\"kind\":\"%s\",\"name\":\"%s\","
        "\"family\":\"%s\",\"body_kind\":\"%s\","
        "\"source_offset\":%zu,\"source_length\":%zu,"
        "\"body_offset\":%zu,\"body_length\":%zu,"
        "\"execution_model\":\"%s\",\"safety_notes\":\"%s\","
        "\"source_execution_performed\":false}\n",
        PL_HW_SYNTAX_SCHEMA, info->keyword, node->name, info->family,
        pl_hw_body_kind_name(node->body_kind), node->source_offset,
        node->source_length, node->body_offset, node->body_length,
        info->execution_model, info->safety_notes) >= 0;
}

int pl_hw_syntax_write_catalog_json(FILE *output) {
    size_t index;
    if (output == NULL) return 0;
    if (fprintf(output, "{\"schema\":\"pytyliki.hardware-construct-catalog.v1\","
                        "\"construct_count\":%u,\"constructs\":[",
                PL_HW_SYNTAX_CONSTRUCT_COUNT) < 0)
        return 0;
    for (index = 0u; index < ARRAY_COUNT(construct_catalog); ++index) {
        const pl_hw_construct_info *info = &construct_catalog[index];
        if (fprintf(output,
                    "%s{\"keyword\":\"%s\",\"family\":\"%s\","
                    "\"execution_model\":\"%s\",\"safety_notes\":\"%s\"}",
                    index == 0u ? "" : ",", info->keyword, info->family,
                    info->execution_model, info->safety_notes) < 0)
            return 0;
    }
    return fprintf(output, "]}\n") >= 0;
}
