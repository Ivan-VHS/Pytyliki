#include "interop.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

_Static_assert(sizeof(pytyliki_diagnostic_v1) == 144u,
               "diagnostics v1 size must remain stable");
_Static_assert(_Alignof(pytyliki_diagnostic_v1) == 8u,
               "diagnostics v1 alignment must remain stable");
_Static_assert(offsetof(pytyliki_diagnostic_v1, code) == 8u,
               "diagnostics code offset must remain stable");
_Static_assert(offsetof(pytyliki_diagnostic_v1, source_offset) == 16u,
               "diagnostics source offset must remain stable");
_Static_assert(offsetof(pytyliki_diagnostic_v1, detail) == 32u,
               "diagnostics detail offset must remain stable");
_Static_assert(offsetof(pytyliki_diagnostic_v1, message) == 48u,
               "diagnostics message offset must remain stable");

typedef struct {
    pytyliki_diagnostic_v1 v1;
    uint8_t extension[8];
} extended_diagnostic;

int main(void) {
    const double left[] = {3.0, 4.0};
    const double right[] = {0.0, 0.0};
    const double non_finite[] = {1.0, NAN};
    const double huge_left[] = {DBL_MAX};
    const double huge_right[] = {-DBL_MAX};
    double output = -17.0;
    pytyliki_diagnostic_v1 diagnostic;
    pytyliki_diagnostic_v1 unchanged;
    extended_diagnostic extended;
    size_t index;

    assert(pytyliki_diagnostics_abi_version() == PYTYLIKI_DIAGNOSTICS_ABI_VERSION);
    assert(pytyliki_v1_diagnostic_init((uint32_t)sizeof(diagnostic), &diagnostic) ==
           PYTYLIKI_ABI_OK);
    assert(diagnostic.struct_size == sizeof(diagnostic));
    assert(diagnostic.abi_version == PYTYLIKI_DIAGNOSTICS_ABI_VERSION);
    assert(diagnostic.code == PYTYLIKI_DIAG_OK);
    assert(diagnostic.source_offset == PYTYLIKI_DIAGNOSTIC_NO_SOURCE);

    assert(pytyliki_v1_distance_f64_ex(
        left, right, 2u, &output, (uint32_t)sizeof(diagnostic), &diagnostic) ==
        PYTYLIKI_ABI_OK);
    assert(output == 5.0);
    assert(diagnostic.code == PYTYLIKI_DIAG_OK);
    assert(diagnostic.severity == PYTYLIKI_DIAG_SEVERITY_INFO);
    assert(diagnostic.detail == 2u);
    assert(diagnostic.message_size == 3u);
    assert(strcmp(diagnostic.message, "ok") == 0);

    output = -17.0;
    assert(pytyliki_v1_distance_f64_ex(
        left, right, 0u, &output, (uint32_t)sizeof(diagnostic), &diagnostic) ==
        PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(output == -17.0);
    assert(diagnostic.code == PYTYLIKI_DIAG_INVALID_ARGUMENT);
    assert(diagnostic.severity == PYTYLIKI_DIAG_SEVERITY_ERROR);
    assert(strstr(diagnostic.message, "nonzero count") != NULL);

    assert(pytyliki_v1_distance_f64_ex(
        left, right, 4097u, &output, (uint32_t)sizeof(diagnostic), &diagnostic) ==
        PYTYLIKI_ABI_LIMIT_EXCEEDED);
    assert(diagnostic.code == PYTYLIKI_DIAG_LIMIT_EXCEEDED);
    assert(diagnostic.detail == 4096u);

    assert(pytyliki_v1_distance_f64_ex(
        left, non_finite, 2u, &output, (uint32_t)sizeof(diagnostic), &diagnostic) ==
        PYTYLIKI_ABI_NON_FINITE_INPUT);
    assert(diagnostic.code == PYTYLIKI_DIAG_NON_FINITE_INPUT);
    assert(diagnostic.detail == 1u);

    assert(pytyliki_v1_distance_f64_ex(
        huge_left, huge_right, 1u, &output, (uint32_t)sizeof(diagnostic), &diagnostic) ==
        PYTYLIKI_ABI_NUMERIC_OVERFLOW);
    assert(diagnostic.code == PYTYLIKI_DIAG_NUMERIC_OVERFLOW);
    assert(isfinite(output));
    assert(output == -17.0);

    memset(&unchanged, 0x5a, sizeof(unchanged));
    diagnostic = unchanged;
    assert(pytyliki_v1_distance_f64_ex(
        left, right, 2u, &output, (uint32_t)sizeof(diagnostic) - 1u, &diagnostic) ==
        PYTYLIKI_ABI_BUFFER_TOO_SMALL);
    assert(memcmp(&diagnostic, &unchanged, sizeof(diagnostic)) == 0);

    diagnostic = unchanged;
    diagnostic.abi_version = 2u;
    unchanged.abi_version = 2u;
    assert(pytyliki_v1_distance_f64_ex(
        left, right, 2u, &output, (uint32_t)sizeof(diagnostic), &diagnostic) ==
        PYTYLIKI_ABI_UNSUPPORTED_ABI);
    assert(memcmp(&diagnostic, &unchanged, sizeof(diagnostic)) == 0);

    diagnostic = unchanged;
    diagnostic.struct_size = (uint32_t)sizeof(diagnostic) + 8u;
    assert(pytyliki_v1_distance_f64_ex(
        left, right, 2u, &output, (uint32_t)sizeof(diagnostic), &diagnostic) ==
        PYTYLIKI_ABI_INVALID_ARGUMENT);

    assert(pytyliki_v1_diagnostic_init(
        (uint32_t)sizeof(diagnostic) - 1u, &diagnostic) ==
        PYTYLIKI_ABI_BUFFER_TOO_SMALL);
    assert(pytyliki_v1_diagnostic_init((uint32_t)sizeof(diagnostic), NULL) ==
           PYTYLIKI_ABI_INVALID_ARGUMENT);
    assert(pytyliki_v1_distance_f64_ex(left, right, 2u, &output, 1u, NULL) ==
           PYTYLIKI_ABI_INVALID_ARGUMENT);

    memset(&extended, 0xa5, sizeof(extended));
    assert(pytyliki_v1_diagnostic_init((uint32_t)sizeof(extended), &extended.v1) ==
           PYTYLIKI_ABI_OK);
    assert(extended.v1.struct_size == sizeof(extended));
    for (index = 0u; index < sizeof(extended.extension); ++index) {
        assert(extended.extension[index] == 0xa5u);
    }
    assert(pytyliki_v1_distance_f64_ex(
        left, right, 2u, &output, (uint32_t)sizeof(extended), &extended.v1) ==
        PYTYLIKI_ABI_OK);
    for (index = 0u; index < sizeof(extended.extension); ++index) {
        assert(extended.extension[index] == 0xa5u);
    }

    assert(pytyliki_v1_distance_f64_ex(left, right, 2u, &output, 0u, NULL) ==
           PYTYLIKI_ABI_OK);
    assert(pytyliki_v1_distance_f64_ex(NULL, right, 2u, &output, 0u, NULL) ==
           PYTYLIKI_ABI_INVALID_ARGUMENT);
    return 0;
}
