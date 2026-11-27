#include "hardware_resources.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    pl_hw_address_space space={1u,PL_HW_SPACE_PHYSICAL,48u,PL_HW_OWNER_CPU,0u};
    pl_hw_memory_region region={2u,1u,0x1000u,0x1000u,0u,0x100u,3u,PL_HW_CACHE_DEVICE,PL_HW_OWNER_CPU,0u};
    pl_hw_register_bank bank={3u,2u};
    pl_hw_register reg={4u,3u,0x10u,32u,PL_HW_ACCESS_READ_WRITE,1u,4u,PL_HW_SIDE_EFFECT_NONE,PL_HW_OWNER_CPU};
    pl_hw_dma_buffer dma={5u,9u,1u,0x4000u,0x1000u,0x100u,PL_HW_DMA_TO_DEVICE,PL_HW_DMA_EXPLICIT_SYNC,PL_HW_OWNER_CPU,PL_HW_DMA_CPU_OWNED,7u};
    pl_hw_resource_model model={7u,1u,1u,1u,1u,1u,&space,&region,&bank,&reg,&dma};
    assert(pl_hw_resource_validate(&model)==PL_HW_RESOURCE_OK);
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_PREPARED_FOR_DEVICE,PL_HW_ACTOR_CPU,7u,0,0,0)==PL_HW_RESOURCE_OK);
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_DEVICE_OWNED,PL_HW_ACTOR_ADAPTER,7u,0,1,0)==PL_HW_RESOURCE_PERMISSION);
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_DEVICE_OWNED,PL_HW_ACTOR_ADAPTER,7u,1,1,0)==PL_HW_RESOURCE_OK);
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_CPU_OWNED,PL_HW_ACTOR_CPU,7u,0,0,0)==PL_HW_RESOURCE_OWNERSHIP);
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_COMPLETED_FOR_CPU,PL_HW_ACTOR_ADAPTER,7u,1,0,0)==PL_HW_RESOURCE_OWNERSHIP);
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_COMPLETED_FOR_CPU,PL_HW_ACTOR_ADAPTER,7u,1,0,1)==PL_HW_RESOURCE_OK);
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_CPU_OWNED,PL_HW_ACTOR_CPU,7u,0,0,0)==PL_HW_RESOURCE_OK);
    dma.state=PL_HW_DMA_CPU_OWNED; dma.owner=PL_HW_OWNER_CPU;
    dma.generation=6u;
    assert(pl_hw_dma_transition(&dma,PL_HW_DMA_PREPARED_FOR_DEVICE,PL_HW_ACTOR_CPU,7u,0,0,0)==PL_HW_RESOURCE_STALE_GENERATION);
    dma.generation=7u;
    assert(pl_hw_resource_validate(&model)==PL_HW_RESOURCE_OK);
    {
        pl_hw_memory_region bad=region;
        bad.base=UINT64_MAX-7u; bad.length=16u;
        pl_hw_resource_model invalid=model; invalid.regions=&bad;
        assert(pl_hw_resource_validate(&invalid)==PL_HW_RESOURCE_RANGE_OVERFLOW);
    }
    {
        pl_hw_memory_region second={6u,1u,0x1800u,0x100u,0u,0x100u,3u,PL_HW_CACHE_DEVICE,PL_HW_OWNER_CPU,0u};
        pl_hw_resource_model invalid=model; invalid.region_count=2u;
        pl_hw_memory_region regions[2]={region,second}; invalid.regions=regions;
        assert(pl_hw_resource_validate(&invalid)==PL_HW_RESOURCE_OVERLAP);
    }
    {
        pl_hw_register bad=reg; bad.offset=0xfffu;
        pl_hw_resource_model invalid=model; invalid.registers=&bad;
        assert(pl_hw_resource_validate(&invalid)==PL_HW_RESOURCE_ALIGNMENT);
    }
    puts("hardware resource validator tests passed");
    return 0;
}
