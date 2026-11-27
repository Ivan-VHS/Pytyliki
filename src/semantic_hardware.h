#ifndef PYTYLIKI_SEMANTIC_HARDWARE_H
#define PYTYLIKI_SEMANTIC_HARDWARE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SH_MAX_SOURCE_BYTES 65536u
#define PL_SH_MAX_TOKENS 8192u
#define PL_SH_MAX_TYPES 64u
#define PL_SH_MAX_VALUES 512u
#define PL_SH_MAX_SYMBOLS 192u
#define PL_SH_MAX_NAME 48u
#define PL_SH_DIMENSIONS 8u
#define PL_SH_NO_VALUE UINT32_MAX
#define PL_SH_NO_SYMBOL UINT16_MAX
#define PL_SH_SCHEMA "pytyliki.hardware-ir.v1"
#define PL_SH_REAPER_SCHEMA "reaper.typed-hardware-ir.v1"

typedef enum {
    PL_SH_TYPE_INVALID = 0,
    PL_SH_TYPE_I64,
    PL_SH_TYPE_U32,
    PL_SH_TYPE_U64,
    PL_SH_TYPE_F64,
    PL_SH_TYPE_BOOL,
    PL_SH_TYPE_UNIT,
    PL_SH_TYPE_QUANTITY
} pl_sh_type_kind;

typedef struct {
    pl_sh_type_kind kind;
    int8_t dimensions[PL_SH_DIMENSIONS];
    uint16_t bit_width;
} pl_sh_type;

typedef enum {
    PL_SH_OP_INVALID = 0,
    PL_SH_OP_CONST_F64,
    PL_SH_OP_INPUT,
    PL_SH_OP_ADD,
    PL_SH_OP_SUB,
    PL_SH_OP_MUL,
    PL_SH_OP_DIV,
    PL_SH_OP_NEG,
    PL_SH_OP_MMIO_READ,
    PL_SH_OP_MMIO_WRITE
} pl_sh_opcode;

typedef enum {
    PL_SH_SYMBOL_INPUT = 1,
    PL_SH_SYMBOL_DERIVE,
    PL_SH_SYMBOL_REGISTER,
    PL_SH_SYMBOL_OPERATION
} pl_sh_symbol_kind;

typedef enum {
    PL_SH_ACCESS_NONE = 0,
    PL_SH_ACCESS_READ_ONLY,
    PL_SH_ACCESS_WRITE_ONLY,
    PL_SH_ACCESS_READ_WRITE
} pl_sh_access;

typedef enum {
    PL_SH_EFFECT_NONE = 0,
    PL_SH_EFFECT_MEMORY_READ = 1u << 0,
    PL_SH_EFFECT_MEMORY_WRITE = 1u << 1,
    PL_SH_EFFECT_MMIO_READ = 1u << 2,
    PL_SH_EFFECT_MMIO_WRITE = 1u << 3,
    PL_SH_EFFECT_DMA_SUBMIT = 1u << 4,
    PL_SH_EFFECT_IRQ = 1u << 5,
    PL_SH_EFFECT_CLOCK_WRITE = 1u << 6,
    PL_SH_EFFECT_FIRMWARE_WRITE = 1u << 7
} pl_sh_effect;

typedef struct {
    pl_sh_opcode opcode;
    uint16_t type_index;
    uint16_t symbol_index;
    uint32_t lhs;
    uint32_t rhs;
    double immediate;
    uint32_t source_start;
    uint32_t source_end;
    uint32_t source_line;
    uint32_t source_column;
} pl_sh_value;

typedef struct {
    char name[PL_SH_MAX_NAME];
    pl_sh_symbol_kind kind;
    uint16_t type_index;
    uint16_t register_access;
    uint32_t value_index;
    uint32_t source_start;
    uint32_t source_end;
    uint32_t source_line;
    uint32_t source_column;
} pl_sh_symbol;

typedef struct {
    char source_name[PL_SH_MAX_NAME];
    char module_name[PL_SH_MAX_NAME];
    char target_name[PL_SH_MAX_NAME];
    uint32_t source_bytes;
    pl_sh_type types[PL_SH_MAX_TYPES];
    size_t type_count;
    pl_sh_value values[PL_SH_MAX_VALUES];
    size_t value_count;
    pl_sh_symbol symbols[PL_SH_MAX_SYMBOLS];
    size_t symbol_count;
    uint32_t allowed_effects;
    uint32_t inferred_effects;
    uint32_t last_value;
} pl_sh_module;

typedef struct {
    char code[24];
    char message[192];
    uint32_t source_offset;
    uint32_t line;
    uint32_t column;
} pl_sh_error;

/* Compile the bounded semantic source subset into typed, topologically ordered HIR. */
int pl_sh_compile(const char *source, const char *source_name,
                  pl_sh_module *out, pl_sh_error *error);

/* Re-check all type, reference, source-span and effect invariants on an HIR module. */
int pl_sh_verify(const pl_sh_module *module, pl_sh_error *error);

/* Emit the Pytyliki HIR envelope or the Reaper typed-HIR adapter envelope. */
int pl_sh_write_ir_json(const pl_sh_module *module, FILE *output);
int pl_sh_write_reaper_ir_json(const pl_sh_module *module, FILE *output);

/* Emit a pure numeric C17 function. Effectful and non-numeric modules are rejected. */
int pl_sh_write_c17(const pl_sh_module *module, FILE *output, pl_sh_error *error);

const char *pl_sh_type_name(pl_sh_type_kind kind);
const char *pl_sh_opcode_name(pl_sh_opcode opcode);

#ifdef __cplusplus
}
#endif
#endif
