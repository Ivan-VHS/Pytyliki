#ifndef PYTYLIKI_HARDWARE_RESOURCE_H
#define PYTYLIKI_HARDWARE_RESOURCE_H

#include <stddef.h>
#include <stdint.h>

#define PL_RESOURCE_MAX_SPACES 128u
#define PL_RESOURCE_MAX_REGIONS 512u
#define PL_RESOURCE_MAX_BANKS 128u
#define PL_RESOURCE_MAX_REGISTERS 2048u
#define PL_RESOURCE_MAX_DMA_BUFFERS 512u

typedef enum {
    PL_RESOURCE_SPACE_VIRTUAL = 0, PL_RESOURCE_SPACE_PHYSICAL,
    PL_RESOURCE_SPACE_IO_PORT, PL_RESOURCE_SPACE_DEVICE_BAR,
    PL_RESOURCE_SPACE_DMA_IOVA
} pl_resource_space_kind;
typedef enum {
    PL_RESOURCE_OWNER_CPU = 0, PL_RESOURCE_OWNER_DEVICE,
    PL_RESOURCE_OWNER_SHARED, PL_RESOURCE_OWNER_UNOWNED
} pl_resource_owner;
typedef enum {
    PL_RESOURCE_ACCESS_READ_ONLY = 0, PL_RESOURCE_ACCESS_WRITE_ONLY,
    PL_RESOURCE_ACCESS_READ_WRITE, PL_RESOURCE_ACCESS_WRITE_ONE_TO_CLEAR,
    PL_RESOURCE_ACCESS_WRITE_ONE_TO_SET, PL_RESOURCE_ACCESS_READ_CLEAR
} pl_resource_register_access;
typedef enum {
    PL_DMA_TO_DEVICE = 0, PL_DMA_FROM_DEVICE, PL_DMA_BIDIRECTIONAL
} pl_dma_direction;
typedef enum {
    PL_DMA_COHERENT = 0, PL_DMA_NON_COHERENT_EXPLICIT_SYNC,
    PL_DMA_COHERENCY_UNKNOWN
} pl_dma_coherency;
typedef enum {
    PL_DMA_CPU_OWNED = 0, PL_DMA_PREPARED_FOR_DEVICE,
    PL_DMA_DEVICE_OWNED, PL_DMA_COMPLETED_FOR_CPU, PL_DMA_RELEASED
} pl_dma_owner_state;

typedef struct {
    uint32_t id;
    pl_resource_space_kind kind;
    uint8_t address_bits;
    pl_resource_owner owner;
    uint32_t device_id;
} pl_resource_address_space;

typedef struct {
    uint32_t id;
    uint32_t space_id;
    uint64_t base;
    uint64_t length;
    uint32_t alignment;
    uint32_t permissions;
    pl_resource_owner owner;
    uint32_t device_id;
} pl_resource_memory_region;

typedef struct {
    uint32_t id;
    uint32_t space_id;
    uint32_t region_id;
} pl_resource_register_bank;

typedef struct {
    uint32_t id;
    uint32_t bank_id;
    uint64_t offset;
    uint16_t width_bits;
    pl_resource_register_access access;
    uint32_t alignment;
    pl_resource_owner owner;
    uint8_t is_volatile;
    uint8_t has_side_effects;
} pl_resource_register;

typedef struct {
    uint32_t id;
    uint32_t device_id;
    uint32_t space_id;
    uint64_t base;
    uint64_t length;
    uint32_t alignment;
    pl_dma_direction direction;
    pl_dma_coherency coherency;
    pl_resource_owner owner;
    pl_dma_owner_state state;
    uint32_t generation;
} pl_resource_dma_buffer;

typedef struct {
    uint32_t generation;
    size_t space_count, region_count, bank_count, register_count, dma_count;
    const pl_resource_address_space *spaces;
    const pl_resource_memory_region *regions;
    const pl_resource_register_bank *banks;
    const pl_resource_register *registers;
    const pl_resource_dma_buffer *dma_buffers;
} pl_resource_model;

typedef enum {
    PL_RESOURCE_OK = 0, PL_RESOURCE_INVALID_ARGUMENT,
    PL_RESOURCE_LIMIT_EXCEEDED, PL_RESOURCE_DUPLICATE_ID,
    PL_RESOURCE_MISSING_REFERENCE, PL_RESOURCE_INVALID_RANGE,
    PL_RESOURCE_INVALID_ALIGNMENT, PL_RESOURCE_INVALID_OWNER,
    PL_RESOURCE_INVALID_REGISTER, PL_RESOURCE_INVALID_DMA_STATE,
    PL_RESOURCE_STALE_GENERATION
} pl_resource_status;

typedef struct {
    pl_resource_status status;
    uint32_t object_id;
    char message[128];
} pl_resource_error;

pl_resource_status pl_resource_model_validate(
    const pl_resource_model *model, pl_resource_error *error
);
pl_resource_status pl_resource_region_contains(
    const pl_resource_model *model, uint32_t region_id,
    uint64_t offset, uint64_t length, uint32_t alignment
);
pl_resource_status pl_dma_transition_validate(
    pl_dma_owner_state current, pl_dma_owner_state next,
    uint32_t current_generation, uint32_t observed_generation
);
const char *pl_resource_status_name(pl_resource_status status);

#endif
