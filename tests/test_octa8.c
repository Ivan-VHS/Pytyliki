#include "octa8.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    int64_t integer = INT64_MIN;
    uint8_t boolean = 1u;
    double number = 3.5;
    double bad_number = INFINITY;
    pytyliki_octa8_view value;

    value.version = PYTYLIKI_OCTA8_ABI_VERSION;
    value.kind = PYTYLIKI_OCTA8_NULL;
    value.data = NULL;
    value.size_bytes = 0u;
    assert(pytyliki_octa8_view_valid(&value));

    value.kind = PYTYLIKI_OCTA8_BOOLEAN;
    value.data = &boolean;
    value.size_bytes = sizeof(boolean);
    assert(pytyliki_octa8_view_valid(&value));
    boolean = 2u;
    assert(!pytyliki_octa8_view_valid(&value));
    boolean = 1u;

    value.kind = PYTYLIKI_OCTA8_INTEGER;
    value.data = &integer;
    value.size_bytes = sizeof(integer);
    assert(pytyliki_octa8_view_valid(&value));

    value.kind = PYTYLIKI_OCTA8_NUMBER;
    value.data = &number;
    value.size_bytes = sizeof(number);
    assert(pytyliki_octa8_view_valid(&value));
    value.data = &bad_number;
    assert(!pytyliki_octa8_view_valid(&value));

    value.version = 2u;
    assert(!pytyliki_octa8_view_valid(&value));
    assert(pytyliki_octa8_kind_name(PYTYLIKI_OCTA8_RECORD) != NULL);
    assert(pytyliki_octa8_kind_name(99u) == NULL);
    puts("OCTA-8 C11 checks passed");
    return 0;
}
