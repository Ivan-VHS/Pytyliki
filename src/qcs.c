#include "qcs.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define QCS_MAX_NESTING 256u
#define QCS_NO_PARENT ((size_t)-1)

static const char *const domains[] = {
    "regfile", "irq", "mmio", "page", "pmu", "trap", "atomic", "slab",
    "boot", "bus", "contextSwitch", "heap", "arena", "borrow", "cache",
    "dma", "mmu", "bitfield", "rc", "zeroCopy", "stackGuard", "simd",
    "spinlock", "fence", "fiber", "pipeline", "affinity", "timer", "event",
    "actorBox", "batchIo", "watchdog", "crypto", "enclave", "privilege",
    "hash", "trng", "trace", "sandbox", "verify", "memTag", "tmr",
    "secretZeroize", "logic", "clock", "fpga", "dsp", "adc", "pwm", "spi",
    "i2c", "uart", "canFd", "fsm", "vm", "bytecode", "jit", "ir", "macro",
    "hypervisor", "linker", "elf64", "debug", "abi", "bridgeAll"
};

static int set_error(pl_qcs_error *e, size_t line, size_t column, const char *message) {
    if (e != NULL) {
        (void)snprintf(e->message, sizeof(e->message), "%s", message);
        e->line = line;
        e->column = column;
    }
    return 0;
}

static int append_token(pl_qcs_tokens *out, pl_qcs_token_kind kind,
                        const char *text, size_t length, size_t line, size_t column) {
    pl_qcs_token *items;
    size_t capacity;
    char *copy;
    if (out->length == out->capacity) {
        capacity = out->capacity == 0 ? 32u : out->capacity * 2u;
        if (capacity < out->capacity || capacity > SIZE_MAX / sizeof(*out->items)) return 0;
        items = (pl_qcs_token *)realloc(out->items, capacity * sizeof(*out->items));
        if (items == NULL) return 0;
        out->items = items;
        out->capacity = capacity;
    }
    copy = (char *)malloc(length + 1u);
    if (copy == NULL) return 0;
    if (length != 0) memcpy(copy, text, length);
    copy[length] = '\0';
    out->items[out->length++] = (pl_qcs_token){kind, copy, line, column};
    return 1;
}

static int is_ident_start(unsigned char c) {
    return isalpha(c) != 0 || c == '_' || c >= 0x80u;
}

static int is_ident_more(unsigned char c) {
    return isalnum(c) != 0 || c == '_' || c >= 0x80u;
}

static int is_keyword(const char *s) {
    static const char *const words[] = {
        "func", "struct", "enum", "protocol", "let", "var", "inout", "returns",
        "guard", "if", "else", "switch", "case", "for", "in", "while", "return",
        "defer", "break", "continue", "true", "false", "nil", "end", "deferred",
        "domain", "state"
    };
    size_t i;
    for (i = 0; i < sizeof(words) / sizeof(words[0]); ++i) {
        if (strcmp(s, words[i]) == 0) return 1;
    }
    return 0;
}

static int is_flow_word(const char *s) {
    return strcmp(s, "then") == 0 || strcmp(s, "send") == 0 || strcmp(s, "back") == 0;
}

int pl_qcs_lex(const char *source, pl_qcs_tokens *out, pl_qcs_error *error) {
    const char *p;
    size_t line = 1u, column = 1u;
    if (source == NULL || out == NULL) return set_error(error, 1u, 1u, "source and token output are required");
    memset(out, 0, sizeof(*out));
    if (error != NULL) memset(error, 0, sizeof(*error));
    p = source;
    while (*p != '\0') {
        unsigned char c = (unsigned char)*p;
        size_t start_column = column;
        const char *start = p;
        if (c == ' ' || c == '\t' || c == '\r') { ++p; ++column; continue; }
        if (c == '\n') {
            if (!append_token(out, PL_QCS_NEWLINE, p, 1u, line, column)) goto oom;
            ++p; ++line; column = 1u; continue;
        }
        if (p[0] == '/' && p[1] == '/') {
            while (*p != '\0' && *p != '\n') { ++p; ++column; }
            continue;
        }
        if (p[0] == '/' && p[1] == '*') {
            size_t sl = line, sc = column;
            p += 2; column += 2u;
            while (*p != '\0' && !(p[0] == '*' && p[1] == '/')) {
                if (*p == '\n') { ++line; column = 1u; ++p; }
                else { ++p; ++column; }
            }
            if (*p == '\0') { pl_qcs_tokens_free(out); return set_error(error, sl, sc, "unterminated block comment"); }
            p += 2; column += 2u; continue;
        }
        if (p[0] == '[' && p[1] == '@') {
            if (!append_token(out, PL_QCS_FRAME_OPEN, p, 2u, line, column)) goto oom;
            p += 2; column += 2u; continue;
        }
        if (p[0] == '@' && p[1] == ']') {
            if (!append_token(out, PL_QCS_FRAME_CLOSE, p, 2u, line, column)) goto oom;
            p += 2; column += 2u; continue;
        }
        if (c == '<' || c == '>' || c == '|' || c == '~') {
            pl_qcs_tokens_free(out);
            return set_error(error, line, column, "forbidden QCS punctuation");
        }
        if (c == '"') {
            char *decoded = (char *)malloc(strlen(p) + 1u);
            size_t used = 0u;
            int closed = 0;
            if (decoded == NULL) goto oom;
            ++p; ++column;
            while (*p != '\0') {
                char ch = *p++;
                ++column;
                if (ch == '"') { closed = 1; break; }
                if (ch == '\n') { free(decoded); pl_qcs_tokens_free(out); return set_error(error, line, start_column, "string literal cannot span lines"); }
                if (ch == '\\') {
                    char esc = *p;
                    if (esc == '\0') break;
                    ++p; ++column;
                    switch (esc) {
                    case 'n': ch = '\n'; break;
                    case 'r': ch = '\r'; break;
                    case 't': ch = '\t'; break;
                    case '"': ch = '"'; break;
                    case '\\': ch = '\\'; break;
                    default: free(decoded); pl_qcs_tokens_free(out); return set_error(error, line, column - 1u, "unknown string escape");
                    }
                }
                decoded[used++] = ch;
            }
            if (!closed) { free(decoded); pl_qcs_tokens_free(out); return set_error(error, line, start_column, "unterminated string literal"); }
            decoded[used] = '\0';
            if (!append_token(out, PL_QCS_STRING, decoded, used, line, start_column)) { free(decoded); goto oom; }
            free(decoded);
            continue;
        }
        if (isdigit(c)) {
            int is_duration = 0;
            while (isalnum((unsigned char)*p) || *p == '_') { if (isalpha((unsigned char)*p)) is_duration = 1; ++p; ++column; }
            if (*p == '.' && isdigit((unsigned char)p[1])) {
                ++p; ++column;
                while (isdigit((unsigned char)*p) || *p == '_') { ++p; ++column; }
            }
            if (is_duration) {
                size_t n = (size_t)(p - start);
                if (!(n >= 2u && (strcmp(p - 2, "ns") == 0 || strcmp(p - 2, "us") == 0 ||
                     strcmp(p - 2, "ms") == 0 || strcmp(p - 2, "mv") == 0 ||
                     strcmp(p - 2, "hz") == 0 || strcmp(p - 2, "s") == 0 ||
                     strcmp(p - 3, "khz") == 0 || strcmp(p - 3, "ghz") == 0))) {
                    pl_qcs_tokens_free(out);
                    return set_error(error, line, start_column, "unsupported numeric suffix");
                }
            }
            if (!append_token(out, is_duration ? PL_QCS_DURATION : PL_QCS_NUMBER,
                              start, (size_t)(p - start), line, start_column)) goto oom;
            continue;
        }
        if (is_ident_start(c)) {
            while (is_ident_more((unsigned char)*p)) { ++p; ++column; }
            {
                size_t n = (size_t)(p - start);
                char *word = (char *)malloc(n + 1u);
                pl_qcs_token_kind kind;
                if (word == NULL) goto oom;
                memcpy(word, start, n); word[n] = '\0';
                kind = is_flow_word(word) ? PL_QCS_FLOW :
                       is_keyword(word) ? PL_QCS_KEYWORD : PL_QCS_IDENTIFIER;
                if (!append_token(out, kind, start, n, line, start_column)) { free(word); goto oom; }
                free(word);
            }
            continue;
        }
        if ((p[0] == '=' && p[1] == '=') || (p[0] == '!' && p[1] == '=') ||
            (p[0] == '&' && p[1] == '+') || (p[0] == '+' && p[1] == '+') ||
            (p[0] == '&' && p[1] == '&') || (p[0] == '|' && p[1] == '|')) {
            if (p[0] == '|' ) { pl_qcs_tokens_free(out); return set_error(error, line, column, "forbidden QCS punctuation"); }
            if (!append_token(out, PL_QCS_OPERATOR, p, 2u, line, column)) goto oom;
            p += 2; column += 2u; continue;
        }
        if (strchr("(){}[],.:;", (int)c) != NULL) {
            if (!append_token(out, PL_QCS_PUNCTUATION, p, 1u, line, column)) goto oom;
            ++p; ++column; continue;
        }
        if (strchr("=!?+-*/%&^", (int)c) != NULL) {
            if (!append_token(out, PL_QCS_OPERATOR, p, 1u, line, column)) goto oom;
            ++p; ++column; continue;
        }
        pl_qcs_tokens_free(out);
        return set_error(error, line, column, "unexpected character in QCS source");
    }
    if (!append_token(out, PL_QCS_EOF, "", 0u, line, column)) goto oom;
    return 1;
oom:
    pl_qcs_tokens_free(out);
    return set_error(error, line, column, "out of memory while tokenizing QCS source");
}

void pl_qcs_tokens_free(pl_qcs_tokens *tokens) {
    size_t i;
    if (tokens == NULL) return;
    for (i = 0; i < tokens->length; ++i) free(tokens->items[i].text);
    free(tokens->items);
    memset(tokens, 0, sizeof(*tokens));
}

const char *pl_qcs_token_kind_name(pl_qcs_token_kind kind) {
    switch (kind) {
    case PL_QCS_EOF: return "EOF";
    case PL_QCS_NEWLINE: return "NEWLINE";
    case PL_QCS_IDENTIFIER: return "IDENTIFIER";
    case PL_QCS_KEYWORD: return "KEYWORD";
    case PL_QCS_FLOW: return "FLOW";
    case PL_QCS_NUMBER: return "NUMBER";
    case PL_QCS_DURATION: return "DURATION";
    case PL_QCS_STRING: return "STRING";
    case PL_QCS_FRAME_OPEN: return "FRAME_OPEN";
    case PL_QCS_FRAME_CLOSE: return "FRAME_CLOSE";
    case PL_QCS_OPERATOR: return "OPERATOR";
    case PL_QCS_PUNCTUATION: return "PUNCTUATION";
    default: return "UNKNOWN";
    }
}

size_t pl_qcs_domain_count(void) { return sizeof(domains) / sizeof(domains[0]); }

int pl_qcs_domain_known(const char *name) {
    size_t i;
    if (name == NULL) return 0;
    for (i = 0; i < pl_qcs_domain_count(); ++i) if (strcmp(name, domains[i]) == 0) return 1;
    return 0;
}

static int add_node(pl_qcs_ast *ast, pl_qcs_node_kind kind, const char *name,
                    size_t parent, size_t line, size_t column) {
    pl_qcs_node *items;
    size_t capacity, n = name == NULL ? 0u : strlen(name);
    char *copy;
    if (ast->length == ast->capacity) {
        capacity = ast->capacity == 0 ? 16u : ast->capacity * 2u;
        if (capacity < ast->capacity || capacity > SIZE_MAX / sizeof(*ast->items)) return 0;
        items = (pl_qcs_node *)realloc(ast->items, capacity * sizeof(*ast->items));
        if (items == NULL) return 0;
        ast->items = items; ast->capacity = capacity;
    }
    copy = (char *)malloc(n + 1u);
    if (copy == NULL) return 0;
    if (n != 0) memcpy(copy, name, n);
    copy[n] = '\0';
    ast->items[ast->length++] = (pl_qcs_node){kind, copy, parent, line, column};
    return 1;
}

static int tok(const pl_qcs_tokens *t, size_t i, const char *value) {
    return i < t->length && strcmp(t->items[i].text, value) == 0;
}

static int frame_is_end(const pl_qcs_tokens *t, size_t i) {
    return i + 2u < t->length && t->items[i].kind == PL_QCS_FRAME_OPEN &&
           tok(t, i + 1u, "end") && t->items[i + 2u].kind == PL_QCS_FRAME_CLOSE;
}

static int parse_header(const pl_qcs_tokens *t, size_t begin, size_t end,
                        char *domain, size_t domain_cap, pl_qcs_error *error) {
    size_t i, attr_count = 0u;
    char seen[32][48];
    int has_domain = 0;
    domain[0] = '\0';
    for (i = begin; i < end; ++i) {
        if (t->items[i].kind == PL_QCS_IDENTIFIER || t->items[i].kind == PL_QCS_KEYWORD) {
            if (i + 1u < end && tok(t, i + 1u, ":")) {
                size_t j;
                const char *key = t->items[i].text;
                for (j = 0; j < attr_count; ++j) {
                    if (strcmp(seen[j], key) == 0)
                        return set_error(error, t->items[i].line, t->items[i].column, "duplicate frame attribute");
                }
                if (attr_count < 32u) {
                    (void)snprintf(seen[attr_count], sizeof(seen[attr_count]), "%s", key);
                    ++attr_count;
                }
                if (strcmp(key, "domain") == 0) {
                    if (i + 2u >= end || t->items[i + 2u].kind != PL_QCS_IDENTIFIER)
                        return set_error(error, t->items[i].line, t->items[i].column, "domain must name a registered hardware domain");
                    (void)snprintf(domain, domain_cap, "%s", t->items[i + 2u].text);
                    has_domain = 1;
                } else if (strcmp(key, "state") == 0) {
                    if (i + 2u >= end ||
                        !(tok(t, i + 2u, "!") || tok(t, i + 2u, "?") || tok(t, i + 2u, "deferred")))
                        return set_error(error, t->items[i].line, t->items[i].column, "state must be !, ?, or deferred");
                }
            }
        }
    }
    if (!has_domain) return set_error(error, t->items[begin - 1u].line, t->items[begin - 1u].column,
                                       "frame header requires domain");
    if (!pl_qcs_domain_known(domain)) return set_error(error, t->items[begin - 1u].line,
                                                        t->items[begin - 1u].column, "unknown QCS domain");
    return 1;
}

int pl_qcs_parse(const pl_qcs_tokens *tokens, pl_qcs_ast *out, pl_qcs_error *error) {
    size_t i, frame_stack[QCS_MAX_NESTING], frame_depth = 0u;
    char bracket_stack[QCS_MAX_NESTING];
    size_t bracket_depth = 0u, current_parent;
    if (tokens == NULL || out == NULL || tokens->items == NULL || tokens->length == 0u)
        return set_error(error, 1u, 1u, "tokens and AST output are required");
    memset(out, 0, sizeof(*out));
    out->root = 0u;
    if (error != NULL) memset(error, 0, sizeof(*error));
    if (!add_node(out, PL_QCS_NODE_MODULE, "module", QCS_NO_PARENT, 1u, 1u))
        return set_error(error, 1u, 1u, "out of memory building QCS AST");
    current_parent = out->root;

    for (i = 0u; i < tokens->length; ++i) {
        const pl_qcs_token *token = &tokens->items[i];
        if (token->kind == PL_QCS_FRAME_OPEN) {
            if (frame_is_end(tokens, i)) {
                if (frame_depth == 0u) { pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "frame end without matching frame open"); }
                --frame_depth;
                current_parent = frame_depth == 0u ? out->root : frame_stack[frame_depth - 1u];
                i += 2u;
                continue;
            } else {
                size_t close = i + 1u, node_index;
                char domain[48];
                while (close < tokens->length && tokens->items[close].kind != PL_QCS_FRAME_CLOSE) ++close;
                if (close >= tokens->length) { pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "unterminated frame header; expected @]"); }
                if (!parse_header(tokens, i + 1u, close, domain, sizeof(domain), error)) { pl_qcs_ast_free(out); return 0; }
                if (frame_depth >= QCS_MAX_NESTING) { pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "QCS frame nesting limit exceeded"); }
                if (!add_node(out, PL_QCS_NODE_FRAME, domain, current_parent, token->line, token->column)) {
                    pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "out of memory building QCS AST");
                }
                node_index = out->length - 1u;
                frame_stack[frame_depth++] = node_index;
                current_parent = node_index;
                ++out->frame_count;
                i = close;
                continue;
            }
        }
        if (token->kind == PL_QCS_FRAME_CLOSE) {
            pl_qcs_ast_free(out);
            return set_error(error, token->line, token->column, "unexpected @] outside a frame header or end marker");
        }
        if (token->kind == PL_QCS_KEYWORD && strcmp(token->text, "func") == 0) {
            if (i + 1u >= tokens->length || tokens->items[i + 1u].kind != PL_QCS_IDENTIFIER) {
                pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "expected function name after func");
            }
            if (!add_node(out, PL_QCS_NODE_FUNCTION, tokens->items[i + 1u].text,
                          current_parent, token->line, token->column)) {
                pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "out of memory building QCS AST");
            }
            ++out->function_count;
        }
        if (token->kind == PL_QCS_FLOW) {
            if (i == 0u || i + 1u >= tokens->length ||
                tokens->items[i - 1u].kind == PL_QCS_NEWLINE ||
                tokens->items[i + 1u].kind == PL_QCS_NEWLINE ||
                tokens->items[i + 1u].kind == PL_QCS_EOF ||
                tokens->items[i + 1u].kind == PL_QCS_FLOW) {
                pl_qcs_ast_free(out);
                return set_error(error, token->line, token->column, "flow operator requires a source and destination expression");
            }
            if (!add_node(out, PL_QCS_NODE_FLOW, token->text, current_parent, token->line, token->column)) {
                pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "out of memory building QCS AST");
            }
            ++out->flow_count;
        }
        if (token->kind == PL_QCS_PUNCTUATION) {
            char c = token->text[0];
            if (c == '(' || c == '{' || c == '[') {
                if (bracket_depth >= QCS_MAX_NESTING) { pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "delimiter nesting limit exceeded"); }
                bracket_stack[bracket_depth++] = c;
            } else if (c == ')' || c == '}' || c == ']') {
                char expected = c == ')' ? '(' : c == '}' ? '{' : '[';
                if (bracket_depth == 0u || bracket_stack[bracket_depth - 1u] != expected) {
                    pl_qcs_ast_free(out); return set_error(error, token->line, token->column, "unmatched closing delimiter");
                }
                --bracket_depth;
            }
        }
        if (token->kind == PL_QCS_EOF) break;
    }
    if (frame_depth != 0u) {
        const pl_qcs_node *n = &out->items[frame_stack[frame_depth - 1u]];
        pl_qcs_ast_free(out);
        return set_error(error, n->line, n->column, "unterminated frame; expected [@ end @]");
    }
    if (bracket_depth != 0u) {
        pl_qcs_ast_free(out);
        return set_error(error, 1u, 1u, "unclosed delimiter in QCS source");
    }
    return 1;
}

int pl_qcs_parse_source(const char *source, pl_qcs_ast *out, pl_qcs_error *error) {
    pl_qcs_tokens tokens = {0};
    int ok;
    if (!pl_qcs_lex(source, &tokens, error)) return 0;
    ok = pl_qcs_parse(&tokens, out, error);
    pl_qcs_tokens_free(&tokens);
    return ok;
}

void pl_qcs_ast_free(pl_qcs_ast *ast) {
    size_t i;
    if (ast == NULL) return;
    for (i = 0; i < ast->length; ++i) free(ast->items[i].name);
    free(ast->items);
    memset(ast, 0, sizeof(*ast));
}

const char *pl_qcs_node_kind_name(pl_qcs_node_kind kind) {
    switch (kind) {
    case PL_QCS_NODE_MODULE: return "module";
    case PL_QCS_NODE_FRAME: return "frame";
    case PL_QCS_NODE_FUNCTION: return "function";
    case PL_QCS_NODE_FLOW: return "flow";
    default: return "invalid";
    }
}

static void json_string(FILE *f, const char *s) {
    const unsigned char *p = (const unsigned char *)(s == NULL ? "" : s);
    fputc('"', f);
    while (*p != '\0') {
        if (*p == '"' || *p == '\\') { fputc('\\', f); fputc(*p, f); }
        else if (*p < 32u) fprintf(f, "\\u%04x", (unsigned)*p);
        else fputc(*p, f);
        ++p;
    }
    fputc('"', f);
}

int pl_qcs_ast_write_json(const pl_qcs_ast *ast, FILE *output) {
    size_t i;
    if (ast == NULL || output == NULL || ast->items == NULL || ast->length == 0u) return 0;
    fprintf(output, "{\"kind\":\"module\",\"frames\":%zu,\"functions\":%zu,\"flows\":%zu,\"nodes\":[",
            ast->frame_count, ast->function_count, ast->flow_count);
    for (i = 0; i < ast->length; ++i) {
        const pl_qcs_node *n = &ast->items[i];
        if (i != 0u) fputc(',', output);
        fprintf(output, "{\"kind\":\"%s\",\"name\":", pl_qcs_node_kind_name(n->kind));
        json_string(output, n->name);
        if (n->parent == QCS_NO_PARENT) fputs(",\"parent\":null", output);
        else fprintf(output, ",\"parent\":%zu", n->parent);
        fprintf(output, ",\"line\":%zu,\"column\":%zu}", n->line, n->column);
    }
    fputs("]}\n", output);
    return ferror(output) == 0;
}
