#ifndef PYTYLIKI_ARCHITECTURE_ATLAS_H
#define PYTYLIKI_ARCHITECTURE_ATLAS_H

#include <stddef.h>

#define PL_ARCH_ATLAS_COUNT 14u

typedef struct {
    unsigned id;
    const char *name;
    const char *family;
    const char *class_name;
    unsigned word_bits;
    const char *endianness;
    const char *execution_model;
    const char *toolchain_hint;
    const char *diagnostic_focus;
} pl_architecture_profile;

/* Immutable reference data: profiles describe targets; they do not probe or alter hardware. */
size_t pl_architecture_atlas_catalog(const pl_architecture_profile **out);
int pl_architecture_atlas_find(const char *name, pl_architecture_profile *out);
int pl_architecture_atlas_current(char *out, size_t capacity);

#endif
