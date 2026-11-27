#include "hardware_resource.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void make_model(pl_resource_model *model,
                       pl_resource_device *device,
                       pl_resource_address_space spaces[2],
                       pl_resource_memory_region *region,
                       pl_resource_register_bank *bank,
                       pl_resource_register *reg,
                       pl_resource_dma_buffer *dma) {
    *device = (pl_resource_device){9u, 7u};
    spaces[0] = (pl_resource_address_space){1u, PL_RESOURCE_SPACE_DEVICE_BAR, 32u,
        PL_RESOURCE_OWNER_DEVICE, 9u};
    spaces[1] = (pl_resource_address_space){2u, PL_RESOURCE_SPACE_DMA_IOVA, 32u,
        PL_RESOURCE_OWNER_SHARED, 9u};
    *region = (pl_resource_memory_region){2u, 1u, UINT64_C(0x1000), 4096u, 4096u,
        PL_RESOURCE_PERMISSION_READ | PL_RESOURCE_PERMISSION_WRITE,
        PL_RESOURCE_OWNER_DEVICE, 9u};
    *bank = (pl_resource_register_bank){3u, 1u, 2u};
    *reg = (pl_resource_register){4u, 3u, 0x40u, 32u,
        PL_RESOURCE_ACCESS_READ_WRITE, 4u, PL_RESOURCE_OWNER_DEVICE, 1u, 0u};
    *dma = (pl_resource_dma_buffer){5u, 9u, 2u, UINT64_C(0x2000), 4096u, 4096u,
        PL_DMA_TO_DEVICE, PL_DMA_COHERENT, PL_RESOURCE_OWNER_SHARED,
        PL_DMA_CPU_OWNED, 7u};
    memset(model, 0, sizeof(*model));
    model->generation = 7u;
    model->device_count = 1u; model->devices = device;
    model->space_count = 2u; model->spaces = spaces;
    model->region_count = 1u; model->regions = region;
    model->bank_count = 1u; model->banks = bank;
    model->register_count = 1u; model->registers = reg;
    model->dma_count = 1u; model->dma_buffers = dma;
}

static void test_resource_contract_and_register_range(void) {
    pl_resource_model model;
    pl_resource_device device;
    pl_resource_address_space spaces[2];
    pl_resource_memory_region region;
    pl_resource_register_bank bank;
    pl_resource_register reg;
    pl_resource_dma_buffer dma;
    pl_resource_error error;
    make_model(&model, &device, spaces, &region, &bank, &reg, &dma);
    assert(pl_resource_model_validate(&model, &error) == PL_RESOURCE_OK);
    assert(pl_resource_region_contains(&model, 2u, 0x40u, 4u, 4u) == PL_RESOURCE_OK);
    assert(pl_resource_region_contains(&model, 2u, 4094u, 4u, 2u) == PL_RESOURCE_INVALID_RANGE);
    reg.offset = 4094u;
    assert(pl_resource_model_validate(&model, &error) == PL_RESOURCE_INVALID_RANGE);
}

static void test_overflow_alignment_and_dma_ownership(void) {
    pl_resource_model model;
    pl_resource_device device;
    pl_resource_address_space spaces[2];
    pl_resource_memory_region region;
    pl_resource_register_bank bank;
    pl_resource_register reg;
    pl_resource_dma_buffer dma;
    pl_resource_error error;
    make_model(&model, &device, spaces, &region, &bank, &reg, &dma);
    region.base = UINT64_MAX - 4u;
    region.length = 16u;
    assert(pl_resource_model_validate(&model, &error) == PL_RESOURCE_INVALID_RANGE);
    make_model(&model, &device, spaces, &region, &bank, &reg, &dma);
    dma.state = PL_DMA_DEVICE_OWNED;
    assert(pl_resource_model_validate(&model, &error) == PL_RESOURCE_OK);
    assert(pl_dma_transition_validate(PL_DMA_DEVICE_OWNED, PL_DMA_CPU_OWNED, 7u, 7u) ==
           PL_RESOURCE_INVALID_DMA_STATE);
    assert(pl_dma_transition_validate(PL_DMA_DEVICE_OWNED, PL_DMA_COMPLETED_FOR_CPU, 7u, 7u) ==
           PL_RESOURCE_OK);
    assert(pl_dma_transition_validate(PL_DMA_CPU_OWNED, PL_DMA_DEVICE_OWNED, 7u, 6u) ==
           PL_RESOURCE_STALE_GENERATION);
    make_model(&model, &device, spaces, &region, &bank, &reg, &dma);
    dma.device_id = 999u;
    assert(pl_resource_model_validate(&model, &error) == PL_RESOURCE_MISSING_REFERENCE);
}

static void test_rejects_overlapping_regions(void) {
    pl_resource_model model;
    pl_resource_device device;
    pl_resource_address_space spaces[2];
    pl_resource_memory_region regions[2];
    pl_resource_register_bank bank;
    pl_resource_register reg;
    pl_resource_dma_buffer dma;
    pl_resource_error error;
    make_model(&model, &device, spaces, &regions[0], &bank, &reg, &dma);
    regions[1] = regions[0];
    regions[1].id = 6u;
    regions[1].base = 0x1800u;
    model.region_count = 2u;
    model.regions = regions;
    assert(pl_resource_model_validate(&model, &error) == PL_RESOURCE_INVALID_RANGE);
}

int main(void) {
    test_resource_contract_and_register_range();
    test_overflow_alignment_and_dma_ownership();
    test_rejects_overlapping_regions();
    puts("Pytyliki typed hardware resource tests passed");
    return 0;
}
