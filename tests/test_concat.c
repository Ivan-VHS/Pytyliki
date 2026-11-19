#include "concat.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

int main(void) {
    const char *text[] = {"con", "cat", "enation"};
    char output[32];
    size_t written = 99u;

    assert(pl_concat_text(text, 3u, output, sizeof(output), &written) == 0);
    assert(strcmp(output, "concatenation") == 0);
    assert(written == strlen("concatenation"));

    assert(pl_concat_text(text, 3u, output, 5u, &written) == -2);
    assert(written == 0u);
    assert(pl_concat_text(NULL, 0u, output, sizeof(output), &written) == 0);
    assert(output[0] == '\0' && written == 0u);

    {
        const unsigned char a[] = {0x00u, 0x7fu};
        const unsigned char b[] = {0x80u, 0xffu};
        const void *parts[] = {a, b};
        const size_t lengths[] = {sizeof(a), sizeof(b)};
        unsigned char bytes[4] = {0u};
        assert(pl_concat_bytes(parts, lengths, 2u, bytes, sizeof(bytes), &written) == 0);
        assert(written == 4u);
        assert(memcmp(bytes, "\0\x7f\x80\xff", 4u) == 0);
    }

    {
        const void *parts[] = {"x"};
        const size_t lengths[] = {2u};
        unsigned char bytes[1];
        assert(pl_concat_bytes(parts, lengths, 1u, bytes, sizeof(bytes), &written) == -2);
        assert(written == 0u);
    }

    assert(pl_concat_bytes(NULL, NULL, 0u, NULL, 0u, &written) == 0);
    assert(written == 0u);
    assert(pl_concat_bytes(NULL, NULL, 1u, output, sizeof(output), &written) == -1);
    return 0;
}
