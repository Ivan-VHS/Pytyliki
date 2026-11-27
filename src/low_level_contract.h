#ifndef PYTYLIKI_LOW_LEVEL_CONTRACT_H
#define PYTYLIKI_LOW_LEVEL_CONTRACT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_LL_ABI_VERSION UINT32_C(0x00010000)
#define PL_LL_TARGET_ID_MAX 48u
#define PL_LL_REGION_NAME_MAX 40u
#define PL_LL_MAX_REGIONS 64u
#define PL_LL_MAX_IRQ_VECTORS 64u

typedef enum {
    PL_LL_OK = 0,
    PL_LL_INVALID_ARGUMENT,
    PL_LL_OUT_OF_RANGE,
    PL_LL_OVERLAP,
    PL_LL_PERMISSION_DENIED,
    PL_LL_CAPABILITY_DENIED,
    PL_LL_UNSUPPORTED,
    PL_LL_BACKEND_ERROR,
    PL_LL_CAPACITY_EXCEEDED,
    PL_LL_NOT_FOUND,
    PL_LL_INVALID_STATE
} pl_ll_status;

typedef enum {
    PL_LL_LITTLE_ENDIAN = 1,
    PL_LL_BIG_ENDIAN = 2
} pl_ll_byte_order;

typedef enum {
    PL_LL_POWER_IDLE = 1,
    PL_LL_POWER_PERFORMANCE = 2,
    PL_LL_POWER_THROTTLED = 3,
    PL_LL_POWER_SLEEP = 4,
    PL_LL_POWER_SHUTDOWN = 5
} pl_ll_power_state;

#define PL_LL_POWER_MASK(state) (UINT32_C(1) << ((unsigned)(state) - 1u))

enum {
    PL_LL_REGION_READ = UINT32_C(1) << 0,
    PL_LL_REGION_WRITE = UINT32_C(1) << 1,
    PL_LL_REGION_EXECUTE = UINT32_C(1) << 2,
    PL_LL_REGION_DEVICE = UINT32_C(1) << 3,
    PL_LL_REGION_DMA = UINT32_C(1) << 4,
    PL_LL_REGION_CACHEABLE = UINT32_C(1) << 5
};

enum {
    PL_LL_CAP_MEMORY_READ = UINT64_C(1) << 0,
    PL_LL_CAP_MEMORY_WRITE = UINT64_C(1) << 1,
    PL_LL_CAP_MMIO_READ = UINT64_C(1) << 2,
    PL_LL_CAP_MMIO_WRITE = UINT64_C(1) << 3,
    PL_LL_CAP_IRQ_DISPATCH = UINT64_C(1) << 4,
    PL_LL_CAP_CLOCK_REQUEST = UINT64_C(1) << 5,
    PL_LL_CAP_POWER_STATE_REQUEST = UINT64_C(1) << 6,
    PL_LL_CAP_DEBUG_READ = UINT64_C(1) << 7,
    PL_LL_CAP_DMA_SUBMIT = UINT64_C(1) << 8,
    PL_LL_CAP_CODE_PATCH = UINT64_C(1) << 9
};

enum {
    PL_LL_IRQ_BOUNDED = UINT32_C(1) << 0,
    PL_LL_IRQ_NO_BLOCK = UINT32_C(1) << 1,
    PL_LL_IRQ_NO_ALLOC = UINT32_C(1) << 2,
    PL_LL_IRQ_REQUIRED_FLAGS = PL_LL_IRQ_BOUNDED | PL_LL_IRQ_NO_BLOCK |
                               PL_LL_IRQ_NO_ALLOC
};

typedef struct {
    uint32_t abi_version;
    char id[PL_LL_TARGET_ID_MAX];
    uint8_t address_bits;
    uint8_t word_bits;
    uint8_t byte_order;
    uint8_t reserved;
    uint16_t bus_width_bits;
    uint16_t reserved2;
    uint32_t cache_line_bytes;
    uint32_t cpu_count;
    uint64_t clock_min_hz;
    uint64_t clock_max_hz;
    uint32_t supported_power_states;
} pl_ll_target;

typedef struct {
    uint64_t base;
    uint64_t size;
    uint32_t flags;
    uint32_t access_alignment;
    char name[PL_LL_REGION_NAME_MAX];
} pl_ll_memory_region;

typedef struct {
    uint8_t address_bits;
    size_t count;
    pl_ll_memory_region regions[PL_LL_MAX_REGIONS];
} pl_ll_memory_map;

typedef enum {
    PL_LL_BUS_READ = 1,
    PL_LL_BUS_WRITE = 2,
    PL_LL_BUS_CLOCK_REQUEST = 3,
    PL_LL_BUS_POWER_REQUEST = 4,
    PL_LL_BUS_IRQ_DISPATCH = 5
} pl_ll_audit_operation;

typedef struct {
    void *context;
    /* Values are width-limited integers in the target's declared byte order. */
    int (*read)(void *context, uint64_t address, uint8_t width_bytes,
                uint64_t *out_value);
    int (*write)(void *context, uint64_t address, uint8_t width_bytes,
                 uint64_t value);
    int (*fence)(void *context);
    int (*request_clock_hz)(void *context, uint64_t frequency_hz);
    int (*request_power_state)(void *context, pl_ll_power_state state);
} pl_ll_bus_backend;

typedef void (*pl_ll_audit_fn)(void *context, pl_ll_audit_operation operation,
                               uint64_t address, uint8_t width_bytes,
                               pl_ll_status result);

typedef struct {
    const pl_ll_target *target;
    const pl_ll_memory_map *memory_map;
    pl_ll_bus_backend backend;
    uint64_t capabilities;
    pl_ll_audit_fn audit;
    void *audit_context;
} pl_ll_bus;

typedef enum {
    PL_LL_IRQ_UNHANDLED = 0,
    PL_LL_IRQ_HANDLED = 1,
    PL_LL_IRQ_DEFERRED = 2
} pl_ll_irq_result;

typedef pl_ll_irq_result (*pl_ll_irq_handler)(uint32_t vector, void *context);

typedef struct {
    uint32_t vector;
    uint32_t flags;
    pl_ll_irq_handler handler;
    void *context;
} pl_ll_irq_entry;

typedef struct {
    size_t count;
    pl_ll_irq_entry entries[PL_LL_MAX_IRQ_VECTORS];
} pl_ll_irq_registry;

int pl_ll_target_validate(const pl_ll_target *target);
const char *pl_ll_status_string(pl_ll_status status);

pl_ll_status pl_ll_memory_map_init(pl_ll_memory_map *memory_map,
                                   uint8_t address_bits);
pl_ll_status pl_ll_memory_map_add(pl_ll_memory_map *memory_map,
                                  const pl_ll_memory_region *region);
const pl_ll_memory_region *pl_ll_memory_map_find(
    const pl_ll_memory_map *memory_map, uint64_t address, uint64_t size,
    uint32_t required_flags);

pl_ll_status pl_ll_bus_init(pl_ll_bus *bus, const pl_ll_target *target,
                            const pl_ll_memory_map *memory_map,
                            uint64_t capabilities,
                            const pl_ll_bus_backend *backend,
                            pl_ll_audit_fn audit, void *audit_context);
pl_ll_status pl_ll_bus_read(pl_ll_bus *bus, uint64_t address,
                            uint8_t width_bytes, uint64_t *out_value);
pl_ll_status pl_ll_bus_write(pl_ll_bus *bus, uint64_t address,
                             uint8_t width_bytes, uint64_t value);
pl_ll_status pl_ll_bus_request_clock(pl_ll_bus *bus, uint64_t frequency_hz);
pl_ll_status pl_ll_bus_request_power_state(pl_ll_bus *bus,
                                           pl_ll_power_state state);

void pl_ll_irq_registry_init(pl_ll_irq_registry *registry);
pl_ll_status pl_ll_irq_register(pl_ll_irq_registry *registry, uint32_t vector,
                                uint32_t flags, pl_ll_irq_handler handler,
                                void *context);
pl_ll_status pl_ll_irq_dispatch(const pl_ll_irq_registry *registry,
                                uint64_t capabilities, uint32_t vector,
                                pl_ll_irq_result *out_result);

#ifdef __cplusplus
}
#endif
#endif
