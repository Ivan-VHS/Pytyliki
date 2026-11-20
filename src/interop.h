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
#define PYTYLIKI_EMBEDDED_ID_CAP 32u
#define PYTYLIKI_EMBEDDED_ARCH_CAP 32u
#define PYTYLIKI_EMBEDDED_ABI_CAP 32u
#define PYTYLIKI_EMBEDDED_COMPILER_CAP 48u
#define PYTYLIKI_EMBEDDED_DESCRIPTION_CAP 128u
typedef struct {
    char id[PYTYLIKI_EMBEDDED_ID_CAP];
    char architecture[PYTYLIKI_EMBEDDED_ARCH_CAP];
    char abi[PYTYLIKI_EMBEDDED_ABI_CAP];
    char compiler[PYTYLIKI_EMBEDDED_COMPILER_CAP];
    char description[PYTYLIKI_EMBEDDED_DESCRIPTION_CAP];
    uint32_t word_bits;
    uint32_t capabilities;
} pytyliki_embedded_target_v1;
typedef struct {
    uint64_t source_bytes;
    uint64_t line_count;
    uint64_t include_count;
    uint64_t heap_calls;
    uint64_t blocking_calls;
    uint64_t register_markers;
    uint64_t host_io_calls;
    uint32_t warnings;
} pytyliki_embedded_source_report_v1;
/* Contiguous float64 buffers, fixed-width uint64 counts, synchronous calls.
 * Implementations never retain caller pointers or allocate caller-owned data. */
PYTYLIKI_FFI_API uint32_t pytyliki_abi_version(void);
PYTYLIKI_FFI_API int32_t pytyliki_v1_dot_f64(const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_norm_f64(const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_distance_f64(const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_orthogonal_f64(const double *, const double *, uint64_t, double, int32_t *);
PYTYLIKI_FFI_API uint64_t pytyliki_v1_embedded_target_count(void);
PYTYLIKI_FFI_API int32_t pytyliki_v1_embedded_target_at(uint64_t, pytyliki_embedded_target_v1 *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_embedded_inspect_source(const char *, uint64_t, pytyliki_embedded_source_report_v1 *);
#ifdef __cplusplus
}
#endif
#endif
