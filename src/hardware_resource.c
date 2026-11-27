#include "hardware_resource.h"

#include <stdio.h>
#include <string.h>

static pl_resource_status fail(pl_resource_error *error,
                               pl_resource_status status,
                               uint32_t id, const char *message) {
    if (error != NULL) {
        error->status = status;
        error->object_id = id;
        (void)snprintf(error->message, sizeof(error->message), "%s", message);
    }
    return status;
}

static const pl_resource_device *device_by_id(
    const pl_resource_model *m, uint32_t id) {
    size_t i;
    for (i = 0u; i < m->device_count; ++i)
        if (m->devices[i].id == id) return &m->devices[i];
    return NULL;
}
static const pl_resource_address_space *space_by_id(
    const pl_resource_model *m, uint32_t id) {
    size_t i;
    for (i = 0u; i < m->space_count; ++i)
        if (m->spaces[i].id == id) return &m->spaces[i];
    return NULL;
}
static const pl_resource_memory_region *region_by_id(
    const pl_resource_model *m, uint32_t id) {
    size_t i;
    for (i = 0u; i < m->region_count; ++i)
        if (m->regions[i].id == id) return &m->regions[i];
    return NULL;
}
static const pl_resource_register_bank *bank_by_id(
    const pl_resource_model *m, uint32_t id) {
    size_t i;
    for (i = 0u; i < m->bank_count; ++i)
        if (m->banks[i].id == id) return &m->banks[i];
    return NULL;
}
static int power_of_two(uint32_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}
static int id_unique(uint32_t id, const uint32_t *ids, size_t count) {
    size_t i;
    for (i = 0u; i < count; ++i) if (ids[i] == id) return 0;
    return 1;
}

pl_resource_status pl_resource_region_contains(
    const pl_resource_model *model, uint32_t region_id,
    uint64_t offset, uint64_t length, uint32_t alignment
) {
    const pl_resource_memory_region *region;
    if (model == NULL || length == 0u ||
        model->region_count > PL_RESOURCE_MAX_REGIONS ||
        (model->region_count != 0u && model->regions == NULL))
        return PL_RESOURCE_INVALID_ARGUMENT;
    region = region_by_id(model, region_id);
    if (region == NULL) return PL_RESOURCE_MISSING_REFERENCE;
    if (offset > region->length || length > region->length - offset)
        return PL_RESOURCE_INVALID_RANGE;
    if (!power_of_two(alignment) || (offset % alignment) != 0u ||
        (region->base % alignment) != 0u)
        return PL_RESOURCE_INVALID_ALIGNMENT;
    return PL_RESOURCE_OK;
}

pl_resource_status pl_resource_model_validate(
    const pl_resource_model *m, pl_resource_error *error
) {
    uint32_t ids[PL_RESOURCE_MAX_REGISTERS > PL_RESOURCE_MAX_DMA_BUFFERS ?
                 PL_RESOURCE_MAX_REGISTERS : PL_RESOURCE_MAX_DMA_BUFFERS];
    size_t i, j;
    if (error != NULL) memset(error, 0, sizeof(*error));
    if (m == NULL || m->generation == 0u ||
        (m->device_count != 0u && m->devices == NULL) ||
        (m->space_count != 0u && m->spaces == NULL) ||
        (m->region_count && m->regions == NULL) ||
        (m->bank_count && m->banks == NULL) ||
        (m->register_count && m->registers == NULL) ||
        (m->dma_count && m->dma_buffers == NULL))
        return fail(error, PL_RESOURCE_INVALID_ARGUMENT, 0u, "model arrays and generation are required");
    if (m->device_count > PL_RESOURCE_MAX_DEVICES ||
        m->space_count > PL_RESOURCE_MAX_SPACES ||
        m->region_count > PL_RESOURCE_MAX_REGIONS ||
        m->bank_count > PL_RESOURCE_MAX_BANKS ||
        m->register_count > PL_RESOURCE_MAX_REGISTERS ||
        m->dma_count > PL_RESOURCE_MAX_DMA_BUFFERS)
        return fail(error, PL_RESOURCE_LIMIT_EXCEEDED, 0u, "resource model exceeds a bounded collection limit");

    for (i = 0u; i < m->device_count; ++i) {
        const pl_resource_device *device = &m->devices[i];
        if (device->id == 0u)
            return fail(error, PL_RESOURCE_DUPLICATE_ID, device->id, "device ID must be non-zero");
        if (device->generation != m->generation)
            return fail(error, PL_RESOURCE_STALE_GENERATION, device->id, "device generation does not match resource snapshot");
        for (j = 0u; j < i; ++j)
            if (m->devices[j].id == device->id)
                return fail(error, PL_RESOURCE_DUPLICATE_ID, device->id, "device ID is duplicated");
    }
    for (i = 0u; i < m->space_count; ++i) {
        const pl_resource_address_space *s = &m->spaces[i];
        if (s->id == 0u || !id_unique(s->id, ids, i))
            return fail(error, PL_RESOURCE_DUPLICATE_ID, s->id, "address-space ID is zero or duplicated");
        ids[i] = s->id;
        if ((unsigned)s->kind > (unsigned)PL_RESOURCE_SPACE_DMA_IOVA ||
            (unsigned)s->owner > (unsigned)PL_RESOURCE_OWNER_UNOWNED ||
            s->address_bits == 0u || s->address_bits > 64u)
            return fail(error, PL_RESOURCE_INVALID_ARGUMENT, s->id, "address-space kind, owner or width is invalid");
        if (s->device_id != 0u && device_by_id(m, s->device_id) == NULL)
            return fail(error, PL_RESOURCE_MISSING_REFERENCE, s->id, "address space references a missing device");
    }
    for (i = 0u; i < m->region_count; ++i) {
        const pl_resource_memory_region *r = &m->regions[i];
        const pl_resource_address_space *s = space_by_id(m, r->space_id);
        if (r->id == 0u || s == NULL)
            return fail(error, PL_RESOURCE_MISSING_REFERENCE, r->id, "memory region references a missing address space");
        for (j = 0u; j < i; ++j)
            if (m->regions[j].id == r->id)
                return fail(error, PL_RESOURCE_DUPLICATE_ID, r->id, "memory region ID is duplicated");
        if (r->length == 0u || r->base > UINT64_MAX - (r->length - 1u))
            return fail(error, PL_RESOURCE_INVALID_RANGE, r->id, "memory region is empty or overflows 64-bit address space");
        if (s->address_bits < 64u) {
            uint64_t limit = UINT64_C(1) << s->address_bits;
            if (r->base >= limit || r->length > limit - r->base)
                return fail(error, PL_RESOURCE_INVALID_RANGE, r->id, "memory region exceeds its address-space width");
        }
        if (!power_of_two(r->alignment) || (r->base % r->alignment) != 0u)
            return fail(error, PL_RESOURCE_INVALID_ALIGNMENT, r->id, "memory region base alignment is invalid");
        if ((unsigned)r->owner > (unsigned)PL_RESOURCE_OWNER_UNOWNED)
            return fail(error, PL_RESOURCE_INVALID_OWNER, r->id, "memory region owner is invalid");
        if (r->permissions == 0u || (r->permissions & ~PL_RESOURCE_PERMISSION_ALL) != 0u)
            return fail(error, PL_RESOURCE_INVALID_ARGUMENT, r->id, "memory region permissions are empty or unknown");
        if (r->device_id != 0u && device_by_id(m, r->device_id) == NULL)
            return fail(error, PL_RESOURCE_MISSING_REFERENCE, r->id, "memory region references a missing device");
        for (j = 0u; j < i; ++j) {
            const pl_resource_memory_region *other = &m->regions[j];
            const uint64_t current_last = r->base + r->length - 1u;
            const uint64_t other_last = other->base + other->length - 1u;
            if (other->space_id == r->space_id &&
                r->base <= other_last && other->base <= current_last)
                return fail(error, PL_RESOURCE_INVALID_RANGE, r->id, "memory regions overlap in the same address space");
        }
    }
    for (i = 0u; i < m->bank_count; ++i) {
        const pl_resource_register_bank *b = &m->banks[i];
        const pl_resource_memory_region *r = region_by_id(m, b->region_id);
        if (b->id == 0u || space_by_id(m, b->space_id) == NULL || r == NULL ||
            r->space_id != b->space_id)
            return fail(error, PL_RESOURCE_MISSING_REFERENCE, b->id, "register bank has an invalid space or region reference");
        for (j = 0u; j < i; ++j)
            if (m->banks[j].id == b->id)
                return fail(error, PL_RESOURCE_DUPLICATE_ID, b->id, "register bank ID is duplicated");
    }
    for (i = 0u; i < m->register_count; ++i) {
        const pl_resource_register *r = &m->registers[i];
        const pl_resource_register_bank *b = bank_by_id(m, r->bank_id);
        const pl_resource_memory_region *region;
        uint32_t bytes;
        if (r->id == 0u || b == NULL)
            return fail(error, PL_RESOURCE_MISSING_REFERENCE, r->id, "register references a missing bank");
        for (j = 0u; j < i; ++j)
            if (m->registers[j].id == r->id)
                return fail(error, PL_RESOURCE_DUPLICATE_ID, r->id, "register ID is duplicated");
        if (r->width_bits != 8u && r->width_bits != 16u &&
            r->width_bits != 32u && r->width_bits != 64u && r->width_bits != 128u)
            return fail(error, PL_RESOURCE_INVALID_REGISTER, r->id, "register width is unsupported");
        if ((unsigned)r->access > (unsigned)PL_RESOURCE_ACCESS_READ_CLEAR ||
            (unsigned)r->owner > (unsigned)PL_RESOURCE_OWNER_UNOWNED || r->is_volatile > 1u ||
            r->has_side_effects > 1u)
            return fail(error, PL_RESOURCE_INVALID_REGISTER, r->id, "register access, owner or flags are invalid");
        bytes = (uint32_t)r->width_bits / 8u;
        region = region_by_id(m, b->region_id);
        if (region == NULL || r->offset > region->length ||
            bytes > region->length - r->offset)
            return fail(error, PL_RESOURCE_INVALID_RANGE, r->id, "register byte range exceeds its backing region");
        if (!power_of_two(r->alignment) || (r->offset % r->alignment) != 0u ||
            (r->offset % bytes) != 0u)
            return fail(error, PL_RESOURCE_INVALID_ALIGNMENT, r->id, "register offset alignment is invalid");
    }
    for (i = 0u; i < m->dma_count; ++i) {
        const pl_resource_dma_buffer *d = &m->dma_buffers[i];
        const pl_resource_address_space *s = space_by_id(m, d->space_id);
        if (d->id == 0u || d->device_id == 0u || s == NULL ||
            device_by_id(m, d->device_id) == NULL)
            return fail(error, PL_RESOURCE_MISSING_REFERENCE, d->id, "DMA buffer has a missing device or address space");
        if (s->kind != PL_RESOURCE_SPACE_DMA_IOVA)
            return fail(error, PL_RESOURCE_INVALID_ARGUMENT, d->id, "DMA buffer requires a DMA IOVA address space");
        for (j = 0u; j < i; ++j)
            if (m->dma_buffers[j].id == d->id)
                return fail(error, PL_RESOURCE_DUPLICATE_ID, d->id, "DMA buffer ID is duplicated");
        if (d->generation != m->generation)
            return fail(error, PL_RESOURCE_STALE_GENERATION, d->id, "DMA buffer generation does not match the resource snapshot");
        if (d->length == 0u || d->base > UINT64_MAX - (d->length - 1u))
            return fail(error, PL_RESOURCE_INVALID_RANGE, d->id, "DMA range is empty or overflows");
        if (s->address_bits < 64u) {
            uint64_t limit = UINT64_C(1) << s->address_bits;
            if (d->base >= limit || d->length > limit - d->base)
                return fail(error, PL_RESOURCE_INVALID_RANGE, d->id, "DMA range exceeds address-space width");
        }
        if (!power_of_two(d->alignment) || d->base % d->alignment != 0u)
            return fail(error, PL_RESOURCE_INVALID_ALIGNMENT, d->id, "DMA base alignment is invalid");
        if ((unsigned)d->direction > (unsigned)PL_DMA_BIDIRECTIONAL ||
            (unsigned)d->coherency > (unsigned)PL_DMA_COHERENCY_UNKNOWN ||
            (unsigned)d->owner > (unsigned)PL_RESOURCE_OWNER_SHARED ||
            (unsigned)d->state > (unsigned)PL_DMA_RELEASED)
            return fail(error, PL_RESOURCE_INVALID_DMA_STATE, d->id, "DMA direction, coherency, owner or state is invalid");
        if (d->owner == PL_RESOURCE_OWNER_CPU && d->state == PL_DMA_DEVICE_OWNED)
            return fail(error, PL_RESOURCE_INVALID_DMA_STATE, d->id, "CPU-owned DMA buffer cannot be device-owned");
        if (d->owner == PL_RESOURCE_OWNER_DEVICE &&
            (d->state == PL_DMA_CPU_OWNED || d->state == PL_DMA_COMPLETED_FOR_CPU))
            return fail(error, PL_RESOURCE_INVALID_DMA_STATE, d->id, "device-owned buffer cannot be in a CPU ownership state");
    }
    return PL_RESOURCE_OK;
}

pl_resource_status pl_dma_transition_validate(
    pl_dma_owner_state current, pl_dma_owner_state next,
    uint32_t current_generation, uint32_t observed_generation
) {
    if (current_generation == 0u || current_generation != observed_generation)
        return PL_RESOURCE_STALE_GENERATION;
    if (current > PL_DMA_RELEASED || next > PL_DMA_RELEASED)
        return PL_RESOURCE_INVALID_DMA_STATE;
    switch (current) {
    case PL_DMA_CPU_OWNED:
        return (next == PL_DMA_PREPARED_FOR_DEVICE ||
                next == PL_DMA_RELEASED) ? PL_RESOURCE_OK : PL_RESOURCE_INVALID_DMA_STATE;
    case PL_DMA_PREPARED_FOR_DEVICE:
        return (next == PL_DMA_DEVICE_OWNED ||
                next == PL_DMA_CPU_OWNED) ? PL_RESOURCE_OK : PL_RESOURCE_INVALID_DMA_STATE;
    case PL_DMA_DEVICE_OWNED:
        return next == PL_DMA_COMPLETED_FOR_CPU
            ? PL_RESOURCE_OK : PL_RESOURCE_INVALID_DMA_STATE;
    case PL_DMA_COMPLETED_FOR_CPU:
        return (next == PL_DMA_CPU_OWNED ||
                next == PL_DMA_RELEASED) ? PL_RESOURCE_OK : PL_RESOURCE_INVALID_DMA_STATE;
    case PL_DMA_RELEASED:
    default:
        return PL_RESOURCE_INVALID_DMA_STATE;
    }
}

const char *pl_resource_status_name(pl_resource_status status) {
    switch (status) {
    case PL_RESOURCE_OK: return "ok";
    case PL_RESOURCE_INVALID_ARGUMENT: return "invalid-argument";
    case PL_RESOURCE_LIMIT_EXCEEDED: return "limit-exceeded";
    case PL_RESOURCE_DUPLICATE_ID: return "duplicate-id";
    case PL_RESOURCE_MISSING_REFERENCE: return "missing-reference";
    case PL_RESOURCE_INVALID_RANGE: return "invalid-range";
    case PL_RESOURCE_INVALID_ALIGNMENT: return "invalid-alignment";
    case PL_RESOURCE_INVALID_OWNER: return "invalid-owner";
    case PL_RESOURCE_INVALID_REGISTER: return "invalid-register";
    case PL_RESOURCE_INVALID_DMA_STATE: return "invalid-dma-state";
    case PL_RESOURCE_STALE_GENERATION: return "stale-generation";
    }
    return "unknown";
}
