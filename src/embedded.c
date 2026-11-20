#include "embedded.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const pl_embedded_profile g_profiles[] = {
    {
        "cortex-m4-reference", "arm-cortex-m", "arm-none-eabi-gcc",
        "-mcpu=cortex-m4 -mthumb", UINT64_C(84000000),
        {"FLASH", UINT32_C(0x08000000), UINT64_C(512) * 1024u},
        {"SRAM", UINT32_C(0x20000000), UINT64_C(128) * 1024u},
        2048u, 1
    },
    {
        "riscv32imc-reference", "riscv32", "riscv64-unknown-elf-gcc",
        "-march=rv32imc -mabi=ilp32", UINT64_C(50000000),
        {"FLASH", UINT32_C(0x20000000), UINT64_C(256) * 1024u},
        {"SRAM", UINT32_C(0x80000000), UINT64_C(64) * 1024u},
        2048u, 1
    },
    {
        "avr-atmega328p", "avr", "avr-gcc", "-mmcu=atmega328p",
        UINT64_C(16000000),
        {"FLASH", 0u, UINT64_C(32) * 1024u},
        {"SRAM", UINT32_C(0x800100), UINT64_C(2) * 1024u},
        256u, 0
    }
};

static int add_u64(uint64_t a, uint64_t b, uint64_t *out) {
    if (!out || UINT64_MAX - a < b) return -1;
    *out = a + b;
    return 0;
}

size_t pl_embedded_profile_count(void) {
    return sizeof(g_profiles) / sizeof(g_profiles[0]);
}

const pl_embedded_profile *pl_embedded_profile_at(size_t index) {
    return index < pl_embedded_profile_count() ? &g_profiles[index] : NULL;
}

const pl_embedded_profile *pl_embedded_profile_find(const char *id) {
    size_t i;
    if (!id || !id[0]) return NULL;
    for (i = 0u; i < pl_embedded_profile_count(); ++i)
        if (strcmp(g_profiles[i].id, id) == 0) return &g_profiles[i];
    return NULL;
}

int pl_embedded_budget_evaluate(const pl_embedded_profile *profile,
                                const pl_embedded_sections *sections,
                                pl_embedded_memory_budget *out) {
    uint64_t flash = 0u, sram = 0u, static_sram = 0u;
    if (!profile || !sections || !out || !profile->flash.size_bytes ||
        !profile->sram.size_bytes ||
        sections->stack_reserve_bytes > profile->sram.size_bytes ||
        add_u64(sections->text_bytes, sections->rodata_bytes, &flash) != 0 ||
        add_u64(flash, sections->data_bytes, &flash) != 0 ||
        add_u64(sections->data_bytes, sections->bss_bytes, &static_sram) != 0 ||
        add_u64(static_sram, sections->noinit_bytes, &static_sram) != 0 ||
        add_u64(static_sram, sections->stack_reserve_bytes, &sram) != 0)
        return -1;
    memset(out, 0, sizeof(*out));
    out->flash_used_bytes = flash;
    out->sram_used_bytes = sram;
    out->flash_total_bytes = profile->flash.size_bytes;
    out->sram_total_bytes = profile->sram.size_bytes;
    out->flash_percent = (double)flash * 100.0 / (double)profile->flash.size_bytes;
    out->sram_percent = (double)sram * 100.0 / (double)profile->sram.size_bytes;
    out->flash_fits = flash <= profile->flash.size_bytes;
    out->sram_fits = sram <= profile->sram.size_bytes;
    return 0;
}

void pl_embedded_register_bank_init(pl_embedded_register_bank *bank) {
    if (bank) memset(bank, 0, sizeof(*bank));
}

static pl_embedded_register *find_register(pl_embedded_register_bank *bank,
                                           const char *name) {
    size_t i;
    if (!bank || !name) return NULL;
    for (i = 0u; i < bank->count; ++i)
        if (strcmp(bank->registers[i].name, name) == 0) return &bank->registers[i];
    return NULL;
}

int pl_embedded_register_add(pl_embedded_register_bank *bank,
                             const char *name, uint32_t address,
                             uint32_t reset_value, uint32_t writable_mask,
                             int readable, int writable) {
    pl_embedded_register *item;
    size_t length;
    if (!bank || !name || !name[0] || bank->count >= PL_EMBEDDED_MAX_REGISTERS ||
        find_register(bank, name)) return -1;
    length = strlen(name);
    if (length >= PL_EMBEDDED_REGISTER_NAME_MAX || (!readable && !writable)) return -1;
    item = &bank->registers[bank->count++];
    memset(item, 0, sizeof(*item));
    memcpy(item->name, name, length + 1u);
    item->address = address;
    item->value = reset_value;
    item->writable_mask = writable ? writable_mask : 0u;
    item->readable = readable != 0;
    item->writable = writable != 0;
    return 0;
}

int pl_embedded_register_read(const pl_embedded_register_bank *bank,
                              const char *name, uint32_t *out_value) {
    size_t i;
    if (!bank || !name || !out_value) return -1;
    for (i = 0u; i < bank->count; ++i) {
        const pl_embedded_register *item = &bank->registers[i];
        if (strcmp(item->name, name) == 0) {
            if (!item->readable) return -2;
            *out_value = item->value;
            return 0;
        }
    }
    return -1;
}

int pl_embedded_register_write(pl_embedded_register_bank *bank,
                               const char *name, uint32_t value) {
    pl_embedded_register *item = find_register(bank, name);
    if (!item) return -1;
    if (!item->writable) return -2;
    if ((value & ~item->writable_mask) != 0u) return -3;
    item->value = (item->value & ~item->writable_mask) | value;
    return 0;
}

int pl_embedded_template_write(const char *path) {
    static const char template_text[] =
        "#include <stdint.h>\n"
        "/* Hardware-neutral bring-up loop. Replace this with device-specific\n"
        " * startup and SVD-derived GPIO operations after checking the datasheet. */\n"
        "static volatile uint32_t heartbeat;\n"
        "int main(void) {\n"
        "    for (;;) { heartbeat++; }\n"
        "}\n";
    FILE *file;
    size_t length = sizeof(template_text) - 1u;
    if (!path || !path[0]) return -1;
#if defined(_MSC_VER)
    {
        errno_t open_error = fopen_s(&file, path, "wb");
        if (open_error != 0 || !file) return -1;
    }
#else
    file = fopen(path, "wb");
    if (!file) return -1;
#endif
    if (fwrite(template_text, 1u, length, file) != length) {
        fclose(file);
        return -1;
    }
    return fclose(file) == 0 ? 0 : -1;
}

static int parse_u64(const char *text, uint64_t *out) {
    char *end = NULL;
    unsigned long long value;
    if (!text || !text[0] || !out || text[0] == '-') return -1;
    errno = 0;
    value = strtoull(text, &end, 0);
    if (errno == ERANGE || end == text || !end || *end) return -1;
    *out = (uint64_t)value;
    return 0;
}

static void print_profiles(void) {
    size_t i;
    puts("Pytyliki native Embedded C MCU profiles:");
    for (i = 0u; i < pl_embedded_profile_count(); ++i) {
        const pl_embedded_profile *p = pl_embedded_profile_at(i);
        printf("%-26s | %-12s | %s | %llu Hz | FLASH %llu B | SRAM %llu B | stack %llu B%s\n",
               p->id, p->architecture, p->compiler,
               (unsigned long long)p->clock_hz,
               (unsigned long long)p->flash.size_bytes,
               (unsigned long long)p->sram.size_bytes,
               (unsigned long long)p->stack_reserve_bytes,
               p->is_reference_profile ? " | reference profile" : "");
    }
    puts("Reference addresses and sizes must be verified against the exact MCU datasheet.");
}

int pl_embedded_cli(int argc, char **argv) {
    const char *command;
    if (argc < 2 || !argv) {
        fprintf(stderr, "Usage: pytyliki --embedded profiles|profile ID|budget ID TEXT RODATA DATA BSS STACK|template FILE.c\n");
        return 2;
    }
    command = argv[1];
    if (strcmp(command, "profiles") == 0) {
        if (argc != 2) return 2;
        print_profiles();
        return 0;
    }
    if (strcmp(command, "profile") == 0) {
        const pl_embedded_profile *p;
        if (argc != 3 || !(p = pl_embedded_profile_find(argv[2]))) {
            fprintf(stderr, "Unknown or incomplete embedded profile.\n");
            return 2;
        }
        printf("%s | %s | %s | %llu Hz\nFLASH: base=0x%08X size=%llu bytes\nSRAM: base=0x%08X size=%llu bytes\nStack reserve: %llu bytes\nFlags: %s\n",
               p->id, p->architecture, p->compiler, (unsigned long long)p->clock_hz,
               p->flash.base, (unsigned long long)p->flash.size_bytes,
               p->sram.base, (unsigned long long)p->sram.size_bytes,
               (unsigned long long)p->stack_reserve_bytes, p->compiler_flags);
        return 0;
    }
    if (strcmp(command, "template") == 0) {
        if (argc != 3 || pl_embedded_template_write(argv[2]) != 0) {
            fprintf(stderr, "Could not write Embedded C template.\n");
            return 1;
        }
        printf("Embedded C starter written: %s\n", argv[2]);
        return 0;
    }
    if (strcmp(command, "budget") == 0) {
        const pl_embedded_profile *p;
        pl_embedded_sections sections = {0};
        pl_embedded_memory_budget budget;
        uint64_t *values[] = {
            &sections.text_bytes, &sections.rodata_bytes, &sections.data_bytes,
            &sections.bss_bytes, &sections.stack_reserve_bytes
        };
        size_t i;
        if (argc != 8 || !(p = pl_embedded_profile_find(argv[2]))) return 2;
        for (i = 0u; i < 5u; ++i)
            if (parse_u64(argv[i + 3u], values[i]) != 0) return 2;
        if (pl_embedded_budget_evaluate(p, &sections, &budget) != 0) return 2;
        printf("FLASH %llu/%llu bytes (%.2f%%): %s\nSRAM incl. stack %llu/%llu bytes (%.2f%%): %s\n",
               (unsigned long long)budget.flash_used_bytes,
               (unsigned long long)budget.flash_total_bytes, budget.flash_percent,
               budget.flash_fits ? "PASS" : "OVER BUDGET",
               (unsigned long long)budget.sram_used_bytes,
               (unsigned long long)budget.sram_total_bytes, budget.sram_percent,
               budget.sram_fits ? "PASS" : "OVER BUDGET");
        return budget.flash_fits && budget.sram_fits ? 0 : 3;
    }
    fprintf(stderr, "Unknown Embedded C command: %s\n", command);
    return 2;
}
