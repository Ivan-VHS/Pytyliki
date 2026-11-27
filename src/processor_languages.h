#ifndef PYTYLIKI_PROCESSOR_LANGUAGES_H
#define PYTYLIKI_PROCESSOR_LANGUAGES_H

#include <stddef.h>

#define PL_LANGUAGE_COUNT 66u
#define PL_LANGUAGE_SOURCE_MAX (1024u * 1024u)
#define PL_LANGUAGE_ARGUMENT_COUNT 10u
#define PL_LANGUAGE_ARGUMENT_SIZE 256u

typedef enum {
    PL_LANGUAGE_ASSEMBLY = 0,
    PL_LANGUAGE_SYSTEMS,
    PL_LANGUAGE_BYTECODE,
    PL_LANGUAGE_INTERMEDIATE,
    PL_LANGUAGE_HARDWARE,
    PL_LANGUAGE_MICROCODE
} pl_language_kind;

typedef enum {
    PL_LANGUAGE_ROUTE_NONE = 0,
    PL_LANGUAGE_ROUTE_MASM,
    PL_LANGUAGE_ROUTE_GAS,
    PL_LANGUAGE_ROUTE_NASM64,
    PL_LANGUAGE_ROUTE_CLANG,
    PL_LANGUAGE_ROUTE_CLANGXX,
    PL_LANGUAGE_ROUTE_RUSTC,
    PL_LANGUAGE_ROUTE_ZIG,
    PL_LANGUAGE_ROUTE_LDC,
    PL_LANGUAGE_ROUTE_PTXAS,
    PL_LANGUAGE_ROUTE_LLVM_MC,
    PL_LANGUAGE_ROUTE_WAT2WASM,
    PL_LANGUAGE_ROUTE_LLC,
    PL_LANGUAGE_ROUTE_SPIRV_AS,
    PL_LANGUAGE_ROUTE_QBE,
    PL_LANGUAGE_ROUTE_IVERILOG,
    PL_LANGUAGE_ROUTE_OPENCL
} pl_language_route;

enum {
    PL_LANGUAGE_COMMENT_HASH = 1u << 0,
    PL_LANGUAGE_COMMENT_SEMICOLON = 1u << 1,
    PL_LANGUAGE_COMMENT_SLASH = 1u << 2,
    PL_LANGUAGE_COMMENT_BLOCK = 1u << 3,
    PL_LANGUAGE_COMMENT_DASH_DASH = 1u << 4,
    PL_LANGUAGE_COMMENT_BACKSLASH = 1u << 5
};

typedef struct {
    const char *id;
    const char *name;
    const char *family;
    const char *extension;
    const char *toolchain;
    const char *target_summary;
    const char *support_note;
    pl_language_kind kind;
    pl_language_route route;
    unsigned long long target_mask;
    unsigned comment_styles;
} pl_language_profile;

typedef struct {
    size_t source_bytes;
    size_t line_count;
    size_t token_count;
    size_t comment_count;
    size_t string_count;
    size_t max_delimiter_depth;
    size_t error_offset;
    int structurally_valid;
    int unterminated_string;
    int unterminated_comment;
} pl_language_report;

typedef struct {
    char executable[64];
    char arguments[PL_LANGUAGE_ARGUMENT_COUNT][PL_LANGUAGE_ARGUMENT_SIZE];
    size_t argument_count;
    int plan_only;
    int external_toolchain_required;
} pl_language_build_plan;

/* Five native workbench operations: enumerate, detect, inspect, target-check, build-plan. */
size_t pl_language_catalog(const pl_language_profile **out);
const pl_language_profile *pl_language_find(const char *id_or_name);
const char *pl_language_kind_name(pl_language_kind kind);
int pl_language_detect(const char *path, const char *source, size_t length,
                       const pl_language_profile **out);
int pl_language_inspect(const char *id_or_name, const char *source, size_t length,
                        pl_language_report *out);
int pl_language_check_target(const char *id_or_name, const char *target,
                             int *compatible);
int pl_language_build_plan_create(const char *id_or_name, const char *target,
                                  const char *source_path, const char *output_path,
                                  pl_language_build_plan *out);

#endif
