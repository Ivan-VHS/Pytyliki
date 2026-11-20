#include "embedded.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const pl_embedded_profile *profile;
    pl_embedded_sections sections = {0};
    pl_embedded_memory_budget budget;
    pl_embedded_register_bank bank;
    uint32_t value = 0u;
    size_t i;

    assert(pl_embedded_profile_count() == 3u);
    profile = pl_embedded_profile_find("cortex-m4-reference");
    assert(profile != NULL);
    assert(profile->flash.size_bytes == UINT64_C(512) * 1024u);
    assert(profile->sram.size_bytes == UINT64_C(128) * 1024u);
    assert(pl_embedded_profile_at(3u) == NULL);
    assert(pl_embedded_profile_find("missing") == NULL);

    sections.text_bytes = 1024u;
    sections.rodata_bytes = 256u;
    sections.data_bytes = 128u;
    sections.bss_bytes = 512u;
    sections.noinit_bytes = 64u;
    sections.stack_reserve_bytes = 2048u;
    assert(pl_embedded_budget_evaluate(profile, &sections, &budget) == 0);
    assert(budget.flash_used_bytes == 1408u);
    assert(budget.sram_used_bytes == 2752u);
    assert(budget.flash_fits && budget.sram_fits);
    sections.text_bytes = profile->flash.size_bytes + 1u;
    assert(pl_embedded_budget_evaluate(profile, &sections, &budget) == 0);
    assert(!budget.flash_fits);
    sections.text_bytes = 0u;
    sections.stack_reserve_bytes = UINT64_MAX;
    assert(pl_embedded_budget_evaluate(profile, &sections, &budget) != 0);

    pl_embedded_register_bank_init(&bank);
    assert(pl_embedded_register_add(&bank, "GPIOA.ODR", 0x40020014u, 0u,
                                    0x0000FFFFu, 1, 1) == 0);
    assert(pl_embedded_register_add(&bank, "GPIOA.IDR", 0x40020010u, 3u,
                                    0u, 1, 0) == 0);
    assert(pl_embedded_register_add(&bank, "GPIOA.ODR", 0u, 0u, 0u, 1, 1) != 0);
    assert(pl_embedded_register_write(&bank, "GPIOA.ODR", 0x25u) == 0);
    assert(pl_embedded_register_read(&bank, "GPIOA.ODR", &value) == 0 && value == 0x25u);
    assert(pl_embedded_register_read(&bank, "GPIOA.IDR", &value) == 0 && value == 3u);
    assert(pl_embedded_register_write(&bank, "GPIOA.ODR", 0x10000u) == -3);
    assert(pl_embedded_register_write(&bank, "GPIOA.IDR", 1u) == -2);
    assert(pl_embedded_register_read(&bank, "missing", &value) != 0);
    for (i = 0u; i < PL_EMBEDDED_MAX_REGISTERS; ++i) {
        char name[PL_EMBEDDED_REGISTER_NAME_MAX];
        (void)snprintf(name, sizeof(name), "R%lu", (unsigned long)i);
        if (i < PL_EMBEDDED_MAX_REGISTERS - 2u)
            assert(pl_embedded_register_add(&bank, name, (uint32_t)i, 0u, UINT32_MAX, 1, 1) == 0);
    }
    assert(bank.count == PL_EMBEDDED_MAX_REGISTERS);

    puts("Pytyliki Embedded C tests passed.");
    return 0;
}
