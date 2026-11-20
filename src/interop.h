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
/* Contiguous float64 buffers, fixed-width uint64 counts, synchronous calls.
 * Implementations never retain caller pointers or allocate caller-owned data. */
PYTYLIKI_FFI_API uint32_t pytyliki_abi_version(void);
PYTYLIKI_FFI_API int32_t pytyliki_v1_dot_f64(const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_norm_f64(const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_distance_f64(const double *, const double *, uint64_t, double *);
PYTYLIKI_FFI_API int32_t pytyliki_v1_orthogonal_f64(const double *, const double *, uint64_t, double, int32_t *);
#ifdef __cplusplus
}
#endif
#endif
