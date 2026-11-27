#include "low_level_contract.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned char ram[256];
    unsigned char mmio[256];
    unsigned fence_count;
    unsigned audit_count;
    uint64_t last_clock;
    pl_ll_power_state last_power;
    int reject_clock;
    int reject_power;
} fake_hardware;

static unsigned char *select_region(fake_hardware *fake, uint64_t address,
                                    uint8_t width_bytes) {
    if (address >= UINT64_C(0x1000) &&
        address - UINT64_C(0x1000) <= sizeof(fake->ram) - width_bytes)
        return fake->ram + (size_t)(address - UINT64_C(0x1000));
    if (address >= UINT64_C(0x4000) &&
        address - UINT64_C(0x4000) <= sizeof(fake->mmio) - width_bytes)
        return fake->mmio + (size_t)(address - UINT64_C(0x4000));
    return NULL;
}

static int fake_read(void *context, uint64_t address, uint8_t width_bytes,
                     uint64_t *out_value) {
    fake_hardware *fake = (fake_hardware *)context;
    unsigned char *bytes = select_region(fake, address, width_bytes);
    uint64_t value = 0u;
    unsigned index;
    if (bytes == NULL || out_value == NULL) return -1;
    for (index = 0u; index < width_bytes; ++index)
        value |= (uint64_t)bytes[index] << (index * 8u);
    *out_value = value;
    return 0;
}

static int fake_write(void *context, uint64_t address, uint8_t width_bytes,
                      uint64_t value) {
    fake_hardware *fake = (fake_hardware *)context;
    unsigned char *bytes = select_region(fake, address, width_bytes);
    unsigned index;
    if (bytes == NULL) return -1;
    for (index = 0u; index < width_bytes; ++index)
        bytes[index] = (unsigned char)(value >> (index * 8u));
    return 0;
}

static int fake_fence(void *context) {
    fake_hardware *fake = (fake_hardware *)context;
    ++fake->fence_count;
    return 0;
}

static int fake_clock(void *context, uint64_t frequency_hz) {
    fake_hardware *fake = (fake_hardware *)context;
    if (fake->reject_clock) return -1;
    fake->last_clock = frequency_hz;
    return 0;
}

static int fake_power(void *context, pl_ll_power_state state) {
    fake_hardware *fake = (fake_hardware *)context;
    if (fake->reject_power) return -1;
    fake->last_power = state;
    return 0;
}

static void audit(void *context, pl_ll_audit_operation operation,
                  uint64_t address, uint8_t width_bytes, pl_ll_status result) {
    fake_hardware *fake = (fake_hardware *)context;
    (void)operation;
    (void)address;
    (void)width_bytes;
    (void)result;
    ++fake->audit_count;
}

static pl_ll_irq_result irq_handler(uint32_t vector, void *context) {
    uint32_t *last_vector = (uint32_t *)context;
    *last_vector = vector;
    return PL_LL_IRQ_HANDLED;
}

static void test_target_and_memory_map(void) {
    pl_ll_target target;
    pl_ll_memory_map map;
    pl_ll_memory_region region;
    memset(&target, 0, sizeof(target));
    target.abi_version = PL_LL_ABI_VERSION;
    memcpy(target.id, "test-x64", sizeof("test-x64"));
    target.address_bits = 32u;
    target.word_bits = 64u;
    target.byte_order = PL_LL_LITTLE_ENDIAN;
    target.bus_width_bits = 64u;
    target.cache_line_bytes = 64u;
    target.cpu_count = 4u;
    target.clock_min_hz = UINT64_C(100000000);
    target.clock_max_hz = UINT64_C(4000000000);
    target.supported_power_states = PL_LL_POWER_MASK(PL_LL_POWER_IDLE) |
        PL_LL_POWER_MASK(PL_LL_POWER_PERFORMANCE) |
        PL_LL_POWER_MASK(PL_LL_POWER_THROTTLED);
    assert(pl_ll_target_validate(&target));
    target.word_bits = 24u;
    assert(!pl_ll_target_validate(&target));
    target.word_bits = 64u;
    assert(pl_ll_memory_map_init(&map, 32u) == PL_LL_OK);

    memset(&region, 0, sizeof(region));
    memcpy(region.name, "RAM", sizeof("RAM"));
    region.base = UINT64_C(0x1000);
    region.size = 256u;
    region.flags = PL_LL_REGION_READ | PL_LL_REGION_WRITE | PL_LL_REGION_CACHEABLE;
    region.access_alignment = 1u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OK);

    memcpy(region.name, "ROM", sizeof("ROM"));
    region.base = UINT64_C(0x2000);
    region.size = 256u;
    region.flags = PL_LL_REGION_READ | PL_LL_REGION_EXECUTE;
    region.access_alignment = 1u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OK);

    memcpy(region.name, "MMIO", sizeof("MMIO"));
    region.base = UINT64_C(0x4000);
    region.size = 256u;
    region.flags = PL_LL_REGION_READ | PL_LL_REGION_WRITE | PL_LL_REGION_DEVICE;
    region.access_alignment = 4u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OK);

    memcpy(region.name, "OVERLAP", sizeof("OVERLAP"));
    region.base = UINT64_C(0x40F0);
    region.size = 64u;
    region.flags = PL_LL_REGION_READ;
    region.access_alignment = 1u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OVERLAP);

    memcpy(region.name, "OUT_OF_RANGE", sizeof("OUT_OF_RANGE"));
    region.base = UINT64_C(0xFFFFFFF0);
    region.size = 64u;
    region.flags = PL_LL_REGION_READ;
    region.access_alignment = 1u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OUT_OF_RANGE);
    assert(pl_ll_memory_map_find(&map, UINT64_C(0x1000), 4u,
                                 PL_LL_REGION_READ) != NULL);
    assert(pl_ll_memory_map_find(&map, UINT64_C(0x1000), 257u,
                                 PL_LL_REGION_READ) == NULL);
    memcpy(region.name, "W_X_REJECTED", sizeof("W_X_REJECTED"));
    region.base = UINT64_C(0x6000);
    region.size = 64u;
    region.flags = PL_LL_REGION_READ | PL_LL_REGION_WRITE | PL_LL_REGION_EXECUTE;
    region.access_alignment = 1u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_INVALID_ARGUMENT);
}

static void test_bus_capabilities_and_mmio_fences(void) {
    pl_ll_target target;
    pl_ll_memory_map map;
    pl_ll_memory_region region;
    pl_ll_bus_backend backend;
    pl_ll_bus bus;
    pl_ll_bus read_only_bus;
    fake_hardware fake;
    uint64_t value = 0u;
    memset(&target, 0, sizeof(target));
    target.abi_version = PL_LL_ABI_VERSION;
    memcpy(target.id, "bus-test", sizeof("bus-test"));
    target.address_bits = 32u;
    target.word_bits = 32u;
    target.byte_order = PL_LL_LITTLE_ENDIAN;
    target.bus_width_bits = 32u;
    target.cpu_count = 1u;
    target.clock_min_hz = 100u;
    target.clock_max_hz = 1000u;
    target.supported_power_states = PL_LL_POWER_MASK(PL_LL_POWER_IDLE) |
        PL_LL_POWER_MASK(PL_LL_POWER_THROTTLED);
    assert(pl_ll_memory_map_init(&map, 32u) == PL_LL_OK);
    memset(&region, 0, sizeof(region));
    memcpy(region.name, "RAM", sizeof("RAM"));
    region.base = UINT64_C(0x1000);
    region.size = 256u;
    region.flags = PL_LL_REGION_READ | PL_LL_REGION_WRITE;
    region.access_alignment = 1u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OK);
    memcpy(region.name, "ROM", sizeof("ROM"));
    region.base = UINT64_C(0x2000);
    region.size = 256u;
    region.flags = PL_LL_REGION_READ | PL_LL_REGION_EXECUTE;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OK);
    memcpy(region.name, "MMIO", sizeof("MMIO"));
    region.base = UINT64_C(0x4000);
    region.size = 256u;
    region.flags = PL_LL_REGION_READ | PL_LL_REGION_WRITE | PL_LL_REGION_DEVICE;
    region.access_alignment = 4u;
    assert(pl_ll_memory_map_add(&map, &region) == PL_LL_OK);

    memset(&fake, 0, sizeof(fake));
    memset(&backend, 0, sizeof(backend));
    backend.context = &fake;
    backend.read = fake_read;
    backend.write = fake_write;
    backend.fence = fake_fence;
    backend.request_clock_hz = fake_clock;
    backend.request_power_state = fake_power;
    assert(pl_ll_bus_init(&bus, &target, &map,
        PL_LL_CAP_MEMORY_READ | PL_LL_CAP_MEMORY_WRITE |
        PL_LL_CAP_MMIO_READ | PL_LL_CAP_MMIO_WRITE |
        PL_LL_CAP_CLOCK_REQUEST | PL_LL_CAP_POWER_STATE_REQUEST,
        &backend, audit, &fake) == PL_LL_OK);

    assert(pl_ll_bus_write(&bus, UINT64_C(0x1000), 4u,
                           UINT64_C(0xAABBCCDD)) == PL_LL_OK);
    assert(pl_ll_bus_read(&bus, UINT64_C(0x1000), 4u, &value) == PL_LL_OK);
    assert(value == UINT64_C(0xAABBCCDD));
    assert(pl_ll_bus_read(&bus, UINT64_C(0x1001), 4u, &value) == PL_LL_OUT_OF_RANGE);
    assert(pl_ll_bus_write(&bus, UINT64_C(0x2000), 4u, 1u) == PL_LL_PERMISSION_DENIED);
    assert(pl_ll_bus_read(&bus, UINT64_C(0x3000), 4u, &value) == PL_LL_PERMISSION_DENIED);

    assert(pl_ll_bus_write(&bus, UINT64_C(0x4000), 4u,
                           UINT64_C(0x12345678)) == PL_LL_OK);
    assert(fake.fence_count == 2u);
    assert(pl_ll_bus_read(&bus, UINT64_C(0x4000), 4u, &value) == PL_LL_OK);
    assert(value == UINT64_C(0x12345678));
    assert(fake.fence_count == 4u);
    assert(pl_ll_bus_write(&bus, UINT64_C(0x4001), 4u, 1u) == PL_LL_OUT_OF_RANGE);

    assert(pl_ll_bus_init(&read_only_bus, &target, &map,
        PL_LL_CAP_MEMORY_READ, &backend, NULL, NULL) == PL_LL_OK);
    assert(pl_ll_bus_write(&read_only_bus, UINT64_C(0x1000), 4u, 1u) ==
           PL_LL_CAPABILITY_DENIED);
    assert(pl_ll_bus_read(&read_only_bus, UINT64_C(0x4000), 4u, &value) ==
           PL_LL_CAPABILITY_DENIED);

    assert(pl_ll_bus_request_clock(&bus, 200u) == PL_LL_OK);
    assert(fake.last_clock == 200u);
    assert(pl_ll_bus_request_clock(&bus, 99u) == PL_LL_OUT_OF_RANGE);
    fake.reject_clock = 1;
    assert(pl_ll_bus_request_clock(&bus, 300u) == PL_LL_BACKEND_ERROR);
    assert(pl_ll_bus_request_power_state(&bus, PL_LL_POWER_IDLE) == PL_LL_OK);
    assert(fake.last_power == PL_LL_POWER_IDLE);
    assert(pl_ll_bus_request_power_state(&bus, PL_LL_POWER_SLEEP) == PL_LL_UNSUPPORTED);
    assert(fake.audit_count >= 13u);

    backend.fence = NULL;
    assert(pl_ll_bus_init(&bus, &target, &map,
        PL_LL_CAP_MMIO_READ | PL_LL_CAP_MMIO_WRITE,
        &backend, NULL, NULL) == PL_LL_OK);
    assert(pl_ll_bus_read(&bus, UINT64_C(0x4000), 4u, &value) == PL_LL_UNSUPPORTED);
}

static void test_interrupt_contract(void) {
    pl_ll_irq_registry registry;
    pl_ll_irq_result result = PL_LL_IRQ_UNHANDLED;
    uint32_t called_vector = 0u;
    pl_ll_irq_registry_init(&registry);
    assert(pl_ll_irq_register(&registry, 7u, PL_LL_IRQ_REQUIRED_FLAGS,
                              irq_handler, &called_vector) == PL_LL_OK);
    assert(pl_ll_irq_register(&registry, 7u, PL_LL_IRQ_REQUIRED_FLAGS,
                              irq_handler, &called_vector) == PL_LL_OVERLAP);
    assert(pl_ll_irq_register(&registry, 8u, PL_LL_IRQ_BOUNDED,
                              irq_handler, &called_vector) == PL_LL_INVALID_ARGUMENT);
    assert(pl_ll_irq_dispatch(&registry, 0u, 7u, &result) == PL_LL_CAPABILITY_DENIED);
    assert(pl_ll_irq_dispatch(&registry, PL_LL_CAP_IRQ_DISPATCH, 7u,
                              &result) == PL_LL_OK);
    assert(result == PL_LL_IRQ_HANDLED && called_vector == 7u);
    assert(pl_ll_irq_dispatch(&registry, PL_LL_CAP_IRQ_DISPATCH, 99u,
                              &result) == PL_LL_NOT_FOUND);
}

int main(void) {
    test_target_and_memory_map();
    test_bus_capabilities_and_mmio_fences();
    test_interrupt_contract();
    puts("Pytyliki low-level hardware contract tests passed");
    return 0;
}
