#ifndef PYTYLIKI_HARDWARE_RESOURCES_H
#define PYTYLIKI_HARDWARE_RESOURCES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_HW_RESOURCE_MAX_SPACES 64u
#define PL_HW_RESOURCE_MAX_REGIONS 256u
#define PL_HW_RESOURCE_MAX_BANKS 64u
#define PL_HW_RESOURCE_MAX_REGISTERS 512u
#define PL_HW_RESOURCE_MAX_DMA_BUFFERS 256u
#define PL_HW_RESOURCE_MAX_DEVICES 128u
#define PL_HW_RESOURCE_SCHEMA "pytyliki.hardware-resource-contract.v1"

typedef enum { PL_HW_SPACE_VIRTUAL=0, PL_HW_SPACE_PHYSICAL, PL_HW_SPACE_IO_PORT,
               PL_HW_SPACE_DEVICE_BAR, PL_HW_SPACE_DMA_IOVA } pl_hw_address_space_kind;
typedef enum { PL_HW_OWNER_CPU=0, PL_HW_OWNER_DEVICE, PL_HW_OWNER_SHARED,
               PL_HW_OWNER_UNOWNED } pl_hw_resource_owner;
typedef enum { PL_HW_ACCESS_READ_ONLY=0, PL_HW_ACCESS_WRITE_ONLY,
               PL_HW_ACCESS_READ_WRITE, PL_HW_ACCESS_WRITE_ONE_CLEAR,
               PL_HW_ACCESS_WRITE_ONE_SET, PL_HW_ACCESS_READ_CLEAR } pl_hw_register_access;
typedef enum { PL_HW_SIDE_EFFECT_NONE=0, PL_HW_SIDE_EFFECT_ACKNOWLEDGE,
               PL_HW_SIDE_EFFECT_CLEAR_ON_READ, PL_HW_SIDE_EFFECT_TRIGGER,
               PL_HW_SIDE_EFFECT_DOORBELL, PL_HW_SIDE_EFFECT_RESET } pl_hw_register_side_effect;
typedef enum { PL_HW_CACHE_DEFAULT=0, PL_HW_CACHE_UNCACHED,
               PL_HW_CACHE_WRITE_COMBINING, PL_HW_CACHE_WRITE_THROUGH,
               PL_HW_CACHE_WRITE_BACK, PL_HW_CACHE_DEVICE } pl_hw_cache_policy;
typedef enum { PL_HW_DMA_TO_DEVICE=0, PL_HW_DMA_FROM_DEVICE,
               PL_HW_DMA_BIDIRECTIONAL } pl_hw_dma_direction;
typedef enum { PL_HW_DMA_COHERENCY_UNKNOWN=0, PL_HW_DMA_COHERENT,
               PL_HW_DMA_EXPLICIT_SYNC } pl_hw_dma_coherency;
typedef enum { PL_HW_DMA_CPU_OWNED=0, PL_HW_DMA_PREPARED_FOR_DEVICE,
               PL_HW_DMA_DEVICE_OWNED, PL_HW_DMA_COMPLETED_FOR_CPU,
               PL_HW_DMA_RELEASED } pl_hw_dma_state;
typedef enum { PL_HW_ACTOR_CPU=0, PL_HW_ACTOR_ADAPTER } pl_hw_actor;

typedef enum { PL_HW_RESOURCE_OK=0, PL_HW_RESOURCE_INVALID_ARGUMENT,
               PL_HW_RESOURCE_LIMIT_EXCEEDED, PL_HW_RESOURCE_DUPLICATE_ID,
               PL_HW_RESOURCE_MISSING_REFERENCE, PL_HW_RESOURCE_RANGE_OVERFLOW,
               PL_HW_RESOURCE_ALIGNMENT, PL_HW_RESOURCE_PERMISSION,
               PL_HW_RESOURCE_OVERLAP, PL_HW_RESOURCE_OWNERSHIP,
               PL_HW_RESOURCE_STALE_GENERATION } pl_hw_resource_status;

typedef struct {
    uint32_t id;
    pl_hw_address_space_kind kind;
    uint8_t address_bits;
    pl_hw_resource_owner owner;
    uint32_t device_id;
} pl_hw_address_space;

typedef struct {
    uint32_t id;
    uint32_t space_id;
    uint64_t base;
    uint64_t length;
    uint64_t alias_group;
    uint32_t alignment;
    uint8_t permissions; /* bit 0 read, bit 1 write, bit 2 execute */
    pl_hw_cache_policy cache_policy;
    pl_hw_resource_owner owner;
    uint32_t device_id;
} pl_hw_memory_region;

typedef struct {
    uint32_t id;
    uint32_t region_id;
} pl_hw_register_bank;

typedef struct {
    uint32_t id;
    uint32_t bank_id;
    uint64_t offset;
    uint8_t width_bits;
    pl_hw_register_access access;
    uint8_t is_volatile;
    uint32_t alignment;
    pl_hw_register_side_effect side_effect;
    pl_hw_resource_owner owner;
    uint64_t reserved_mask;
    uint64_t reset_value;
} pl_hw_register;

typedef struct {
    uint32_t id;
    uint32_t generation;
} pl_hw_device;

typedef struct {
    uint32_t id;
    uint32_t device_id;
    uint32_t space_id;
    uint64_t base;
    uint64_t length;
    uint32_t alignment;
    pl_hw_dma_direction direction;
    pl_hw_dma_coherency coherency;
    pl_hw_resource_owner owner;
    pl_hw_dma_state state;
    uint32_t generation;
} pl_hw_dma_buffer;

typedef struct {
    uint32_t generation;
    size_t space_count;
    size_t region_count;
    size_t bank_count;
    size_t register_count;
    size_t dma_buffer_count;
    const pl_hw_address_space *spaces;
    const pl_hw_memory_region *regions;
    const pl_hw_register_bank *banks;
    const pl_hw_register *registers;
    const pl_hw_dma_buffer *dma_buffers;
    size_t device_count;
    const pl_hw_device *devices;
} pl_hw_resource_model;

pl_hw_resource_status pl_hw_resource_validate(const pl_hw_resource_model *model);
pl_hw_resource_status pl_hw_dma_transition(
    pl_hw_dma_buffer *buffer, pl_hw_dma_state next, pl_hw_actor actor,
    uint32_t expected_generation, int capability_granted,
    int submission_accepted, int completion_observed);
const char *pl_hw_resource_status_name(pl_hw_resource_status status);

#ifdef __cplusplus
}
#endif
#endif
