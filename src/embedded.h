#ifndef PYTYLIKI_EMBEDDED_H
#define PYTYLIKI_EMBEDDED_H

#include <stddef.h>
#include <stdint.h>

#define PL_EMBEDDED_PROFILE_ID_MAX 40
#define PL_EMBEDDED_COMPILER_MAX 48
#define PL_EMBEDDED_FLAGS_MAX 96
#define PL_EMBEDDED_REGISTER_NAME_MAX 48
#define PL_EMBEDDED_MAX_REGISTERS 64

typedef struct {
    char name[12];
    uint32_t base;
    uint64_t size_bytes;
} pl_embedded_region;

typedef struct {
    char id[PL_EMBEDDED_PROFILE_ID_MAX];
    char architecture[24];
    char compiler[PL_EMBEDDED_COMPILER_MAX];
    char compiler_flags[PL_EMBEDDED_FLAGS_MAX];
    uint64_t clock_hz;
    pl_embedded_region flash;
    pl_embedded_region sram;
    uint64_t stack_reserve_bytes;
    int is_reference_profile;
} pl_embedded_profile;

typedef struct {
    uint64_t text_bytes;
    uint64_t rodata_bytes;
    uint64_t data_bytes;
    uint64_t bss_bytes;
    uint64_t noinit_bytes;
    uint64_t stack_reserve_bytes;
} pl_embedded_sections;

typedef struct {
    uint64_t flash_used_bytes;
    uint64_t sram_used_bytes;
    uint64_t flash_total_bytes;
    uint64_t sram_total_bytes;
    double flash_percent;
    double sram_percent;
    int flash_fits;
    int sram_fits;
} pl_embedded_memory_budget;

typedef struct {
    char name[PL_EMBEDDED_REGISTER_NAME_MAX];
    uint32_t address;
    uint32_t value;
    uint32_t writable_mask;
    int readable;
    int writable;
} pl_embedded_register;

typedef struct {
    pl_embedded_register registers[PL_EMBEDDED_MAX_REGISTERS];
    size_t count;
} pl_embedded_register_bank;

size_t pl_embedded_profile_count(void);
const pl_embedded_profile *pl_embedded_profile_at(size_t index);
const pl_embedded_profile *pl_embedded_profile_find(const char *id);

int pl_embedded_budget_evaluate(const pl_embedded_profile *profile,
                                const pl_embedded_sections *sections,
                                pl_embedded_memory_budget *out);

void pl_embedded_register_bank_init(pl_embedded_register_bank *bank);
int pl_embedded_register_add(pl_embedded_register_bank *bank,
                             const char *name, uint32_t address,
                             uint32_t reset_value, uint32_t writable_mask,
                             int readable, int writable);
int pl_embedded_register_read(const pl_embedded_register_bank *bank,
                              const char *name, uint32_t *out_value);
int pl_embedded_register_write(pl_embedded_register_bank *bank,
                               const char *name, uint32_t value);

int pl_embedded_template_write(const char *path);
int pl_embedded_cli(int argc, char **argv);

#endif
