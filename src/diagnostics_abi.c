#include "interop.h"
#include "metric_geometry.h"

#include <math.h>
#include <string.h>

static int32_t validate_diagnostic_buffer(
    uint32_t buffer_size,
    const pytyliki_diagnostic_v1 *diagnostic
) {
    if (diagnostic == NULL) {
        return buffer_size == 0u ? PYTYLIKI_ABI_OK : PYTYLIKI_ABI_INVALID_ARGUMENT;
    }
    if (buffer_size < PYTYLIKI_DIAGNOSTIC_V1_SIZE) {
        return PYTYLIKI_ABI_BUFFER_TOO_SMALL;
    }
    if (diagnostic->struct_size < PYTYLIKI_DIAGNOSTIC_V1_SIZE) {
        return PYTYLIKI_ABI_BUFFER_TOO_SMALL;
    }
    if (diagnostic->struct_size != buffer_size) {
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    }
    if (diagnostic->abi_version != PYTYLIKI_DIAGNOSTICS_ABI_VERSION) {
        return PYTYLIKI_ABI_UNSUPPORTED_ABI;
    }
    return PYTYLIKI_ABI_OK;
}

static void write_diagnostic(
    pytyliki_diagnostic_v1 *out,
    int32_t code,
    uint32_t severity,
    uint64_t detail,
    const char *message
) {
    pytyliki_diagnostic_v1 value;
    size_t message_size;

    if (out == NULL || message == NULL) {
        return;
    }

    memset(&value, 0, sizeof(value));
    value.struct_size = out->struct_size;
    value.abi_version = PYTYLIKI_DIAGNOSTICS_ABI_VERSION;
    value.code = code;
    value.severity = severity;
    value.source_offset = PYTYLIKI_DIAGNOSTIC_NO_SOURCE;
    value.source_length = 0u;
    value.detail = detail;

    message_size = strlen(message) + 1u;
    if (message_size > sizeof(value.message)) {
        message_size = sizeof(value.message);
    }
    memcpy(value.message, message, message_size);
    value.message[sizeof(value.message) - 1u] = '\0';
    value.message_size = (uint32_t)message_size;

    /* Write only the known v1 prefix; any caller extension remains untouched. */
    memcpy(out, &value, sizeof(value));
}

uint32_t PYTYLIKI_FFI_CALL pytyliki_diagnostics_abi_version(void) {
    return PYTYLIKI_DIAGNOSTICS_ABI_VERSION;
}

int32_t PYTYLIKI_FFI_CALL pytyliki_v1_diagnostic_init(
    uint32_t out_size,
    pytyliki_diagnostic_v1 *out
) {
    if (out == NULL) {
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    }
    if (out_size < PYTYLIKI_DIAGNOSTIC_V1_SIZE) {
        return PYTYLIKI_ABI_BUFFER_TOO_SMALL;
    }

    memset(out, 0, sizeof(*out));
    out->struct_size = out_size;
    out->abi_version = PYTYLIKI_DIAGNOSTICS_ABI_VERSION;
    out->source_offset = PYTYLIKI_DIAGNOSTIC_NO_SOURCE;
    return PYTYLIKI_ABI_OK;
}

int32_t PYTYLIKI_FFI_CALL pytyliki_v1_distance_f64_ex(
    const double *left,
    const double *right,
    uint64_t count,
    double *out,
    uint32_t diagnostic_buffer_size,
    pytyliki_diagnostic_v1 *diagnostic
) {
    int32_t status;
    size_t index;
    double result = 0.0;

    status = validate_diagnostic_buffer(diagnostic_buffer_size, diagnostic);
    if (status != PYTYLIKI_ABI_OK) {
        return status;
    }

    if (out == NULL || left == NULL || right == NULL || count == 0u) {
        write_diagnostic(
            diagnostic, PYTYLIKI_DIAG_INVALID_ARGUMENT,
            PYTYLIKI_DIAG_SEVERITY_ERROR, count,
            "Distance requires two vectors, a nonzero count and an output"
        );
        return PYTYLIKI_ABI_INVALID_ARGUMENT;
    }

    if (count > (uint64_t)PL_METRIC_MAX_DIMENSION || count > (uint64_t)SIZE_MAX) {
        write_diagnostic(
            diagnostic, PYTYLIKI_DIAG_LIMIT_EXCEEDED,
            PYTYLIKI_DIAG_SEVERITY_ERROR, (uint64_t)PL_METRIC_MAX_DIMENSION,
            "Vector dimension exceeds the ABI limit"
        );
        return PYTYLIKI_ABI_LIMIT_EXCEEDED;
    }

    for (index = 0u; index < (size_t)count; ++index) {
        if (!isfinite(left[index]) || !isfinite(right[index])) {
            write_diagnostic(
                diagnostic, PYTYLIKI_DIAG_NON_FINITE_INPUT,
                PYTYLIKI_DIAG_SEVERITY_ERROR, (uint64_t)index,
                "Vector contains a non-finite value"
            );
            return PYTYLIKI_ABI_NON_FINITE_INPUT;
        }
    }

    if (pl_metric_distance(left, right, (size_t)count, &result) != 0) {
        write_diagnostic(
            diagnostic, PYTYLIKI_DIAG_NUMERIC_OVERFLOW,
            PYTYLIKI_DIAG_SEVERITY_ERROR, count,
            "Distance result is outside the finite float64 range"
        );
        return PYTYLIKI_ABI_NUMERIC_OVERFLOW;
    }

    *out = result;
    write_diagnostic(
        diagnostic, PYTYLIKI_DIAG_OK, PYTYLIKI_DIAG_SEVERITY_INFO,
        count, "ok"
    );
    return PYTYLIKI_ABI_OK;
}
