#ifndef PYTYLIKI_INTEROP_H
#define PYTYLIKI_INTEROP_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(PYTYLIKI_FFI_BUILD)
#    define PYTYLIKI_FFI_API __declspec(dllexport)
#  else
#    define PYTYLIKI_FFI_API __declspec(dllimport)
#  endif
#else
#  define PYTYLIKI_FFI_API __attribute__((visibility("default")))
#endif

#if defined(_MSC_VER)
#  define PYTYLIKI_FFI_CALL __cdecl
#elif defined(_WIN32) && (defined(__i386__) || defined(__i386))
#  define PYTYLIKI_FFI_CALL __attribute__((cdecl))
#else
#  define PYTYLIKI_FFI_CALL
#endif

#if defined(_MSC_VER)
#  define PYTYLIKI_ABI_ALIGN8 __declspec(align(8))
#elif defined(__GNUC__) || defined(__clang__)
#  define PYTYLIKI_ABI_ALIGN8 __attribute__((aligned(8)))
#else
#  define PYTYLIKI_ABI_ALIGN8
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PYTYLIKI_ABI_VERSION 1u

enum {
    PYTYLIKI_ABI_OK = 0,
    PYTYLIKI_ABI_INVALID_ARGUMENT = -1,
    PYTYLIKI_ABI_UNSUPPORTED_ABI = -2,
    PYTYLIKI_ABI_BUFFER_TOO_SMALL = -3,
    PYTYLIKI_ABI_NOT_FOUND = -4,
    PYTYLIKI_ABI_LIMIT_EXCEEDED = -5,
    PYTYLIKI_ABI_NON_FINITE_INPUT = -6,
    PYTYLIKI_ABI_NUMERIC_OVERFLOW = -7
};

/* Additive diagnostics extension. Existing ABI v1 entry points remain unchanged. */
#define PYTYLIKI_DIAGNOSTICS_ABI_VERSION 1u
#define PYTYLIKI_DIAGNOSTIC_MESSAGE_CAP 96u
#define PYTYLIKI_DIAGNOSTIC_V1_SIZE UINT32_C(144)
#define PYTYLIKI_DIAGNOSTIC_NO_SOURCE UINT64_MAX

typedef enum {
    PYTYLIKI_DIAG_OK = 0,
    PYTYLIKI_DIAG_INVALID_ARGUMENT = 1001,
    PYTYLIKI_DIAG_BUFFER_TOO_SMALL = 1002,
    PYTYLIKI_DIAG_UNSUPPORTED_ABI = 1003,
    PYTYLIKI_DIAG_LIMIT_EXCEEDED = 1004,
    PYTYLIKI_DIAG_NON_FINITE_INPUT = 1005,
    PYTYLIKI_DIAG_NUMERIC_OVERFLOW = 1006
} pytyliki_diagnostic_code_v1;

typedef enum {
    PYTYLIKI_DIAG_SEVERITY_INFO = 1,
    PYTYLIKI_DIAG_SEVERITY_WARNING = 2,
    PYTYLIKI_DIAG_SEVERITY_ERROR = 3,
    PYTYLIKI_DIAG_SEVERITY_FATAL = 4
} pytyliki_diagnostic_severity_v1;

/* Caller-owned value. struct_size is the caller's writable capacity in bytes.
 * source_offset == PYTYLIKI_DIAGNOSTIC_NO_SOURCE means no source range applies.
 * detail is code-specific numeric context; reserved must remain zero.
 * The callee writes only the v1 prefix and never retains this pointer. */
typedef struct PYTYLIKI_ABI_ALIGN8 {
    uint32_t struct_size;
    uint32_t abi_version;
    int32_t code;
    uint32_t severity;
    uint64_t source_offset;
    uint64_t source_length;
    uint64_t detail;
    uint32_t message_size;
    uint32_t reserved;
    char message[PYTYLIKI_DIAGNOSTIC_MESSAGE_CAP];
} pytyliki_diagnostic_v1;

#if defined(__cplusplus)
#  define PYTYLIKI_ABI_STATIC_ASSERT(condition, message) static_assert((condition), message)
#  define PYTYLIKI_ABI_ALIGNOF(type) alignof(type)
#else
#  define PYTYLIKI_ABI_STATIC_ASSERT(condition, message) _Static_assert((condition), message)
#  define PYTYLIKI_ABI_ALIGNOF(type) _Alignof(type)
#endif
PYTYLIKI_ABI_STATIC_ASSERT(sizeof(pytyliki_diagnostic_v1) == PYTYLIKI_DIAGNOSTIC_V1_SIZE,
                           "pytyliki_diagnostic_v1 size changed");
PYTYLIKI_ABI_STATIC_ASSERT(PYTYLIKI_ABI_ALIGNOF(pytyliki_diagnostic_v1) == 8u,
                           "pytyliki_diagnostic_v1 alignment changed");
PYTYLIKI_ABI_STATIC_ASSERT(offsetof(pytyliki_diagnostic_v1, code) == 8u,
                           "pytyliki_diagnostic_v1.code offset changed");
PYTYLIKI_ABI_STATIC_ASSERT(offsetof(pytyliki_diagnostic_v1, source_offset) == 16u,
                           "pytyliki_diagnostic_v1.source_offset offset changed");
PYTYLIKI_ABI_STATIC_ASSERT(offsetof(pytyliki_diagnostic_v1, detail) == 32u,
                           "pytyliki_diagnostic_v1.detail offset changed");
PYTYLIKI_ABI_STATIC_ASSERT(offsetof(pytyliki_diagnostic_v1, message_size) == 40u,
                           "pytyliki_diagnostic_v1.message_size offset changed");
PYTYLIKI_ABI_STATIC_ASSERT(offsetof(pytyliki_diagnostic_v1, message) == 48u,
                           "pytyliki_diagnostic_v1.message offset changed");
#undef PYTYLIKI_ABI_STATIC_ASSERT
#undef PYTYLIKI_ABI_ALIGNOF

#define PYTYLIKI_EMBEDDED_PROFILE_ID_CAP 40u
#define PYTYLIKI_EMBEDDED_ARCH_CAP 24u
#define PYTYLIKI_EMBEDDED_COMPILER_CAP 48u
#define PYTYLIKI_EMBEDDED_FLAGS_CAP 96u

typedef struct {
    char id[PYTYLIKI_EMBEDDED_PROFILE_ID_CAP];
    char architecture[PYTYLIKI_EMBEDDED_ARCH_CAP];
    char compiler[PYTYLIKI_EMBEDDED_COMPILER_CAP];
    char compiler_flags[PYTYLIKI_EMBEDDED_FLAGS_CAP];
    uint64_t clock_hz;
    uint32_t flash_base;
    uint64_t flash_size_bytes;
    uint32_t sram_base;
    uint64_t sram_size_bytes;
    uint64_t stack_reserve_bytes;
    int32_t is_reference_profile;
} pytyliki_embedded_profile_v1;

typedef struct {
    uint64_t text_bytes;
    uint64_t rodata_bytes;
    uint64_t data_bytes;
    uint64_t bss_bytes;
    uint64_t noinit_bytes;
    uint64_t stack_reserve_bytes;
} pytyliki_embedded_sections_v1;

typedef struct {
    uint64_t flash_used_bytes;
    uint64_t sram_used_bytes;
    uint64_t flash_total_bytes;
    uint64_t sram_total_bytes;
    double flash_percent;
    double sram_percent;
    int32_t flash_fits;
    int32_t sram_fits;
} pytyliki_embedded_budget_v1;

#define PYTYLIKI_PLUGIN_ID_CAP 32u
#define PYTYLIKI_PLUGIN_NAME_CAP 64u
#define PYTYLIKI_PLUGIN_DOMAIN_CAP 64u
#define PYTYLIKI_PLUGIN_SERVICE_CAP 48u
#define PYTYLIKI_PLUGIN_CAPABILITIES_CAP 256u

typedef struct {
    char id[PYTYLIKI_PLUGIN_ID_CAP];
    char name[PYTYLIKI_PLUGIN_NAME_CAP];
    char domain[PYTYLIKI_PLUGIN_DOMAIN_CAP];
    char service[PYTYLIKI_PLUGIN_SERVICE_CAP];
    char capabilities[PYTYLIKI_PLUGIN_CAPABILITIES_CAP];
} pytyliki_plugin_descriptor_v1;

/* All calls are synchronous. The implementation never retains caller pointers,
 * allocates caller-owned memory, or transfers ownership across this boundary. */
PYTYLIKI_FFI_API uint32_t PYTYLIKI_FFI_CALL pytyliki_abi_version(void);
PYTYLIKI_FFI_API uint32_t PYTYLIKI_FFI_CALL pytyliki_diagnostics_abi_version(void);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_diagnostic_init(
    uint32_t out_size, pytyliki_diagnostic_v1 *out);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_distance_f64_ex(
    const double *left, const double *right, uint64_t count, double *out,
    uint32_t diagnostic_buffer_size, pytyliki_diagnostic_v1 *diagnostic);

PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_dot_f64(
    const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_norm_f64(
    const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_distance_f64(
    const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_orthogonal_f64(
    const double *, const double *, uint64_t, double, int32_t *);

/* Embedded C metadata and host-side FLASH/SRAM budget evaluation.
 * All calls are synchronous and never access physical MCU addresses. */
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_embedded_profile_count(uint64_t *);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_embedded_profile_at(
    uint64_t, pytyliki_embedded_profile_v1 *);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_embedded_budget_evaluate(
    const char *, const pytyliki_embedded_sections_v1 *, pytyliki_embedded_budget_v1 *);

/* Cross-repository plugin catalogue discovery. These functions return bounded
 * metadata only; they do not load or execute plugin code. */
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_plugin_count(uint64_t *);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_plugin_at(
    uint64_t, pytyliki_plugin_descriptor_v1 *);
PYTYLIKI_FFI_API int32_t PYTYLIKI_FFI_CALL pytyliki_v1_plugin_find(
    const char *, pytyliki_plugin_descriptor_v1 *);

#ifdef __cplusplus
}
#endif
#endif
