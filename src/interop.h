#ifndef PYTYLIKI_INTEROP_H
#define PYTYLIKI_INTEROP_H
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
#ifdef __cplusplus
extern "C" {
#endif
#define PYTYLIKI_ABI_VERSION 1u
enum { PYTYLIKI_ABI_OK = 0, PYTYLIKI_ABI_INVALID_ARGUMENT = -1, PYTYLIKI_ABI_UNSUPPORTED_ABI = -2 };

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

/* Contiguous float64 buffers, fixed-width uint64 counts, synchronous calls.
 * Implementations never retain caller pointers or allocate caller-owned data. */
PYTYLIKI_FFI_API uint32_t pytyliki_abi_version(void);
PYTYLIKI_FFI_API int32_t pytyliki_v1_dot_f64(const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_norm_f64(const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_distance_f64(const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_orthogonal_f64(const double *, const double *, uint64_t, double, int32_t *);

/* Embedded C metadata and host-side FLASH/SRAM budget evaluation.
 * All calls are synchronous and never access physical MCU addresses. */
PYTYLIKI_FFI_API int32_t pytyliki_v1_embedded_profile_count(uint64_t *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_embedded_profile_at(uint64_t, pytyliki_embedded_profile_v1 *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_embedded_budget_evaluate(
    const char *, const pytyliki_embedded_sections_v1 *, pytyliki_embedded_budget_v1 *);

/* Cross-repository plugin catalogue discovery. These functions return bounded
 * metadata only; they do not load or execute plugin code. */
PYTYLIKI_FFI_API int32_t pytyliki_v1_plugin_count(uint64_t *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_plugin_at(uint64_t, pytyliki_plugin_descriptor_v1 *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_plugin_find(const char *, pytyliki_plugin_descriptor_v1 *);
#ifdef __cplusplus
}
#endif
#endif
