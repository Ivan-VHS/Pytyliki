#include "low_level_contract.h"

#include <stdatomic.h>
#include <string.h>

#define PL_LL_REGION_KNOWN_FLAGS (PL_LL_REGION_READ | PL_LL_REGION_WRITE | \
                                  PL_LL_REGION_EXECUTE | PL_LL_REGION_DEVICE | \
                                  PL_LL_REGION_DMA | PL_LL_REGION_CACHEABLE)
#define PL_LL_CAP_KNOWN_FLAGS (PL_LL_CAP_MEMORY_READ | PL_LL_CAP_MEMORY_WRITE | \
                               PL_LL_CAP_MMIO_READ | PL_LL_CAP_MMIO_WRITE | \
                               PL_LL_CAP_IRQ_DISPATCH | PL_LL_CAP_CLOCK_REQUEST | \
                               PL_LL_CAP_POWER_STATE_REQUEST | PL_LL_CAP_DEBUG_READ | \
                               PL_LL_CAP_DMA_SUBMIT | PL_LL_CAP_CODE_PATCH)

static int is_power_of_two_u32(uint32_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}

static int range_end(uint64_t base, uint64_t size, uint64_t *end) {
    if (size == 0u || end == NULL || size > UINT64_MAX - base) return 0;
    *end = base + size;
    return 1;
}

static int width_valid(uint8_t width_bytes) {
    return width_bytes == 1u || width_bytes == 2u ||
           width_bytes == 4u || width_bytes == 8u;
}

static uint64_t width_mask(uint8_t width_bytes) {
    if (width_bytes == 8u) return UINT64_MAX;
    return (UINT64_C(1) << (width_bytes * 8u)) - 1u;
}

int pl_ll_target_validate(const pl_ll_target *target) {
    size_t id_length;
    if (target == NULL || target->abi_version != PL_LL_ABI_VERSION) return 0;
    {
        const char *terminator = (const char *)memchr(target->id, '\0',
                                                       sizeof(target->id));
        if (terminator == NULL) return 0;
        id_length = (size_t)(terminator - target->id);
    }
    if (id_length == 0u) return 0;
    if (target->address_bits == 0u || target->address_bits > 64u) return 0;
    if (target->word_bits != 8u && target->word_bits != 16u &&
        target->word_bits != 32u && target->word_bits != 64u) return 0;
    if (target->byte_order != PL_LL_LITTLE_ENDIAN &&
        target->byte_order != PL_LL_BIG_ENDIAN) return 0;
    if (target->bus_width_bits < 8u || target->bus_width_bits > 512u ||
        (target->bus_width_bits % 8u) != 0u) return 0;
    if (target->cache_line_bytes != 0u &&
        !is_power_of_two_u32(target->cache_line_bytes)) return 0;
    if (target->cpu_count == 0u || target->clock_min_hz == 0u ||
        target->clock_max_hz < target->clock_min_hz) return 0;
    if ((target->supported_power_states &
         ~(PL_LL_POWER_MASK(PL_LL_POWER_IDLE) |
           PL_LL_POWER_MASK(PL_LL_POWER_PERFORMANCE) |
           PL_LL_POWER_MASK(PL_LL_POWER_THROTTLED) |
           PL_LL_POWER_MASK(PL_LL_POWER_SLEEP) |
           PL_LL_POWER_MASK(PL_LL_POWER_SHUTDOWN))) != 0u) return 0;
    return 1;
}

const char *pl_ll_status_string(pl_ll_status status) {
    static const char *const names[] = {
        "ok", "invalid-argument", "out-of-range", "overlap",
        "permission-denied", "capability-denied", "unsupported",
        "backend-error", "capacity-exceeded", "not-found", "invalid-state"
    };
    return (unsigned)status < (sizeof(names) / sizeof(names[0]))
        ? names[(unsigned)status] : "unknown";
}

pl_ll_status pl_ll_memory_map_init(pl_ll_memory_map *memory_map,
                                   uint8_t address_bits) {
    if (memory_map == NULL || address_bits == 0u || address_bits > 64u)
        return PL_LL_INVALID_ARGUMENT;
    memset(memory_map, 0, sizeof(*memory_map));
    memory_map->address_bits = address_bits;
    return PL_LL_OK;
}

pl_ll_status pl_ll_memory_map_add(pl_ll_memory_map *memory_map,
                                  const pl_ll_memory_region *region) {
    uint64_t new_end;
    size_t name_length;
    size_t index;
    if (memory_map == NULL || region == NULL ||
        memory_map->address_bits == 0u || memory_map->address_bits > 64u ||
        memory_map->count > PL_LL_MAX_REGIONS)
        return PL_LL_INVALID_ARGUMENT;
    {
        const char *terminator = (const char *)memchr(region->name, '\0',
                                                       sizeof(region->name));
        if (terminator == NULL) return PL_LL_INVALID_ARGUMENT;
        name_length = (size_t)(terminator - region->name);
    }
    if (name_length == 0u ||
        region->flags == 0u || (region->flags & ~PL_LL_REGION_KNOWN_FLAGS) != 0u ||
        (region->flags & (PL_LL_REGION_READ | PL_LL_REGION_WRITE |
                          PL_LL_REGION_EXECUTE)) == 0u ||
        !is_power_of_two_u32(region->access_alignment) ||
        (region->base % region->access_alignment) != 0u ||
        ((region->flags & PL_LL_REGION_DEVICE) != 0u &&
         (region->flags & (PL_LL_REGION_EXECUTE | PL_LL_REGION_CACHEABLE)) != 0u) ||
        ((region->flags & (PL_LL_REGION_WRITE | PL_LL_REGION_EXECUTE)) ==
         (PL_LL_REGION_WRITE | PL_LL_REGION_EXECUTE)) ||
        ((region->flags & PL_LL_REGION_DMA) != 0u &&
         (region->flags & (PL_LL_REGION_READ | PL_LL_REGION_WRITE)) == 0u))
        return PL_LL_INVALID_ARGUMENT;
    if (!range_end(region->base, region->size, &new_end))
        return PL_LL_OUT_OF_RANGE;
    if (memory_map->address_bits < 64u) {
        uint64_t address_limit = UINT64_C(1) << memory_map->address_bits;
        if (new_end > address_limit) return PL_LL_OUT_OF_RANGE;
    }
    if (memory_map->count >= PL_LL_MAX_REGIONS)
        return PL_LL_CAPACITY_EXCEEDED;
    for (index = 0u; index < memory_map->count; ++index) {
        const pl_ll_memory_region *existing = &memory_map->regions[index];
        uint64_t existing_end;
        if (!range_end(existing->base, existing->size, &existing_end))
            return PL_LL_INVALID_STATE;
        if (region->base < existing_end && existing->base < new_end)
            return PL_LL_OVERLAP;
    }
    memory_map->regions[memory_map->count++] = *region;
    return PL_LL_OK;
}

const pl_ll_memory_region *pl_ll_memory_map_find(
    const pl_ll_memory_map *memory_map, uint64_t address, uint64_t size,
    uint32_t required_flags) {
    size_t index;
    if (memory_map == NULL || size == 0u || required_flags == 0u ||
        memory_map->count > PL_LL_MAX_REGIONS) return NULL;
    for (index = 0u; index < memory_map->count; ++index) {
        const pl_ll_memory_region *region = &memory_map->regions[index];
        uint64_t offset;
        if (address < region->base) continue;
        offset = address - region->base;
        if (region->access_alignment != 0u &&
            is_power_of_two_u32(region->access_alignment) &&
            offset <= region->size && size <= region->size - offset &&
            (region->flags & required_flags) == required_flags &&
            (address % region->access_alignment) == 0u)
            return region;
    }
    return NULL;
}

static void audit_result(pl_ll_bus *bus, pl_ll_audit_operation operation,
                         uint64_t address, uint8_t width_bytes,
                         pl_ll_status result) {
    if (bus != NULL && bus->audit != NULL)
        bus->audit(bus->audit_context, operation, address, width_bytes, result);
}

static pl_ll_status bus_check(pl_ll_bus *bus, uint64_t address,
                              uint8_t width_bytes, int write,
                              const pl_ll_memory_region **out_region) {
    const pl_ll_memory_region *region;
    uint32_t required_region_flags;
    uint64_t required_capability;
    uint64_t max_access_bytes;
    if (bus == NULL || out_region == NULL || !pl_ll_target_validate(bus->target) ||
        bus->memory_map == NULL || bus->memory_map->address_bits != bus->target->address_bits ||
        !width_valid(width_bytes)) return PL_LL_INVALID_ARGUMENT;
    max_access_bytes = (uint64_t)(bus->target->bus_width_bits / 8u);
    if ((uint64_t)width_bytes > max_access_bytes ||
        (address % width_bytes) != 0u) return PL_LL_OUT_OF_RANGE;

    required_region_flags = write ? PL_LL_REGION_WRITE : PL_LL_REGION_READ;
    region = pl_ll_memory_map_find(bus->memory_map, address, width_bytes,
                                   required_region_flags);
    if (region == NULL) return PL_LL_PERMISSION_DENIED;
    if ((region->flags & PL_LL_REGION_DEVICE) != 0u) {
        required_capability = write ? PL_LL_CAP_MMIO_WRITE : PL_LL_CAP_MMIO_READ;
    } else {
        required_capability = write ? PL_LL_CAP_MEMORY_WRITE : PL_LL_CAP_MEMORY_READ;
    }
    if ((bus->capabilities & required_capability) == 0u)
        return PL_LL_CAPABILITY_DENIED;
    *out_region = region;
    return PL_LL_OK;
}

pl_ll_status pl_ll_bus_init(pl_ll_bus *bus, const pl_ll_target *target,
                            const pl_ll_memory_map *memory_map,
                            uint64_t capabilities,
                            const pl_ll_bus_backend *backend,
                            pl_ll_audit_fn audit, void *audit_context) {
    if (bus == NULL || !pl_ll_target_validate(target) || memory_map == NULL ||
        memory_map->address_bits != target->address_bits ||
        memory_map->count > PL_LL_MAX_REGIONS || backend == NULL ||
        backend->read == NULL || (capabilities & ~PL_LL_CAP_KNOWN_FLAGS) != 0u)
        return PL_LL_INVALID_ARGUMENT;
    memset(bus, 0, sizeof(*bus));
    bus->target = target;
    bus->memory_map = memory_map;
    bus->backend = *backend;
    bus->capabilities = capabilities;
    bus->audit = audit;
    bus->audit_context = audit_context;
    return PL_LL_OK;
}

pl_ll_status pl_ll_bus_read(pl_ll_bus *bus, uint64_t address,
                            uint8_t width_bytes, uint64_t *out_value) {
    const pl_ll_memory_region *region = NULL;
    pl_ll_status status;
    uint64_t value = 0u;
    if (out_value == NULL) return PL_LL_INVALID_ARGUMENT;
    status = bus_check(bus, address, width_bytes, 0, &region);
    if (status == PL_LL_OK && (region->flags & PL_LL_REGION_DEVICE) != 0u) {
        if (bus->backend.fence == NULL)
            status = PL_LL_UNSUPPORTED;
        else if (bus->backend.fence(bus->backend.context) != 0)
            status = PL_LL_BACKEND_ERROR;
    }
    if (status == PL_LL_OK) {
        if (bus->backend.read(bus->backend.context, address,
                              width_bytes, &value) != 0)
            status = PL_LL_BACKEND_ERROR;
        else if ((region->flags & PL_LL_REGION_DEVICE) == 0u)
            atomic_thread_fence(memory_order_acquire);
    }
    if (status == PL_LL_OK && (region->flags & PL_LL_REGION_DEVICE) != 0u &&
        bus->backend.fence(bus->backend.context) != 0)
        status = PL_LL_BACKEND_ERROR;
    if (status == PL_LL_OK) *out_value = value & width_mask(width_bytes);
    audit_result(bus, PL_LL_BUS_READ, address, width_bytes, status);
    return status;
}

pl_ll_status pl_ll_bus_write(pl_ll_bus *bus, uint64_t address,
                             uint8_t width_bytes, uint64_t value) {
    const pl_ll_memory_region *region = NULL;
    pl_ll_status status = bus_check(bus, address, width_bytes, 1, &region);
    if (status == PL_LL_OK && bus->backend.write == NULL)
        status = PL_LL_UNSUPPORTED;
    if (status == PL_LL_OK && (region->flags & PL_LL_REGION_DEVICE) != 0u) {
        if (bus->backend.fence == NULL)
            status = PL_LL_UNSUPPORTED;
        else if (bus->backend.fence(bus->backend.context) != 0)
            status = PL_LL_BACKEND_ERROR;
    }
    if (status == PL_LL_OK) {
        if ((region->flags & PL_LL_REGION_DEVICE) == 0u)
            atomic_thread_fence(memory_order_release);
        if (bus->backend.write(bus->backend.context, address, width_bytes,
                               value & width_mask(width_bytes)) != 0)
            status = PL_LL_BACKEND_ERROR;
    }
    if (status == PL_LL_OK && (region->flags & PL_LL_REGION_DEVICE) != 0u &&
        bus->backend.fence(bus->backend.context) != 0)
        status = PL_LL_BACKEND_ERROR;
    audit_result(bus, PL_LL_BUS_WRITE, address, width_bytes, status);
    return status;
}

pl_ll_status pl_ll_bus_request_clock(pl_ll_bus *bus, uint64_t frequency_hz) {
    pl_ll_status status = PL_LL_OK;
    if (bus == NULL || !pl_ll_target_validate(bus->target)) {
        status = PL_LL_INVALID_ARGUMENT;
    } else if ((bus->capabilities & PL_LL_CAP_CLOCK_REQUEST) == 0u) {
        status = PL_LL_CAPABILITY_DENIED;
    } else if (frequency_hz < bus->target->clock_min_hz ||
               frequency_hz > bus->target->clock_max_hz) {
        status = PL_LL_OUT_OF_RANGE;
    } else if (bus->backend.request_clock_hz == NULL) {
        status = PL_LL_UNSUPPORTED;
    } else if (bus->backend.request_clock_hz(bus->backend.context,
                                             frequency_hz) != 0) {
        status = PL_LL_BACKEND_ERROR;
    }
    audit_result(bus, PL_LL_BUS_CLOCK_REQUEST, 0u, 0u, status);
    return status;
}

pl_ll_status pl_ll_bus_request_power_state(pl_ll_bus *bus,
                                           pl_ll_power_state state) {
    pl_ll_status status = PL_LL_OK;
    uint32_t state_mask;
    if (bus == NULL || !pl_ll_target_validate(bus->target) ||
        state < PL_LL_POWER_IDLE || state > PL_LL_POWER_SHUTDOWN) {
        status = PL_LL_INVALID_ARGUMENT;
    } else {
        state_mask = PL_LL_POWER_MASK(state);
        if ((bus->capabilities & PL_LL_CAP_POWER_STATE_REQUEST) == 0u)
            status = PL_LL_CAPABILITY_DENIED;
        else if ((bus->target->supported_power_states & state_mask) == 0u)
            status = PL_LL_UNSUPPORTED;
        else if (bus->backend.request_power_state == NULL)
            status = PL_LL_UNSUPPORTED;
        else if (bus->backend.request_power_state(bus->backend.context,
                                                  state) != 0)
            status = PL_LL_BACKEND_ERROR;
    }
    audit_result(bus, PL_LL_BUS_POWER_REQUEST, 0u, 0u, status);
    return status;
}

void pl_ll_irq_registry_init(pl_ll_irq_registry *registry) {
    if (registry != NULL) memset(registry, 0, sizeof(*registry));
}

pl_ll_status pl_ll_irq_register(pl_ll_irq_registry *registry, uint32_t vector,
                                uint32_t flags, pl_ll_irq_handler handler,
                                void *context) {
    size_t index;
    if (registry == NULL || handler == NULL ||
        registry->count > PL_LL_MAX_IRQ_VECTORS ||
        (flags & PL_LL_IRQ_REQUIRED_FLAGS) != PL_LL_IRQ_REQUIRED_FLAGS ||
        (flags & ~PL_LL_IRQ_REQUIRED_FLAGS) != 0u)
        return PL_LL_INVALID_ARGUMENT;
    for (index = 0u; index < registry->count; ++index) {
        if (registry->entries[index].vector == vector) return PL_LL_OVERLAP;
    }
    if (registry->count >= PL_LL_MAX_IRQ_VECTORS)
        return PL_LL_CAPACITY_EXCEEDED;
    registry->entries[registry->count].vector = vector;
    registry->entries[registry->count].flags = flags;
    registry->entries[registry->count].handler = handler;
    registry->entries[registry->count].context = context;
    ++registry->count;
    return PL_LL_OK;
}

pl_ll_status pl_ll_irq_dispatch(const pl_ll_irq_registry *registry,
                                uint64_t capabilities, uint32_t vector,
                                pl_ll_irq_result *out_result) {
    size_t index;
    if (registry == NULL || out_result == NULL ||
        registry->count > PL_LL_MAX_IRQ_VECTORS)
        return PL_LL_INVALID_ARGUMENT;
    if ((capabilities & PL_LL_CAP_IRQ_DISPATCH) == 0u)
        return PL_LL_CAPABILITY_DENIED;
    for (index = 0u; index < registry->count; ++index) {
        const pl_ll_irq_entry *entry = &registry->entries[index];
        if (entry->vector == vector) {
            if (entry->handler == NULL ||
                (entry->flags & PL_LL_IRQ_REQUIRED_FLAGS) != PL_LL_IRQ_REQUIRED_FLAGS)
                return PL_LL_INVALID_STATE;
            *out_result = entry->handler(vector, entry->context);
            if (*out_result < PL_LL_IRQ_UNHANDLED ||
                *out_result > PL_LL_IRQ_DEFERRED)
                return PL_LL_INVALID_STATE;
            return PL_LL_OK;
        }
    }
    return PL_LL_NOT_FOUND;
}
