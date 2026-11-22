#include "errors.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    const pl_error_entry *entry;
    FILE *stream;
    char output[2048] = {0};
    size_t bytes;
    assert(pl_error_count() == 3u);
    assert(pl_error_at(0u) != NULL);
    assert(pl_error_at(pl_error_count()) == NULL);
    assert(pl_error_find(NULL) == NULL);
    assert(pl_error_find("PYT-E9999") == NULL);
    entry = pl_error_find("PYT-E0107");
    assert(entry != NULL);
    assert(strcmp(entry->severity, "error") == 0);
    assert(strstr(entry->remedy, "0.49") != NULL);
    stream = tmpfile();
    assert(stream != NULL);
    pl_error_print(stream, entry);
    rewind(stream);
    bytes = fread(output, 1u, sizeof(output) - 1u, stream);
    assert(!ferror(stream));
    assert(bytes > 0u);
    assert(strstr(output, "PYT-E0107") != NULL);
    assert(strstr(output, "How to fix it") != NULL);
    fclose(stream);
    puts("Pytyliki error registry tests passed.");
    return 0;
}
