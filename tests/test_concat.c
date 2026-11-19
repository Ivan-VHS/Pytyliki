#include "concat.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    pl_concat_buffer text = {0};
    pl_concat_buffer raw = {0};
    const unsigned char fragment[] = {0x00u, 0x7fu, 0xffu};

    assert(pl_concat_buffer_init(NULL, 8u) != 0);
    assert(pl_concat_buffer_init(&text, 0u) != 0);
    assert(pl_concat_buffer_init(&text, 16u) == 0);
    assert(pl_concat_buffer_append_text(&text, "Harlem") == 0);
    assert(pl_concat_buffer_append_text(&text, " ") == 0);
    assert(pl_concat_buffer_append_format(&text, "%s-%d", "metal", 42) == 0);
    assert(text.length == strlen("Harlem metal-42"));
    assert(strcmp(pl_concat_buffer_cstr(&text), "Harlem metal-42") == 0);
    assert(pl_concat_buffer_append_text(&text, "!") != 0);
    assert(strcmp(pl_concat_buffer_cstr(&text), "Harlem metal-42") == 0);
    assert(pl_concat_buffer_append(NULL, fragment, sizeof(fragment)) != 0);
    assert(pl_concat_buffer_append(&text, NULL, 1u) != 0);
    assert(pl_concat_buffer_append_text(&text, NULL) != 0);
    assert(pl_concat_buffer_append_format(&text, NULL) != 0);
    assert(pl_concat_buffer_append_format(&text, "%s", "too-long") != 0);
    assert(strcmp(pl_concat_buffer_cstr(&text), "Harlem metal-42") == 0);

    assert(pl_concat_buffer_init(&raw, 8u) == 0);
    assert(pl_concat_buffer_append(&raw, fragment, sizeof(fragment)) == 0);
    assert(raw.length == sizeof(fragment));
    assert(memcmp(pl_concat_buffer_data(&raw), fragment, sizeof(fragment)) == 0);
    assert(pl_concat_buffer_append(&raw, NULL, 0u) == 0);

    pl_concat_buffer_free(&raw);
    pl_concat_buffer_free(&raw);
    pl_concat_buffer_free(&text);
    assert(text.data == NULL && text.length == 0u && text.max_length == 0u);

    puts("Pytyliki bounded concatenation tests passed.");
    return 0;
}
