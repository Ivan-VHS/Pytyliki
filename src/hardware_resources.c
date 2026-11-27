#include "hardware_resources.h"

#include <limits.h>
#include <string.h>

static const pl_hw_address_space *find_space(const pl_hw_resource_model *m, uint32_t id) {
    for (size_t i=0u;i<m->space_count;++i) if (m->spaces[i].id==id) return &m->spaces[i];
    return NULL;
}
static const pl_hw_memory_region *find_region(const pl_hw_resource_model *m, uint32_t id) {
    for (size_t i=0u;i<m->region_count;++i) if (m->regions[i].id==id) return &m->regions[i];
    return NULL;
}
static const pl_hw_register_bank *find_bank(const pl_hw_resource_model *m, uint32_t id) {
    for (size_t i=0u;i<m->bank_count;++i) if (m->banks[i].id==id) return &m->banks[i];
    return NULL;
}
static int power_of_two(uint32_t value) { return value!=0u && (value&(value-1u))==0u; }
static int range_valid(uint64_t base,uint64_t length,uint8_t bits) {
    uint64_t end;
    if (length==0u || base>UINT64_MAX-(length-1u)) return 0;
    end=base+length-1u;
    if (bits<64u && (end>>bits)!=0u) return 0;
    return 1;
}
static int duplicate_u32(uint32_t value,const uint32_t *values,size_t count,size_t self) {
    for(size_t i=0u;i<count;++i) if(i!=self && values[i]==value) return 1;
    return 0;
}
pl_hw_resource_status pl_hw_resource_validate(const pl_hw_resource_model *m) {
    if(m==NULL || m->generation==0u ||
       (m->space_count!=0u && m->spaces==NULL) ||
       (m->region_count!=0u && m->regions==NULL) ||
       (m->bank_count!=0u && m->banks==NULL) ||
       (m->register_count!=0u && m->registers==NULL) ||
       (m->dma_buffer_count!=0u && m->dma_buffers==NULL))
        return PL_HW_RESOURCE_INVALID_ARGUMENT;
    if(m->space_count>PL_HW_RESOURCE_MAX_SPACES ||
       m->region_count>PL_HW_RESOURCE_MAX_REGIONS ||
       m->bank_count>PL_HW_RESOURCE_MAX_BANKS ||
       m->register_count>PL_HW_RESOURCE_MAX_REGISTERS ||
       m->dma_buffer_count>PL_HW_RESOURCE_MAX_DMA_BUFFERS)
        return PL_HW_RESOURCE_LIMIT_EXCEEDED;
    for(size_t i=0u;i<m->space_count;++i) {
        const pl_hw_address_space *s=&m->spaces[i];
        if(s->id==0u || s->address_bits==0u || s->address_bits>64u ||
           (unsigned)s->kind>(unsigned)PL_HW_SPACE_DMA_IOVA ||
           (unsigned)s->owner>(unsigned)PL_HW_OWNER_UNOWNED)
            return PL_HW_RESOURCE_INVALID_ARGUMENT;
        for(size_t j=i+1u;j<m->space_count;++j)
            if(s->id==m->spaces[j].id) return PL_HW_RESOURCE_DUPLICATE_ID;
    }
    for(size_t i=0u;i<m->region_count;++i) {
        const pl_hw_memory_region *r=&m->regions[i];
        const pl_hw_address_space *s=find_space(m,r->space_id);
        if(r->id==0u || s==NULL) return PL_HW_RESOURCE_MISSING_REFERENCE;
        if(!power_of_two(r->alignment) || (r->base%r->alignment)!=0u ||
           (r->length%r->alignment)!=0u) return PL_HW_RESOURCE_ALIGNMENT;
        if(!range_valid(r->base,r->length,s->address_bits))
            return PL_HW_RESOURCE_RANGE_OVERFLOW;
        if((r->permissions&~7u)!=0u || (r->permissions==0u) ||
           (unsigned)r->owner>(unsigned)PL_HW_OWNER_UNOWNED ||
           (unsigned)r->cache_policy>(unsigned)PL_HW_CACHE_DEVICE)
            return PL_HW_RESOURCE_PERMISSION;
        for(size_t j=i+1u;j<m->region_count;++j) {
            const pl_hw_memory_region *other=&m->regions[j];
            if(r->id==other->id) return PL_HW_RESOURCE_DUPLICATE_ID;
            if(r->space_id==other->space_id) {
                uint64_t r_end=r->base+r->length;
                uint64_t o_end=other->base+other->length;
                if(r->base<o_end && other->base<r_end &&
                   (r->alias_group==0u || r->alias_group!=other->alias_group ||
                    r->owner!=other->owner))
                    return PL_HW_RESOURCE_OVERLAP;
            }
        }
    }
    for(size_t i=0u;i<m->bank_count;++i) {
        const pl_hw_register_bank *b=&m->banks[i];
        if(b->id==0u || find_region(m,b->region_id)==NULL)
            return PL_HW_RESOURCE_MISSING_REFERENCE;
        for(size_t j=i+1u;j<m->bank_count;++j)
            if(b->id==m->banks[j].id) return PL_HW_RESOURCE_DUPLICATE_ID;
    }
    for(size_t i=0u;i<m->register_count;++i) {
        const pl_hw_register *r=&m->registers[i];
        const pl_hw_register_bank *b=find_bank(m,r->bank_id);
        const pl_hw_memory_region *region;
        uint64_t width_bytes;
        if(r->id==0u || b==NULL) return PL_HW_RESOURCE_MISSING_REFERENCE;
        region=find_region(m,b->region_id);
        if(region==NULL) return PL_HW_RESOURCE_MISSING_REFERENCE;
        if(r->width_bits!=8u && r->width_bits!=16u && r->width_bits!=32u &&
           r->width_bits!=64u) return PL_HW_RESOURCE_INVALID_ARGUMENT;
        width_bytes=(uint64_t)r->width_bits/8u;
        if(!power_of_two(r->alignment) || (r->offset%r->alignment)!=0u ||
           (r->alignment<width_bytes) || r->offset>region->length ||
           width_bytes>region->length-r->offset) return PL_HW_RESOURCE_ALIGNMENT;
        if(r->is_volatile>1u || (unsigned)r->access>(unsigned)PL_HW_ACCESS_READ_CLEAR ||
           (unsigned)r->side_effect>(unsigned)PL_HW_SIDE_EFFECT_RESET ||
           (unsigned)r->owner>(unsigned)PL_HW_OWNER_UNOWNED)
            return PL_HW_RESOURCE_PERMISSION;
        for(size_t j=i+1u;j<m->register_count;++j)
            if(r->id==m->registers[j].id) return PL_HW_RESOURCE_DUPLICATE_ID;
    }
    for(size_t i=0u;i<m->dma_buffer_count;++i) {
        const pl_hw_dma_buffer *b=&m->dma_buffers[i];
        const pl_hw_address_space *s=find_space(m,b->space_id);
        if(b->id==0u || b->device_id==0u || s==NULL)
            return PL_HW_RESOURCE_MISSING_REFERENCE;
        if(!power_of_two(b->alignment) || b->base%b->alignment!=0u ||
           b->length%b->alignment!=0u) return PL_HW_RESOURCE_ALIGNMENT;
        if(!range_valid(b->base,b->length,s->address_bits))
            return PL_HW_RESOURCE_RANGE_OVERFLOW;
        if((unsigned)b->direction>(unsigned)PL_HW_DMA_BIDIRECTIONAL ||
           (unsigned)b->coherency>(unsigned)PL_HW_DMA_EXPLICIT_SYNC ||
           (unsigned)b->state>(unsigned)PL_HW_DMA_RELEASED ||
           (unsigned)b->owner>(unsigned)PL_HW_OWNER_SHARED ||
           b->generation!=m->generation) return PL_HW_RESOURCE_STALE_GENERATION;
        if((b->state==PL_HW_DMA_DEVICE_OWNED && b->owner!=PL_HW_OWNER_DEVICE) ||
           (b->state==PL_HW_DMA_CPU_OWNED && b->owner!=PL_HW_OWNER_CPU) ||
           (b->state==PL_HW_DMA_RELEASED && b->owner!=PL_HW_OWNER_UNOWNED))
            return PL_HW_RESOURCE_OWNERSHIP;
        for(size_t j=i+1u;j<m->dma_buffer_count;++j)
            if(b->id==m->dma_buffers[j].id) return PL_HW_RESOURCE_DUPLICATE_ID;
    }
    return PL_HW_RESOURCE_OK;
}
pl_hw_resource_status pl_hw_dma_transition(pl_hw_dma_buffer *b,
    pl_hw_dma_state next, pl_hw_actor actor, uint32_t expected_generation,
    int capability_granted, int submission_accepted, int completion_observed) {
    if(b==NULL || (unsigned)next>(unsigned)PL_HW_DMA_RELEASED ||
       (unsigned)actor>(unsigned)PL_HW_ACTOR_ADAPTER)
        return PL_HW_RESOURCE_INVALID_ARGUMENT;
    if(b->generation==0u || b->generation!=expected_generation)
        return PL_HW_RESOURCE_STALE_GENERATION;
    if(actor==PL_HW_ACTOR_ADAPTER && !capability_granted)
        return PL_HW_RESOURCE_PERMISSION;
    switch(b->state) {
    case PL_HW_DMA_CPU_OWNED:
        if(next==PL_HW_DMA_PREPARED_FOR_DEVICE && actor==PL_HW_ACTOR_CPU) {
            b->state=next; b->owner=PL_HW_OWNER_SHARED; return PL_HW_RESOURCE_OK;
        }
        if(next==PL_HW_DMA_RELEASED && actor==PL_HW_ACTOR_CPU) {
            b->state=next; b->owner=PL_HW_OWNER_UNOWNED; return PL_HW_RESOURCE_OK;
        }
        break;
    case PL_HW_DMA_PREPARED_FOR_DEVICE:
        if(next==PL_HW_DMA_CPU_OWNED && actor==PL_HW_ACTOR_CPU && !submission_accepted) {
            b->state=next; b->owner=PL_HW_OWNER_CPU; return PL_HW_RESOURCE_OK;
        }
        if(next==PL_HW_DMA_DEVICE_OWNED && actor==PL_HW_ACTOR_ADAPTER &&
           capability_granted && submission_accepted) {
            b->state=next; b->owner=PL_HW_OWNER_DEVICE; return PL_HW_RESOURCE_OK;
        }
        break;
    case PL_HW_DMA_DEVICE_OWNED:
        if(next==PL_HW_DMA_COMPLETED_FOR_CPU && actor==PL_HW_ACTOR_ADAPTER &&
           capability_granted && completion_observed) {
            b->state=next; b->owner=PL_HW_OWNER_SHARED; return PL_HW_RESOURCE_OK;
        }
        break;
    case PL_HW_DMA_COMPLETED_FOR_CPU:
        if(next==PL_HW_DMA_CPU_OWNED && actor==PL_HW_ACTOR_CPU) {
            b->state=next; b->owner=PL_HW_OWNER_CPU; return PL_HW_RESOURCE_OK;
        }
        if(next==PL_HW_DMA_RELEASED && actor==PL_HW_ACTOR_CPU) {
            b->state=next; b->owner=PL_HW_OWNER_UNOWNED; return PL_HW_RESOURCE_OK;
        }
        break;
    case PL_HW_DMA_RELEASED:
    default: break;
    }
    return PL_HW_RESOURCE_OWNERSHIP;
}
const char *pl_hw_resource_status_name(pl_hw_resource_status s) {
    switch(s) {
    case PL_HW_RESOURCE_OK:return "ok";
    case PL_HW_RESOURCE_INVALID_ARGUMENT:return "invalid-argument";
    case PL_HW_RESOURCE_LIMIT_EXCEEDED:return "limit-exceeded";
    case PL_HW_RESOURCE_DUPLICATE_ID:return "duplicate-id";
    case PL_HW_RESOURCE_MISSING_REFERENCE:return "missing-reference";
    case PL_HW_RESOURCE_RANGE_OVERFLOW:return "range-overflow";
    case PL_HW_RESOURCE_ALIGNMENT:return "alignment";
    case PL_HW_RESOURCE_PERMISSION:return "permission";
    case PL_HW_RESOURCE_OVERLAP:return "overlap";
    case PL_HW_RESOURCE_OWNERSHIP:return "ownership";
    case PL_HW_RESOURCE_STALE_GENERATION:return "stale-generation";
    }
    return "unknown";
}
