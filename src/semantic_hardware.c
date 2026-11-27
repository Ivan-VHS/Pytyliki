#include "semantic_hardware.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_TEXT 48u
#define DIM_LIMIT 16
#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

typedef enum {
    TK_EOF = 0, TK_IDENT, TK_NUMBER, TK_SEMI, TK_COLON, TK_EQUAL,
    TK_LPAREN, TK_RPAREN, TK_COMMA, TK_DOT, TK_PLUS, TK_MINUS,
    TK_STAR, TK_SLASH, TK_CARET, TK_LBRACKET, TK_RBRACKET
} token_kind;

typedef struct {
    token_kind kind;
    char text[TOKEN_TEXT];
    double number;
    uint32_t start, end, line, column;
} token;

typedef struct {
    const char *source;
    size_t source_length;
    token tokens[PL_SH_MAX_TOKENS];
    size_t token_count;
    size_t current;
    pl_sh_module *module;
    pl_sh_error *error;
} parser;

typedef struct {
    int8_t dimensions[PL_SH_DIMENSIONS];
    double scale;
    int valid;
} parsed_unit;

static int fail(pl_sh_error *error, const char *code, const char *message,
                uint32_t offset, uint32_t line, uint32_t column) {
    if (error != NULL) {
        (void)snprintf(error->code, sizeof(error->code), "%s", code);
        (void)snprintf(error->message, sizeof(error->message), "%s", message);
        error->source_offset = offset;
        error->line = line;
        error->column = column;
    }
    return 0;
}

static int parser_fail(parser *p, const token *at, const char *code, const char *message) {
    return fail(p->error, code, message,
                at == NULL ? 0u : at->start,
                at == NULL ? 1u : at->line,
                at == NULL ? 1u : at->column);
}

static int token_append(parser *p, token value) {
    if (p->token_count >= PL_SH_MAX_TOKENS)
        return parser_fail(p, &value, "RPR-SH-E001", "token limit exceeded");
    p->tokens[p->token_count++] = value;
    return 1;
}

static int lex_source(parser *p) {
    size_t i = 0u;
    uint32_t line = 1u, column = 1u;
    while (i < p->source_length) {
        unsigned char c = (unsigned char)p->source[i];
        token t;
        memset(&t, 0, sizeof(t));
        if (c == ' ' || c == '\t' || c == '\r') { ++i; ++column; continue; }
        if (c == '\n') { ++i; ++line; column = 1u; continue; }
        if (c == '#' || (c == '/' && i + 1u < p->source_length && p->source[i + 1u] == '/')) {
            while (i < p->source_length && p->source[i] != '\n') { ++i; ++column; }
            continue;
        }
        t.start = (uint32_t)i; t.line = line; t.column = column;
        if (isalpha(c) || c == '_') {
            size_t begin = i;
            while (i < p->source_length &&
                   (isalnum((unsigned char)p->source[i]) || p->source[i] == '_')) { ++i; ++column; }
            if (i - begin >= sizeof(t.text))
                return parser_fail(p, &t, "RPR-SH-E002", "identifier exceeds the supported name limit");
            memcpy(t.text, p->source + begin, i - begin); t.text[i - begin] = '\0';
            t.kind = TK_IDENT; t.end = (uint32_t)i;
        } else if (isdigit(c) || (c == '.' && i + 1u < p->source_length && isdigit((unsigned char)p->source[i + 1u]))) {
            size_t begin = i;
            char *end = NULL;
            while (i < p->source_length && (isdigit((unsigned char)p->source[i]) || p->source[i] == '.')) { ++i; ++column; }
            if (i - begin >= sizeof(t.text))
                return parser_fail(p, &t, "RPR-SH-E003", "numeric literal exceeds the supported length");
            memcpy(t.text, p->source + begin, i - begin); t.text[i - begin] = '\0';
            t.number = strtod(t.text, &end);
            if (end == t.text || *end != '\0' || !isfinite(t.number))
                return parser_fail(p, &t, "RPR-SH-E004", "numeric literal must be finite");
            t.kind = TK_NUMBER; t.end = (uint32_t)i;
        } else {
            ++i; ++column; t.end = (uint32_t)i;
            switch (c) {
            case ';': t.kind = TK_SEMI; break; case ':': t.kind = TK_COLON; break;
            case '=': t.kind = TK_EQUAL; break; case '(': t.kind = TK_LPAREN; break;
            case ')': t.kind = TK_RPAREN; break; case ',': t.kind = TK_COMMA; break;
            case '.': t.kind = TK_DOT; break; case '+': t.kind = TK_PLUS; break;
            case '-': t.kind = TK_MINUS; break; case '*': t.kind = TK_STAR; break;
            case '/': t.kind = TK_SLASH; break; case '^': t.kind = TK_CARET; break;
            case '[': t.kind = TK_LBRACKET; break; case ']': t.kind = TK_RBRACKET; break;
            default: return parser_fail(p, &t, "RPR-SH-E005", "unexpected character in semantic hardware source");
            }
        }
        if (!token_append(p, t)) return 0;
    }
    token eof;
    memset(&eof, 0, sizeof(eof));
    eof.kind = TK_EOF; eof.start = eof.end = (uint32_t)p->source_length;
    eof.line = line; eof.column = column;
    return token_append(p, eof);
}

static const token *peek(parser *p) { return &p->tokens[p->current]; }
static const token *previous(parser *p) { return p->current == 0u ? &p->tokens[0] : &p->tokens[p->current - 1u]; }
static int match(parser *p, token_kind kind) { if (peek(p)->kind != kind) return 0; ++p->current; return 1; }
static int word(parser *p, const char *value) { if (peek(p)->kind != TK_IDENT || strcmp(peek(p)->text, value) != 0) return 0; ++p->current; return 1; }
static int expect(parser *p, token_kind kind, const char *message) {
    if (match(p, kind)) return 1;
    return parser_fail(p, peek(p), "RPR-SH-E006", message);
}
static int expect_word(parser *p, const char *value) {
    if (word(p, value)) return 1;
    char msg[128]; (void)snprintf(msg, sizeof(msg), "expected '%s'", value);
    return parser_fail(p, peek(p), "RPR-SH-E006", msg);
}
static int read_name(parser *p, char out[PL_SH_MAX_NAME]) {
    const token *t = peek(p);
    if (t->kind != TK_IDENT) return parser_fail(p, t, "RPR-SH-E006", "expected an identifier");
    (void)snprintf(out, PL_SH_MAX_NAME, "%s", t->text); ++p->current; return 1;
}

const char *pl_sh_type_name(pl_sh_type_kind kind) {
    switch (kind) {
    case PL_SH_TYPE_I64: return "i64"; case PL_SH_TYPE_U32: return "u32";
    case PL_SH_TYPE_U64: return "u64"; case PL_SH_TYPE_F64: return "f64";
    case PL_SH_TYPE_BOOL: return "bool"; case PL_SH_TYPE_UNIT: return "unit";
    case PL_SH_TYPE_QUANTITY: return "quantity"; default: return "invalid";
    }
}
const char *pl_sh_opcode_name(pl_sh_opcode opcode) {
    switch (opcode) {
    case PL_SH_OP_CONST_F64: return "const.f64"; case PL_SH_OP_INPUT: return "input";
    case PL_SH_OP_ADD: return "add"; case PL_SH_OP_SUB: return "sub";
    case PL_SH_OP_MUL: return "mul"; case PL_SH_OP_DIV: return "div";
    case PL_SH_OP_NEG: return "neg"; case PL_SH_OP_MMIO_READ: return "mmio.read";
    case PL_SH_OP_MMIO_WRITE: return "mmio.write"; default: return "invalid";
    }
}

static uint16_t add_type(pl_sh_module *m, pl_sh_type type) {
    size_t i;
    for (i = 0u; i < m->type_count; ++i)
        if (memcmp(&m->types[i], &type, sizeof(type)) == 0) return (uint16_t)i;
    if (m->type_count >= PL_SH_MAX_TYPES) return UINT16_MAX;
    m->types[m->type_count] = type;
    return (uint16_t)m->type_count++;
}

static int dim_equal(const int8_t *a, const int8_t *b) {
    return memcmp(a, b, PL_SH_DIMENSIONS * sizeof(*a)) == 0;
}
static int dim_combine(const int8_t *a, const int8_t *b, int sign,
                       int8_t *out) {
    size_t i;
    for (i = 0u; i < PL_SH_DIMENSIONS; ++i) {
        int value = (int)a[i] + sign * (int)b[i];
        if (value < -DIM_LIMIT || value > DIM_LIMIT) return 0;
        out[i] = (int8_t)value;
    }
    return 1;
}
static int type_equal(const pl_sh_type *a, const pl_sh_type *b) {
    return a->kind == b->kind && a->bit_width == b->bit_width && dim_equal(a->dimensions, b->dimensions);
}

static int unit_base(const char *name, int8_t dims[PL_SH_DIMENSIONS], double *scale) {
    memset(dims, 0, PL_SH_DIMENSIONS * sizeof(*dims)); *scale = 1.0;
    if (!strcmp(name, "1")) return 1;
    if (!strcmp(name, "m")) dims[0] = 1;
    else if (!strcmp(name, "cm")) { dims[0] = 1; *scale = 0.01; }
    else if (!strcmp(name, "mm")) { dims[0] = 1; *scale = 0.001; }
    else if (!strcmp(name, "km")) { dims[0] = 1; *scale = 1000.0; }
    else if (!strcmp(name, "kg")) dims[1] = 1;
    else if (!strcmp(name, "g")) { dims[1] = 1; *scale = 0.001; }
    else if (!strcmp(name, "s")) dims[2] = 1;
    else if (!strcmp(name, "ms")) { dims[2] = 1; *scale = 0.001; }
    else if (!strcmp(name, "us")) { dims[2] = 1; *scale = 0.000001; }
    else if (!strcmp(name, "ns")) { dims[2] = 1; *scale = 0.000000001; }
    else if (!strcmp(name, "A")) dims[3] = 1;
    else if (!strcmp(name, "K")) dims[4] = 1;
    else if (!strcmp(name, "mol")) dims[5] = 1;
    else if (!strcmp(name, "cd")) dims[6] = 1;
    else if (!strcmp(name, "count")) dims[7] = 1;
    else if (!strcmp(name, "Hz")) dims[2] = -1;
    else if (!strcmp(name, "N")) { dims[0]=1; dims[1]=1; dims[2]=-2; }
    else if (!strcmp(name, "J")) { dims[0]=2; dims[1]=1; dims[2]=-2; }
    else if (!strcmp(name, "W")) { dims[0]=2; dims[1]=1; dims[2]=-3; }
    else if (!strcmp(name, "C")) { dims[2]=1; dims[3]=1; }
    else if (!strcmp(name, "V")) { dims[0]=2; dims[1]=1; dims[2]=-3; dims[3]=-1; }
    else return 0;
    return 1;
}

static int parse_unit(parser *p, parsed_unit *out, token *first) {
    int divide = 0;
    memset(out, 0, sizeof(*out)); out->scale = 1.0; out->valid = 1;
    if (first != NULL) *first = *peek(p);
    if (peek(p)->kind != TK_IDENT) return parser_fail(p, peek(p), "RPR-SH-E010", "expected a physical unit or scalar type");
    for (;;) {
        const token *unit_token = peek(p);
        int8_t dims[PL_SH_DIMENSIONS]; double scale; int exponent = 1; size_t i;
        char unit_name[TOKEN_TEXT];
        if (unit_token->kind != TK_IDENT) return parser_fail(p, unit_token, "RPR-SH-E010", "expected a unit name");
        (void)snprintf(unit_name, sizeof(unit_name), "%s", unit_token->text); ++p->current;
        if (match(p, TK_CARET)) {
            int negative = match(p, TK_MINUS);
            if (peek(p)->kind != TK_NUMBER || floor(peek(p)->number) != peek(p)->number || peek(p)->number > DIM_LIMIT)
                return parser_fail(p, peek(p), "RPR-SH-E011", "unit exponent must be a bounded integer");
            exponent = (int)peek(p)->number; if (negative) exponent = -exponent; ++p->current;
        }
        if (!unit_base(unit_name, dims, &scale)) return parser_fail(p, unit_token, "RPR-SH-E012", "unknown physical unit");
        if (divide) exponent = -exponent;
        for (i = 0u; i < PL_SH_DIMENSIONS; ++i) {
            int v = (int)out->dimensions[i] + (int)dims[i] * exponent;
            if (v < -DIM_LIMIT || v > DIM_LIMIT) return parser_fail(p, unit_token, "RPR-SH-E013", "derived unit dimension exceeds the supported exponent range");
            out->dimensions[i] = (int8_t)v;
        }
        out->scale *= pow(scale, exponent);
        if (match(p, TK_STAR)) { divide = 0; continue; }
        if (match(p, TK_SLASH)) { divide = 1; continue; }
        break;
    }
    return 1;
}

static int parse_type(parser *p, pl_sh_type *out, double *unit_scale) {
    const token *first = peek(p);
    parsed_unit unit;
    memset(out, 0, sizeof(*out)); *unit_scale = 1.0;
    if (first->kind != TK_IDENT) return parser_fail(p, first, "RPR-SH-E014", "expected a declared value type");
    if (!strcmp(first->text, "i64")) { out->kind=PL_SH_TYPE_I64; out->bit_width=64; ++p->current; return 1; }
    if (!strcmp(first->text, "u32")) { out->kind=PL_SH_TYPE_U32; out->bit_width=32; ++p->current; return 1; }
    if (!strcmp(first->text, "u64")) { out->kind=PL_SH_TYPE_U64; out->bit_width=64; ++p->current; return 1; }
    if (!strcmp(first->text, "f64")) { out->kind=PL_SH_TYPE_F64; out->bit_width=64; ++p->current; return 1; }
    if (!strcmp(first->text, "bool")) { out->kind=PL_SH_TYPE_BOOL; ++p->current; return 1; }
    if (!strcmp(first->text, "unit")) { out->kind=PL_SH_TYPE_UNIT; ++p->current; return 1; }
    if (!parse_unit(p, &unit, NULL)) return 0;
    out->kind = PL_SH_TYPE_QUANTITY;
    memcpy(out->dimensions, unit.dimensions, sizeof(out->dimensions));
    *unit_scale = unit.scale;
    return 1;
}

static int add_value(parser *p, pl_sh_value value, uint32_t *index) {
    pl_sh_module *m = p->module;
    if (m->value_count >= PL_SH_MAX_VALUES) return parser_fail(p, peek(p), "RPR-SH-E020", "typed HIR value limit exceeded");
    if (value.type_index >= m->type_count) return parser_fail(p, peek(p), "RPR-SH-E021", "typed HIR references an unknown type");
    *index = (uint32_t)m->value_count; m->values[m->value_count++] = value; return 1;
}
static int add_symbol(parser *p, const char *name, pl_sh_symbol_kind kind,
                      uint16_t type_index, uint16_t access, uint32_t value,
                      const token *where, uint16_t *index) {
    size_t i; pl_sh_module *m=p->module;
    for (i=0u;i<m->symbol_count;++i) if(!strcmp(m->symbols[i].name,name))
        return parser_fail(p, where, "RPR-SH-E030", "duplicate symbol declaration");
    if(m->symbol_count>=PL_SH_MAX_SYMBOLS) return parser_fail(p,where,"RPR-SH-E031","symbol limit exceeded");
    *index=(uint16_t)m->symbol_count;
    (void)snprintf(m->symbols[m->symbol_count].name,PL_SH_MAX_NAME,"%s",name);
    m->symbols[m->symbol_count].kind=kind; m->symbols[m->symbol_count].type_index=type_index;
    m->symbols[m->symbol_count].register_access=access; m->symbols[m->symbol_count].value_index=value;
    m->symbols[m->symbol_count].source_start=where->start; m->symbols[m->symbol_count].source_end=where->end;
    m->symbols[m->symbol_count].source_line=where->line; m->symbols[m->symbol_count].source_column=where->column;
    ++m->symbol_count; return 1;
}

static int expression(parser *p, uint32_t *value);
static int primary(parser *p, uint32_t *value) {
    const token *t=peek(p); pl_sh_value v; uint16_t symbol_index; size_t i;
    memset(&v,0,sizeof(v)); v.lhs=v.rhs=PL_SH_NO_VALUE; v.symbol_index=PL_SH_NO_SYMBOL;
    if(match(p,TK_NUMBER)) {
        pl_sh_type type; memset(&type,0,sizeof(type)); type.kind=PL_SH_TYPE_F64; type.bit_width=64;
        v.opcode=PL_SH_OP_CONST_F64; v.immediate=t->number; v.type_index=add_type(p->module,type);
        v.source_start=t->start; v.source_end=t->end; v.source_line=t->line; v.source_column=t->column;
        if(v.type_index==UINT16_MAX) return parser_fail(p,t,"RPR-SH-E022","type table limit exceeded");
        return add_value(p,v,value);
    }
    if(match(p,TK_LPAREN)) { if(!expression(p,value)||!expect(p,TK_RPAREN,"expected ')' after expression")) return 0; return 1; }
    if(t->kind==TK_IDENT) {
        for(i=0u;i<p->module->symbol_count;++i) if(!strcmp(p->module->symbols[i].name,t->text)) break;
        if(i==p->module->symbol_count) return parser_fail(p,t,"RPR-SH-E032","use of an undeclared symbol");
        symbol_index=(uint16_t)i; ++p->current;
        if(p->module->symbols[i].kind==PL_SH_SYMBOL_REGISTER)
            return parser_fail(p,t,"RPR-SH-E033","registers require an explicit mmio.read or mmio.write operation");
        if(p->module->symbols[i].value_index==PL_SH_NO_VALUE) return parser_fail(p,t,"RPR-SH-E034","symbol has no value binding");
        v.opcode=PL_SH_OP_INPUT; v.type_index=p->module->symbols[i].type_index;
        v.symbol_index=symbol_index; v.lhs=p->module->symbols[i].value_index;
        v.source_start=t->start; v.source_end=t->end; v.source_line=t->line; v.source_column=t->column;
        return add_value(p,v,value);
    }
    return parser_fail(p,t,"RPR-SH-E035","expected a numeric literal, symbol or parenthesized expression");
}
static int unary(parser *p,uint32_t *value) {
    if(match(p,TK_MINUS)) {
        const token *t=previous(p); pl_sh_value v; uint32_t child; memset(&v,0,sizeof(v));
        if(!unary(p,&child)) return 0;
        v.opcode=PL_SH_OP_NEG; v.type_index=p->module->values[child].type_index;
        v.lhs=child; v.rhs=PL_SH_NO_VALUE; v.symbol_index=PL_SH_NO_SYMBOL;
        v.source_start=t->start; v.source_end=p->module->values[child].source_end;
        v.source_line=t->line; v.source_column=t->column;
        return add_value(p,v,value);
    }
    return primary(p,value);
}
static int multiply(parser *p,uint32_t *value) {
    uint32_t left,right;
    if(!unary(p,&left)) return 0;
    while(peek(p)->kind==TK_STAR||peek(p)->kind==TK_SLASH) {
        const token *op=peek(p); token_kind kind=op->kind; pl_sh_value v; pl_sh_type result,left_type,right_type;
        ++p->current; if(!unary(p,&right)) return 0;
        left_type=p->module->types[p->module->values[left].type_index]; right_type=p->module->types[p->module->values[right].type_index];
        if ((left_type.kind != PL_SH_TYPE_QUANTITY && left_type.kind != PL_SH_TYPE_F64 &&
             left_type.kind != PL_SH_TYPE_I64 && left_type.kind != PL_SH_TYPE_U32 &&
             left_type.kind != PL_SH_TYPE_U64) ||
            (right_type.kind != PL_SH_TYPE_QUANTITY && right_type.kind != PL_SH_TYPE_F64 &&
             right_type.kind != PL_SH_TYPE_I64 && right_type.kind != PL_SH_TYPE_U32 &&
             right_type.kind != PL_SH_TYPE_U64))
            return parser_fail(p,op,"RPR-SH-E040","arithmetic operands must have compatible numeric types");
        result=left_type;
        if(left_type.kind==PL_SH_TYPE_QUANTITY || right_type.kind==PL_SH_TYPE_QUANTITY) {
            if(left_type.kind!=PL_SH_TYPE_QUANTITY && right_type.kind==PL_SH_TYPE_QUANTITY) {
                if(!dim_equal(left_type.dimensions,(int8_t[PL_SH_DIMENSIONS]){0})) return parser_fail(p,op,"RPR-SH-E041","scalar operand is not dimensionless");
                result=right_type;
            } else if(left_type.kind==PL_SH_TYPE_QUANTITY && right_type.kind!=PL_SH_TYPE_QUANTITY) {
                if(!dim_equal(right_type.dimensions,(int8_t[PL_SH_DIMENSIONS]){0})) return parser_fail(p,op,"RPR-SH-E041","scalar operand is not dimensionless");
                result=left_type;
            } else result.kind=PL_SH_TYPE_QUANTITY;
            if(!dim_combine(left_type.dimensions,right_type.dimensions,kind==TK_STAR?1:-1,result.dimensions))
                return parser_fail(p,op,"RPR-SH-E042","resulting physical dimension exceeds the supported range");
        } else if(left_type.kind!=right_type.kind || left_type.bit_width!=right_type.bit_width)
            return parser_fail(p,op,"RPR-SH-E043","integer and floating-point types cannot be mixed implicitly");
        memset(&v,0,sizeof(v)); v.opcode=kind==TK_STAR?PL_SH_OP_MUL:PL_SH_OP_DIV;
        v.type_index=add_type(p->module,result); v.lhs=left; v.rhs=right; v.symbol_index=PL_SH_NO_SYMBOL;
        v.source_start=op->start; v.source_end=p->module->values[right].source_end; v.source_line=op->line; v.source_column=op->column;
        if(v.type_index==UINT16_MAX) return parser_fail(p,op,"RPR-SH-E022","type table limit exceeded");
        if(!add_value(p,v,&left)) return 0;
    }
    *value=left; return 1;
}
static int expression(parser *p,uint32_t *value) {
    uint32_t left,right;
    if(!multiply(p,&left)) return 0;
    while(peek(p)->kind==TK_PLUS||peek(p)->kind==TK_MINUS) {
        const token *op=peek(p); token_kind kind=op->kind; pl_sh_value v;
        const pl_sh_type *a,*b; ++p->current; if(!multiply(p,&right)) return 0;
        a=&p->module->types[p->module->values[left].type_index]; b=&p->module->types[p->module->values[right].type_index];
        if(!type_equal(a,b)) return parser_fail(p,op,"RPR-SH-E044","addition and subtraction require identical types and physical dimensions");
        memset(&v,0,sizeof(v)); v.opcode=kind==TK_PLUS?PL_SH_OP_ADD:PL_SH_OP_SUB;
        v.type_index=p->module->values[left].type_index; v.lhs=left; v.rhs=right; v.symbol_index=PL_SH_NO_SYMBOL;
        v.source_start=op->start; v.source_end=p->module->values[right].source_end; v.source_line=op->line; v.source_column=op->column;
        if(!add_value(p,v,&left)) return 0;
    }
    *value=left; return 1;
}

static int parse_unit_type(parser *p, pl_sh_type *type, double *scale) {
    return parse_type(p,type,scale);
}
static int parse_value_decl(parser *p, pl_sh_symbol_kind kind) {
    const token *start=peek(p); char name[PL_SH_MAX_NAME]; pl_sh_type expected;
    double unit_scale; uint16_t type_index,symbol_index; uint32_t value_index;
    if(!read_name(p,name)||!expect(p,TK_COLON,"expected ':' after value name")||!parse_unit_type(p,&expected,&unit_scale)) return 0;
    if(!expect(p,TK_EQUAL,"expected '=' before value expression")) return 0;
    if(!expression(p,&value_index)) return 0;
    if(!expect(p,TK_SEMI,"expected ';' after value declaration")) return 0;
    pl_sh_value *v=&p->module->values[value_index];
    pl_sh_type *actual=&p->module->types[v->type_index];
    if(v->opcode==PL_SH_OP_CONST_F64 && actual->kind==PL_SH_TYPE_F64 &&
       expected.kind!=PL_SH_TYPE_BOOL && expected.kind!=PL_SH_TYPE_UNIT) {
        v->immediate *= unit_scale;
        v->type_index=add_type(p->module,expected);
        if(v->type_index==UINT16_MAX) return parser_fail(p,start,"RPR-SH-E022","type table limit exceeded");
    } else if(!type_equal(actual,&expected)) {
        return parser_fail(p,start,kind==PL_SH_SYMBOL_INPUT?"RPR-SH-E045":"RPR-SH-E046",
                           kind==PL_SH_SYMBOL_INPUT?"input initializer must be a finite numeric literal in the declared unit":"derived expression type does not match its declared type");
    }
    type_index=add_type(p->module,expected);
    if(type_index==UINT16_MAX) return parser_fail(p,start,"RPR-SH-E022","type table limit exceeded");
    v->type_index=type_index;
    if(!add_symbol(p,name,kind,type_index,PL_SH_ACCESS_NONE,value_index,start,&symbol_index)) return 0;
    (void)symbol_index;
    p->module->last_value=value_index;
    return 1;
}

static int parse_register(parser *p) {
    const token *start=peek(p); char name[PL_SH_MAX_NAME],access[PL_SH_MAX_NAME];
    pl_sh_type type; double scale; uint16_t type_index,symbol_index;
    if(!read_name(p,name)||!expect(p,TK_COLON,"expected ':' after register name")||!parse_type(p,&type,&scale)||
       !expect_word(p,"access")||!read_name(p,access)||!expect(p,TK_SEMI,"expected ';' after register declaration")) return 0;
    if(type.kind!=PL_SH_TYPE_I64 && type.kind!=PL_SH_TYPE_U32 && type.kind!=PL_SH_TYPE_U64)
        return parser_fail(p,start,"RPR-SH-E047","register types must be fixed-width integers in this compiler slice");
    pl_sh_access a=!strcmp(access,"read_only")?PL_SH_ACCESS_READ_ONLY:!strcmp(access,"write_only")?PL_SH_ACCESS_WRITE_ONLY:!strcmp(access,"read_write")?PL_SH_ACCESS_READ_WRITE:PL_SH_ACCESS_NONE;
    if(a==PL_SH_ACCESS_NONE) return parser_fail(p,start,"RPR-SH-E048","register access must be read_only, write_only or read_write");
    type_index=add_type(p->module,type); if(type_index==UINT16_MAX) return parser_fail(p,start,"RPR-SH-E022","type table limit exceeded");
    return add_symbol(p,name,PL_SH_SYMBOL_REGISTER,type_index,(uint16_t)a,PL_SH_NO_VALUE,start,&symbol_index);
}

static uint32_t effect_for(const char *name) {
    if(!strcmp(name,"memory.read")) return PL_SH_EFFECT_MEMORY_READ;
    if(!strcmp(name,"memory.write")) return PL_SH_EFFECT_MEMORY_WRITE;
    if(!strcmp(name,"mmio.read")) return PL_SH_EFFECT_MMIO_READ;
    if(!strcmp(name,"mmio.write")) return PL_SH_EFFECT_MMIO_WRITE;
    if(!strcmp(name,"dma.submit")) return PL_SH_EFFECT_DMA_SUBMIT;
    if(!strcmp(name,"irq")) return PL_SH_EFFECT_IRQ;
    if(!strcmp(name,"clock.write")) return PL_SH_EFFECT_CLOCK_WRITE;
    if(!strcmp(name,"firmware.write")) return PL_SH_EFFECT_FIRMWARE_WRITE;
    if(!strcmp(name,"pure")) return PL_SH_EFFECT_NONE;
    return UINT32_MAX;
}
static int parse_allow(parser *p) {
    const token *start=peek(p); char name[PL_SH_MAX_NAME]; uint32_t effect;
    if(!read_name(p,name)) return 0;
    if(!expect(p,TK_DOT,"effect name must use a namespace separator")) return 0;
    char suffix[PL_SH_MAX_NAME]; if(!read_name(p,suffix)) return 0;
    char full[PL_SH_MAX_NAME*2]; (void)snprintf(full,sizeof(full),"%s.%s",name,suffix);
    effect=effect_for(full);
    if(effect==UINT32_MAX) return parser_fail(p,start,"RPR-SH-E050","unknown hardware effect");
    if(!expect(p,TK_SEMI,"expected ';' after allow declaration")) return 0;
    p->module->allowed_effects|=effect; return 1;
}

static int parse_mmio_operation(parser *p) {
    const token *start=peek(p); char name[PL_SH_MAX_NAME],op_name[PL_SH_MAX_NAME],reg_name[PL_SH_MAX_NAME];
    pl_sh_type expected; double scale; uint16_t type_index,reg_index,symbol_index; uint32_t value=PL_SH_NO_VALUE;
    if(!read_name(p,name)||!expect(p,TK_COLON,"expected ':' after operation name")||!parse_type(p,&expected,&scale)||
       !expect(p,TK_EQUAL,"expected '=' before hardware operation")||!expect_word(p,"mmio")||
       !expect(p,TK_DOT,"expected '.' in MMIO operation")||!read_name(p,op_name)||!expect(p,TK_LPAREN,"expected '(' after MMIO operation")||
       !read_name(p,reg_name)) return 0;
    for(reg_index=0u;reg_index<p->module->symbol_count;++reg_index)
        if(!strcmp(p->module->symbols[reg_index].name,reg_name)) break;
    if(reg_index>=p->module->symbol_count||p->module->symbols[reg_index].kind!=PL_SH_SYMBOL_REGISTER)
        return parser_fail(p,start,"RPR-SH-E051","MMIO operation requires a declared register");
    pl_sh_symbol *reg=&p->module->symbols[reg_index];
    pl_sh_opcode opcode; uint32_t effect;
    if(!strcmp(op_name,"read")) {
        opcode=PL_SH_OP_MMIO_READ; effect=PL_SH_EFFECT_MMIO_READ;
        if(reg->register_access==PL_SH_ACCESS_WRITE_ONLY) return parser_fail(p,start,"RPR-SH-E052","cannot read a write-only register");
        if(!type_equal(&expected,&p->module->types[reg->type_index])) return parser_fail(p,start,"RPR-SH-E058","MMIO read result type does not match the register width");
        type_index=reg->type_index;
    } else if(!strcmp(op_name,"write")) {
        opcode=PL_SH_OP_MMIO_WRITE; effect=PL_SH_EFFECT_MMIO_WRITE;
        if(reg->register_access==PL_SH_ACCESS_READ_ONLY) return parser_fail(p,start,"RPR-SH-E053","cannot write a read-only register");
        if(!expect(p,TK_COMMA,"MMIO write requires an explicit value")) return 0;
        if(!expression(p,&value)) return 0;
        if(!type_equal(&p->module->types[p->module->values[value].type_index],&p->module->types[reg->type_index]))
            return parser_fail(p,start,"RPR-SH-E054","MMIO write value type does not match the register width");
        pl_sh_type unit_type; memset(&unit_type,0,sizeof(unit_type)); unit_type.kind=PL_SH_TYPE_UNIT;
        if(expected.kind!=PL_SH_TYPE_UNIT) return parser_fail(p,start,"RPR-SH-E059","MMIO write operation must declare result type unit");
        type_index=add_type(p->module,unit_type);
    } else return parser_fail(p,start,"RPR-SH-E055","MMIO operation must be read or write");
    if(!expect(p,TK_RPAREN,"expected ')' after MMIO operation")||!expect(p,TK_SEMI,"expected ';' after hardware operation")) return 0;
    if((p->module->allowed_effects&effect)==0u) return parser_fail(p,start,"RPR-SH-E056","hardware effect is not present in the module allowlist");
    pl_sh_value v; memset(&v,0,sizeof(v)); v.opcode=opcode; v.type_index=type_index; v.symbol_index=reg_index;
    v.lhs=value; v.rhs=PL_SH_NO_VALUE; v.source_start=start->start; v.source_end=previous(p)->end;
    v.source_line=start->line; v.source_column=start->column;
    if(!add_value(p,v,&value)) return 0;
    if(!add_symbol(p,name,PL_SH_SYMBOL_OPERATION,type_index,PL_SH_ACCESS_NONE,value,start,&symbol_index)) return 0;
    p->module->inferred_effects|=effect; p->module->last_value=value; return 1;
}

static int parse_module(parser *p) {
    const token *start=peek(p); char name[PL_SH_MAX_NAME];
    if(!expect_word(p,"module")||!read_name(p,name)||!expect(p,TK_SEMI,"expected ';' after module declaration")) return 0;
    (void)snprintf(p->module->module_name,sizeof(p->module->module_name),"%s",name);
    (void)start; return 1;
}
static int parse_target(parser *p) {
    char name[PL_SH_MAX_NAME]; if(!expect_word(p,"target")||!read_name(p,name)||!expect(p,TK_SEMI,"expected ';' after target declaration")) return 0;
    if(strcmp(name,"host")&&strcmp(name,"sim")&&strcmp(name,"riscv64")&&strcmp(name,"x86_64"))
        return parser_fail(p,previous(p),"RPR-SH-E057","target is not in the supported semantic profile catalogue");
    (void)snprintf(p->module->target_name,sizeof(p->module->target_name),"%s",name); return 1;
}

int pl_sh_compile(const char *source,const char *source_name,pl_sh_module *out,pl_sh_error *error) {
    parser p; size_t length; int saw_module=0,saw_target=0;
    if(error){memset(error,0,sizeof(*error));}
    if(!source||!out) return fail(error,"RPR-SH-E000","source and output module are required",0u,1u,1u);
    length=strlen(source);
    if(length>PL_SH_MAX_SOURCE_BYTES) return fail(error,"RPR-SH-E000","source exceeds the 64 KiB compiler limit",PL_SH_MAX_SOURCE_BYTES,1u,1u);
    memset(out,0,sizeof(*out)); out->last_value=PL_SH_NO_VALUE; out->source_bytes=(uint32_t)length;
    memset(&p,0,sizeof(p)); p.source=source;p.source_length=length;p.module=out;p.error=error;
    if(source_name) (void)snprintf(out->source_name,sizeof(out->source_name),"%s",source_name);
    else (void)snprintf(out->source_name,sizeof(out->source_name),"<memory>");
    if(!lex_source(&p)) return 0;
    while(peek(&p)->kind!=TK_EOF) {
        if(peek(&p)->kind!=TK_IDENT) return parser_fail(&p,peek(&p),"RPR-SH-E060","expected module, target, allow, input, derive, register or operation declaration");
        if(!strcmp(peek(&p)->text,"module")) { if(saw_module) return parser_fail(&p,peek(&p),"RPR-SH-E061","duplicate module declaration"); if(!parse_module(&p)) return 0; saw_module=1; }
        else if(!strcmp(peek(&p)->text,"target")) { if(saw_target) return parser_fail(&p,peek(&p),"RPR-SH-E062","duplicate target declaration"); if(!parse_target(&p)) return 0; saw_target=1; }
        else if(!strcmp(peek(&p)->text,"allow")) { ++p.current; if(!parse_allow(&p)) return 0; }
        else if(!strcmp(peek(&p)->text,"input")) { ++p.current; if(!parse_value_decl(&p,PL_SH_SYMBOL_INPUT)) return 0; }
        else if(!strcmp(peek(&p)->text,"derive")) { ++p.current; if(!parse_value_decl(&p,PL_SH_SYMBOL_DERIVE)) return 0; }
        else if(!strcmp(peek(&p)->text,"register")) { ++p.current; if(!parse_register(&p)) return 0; }
        else if(!strcmp(peek(&p)->text,"operation")) { ++p.current; if(!parse_mmio_operation(&p)) return 0; }
        else return parser_fail(&p,peek(&p),"RPR-SH-E063","unsupported declaration in semantic compiler v1");
    }
    if(!saw_module||!saw_target) return fail(error,"RPR-SH-E064","module and target declarations are required",0u,1u,1u);
    return pl_sh_verify(out,error);
}

static int constant_value(const pl_sh_module *m, uint32_t index, double *out, size_t depth) {
    const pl_sh_value *v;
    double left, right, result;
    if(!m||!out||index>=m->value_count||depth>m->value_count) return 0;
    v=&m->values[index];
    switch(v->opcode) {
    case PL_SH_OP_CONST_F64: *out=v->immediate; return 1;
    case PL_SH_OP_INPUT: return v->lhs!=PL_SH_NO_VALUE && constant_value(m,v->lhs,out,depth+1u);
    case PL_SH_OP_NEG:
        if(!constant_value(m,v->lhs,&left,depth+1u)) return 0;
        result=-left; break;
    case PL_SH_OP_ADD: case PL_SH_OP_SUB: case PL_SH_OP_MUL: case PL_SH_OP_DIV:
        if(!constant_value(m,v->lhs,&left,depth+1u)||!constant_value(m,v->rhs,&right,depth+1u)) return 0;
        if(v->opcode==PL_SH_OP_ADD) result=left+right;
        else if(v->opcode==PL_SH_OP_SUB) result=left-right;
        else if(v->opcode==PL_SH_OP_MUL) result=left*right;
        else { if(right==0.0) return 0; result=left/right; }
        break;
    default: return 0;
    }
    if(!isfinite(result)) return 0;
    *out=result; return 1;
}

int pl_sh_verify(const pl_sh_module *m,pl_sh_error *error) {
    size_t i, j;
    uint32_t observed_effects = 0u;
    if(error) memset(error,0,sizeof(*error));
    if(!m||m->type_count>PL_SH_MAX_TYPES||m->value_count>PL_SH_MAX_VALUES||m->symbol_count>PL_SH_MAX_SYMBOLS||
       m->source_bytes>PL_SH_MAX_SOURCE_BYTES||m->module_name[0]=='\0'||m->target_name[0]=='\0')
        return fail(error,"RPR-SH-V000","module metadata or table bounds are invalid",0u,1u,1u);
    for(i=0u;i<m->type_count;++i) {
        const pl_sh_type *t=&m->types[i];
        if(t->kind<=PL_SH_TYPE_INVALID||t->kind>PL_SH_TYPE_QUANTITY)
            return fail(error,"RPR-SH-V008","type table contains an unknown type kind",0u,1u,1u);
        if(t->kind!=PL_SH_TYPE_QUANTITY) {
            for(j=0u;j<PL_SH_DIMENSIONS;++j) if(t->dimensions[j]!=0)
                return fail(error,"RPR-SH-V009","non-quantity type carries physical dimensions",0u,1u,1u);
        } else {
            for(j=0u;j<PL_SH_DIMENSIONS;++j) if(t->dimensions[j]<-DIM_LIMIT||t->dimensions[j]>DIM_LIMIT)
                return fail(error,"RPR-SH-V010","quantity dimension exponent is outside the supported range",0u,1u,1u);
        }
    }
    for(i=0u;i<m->value_count;++i) {
        const pl_sh_value *v=&m->values[i];
        const pl_sh_type *result;
        if(v->type_index>=m->type_count||v->source_start>v->source_end||v->source_end>m->source_bytes||
           v->source_line==0u||v->source_column==0u)
            return fail(error,"RPR-SH-V002","HIR value has an invalid type or source span",v->source_start,v->source_line,v->source_column);
        if(v->lhs!=PL_SH_NO_VALUE&&v->lhs>=i)
            return fail(error,"RPR-SH-V003","HIR operands must refer to earlier values",v->source_start,v->source_line,v->source_column);
        if(v->rhs!=PL_SH_NO_VALUE&&v->rhs>=i)
            return fail(error,"RPR-SH-V003","HIR operands must refer to earlier values",v->source_start,v->source_line,v->source_column);
        result=&m->types[v->type_index];
        switch(v->opcode) {
        case PL_SH_OP_CONST_F64:
            if(!isfinite(v->immediate)||
               !(result->kind==PL_SH_TYPE_F64||result->kind==PL_SH_TYPE_QUANTITY||
                 result->kind==PL_SH_TYPE_I64||result->kind==PL_SH_TYPE_U32||result->kind==PL_SH_TYPE_U64)||
               v->lhs!=PL_SH_NO_VALUE||v->rhs!=PL_SH_NO_VALUE)
                return fail(error,"RPR-SH-V011","typed numeric constant must be finite and operand-free",v->source_start,v->source_line,v->source_column);
            break;
        case PL_SH_OP_INPUT:
            if(v->lhs==PL_SH_NO_VALUE||v->rhs!=PL_SH_NO_VALUE||v->symbol_index>=m->symbol_count||
               !type_equal(result,&m->types[m->values[v->lhs].type_index]))
                return fail(error,"RPR-SH-V012","input reference must preserve a declared symbol type",v->source_start,v->source_line,v->source_column);
            break;
        case PL_SH_OP_ADD:
        case PL_SH_OP_SUB:
            if(v->lhs==PL_SH_NO_VALUE||v->rhs==PL_SH_NO_VALUE||
               !type_equal(result,&m->types[m->values[v->lhs].type_index])||
               !type_equal(result,&m->types[m->values[v->rhs].type_index]))
                return fail(error,"RPR-SH-V013","add/sub result and operand types must be identical",v->source_start,v->source_line,v->source_column);
            break;
        case PL_SH_OP_MUL:
        case PL_SH_OP_DIV: {
            const pl_sh_type *a, *b; pl_sh_type expected;
            if(v->lhs==PL_SH_NO_VALUE||v->rhs==PL_SH_NO_VALUE)
                return fail(error,"RPR-SH-V014","mul/div requires two operands",v->source_start,v->source_line,v->source_column);
            a=&m->types[m->values[v->lhs].type_index]; b=&m->types[m->values[v->rhs].type_index];
            if(a->kind==PL_SH_TYPE_QUANTITY && b->kind==PL_SH_TYPE_QUANTITY) {
                expected=*a;
                if(!dim_combine(a->dimensions,b->dimensions,v->opcode==PL_SH_OP_MUL?1:-1,expected.dimensions))
                    return fail(error,"RPR-SH-V015","mul/div physical dimensions overflow",v->source_start,v->source_line,v->source_column);
            } else if(a->kind==PL_SH_TYPE_QUANTITY && b->kind==PL_SH_TYPE_F64 && dim_equal(b->dimensions,(int8_t[PL_SH_DIMENSIONS]){0})) {
                expected=*a;
            } else if(a->kind==PL_SH_TYPE_F64 && dim_equal(a->dimensions,(int8_t[PL_SH_DIMENSIONS]){0}) && b->kind==PL_SH_TYPE_QUANTITY) {
                expected=*b;
                if(v->opcode==PL_SH_OP_DIV && !dim_combine(a->dimensions,b->dimensions,-1,expected.dimensions))
                    return fail(error,"RPR-SH-V015","scalar/quantity physical dimensions overflow",v->source_start,v->source_line,v->source_column);
                if(v->opcode==PL_SH_OP_DIV) expected.kind=PL_SH_TYPE_QUANTITY;
            } else if(a->kind==b->kind && a->kind!=PL_SH_TYPE_QUANTITY && a->bit_width==b->bit_width) {
                expected=*a;
            } else return fail(error,"RPR-SH-V016","mul/div operands have incompatible semantic types",v->source_start,v->source_line,v->source_column);
            if(!type_equal(result,&expected)) return fail(error,"RPR-SH-V017","mul/div result type does not match inferred type",v->source_start,v->source_line,v->source_column);
            if(v->opcode==PL_SH_OP_DIV) {
                double denominator;
                if(constant_value(m,v->rhs,&denominator,0u)&&denominator==0.0)
                    return fail(error,"RPR-SH-V027","division by a statically known zero is rejected",v->source_start,v->source_line,v->source_column);
            }
            break;
        }
        case PL_SH_OP_NEG:
            if(v->lhs==PL_SH_NO_VALUE||v->rhs!=PL_SH_NO_VALUE||!type_equal(result,&m->types[m->values[v->lhs].type_index]))
                return fail(error,"RPR-SH-V018","negation must preserve one operand type",v->source_start,v->source_line,v->source_column);
            break;
        case PL_SH_OP_MMIO_READ:
            if(v->symbol_index>=m->symbol_count||m->symbols[v->symbol_index].kind!=PL_SH_SYMBOL_REGISTER||
               m->symbols[v->symbol_index].register_access==PL_SH_ACCESS_WRITE_ONLY||
               !type_equal(result,&m->types[m->symbols[v->symbol_index].type_index])||v->rhs!=PL_SH_NO_VALUE)
                return fail(error,"RPR-SH-V019","MMIO read does not satisfy its register access/type contract",v->source_start,v->source_line,v->source_column);
            observed_effects|=PL_SH_EFFECT_MMIO_READ;
            break;
        case PL_SH_OP_MMIO_WRITE:
            if(v->symbol_index>=m->symbol_count||m->symbols[v->symbol_index].kind!=PL_SH_SYMBOL_REGISTER||
               m->symbols[v->symbol_index].register_access==PL_SH_ACCESS_READ_ONLY||v->lhs==PL_SH_NO_VALUE||
               result->kind!=PL_SH_TYPE_UNIT||!type_equal(&m->types[m->values[v->lhs].type_index],&m->types[m->symbols[v->symbol_index].type_index]))
                return fail(error,"RPR-SH-V020","MMIO write does not satisfy its register access/type contract",v->source_start,v->source_line,v->source_column);
            observed_effects|=PL_SH_EFFECT_MMIO_WRITE;
            break;
        default:
            return fail(error,"RPR-SH-V021","HIR contains an unknown or unsupported opcode",v->source_start,v->source_line,v->source_column);
        }
    }
    if(observed_effects!=m->inferred_effects)
        return fail(error,"RPR-SH-V022","module inferred-effect summary does not match HIR operations",0u,1u,1u);
    if((m->inferred_effects&~m->allowed_effects)!=0u)
        return fail(error,"RPR-SH-V001","inferred effects exceed the declared allowlist",0u,1u,1u);
    for(i=0u;i<m->symbol_count;++i) {
        const pl_sh_symbol *s=&m->symbols[i];
        if(s->type_index>=m->type_count||s->name[0]=='\0'||s->source_start>s->source_end||s->source_end>m->source_bytes)
            return fail(error,"RPR-SH-V006","symbol has an invalid name, type or source span",s->source_start,s->source_line,s->source_column);
        for(j=0u;j<i;++j) if(!strcmp(s->name,m->symbols[j].name))
            return fail(error,"RPR-SH-V023","symbol table contains a duplicate name",s->source_start,s->source_line,s->source_column);
        if(s->kind!=PL_SH_SYMBOL_REGISTER&&(s->value_index>=m->value_count||!type_equal(&m->types[s->type_index],&m->types[m->values[s->value_index].type_index])))
            return fail(error,"RPR-SH-V007","value symbol references an unknown value or mismatched type",s->source_start,s->source_line,s->source_column);
        if(s->kind==PL_SH_SYMBOL_REGISTER&&(s->register_access<PL_SH_ACCESS_READ_ONLY||s->register_access>PL_SH_ACCESS_READ_WRITE))
            return fail(error,"RPR-SH-V024","register symbol has invalid access metadata",s->source_start,s->source_line,s->source_column);
        if(s->kind<PL_SH_SYMBOL_INPUT||s->kind>PL_SH_SYMBOL_OPERATION)
            return fail(error,"RPR-SH-V025","symbol has an unknown kind",s->source_start,s->source_line,s->source_column);
    }
    if(m->last_value!=PL_SH_NO_VALUE&&m->last_value>=m->value_count)
        return fail(error,"RPR-SH-V026","module result references an unknown value",0u,1u,1u);
    return 1;
}

static int json_string(FILE *f,const char *s) {
    const unsigned char *p=(const unsigned char*)s;
    if(fputc('"',f)==EOF) return 0;
    while(*p) {
        if(*p=='"'||*p=='\\') { if(fputc('\\',f)==EOF||fputc(*p,f)==EOF) return 0; }
        else if(*p<0x20u) { if(fprintf(f,"\\u%04x",(unsigned)*p)<0) return 0; }
        else if(fputc(*p,f)==EOF) return 0;
        ++p;
    }
    return fputc('"',f)!=EOF;
}

static const char *symbol_kind_name(pl_sh_symbol_kind kind) {
    switch(kind) {
    case PL_SH_SYMBOL_INPUT: return "input";
    case PL_SH_SYMBOL_DERIVE: return "derive";
    case PL_SH_SYMBOL_REGISTER: return "register";
    case PL_SH_SYMBOL_OPERATION: return "operation";
    default: return "invalid";
    }
}
static const char *access_name(uint16_t access) {
    switch(access) {
    case PL_SH_ACCESS_READ_ONLY: return "read_only";
    case PL_SH_ACCESS_WRITE_ONLY: return "write_only";
    case PL_SH_ACCESS_READ_WRITE: return "read_write";
    default: return "none";
    }
}
static const char *value_effect_name(pl_sh_opcode opcode) {
    if(opcode==PL_SH_OP_MMIO_READ) return "mmio.read";
    if(opcode==PL_SH_OP_MMIO_WRITE) return "mmio.write";
    return "pure";
}

static int write_ir(const pl_sh_module *m,FILE *f,const char *schema) {
    size_t i,j;
    if(!m||!f) return 0;
    if(fprintf(f,"{\n  \"schema\": \"%s\",\n  \"producer\": \"pytyliki\",\n  \"module\": ",schema)<0||!json_string(f,m->module_name)||fputs(",\n  \"source_name\": ",f)==EOF||!json_string(f,m->source_name)||fputs(",\n  \"target\": ",f)==EOF||!json_string(f,m->target_name)) return 0;
    if(fprintf(f,",\n  \"source_bytes\": %u,\n  \"type_system\": \"pytyliki.dimension-vector.v1\",\n  \"result_value\": ",m->source_bytes)<0) return 0;
    if(m->last_value==PL_SH_NO_VALUE) { if(fputs("null",f)==EOF) return 0; }
    else if(fprintf(f,"%u",m->last_value)<0) return 0;
    if(fprintf(f,",\n  \"effects\": {\"allowed\": %u, \"inferred\": %u},\n  \"types\": [\n",m->allowed_effects,m->inferred_effects)<0) return 0;
    for(i=0u;i<m->type_count;++i) {
        const pl_sh_type *t=&m->types[i];
        if(fprintf(f,"    {\"id\": %zu, \"kind\": \"%s\", \"width\": %u, \"dimensions\": [",i,pl_sh_type_name(t->kind),(unsigned)t->bit_width)<0) return 0;
        for(j=0u;j<PL_SH_DIMENSIONS;++j) if(fprintf(f,"%s%d",j?", ":"",(int)t->dimensions[j])<0) return 0;
        if(fprintf(f,"]}%s\n",i+1u==m->type_count?"":",")<0) return 0;
    }
    if(fputs("  ],\n  \"values\": [\n",f)==EOF) return 0;
    for(i=0u;i<m->value_count;++i) {
        const pl_sh_value *v=&m->values[i];
        if(fprintf(f,"    {\"id\": %zu, \"op\": \"%s\", \"type\": %u, \"lhs\": ",i,pl_sh_opcode_name(v->opcode),(unsigned)v->type_index)<0) return 0;
        if(v->lhs==PL_SH_NO_VALUE) { if(fputs("null",f)==EOF) return 0; } else if(fprintf(f,"%u",v->lhs)<0) return 0;
        if(fputs(", \"rhs\": ",f)==EOF) return 0;
        if(v->rhs==PL_SH_NO_VALUE) { if(fputs("null",f)==EOF) return 0; } else if(fprintf(f,"%u",v->rhs)<0) return 0;
        if(fprintf(f,", \"symbol\": ")<0) return 0;
        if(v->symbol_index==PL_SH_NO_SYMBOL) { if(fputs("null",f)==EOF) return 0; } else if(fprintf(f,"%u",(unsigned)v->symbol_index)<0) return 0;
        if(fprintf(f,", \"immediate\": %.17g, \"effect\": \"%s\", \"span\": {\"start\": %u, \"end\": %u, \"line\": %u, \"column\": %u}}%s\n",v->immediate,value_effect_name(v->opcode),v->source_start,v->source_end,v->source_line,v->source_column,i+1u==m->value_count?"":",")<0) return 0;
    }
    if(fputs("  ],\n  \"symbols\": [\n",f)==EOF) return 0;
    for(i=0u;i<m->symbol_count;++i) {
        const pl_sh_symbol *s=&m->symbols[i];
        if(fprintf(f,"    {\"name\": ")<0||!json_string(f,s->name)) return 0;
        if(fprintf(f,", \"kind\": \"%s\", \"type\": %u, \"access\": \"%s\", \"value\": ", symbol_kind_name(s->kind),(unsigned)s->type_index,access_name(s->register_access))<0) return 0;
        if(s->value_index==PL_SH_NO_VALUE) { if(fputs("null",f)==EOF) return 0; } else if(fprintf(f,"%u",s->value_index)<0) return 0;
        if(fprintf(f,", \"span\": {\"start\": %u, \"end\": %u, \"line\": %u, \"column\": %u}}%s\n",s->source_start,s->source_end,s->source_line,s->source_column,i+1u==m->symbol_count?"":",")<0) return 0;
    }
    return fputs("  ]\n}\n",f)!=EOF;
}
int pl_sh_write_ir_json(const pl_sh_module *m,FILE *f) { return write_ir(m,f,PL_SH_SCHEMA); }
int pl_sh_write_reaper_ir_json(const pl_sh_module *m,FILE *f) { return write_ir(m,f,PL_SH_REAPER_SCHEMA); }

int pl_sh_write_c17(const pl_sh_module *m,FILE *f,pl_sh_error *error) {
    size_t i; int returned=0;
    if(error) memset(error,0,sizeof(*error));
    if(!pl_sh_verify(m,error)) return 0;
    if(m->inferred_effects!=0u) return fail(error,"RPR-SH-C001","C17 host lowering rejects effectful hardware operations",0u,1u,1u);
    if(strcmp(m->target_name,"host")!=0) return fail(error,"RPR-SH-C006","C17 host backend requires the explicit host target profile",0u,1u,1u);
    if(!f) return fail(error,"RPR-SH-C002","output stream is required",0u,1u,1u);
    for(i=0u;i<m->value_count;++i) {
        const pl_sh_value *v=&m->values[i];
        if(v->opcode==PL_SH_OP_MMIO_READ||v->opcode==PL_SH_OP_MMIO_WRITE)
            return fail(error,"RPR-SH-C003","hardware operations cannot be emitted by the pure host C17 backend",v->source_start,v->source_line,v->source_column);
        if(v->type_index>=m->type_count||
           (m->types[v->type_index].kind!=PL_SH_TYPE_QUANTITY&&m->types[v->type_index].kind!=PL_SH_TYPE_F64))
            return fail(error,"RPR-SH-C004","C17 backend v1 supports only f64 and physical-quantity values",v->source_start,v->source_line,v->source_column);
    }
    if(fputs("#include <math.h>\n#include <stddef.h>\n\ndouble pytyliki_model(void) {\n",f)==EOF) return 0;
    for(i=0u;i<m->value_count;++i) {
        const pl_sh_value *v=&m->values[i];
        if(v->opcode==PL_SH_OP_CONST_F64) {
            if(fprintf(f,"    const double v%zu = %.17g;\n",i,v->immediate)<0) return 0;
        } else if(v->opcode==PL_SH_OP_INPUT) {
            if(fprintf(f,"    const double v%zu = v%u;\n",i,v->lhs)<0) return 0;
        } else if(v->opcode==PL_SH_OP_NEG) {
            if(fprintf(f,"    const double v%zu = -v%u;\n",i,v->lhs)<0) return 0;
        } else {
            const char *op=v->opcode==PL_SH_OP_ADD?"+":v->opcode==PL_SH_OP_SUB?"-":v->opcode==PL_SH_OP_MUL?"*":v->opcode==PL_SH_OP_DIV?"/":NULL;
            if(!op||fprintf(f,"    const double v%zu = v%u %s v%u;\n",i,v->lhs,op,v->rhs)<0)
                return fail(error,"RPR-SH-C005","unsupported operation in C17 lowering",v->source_start,v->source_line,v->source_column);
        }
    }
    if(m->last_value!=PL_SH_NO_VALUE) {
        if(fprintf(f,"    return v%u;\n",m->last_value)<0) return 0;
        returned=1;
    }
    if(!returned&&fputs("    return 0.0;\n",f)==EOF) return 0;
    return fputs("}\n",f)!=EOF;
}
