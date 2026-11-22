#ifndef PYTYLIKI_ERRORS_H
#define PYTYLIKI_ERRORS_H
#include <stddef.h>
#include <stdio.h>
typedef struct {
    const char *code;
    const char *severity;
    const char *status;
    const char *title;
    const char *summary;
    const char *causes;
    const char *remedy;
    const char *documentation;
} pl_error_entry;
size_t pl_error_count(void);
const pl_error_entry *pl_error_at(size_t index);
const pl_error_entry *pl_error_find(const char *code);
void pl_error_print(FILE *stream, const pl_error_entry *entry);
#endif
