#ifndef PYTYLIKI_PROCESSOR_SILICON_H
#define PYTYLIKI_PROCESSOR_SILICON_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PL_SILICON_API_VERSION 1u
#define PL_SILICON_TRACE_CAPACITY 64u

typedef enum {
    PL_SILICON_OK = 0,
    PL_SILICON_INVALID_ARGUMENT = -1,
    PL_SILICON_UNSUPPORTED = -2,
    PL_SILICON_OUT_OF_RANGE = -3,
    PL_SILICON_PERMISSION_DENIED = -4,
    PL_SILICON_IO_ERROR = -5,
    PL_SILICON_OVERFLOW = -6,
    PL_SILICON_FULL = -7,
    PL_SILICON_EMPTY = -8
} pl_silicon_status;

typedef enum {
    PL_SILICON_ARCH_UNKNOWN = 0,
    PL_SILICON_ARCH_ARM32,
    PL_SILICON_ARCH_ARM64,
    PL_SILICON_ARCH_RISCV32,
    PL_SILICON_ARCH_RISCV64,
    PL_SILICON_ARCH_X86,
    PL_SILICON_ARCH_X86_64
} pl_silicon_arch;

typedef enum {
    PL_SILICON_BARRIER_DATA = 0,
    PL_SILICON_BARRIER_COMPLETION,
    PL_SILICON_BARRIER_INSTRUCTION
} pl_silicon_barrier;

typedef enum {
    PL_SILICON_COUNTER_CUSTOM = 0,
    PL_SILICON_COUNTER_CORTEX_M_DWT,
    PL_SILICON_COUNTER_RISCV_CYCLE,
    PL_SILICON_COUNTER_X86_TSC
} pl_silicon_counter_kind;

typedef int (*pl_silicon_cycle_read_fn)(void *context, uint64_t *out_value);

typedef struct {
    pl_silicon_cycle_read_fn read;
    void *context;
    uint64_t frequency_hz;
    uint8_t counter_bits;
    pl_silicon_counter_kind kind;
} pl_silicon_cycle_source;

typedef struct {
    uint64_t previous;
    uint64_t total;
    uint64_t peak_delta;
    uint64_t samples;
    bool initialized;
} pl_silicon_cycle_accumulator;

typedef struct {
    volatile uint32_t *demcr;
    volatile uint32_t *dwt_ctrl;
    volatile uint32_t *cyccnt;
} pl_silicon_dwt_registers;

enum {
    PL_SILICON_WIDTH_8 = 1u << 0,
    PL_SILICON_WIDTH_16 = 1u << 1,
    PL_SILICON_WIDTH_32 = 1u << 2
};

enum {
    PL_SILICON_MMIO_READ = 1u << 0,
    PL_SILICON_MMIO_WRITE = 1u << 1
};

typedef struct {
    uintptr_t bus_base;
    volatile uint8_t *mapped_base;
    size_t size_bytes;
    uint32_t width_mask;
    uint32_t permissions;
    pl_silicon_barrier barrier_before;
    pl_silicon_barrier barrier_after;
} pl_silicon_mmio_region;

typedef enum {
    PL_SILICON_ISA_THUMB = 0,
    PL_SILICON_ISA_RISCV32,
    PL_SILICON_ISA_RISCV32C,
    PL_SILICON_ISA_X86,
    PL_SILICON_ISA_X86_64
} pl_silicon_isa;

typedef struct {
    pl_silicon_isa isa;
    const uint8_t *bytes;
    size_t byte_count;
    uintptr_t load_address;
    size_t entry_offset;
} pl_silicon_isa_image;

typedef struct {
    size_t byte_count;
    size_t entry_offset;
    uint8_t required_alignment;
    bool layout_valid;
    bool instruction_decode_performed;
} pl_silicon_isa_report;

typedef struct {
    pl_silicon_arch architecture;
    uint32_t exception_number;
    uint32_t status_primary;
    uint32_t status_secondary;
    uintptr_t fault_address;
    bool fault_address_valid;
    uint64_t cycle_stamp;
} pl_silicon_fault_snapshot;

typedef struct {
    uint64_t cycle_stamp;
    uint32_t event_code;
    uint16_t source;
    uint16_t flags;
} pl_silicon_trace_event;

typedef struct {
    pl_silicon_trace_event events[PL_SILICON_TRACE_CAPACITY];
    atomic_uint_least32_t head;
    atomic_uint_least32_t tail;
    atomic_uint_least32_t dropped;
} pl_silicon_trace_ring;

/* Compile-target identity only; use pl_processor_scan for host CPU inventory. */
pl_silicon_arch pl_silicon_architecture_current(void);
const char *pl_silicon_architecture_name(pl_silicon_arch architecture);
const char *pl_silicon_counter_kind_name(pl_silicon_counter_kind kind);

/* Hardware barriers. Unsupported semantics return PL_SILICON_UNSUPPORTED. */
int pl_silicon_barrier_execute(pl_silicon_barrier barrier);

/* Native support is deliberately limited to an ordered x86 TSC by default.
   RISC-V CSR access requires PYTYLIKI_ENABLE_RISCV_CYCLE_CSR at build time. */
int pl_silicon_cycle_source_native(pl_silicon_cycle_source *out);
int pl_silicon_cycle_source_dwt(pl_silicon_cycle_source *out,
                                volatile uint32_t *cyccnt,
                                uint64_t frequency_hz);
int pl_silicon_dwt_enable(const pl_silicon_dwt_registers *registers,
                          bool reset_counter);
int pl_silicon_cycle_read(const pl_silicon_cycle_source *source,
                          uint64_t *out_value);
void pl_silicon_cycle_accumulator_init(pl_silicon_cycle_accumulator *accumulator);
int pl_silicon_cycle_accumulator_sample(const pl_silicon_cycle_source *source,
                                        pl_silicon_cycle_accumulator *accumulator,
                                        uint64_t *out_delta);

/* MMIO is only available through an explicitly mapped region and allowlist. */
int pl_silicon_mmio_region_init(pl_silicon_mmio_region *region,
                                uintptr_t bus_base,
                                volatile void *mapped_base,
                                size_t size_bytes,
                                uint32_t width_mask,
                                uint32_t permissions,
                                pl_silicon_barrier barrier_before,
                                pl_silicon_barrier barrier_after);
int pl_silicon_mmio_read(const pl_silicon_mmio_region *region,
                         uintptr_t address,
                         unsigned width_bits,
                         uint32_t *out_value);
int pl_silicon_mmio_write(const pl_silicon_mmio_region *region,
                          uintptr_t address,
                          unsigned width_bits,
                          uint32_t value);

/* Layout verification only. This module never decodes or executes code. */
int pl_silicon_isa_image_validate(const pl_silicon_isa_image *image,
                                  pl_silicon_isa_report *out_report);
int pl_silicon_fault_snapshot_validate(const pl_silicon_fault_snapshot *snapshot);

/* Single-producer/single-consumer event ring; check lock-free support before ISR use. */
void pl_silicon_trace_ring_init(pl_silicon_trace_ring *ring);
bool pl_silicon_trace_ring_is_lock_free(const pl_silicon_trace_ring *ring);
int pl_silicon_trace_ring_push(pl_silicon_trace_ring *ring,
                               const pl_silicon_trace_event *event);
int pl_silicon_trace_ring_pop(pl_silicon_trace_ring *ring,
                              pl_silicon_trace_event *out_event);
uint32_t pl_silicon_trace_ring_dropped(const pl_silicon_trace_ring *ring);

#endif
