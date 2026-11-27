#include "processor_silicon.h"

#include <limits.h>
#include <string.h>

#if defined(_MSC_VER)
#include <intrin.h>
#if defined(_M_X64) || (defined(_M_IX86) && defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
#include <emmintrin.h>
#endif
#endif

#define PL_SILICON_DEMCR_TRCENA (UINT32_C(1) << 24)
#define PL_SILICON_DWT_CYCCNTENA UINT32_C(1)
#define PL_SILICON_DWT_NOCYCCNT (UINT32_C(1) << 25)
#define PL_SILICON_TRACE_MASK (PL_SILICON_TRACE_CAPACITY - 1u)

#if (PL_SILICON_TRACE_CAPACITY == 0u) || \
    ((PL_SILICON_TRACE_CAPACITY & (PL_SILICON_TRACE_CAPACITY - 1u)) != 0u)
#error "PL_SILICON_TRACE_CAPACITY must be a power of two"
#endif

static int pl_silicon_valid_barrier(pl_silicon_barrier barrier) {
    return barrier == PL_SILICON_BARRIER_DATA ||
           barrier == PL_SILICON_BARRIER_COMPLETION ||
           barrier == PL_SILICON_BARRIER_INSTRUCTION;
}

static int pl_silicon_barrier_supported(pl_silicon_barrier barrier) {
    if (!pl_silicon_valid_barrier(barrier)) return 0;
#if (defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))) || \
    ((defined(__GNUC__) || defined(__clang__)) && \
     (defined(__arm__) || defined(__thumb__) || defined(__aarch64__)))
    return 1;
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__riscv)
    if (barrier == PL_SILICON_BARRIER_COMPLETION) return 0;
    if (barrier == PL_SILICON_BARRIER_INSTRUCTION) {
#if defined(__riscv_zifencei)
        return 1;
#else
        return 0;
#endif
    }
    return 1;
#elif defined(_MSC_VER) && (defined(_M_X64) || \
      (defined(_M_IX86) && defined(_M_IX86_FP) && (_M_IX86_FP >= 2)))
    return barrier != PL_SILICON_BARRIER_COMPLETION;
#elif (defined(__GNUC__) || defined(__clang__)) && \
      (defined(__x86_64__) || (defined(__i386__) && defined(__SSE2__)))
    return barrier != PL_SILICON_BARRIER_COMPLETION;
#else
    return 0;
#endif
}

pl_silicon_arch pl_silicon_architecture_current(void) {
#if defined(_M_X64) || defined(__x86_64__)
    return PL_SILICON_ARCH_X86_64;
#elif defined(_M_IX86) || defined(__i386__)
    return PL_SILICON_ARCH_X86;
#elif defined(_M_ARM64) || defined(__aarch64__)
    return PL_SILICON_ARCH_ARM64;
#elif defined(_M_ARM) || defined(__arm__) || defined(__thumb__)
    return PL_SILICON_ARCH_ARM32;
#elif defined(__riscv)
#if defined(__riscv_xlen) && (__riscv_xlen == 64)
    return PL_SILICON_ARCH_RISCV64;
#else
    return PL_SILICON_ARCH_RISCV32;
#endif
#else
    return PL_SILICON_ARCH_UNKNOWN;
#endif
}

const char *pl_silicon_architecture_name(pl_silicon_arch architecture) {
    switch (architecture) {
        case PL_SILICON_ARCH_ARM32: return "arm32";
        case PL_SILICON_ARCH_ARM64: return "arm64";
        case PL_SILICON_ARCH_RISCV32: return "riscv32";
        case PL_SILICON_ARCH_RISCV64: return "riscv64";
        case PL_SILICON_ARCH_X86: return "x86";
        case PL_SILICON_ARCH_X86_64: return "x86_64";
        default: return "unknown";
    }
}

const char *pl_silicon_counter_kind_name(pl_silicon_counter_kind kind) {
    switch (kind) {
        case PL_SILICON_COUNTER_CORTEX_M_DWT: return "cortex-m-dwt-cyccnt";
        case PL_SILICON_COUNTER_RISCV_CYCLE: return "riscv-cycle-csr";
        case PL_SILICON_COUNTER_X86_TSC: return "x86-tsc-ticks";
        case PL_SILICON_COUNTER_CUSTOM: return "custom-counter";
        default: return "unknown";
    }
}

int pl_silicon_barrier_execute(pl_silicon_barrier barrier) {
    if (!pl_silicon_valid_barrier(barrier)) return PL_SILICON_INVALID_ARGUMENT;

#if defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
    switch (barrier) {
        case PL_SILICON_BARRIER_DATA: __dmb(0xF); return PL_SILICON_OK;
        case PL_SILICON_BARRIER_COMPLETION: __dsb(0xF); return PL_SILICON_OK;
        case PL_SILICON_BARRIER_INSTRUCTION: __isb(0xF); return PL_SILICON_OK;
        default: return PL_SILICON_INVALID_ARGUMENT;
    }
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__arm__) || defined(__thumb__) || defined(__aarch64__))
    switch (barrier) {
        case PL_SILICON_BARRIER_DATA:
            __asm__ volatile("dmb sy" ::: "memory");
            return PL_SILICON_OK;
        case PL_SILICON_BARRIER_COMPLETION:
            __asm__ volatile("dsb sy" ::: "memory");
            return PL_SILICON_OK;
        case PL_SILICON_BARRIER_INSTRUCTION:
            __asm__ volatile("isb sy" ::: "memory");
            return PL_SILICON_OK;
        default:
            return PL_SILICON_INVALID_ARGUMENT;
    }
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__riscv)
    switch (barrier) {
        case PL_SILICON_BARRIER_DATA:
            __asm__ volatile("fence iorw, iorw" ::: "memory");
            return PL_SILICON_OK;
        case PL_SILICON_BARRIER_INSTRUCTION:
#if defined(__riscv_zifencei)
            __asm__ volatile("fence.i" ::: "memory");
            return PL_SILICON_OK;
#else
            return PL_SILICON_UNSUPPORTED;
#endif
        case PL_SILICON_BARRIER_COMPLETION:
            /* FENCE orders accesses; it does not promise DSB-like completion. */
            return PL_SILICON_UNSUPPORTED;
        default:
            return PL_SILICON_INVALID_ARGUMENT;
    }
#elif defined(_MSC_VER) && (defined(_M_X64) || \
      (defined(_M_IX86) && defined(_M_IX86_FP) && (_M_IX86_FP >= 2)))
    switch (barrier) {
        case PL_SILICON_BARRIER_DATA:
            _mm_mfence();
            return PL_SILICON_OK;
        case PL_SILICON_BARRIER_INSTRUCTION: {
            int registers[4];
            __cpuid(registers, 0);
            return PL_SILICON_OK;
        }
        case PL_SILICON_BARRIER_COMPLETION:
            return PL_SILICON_UNSUPPORTED;
        default:
            return PL_SILICON_INVALID_ARGUMENT;
    }
#elif (defined(__GNUC__) || defined(__clang__)) && \
      (defined(__x86_64__) || (defined(__i386__) && defined(__SSE2__)))
    switch (barrier) {
        case PL_SILICON_BARRIER_DATA:
            __asm__ volatile("mfence" ::: "memory");
            return PL_SILICON_OK;
        case PL_SILICON_BARRIER_INSTRUCTION: {
            uint32_t eax = 0u;
            uint32_t ebx;
            uint32_t ecx;
            uint32_t edx;
            __asm__ volatile("cpuid"
                             : "+a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                             :
                             : "memory");
            (void)ebx;
            (void)ecx;
            (void)edx;
            return PL_SILICON_OK;
        }
        case PL_SILICON_BARRIER_COMPLETION:
            return PL_SILICON_UNSUPPORTED;
        default:
            return PL_SILICON_INVALID_ARGUMENT;
    }
#else
    return PL_SILICON_UNSUPPORTED;
#endif
}

static int pl_silicon_read_dwt(void *context, uint64_t *out_value) {
    const volatile uint32_t *counter = (const volatile uint32_t *)context;
    if (counter == NULL || out_value == NULL) return PL_SILICON_INVALID_ARGUMENT;
    *out_value = (uint64_t)(*counter);
    return PL_SILICON_OK;
}

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
static int pl_silicon_read_native_tsc(void *context, uint64_t *out_value) {
    (void)context;
    if (out_value == NULL) return PL_SILICON_INVALID_ARGUMENT;
    _mm_lfence();
    *out_value = (uint64_t)__rdtsc();
    _mm_lfence();
    return PL_SILICON_OK;
}
#elif (defined(__GNUC__) || defined(__clang__)) && \
      (defined(__x86_64__) || (defined(__i386__) && defined(__SSE2__)))
static int pl_silicon_read_native_tsc(void *context, uint64_t *out_value) {
    uint32_t low;
    uint32_t high;
    (void)context;
    if (out_value == NULL) return PL_SILICON_INVALID_ARGUMENT;
    __asm__ volatile("lfence\n\trdtsc\n\tlfence"
                     : "=a"(low), "=d"(high)
                     :
                     : "memory");
    *out_value = ((uint64_t)high << 32u) | (uint64_t)low;
    return PL_SILICON_OK;
}
#endif

#if defined(__riscv) && defined(PYTYLIKI_ENABLE_RISCV_CYCLE_CSR)
static int pl_silicon_read_riscv_cycle(void *context, uint64_t *out_value) {
    (void)context;
    if (out_value == NULL) return PL_SILICON_INVALID_ARGUMENT;
#if defined(__riscv_xlen) && (__riscv_xlen == 64)
    __asm__ volatile("rdcycle %0" : "=r"(*out_value) : : "memory");
#else
    {
        uint32_t high_before;
        uint32_t high_after;
        uint32_t low;
        do {
            __asm__ volatile("rdcycleh %0" : "=r"(high_before) : : "memory");
            __asm__ volatile("rdcycle %0" : "=r"(low) : : "memory");
            __asm__ volatile("rdcycleh %0" : "=r"(high_after) : : "memory");
        } while (high_before != high_after);
        *out_value = ((uint64_t)high_after << 32u) | (uint64_t)low;
    }
#endif
    return PL_SILICON_OK;
}
#endif

int pl_silicon_cycle_source_native(pl_silicon_cycle_source *out) {
    if (out == NULL) return PL_SILICON_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
#if (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))) || \
    ((defined(__GNUC__) || defined(__clang__)) && \
     (defined(__x86_64__) || (defined(__i386__) && defined(__SSE2__))))
    out->read = pl_silicon_read_native_tsc;
    out->counter_bits = 64u;
    out->kind = PL_SILICON_COUNTER_X86_TSC;
    /* TSC frequency is not inferred from the current core clock. */
    return PL_SILICON_OK;
#elif defined(__riscv) && defined(PYTYLIKI_ENABLE_RISCV_CYCLE_CSR)
    out->read = pl_silicon_read_riscv_cycle;
    out->counter_bits = 64u;
    out->kind = PL_SILICON_COUNTER_RISCV_CYCLE;
    return PL_SILICON_OK;
#else
    return PL_SILICON_UNSUPPORTED;
#endif
}

int pl_silicon_cycle_source_dwt(pl_silicon_cycle_source *out,
                                volatile uint32_t *cyccnt,
                                uint64_t frequency_hz) {
    if (out == NULL || cyccnt == NULL) return PL_SILICON_INVALID_ARGUMENT;
    if (pl_silicon_architecture_current() != PL_SILICON_ARCH_ARM32)
        return PL_SILICON_UNSUPPORTED;
    memset(out, 0, sizeof(*out));
    out->read = pl_silicon_read_dwt;
    out->context = (void *)cyccnt;
    out->frequency_hz = frequency_hz;
    out->counter_bits = 32u;
    out->kind = PL_SILICON_COUNTER_CORTEX_M_DWT;
    return PL_SILICON_OK;
}

int pl_silicon_dwt_enable(const pl_silicon_dwt_registers *registers,
                          bool reset_counter) {
    if (registers == NULL || registers->demcr == NULL ||
        registers->dwt_ctrl == NULL || registers->cyccnt == NULL)
        return PL_SILICON_INVALID_ARGUMENT;
    if (pl_silicon_architecture_current() != PL_SILICON_ARCH_ARM32)
        return PL_SILICON_UNSUPPORTED;
    if ((*registers->dwt_ctrl & PL_SILICON_DWT_NOCYCCNT) != 0u)
        return PL_SILICON_UNSUPPORTED;
    if (!pl_silicon_barrier_supported(PL_SILICON_BARRIER_COMPLETION) ||
        !pl_silicon_barrier_supported(PL_SILICON_BARRIER_INSTRUCTION))
        return PL_SILICON_UNSUPPORTED;
    *registers->demcr |= PL_SILICON_DEMCR_TRCENA;
    if (reset_counter) *registers->cyccnt = 0u;
    *registers->dwt_ctrl |= PL_SILICON_DWT_CYCCNTENA;
    if (pl_silicon_barrier_execute(PL_SILICON_BARRIER_COMPLETION) != PL_SILICON_OK)
        return PL_SILICON_UNSUPPORTED;
    if (pl_silicon_barrier_execute(PL_SILICON_BARRIER_INSTRUCTION) != PL_SILICON_OK)
        return PL_SILICON_UNSUPPORTED;
    return PL_SILICON_OK;
}

int pl_silicon_cycle_read(const pl_silicon_cycle_source *source,
                          uint64_t *out_value) {
    if (source == NULL || source->read == NULL || out_value == NULL ||
        (source->counter_bits != 32u && source->counter_bits != 64u))
        return PL_SILICON_INVALID_ARGUMENT;
    return source->read(source->context, out_value);
}

void pl_silicon_cycle_accumulator_init(pl_silicon_cycle_accumulator *accumulator) {
    if (accumulator != NULL) memset(accumulator, 0, sizeof(*accumulator));
}

int pl_silicon_cycle_accumulator_sample(const pl_silicon_cycle_source *source,
                                        pl_silicon_cycle_accumulator *accumulator,
                                        uint64_t *out_delta) {
    uint64_t now;
    uint64_t delta;
    int status;
    if (accumulator == NULL) return PL_SILICON_INVALID_ARGUMENT;
    status = pl_silicon_cycle_read(source, &now);
    if (status != PL_SILICON_OK) return status;
    if (!accumulator->initialized) {
        accumulator->previous = source->counter_bits == 32u ? (uint32_t)now : now;
        accumulator->initialized = true;
        if (out_delta != NULL) *out_delta = 0u;
        return PL_SILICON_OK;
    }
    if (source->counter_bits == 32u)
        delta = (uint32_t)now - (uint32_t)accumulator->previous;
    else
        delta = now - accumulator->previous;
    accumulator->previous = source->counter_bits == 32u ? (uint32_t)now : now;
    if (UINT64_MAX - accumulator->total < delta)
        accumulator->total = UINT64_MAX;
    else
        accumulator->total += delta;
    if (delta > accumulator->peak_delta) accumulator->peak_delta = delta;
    if (accumulator->samples != UINT64_MAX) accumulator->samples++;
    if (out_delta != NULL) *out_delta = delta;
    return PL_SILICON_OK;
}

static unsigned pl_silicon_width_flag(unsigned width_bits) {
    switch (width_bits) {
        case 8u: return PL_SILICON_WIDTH_8;
        case 16u: return PL_SILICON_WIDTH_16;
        case 32u: return PL_SILICON_WIDTH_32;
        default: return 0u;
    }
}

int pl_silicon_mmio_region_init(pl_silicon_mmio_region *region,
                                uintptr_t bus_base,
                                volatile void *mapped_base,
                                size_t size_bytes,
                                uint32_t width_mask,
                                uint32_t permissions,
                                pl_silicon_barrier barrier_before,
                                pl_silicon_barrier barrier_after) {
    const uint32_t known_widths = PL_SILICON_WIDTH_8 |
                                  PL_SILICON_WIDTH_16 |
                                  PL_SILICON_WIDTH_32;
    const uint32_t known_permissions = PL_SILICON_MMIO_READ |
                                       PL_SILICON_MMIO_WRITE;
    if (region == NULL || mapped_base == NULL || size_bytes == 0u ||
        size_bytes > (size_t)UINTPTR_MAX ||
        bus_base > UINTPTR_MAX - (uintptr_t)size_bytes ||
        width_mask == 0u ||
        (width_mask & ~known_widths) != 0u || permissions == 0u ||
        (permissions & ~known_permissions) != 0u ||
        !pl_silicon_valid_barrier(barrier_before) ||
        !pl_silicon_valid_barrier(barrier_after))
        return PL_SILICON_INVALID_ARGUMENT;
    region->bus_base = bus_base;
    region->mapped_base = (volatile uint8_t *)mapped_base;
    region->size_bytes = size_bytes;
    region->width_mask = width_mask;
    region->permissions = permissions;
    region->barrier_before = barrier_before;
    region->barrier_after = barrier_after;
    return PL_SILICON_OK;
}

static int pl_silicon_mmio_address_check(const pl_silicon_mmio_region *region,
                                         uintptr_t address,
                                         unsigned width_bits,
                                         uint32_t permission,
                                         size_t *out_offset) {
    const unsigned width_flag = pl_silicon_width_flag(width_bits);
    const size_t width_bytes = (size_t)(width_bits / 8u);
    uintptr_t mapped_address;
    size_t offset;
    if (region == NULL || region->mapped_base == NULL || out_offset == NULL ||
        width_flag == 0u)
        return PL_SILICON_INVALID_ARGUMENT;
    if ((region->permissions & permission) == 0u) return PL_SILICON_PERMISSION_DENIED;
    if ((region->width_mask & width_flag) == 0u) return PL_SILICON_PERMISSION_DENIED;
    if (address < region->bus_base) return PL_SILICON_OUT_OF_RANGE;
    if ((address % (uintptr_t)width_bytes) != 0u) return PL_SILICON_INVALID_ARGUMENT;
    offset = (size_t)(address - region->bus_base);
    if (offset > region->size_bytes || width_bytes > region->size_bytes - offset)
        return PL_SILICON_OUT_OF_RANGE;
    mapped_address = (uintptr_t)region->mapped_base;
    if (mapped_address > UINTPTR_MAX - offset) return PL_SILICON_OVERFLOW;
    mapped_address += offset;
    if ((mapped_address % (uintptr_t)width_bytes) != 0u)
        return PL_SILICON_INVALID_ARGUMENT;
    *out_offset = offset;
    return PL_SILICON_OK;
}

static int pl_silicon_mmio_barriers_check(const pl_silicon_mmio_region *region) {
    if (region == NULL) return PL_SILICON_INVALID_ARGUMENT;
    if (!pl_silicon_barrier_supported(region->barrier_before) ||
        !pl_silicon_barrier_supported(region->barrier_after))
        return PL_SILICON_UNSUPPORTED;
    return PL_SILICON_OK;
}

int pl_silicon_mmio_read(const pl_silicon_mmio_region *region,
                         uintptr_t address,
                         unsigned width_bits,
                         uint32_t *out_value) {
    size_t offset;
    int status;
    volatile uint8_t *mapped;
    if (out_value == NULL) return PL_SILICON_INVALID_ARGUMENT;
    status = pl_silicon_mmio_address_check(region, address, width_bits,
                                           PL_SILICON_MMIO_READ, &offset);
    if (status != PL_SILICON_OK) return status;
    status = pl_silicon_mmio_barriers_check(region);
    if (status != PL_SILICON_OK) return status;
    status = pl_silicon_barrier_execute(region->barrier_before);
    if (status != PL_SILICON_OK) return status;
    mapped = region->mapped_base + offset;
    switch (width_bits) {
        case 8u: *out_value = (uint32_t)(*(volatile uint8_t *)mapped); break;
        case 16u: *out_value = (uint32_t)(*(volatile uint16_t *)mapped); break;
        case 32u: *out_value = *(volatile uint32_t *)mapped; break;
        default: return PL_SILICON_INVALID_ARGUMENT;
    }
    return pl_silicon_barrier_execute(region->barrier_after);
}

int pl_silicon_mmio_write(const pl_silicon_mmio_region *region,
                          uintptr_t address,
                          unsigned width_bits,
                          uint32_t value) {
    size_t offset;
    int status;
    volatile uint8_t *mapped;
    status = pl_silicon_mmio_address_check(region, address, width_bits,
                                           PL_SILICON_MMIO_WRITE, &offset);
    if (status != PL_SILICON_OK) return status;
    if (width_bits == 8u && value > UINT8_MAX) return PL_SILICON_OUT_OF_RANGE;
    if (width_bits == 16u && value > UINT16_MAX) return PL_SILICON_OUT_OF_RANGE;
    status = pl_silicon_mmio_barriers_check(region);
    if (status != PL_SILICON_OK) return status;
    status = pl_silicon_barrier_execute(region->barrier_before);
    if (status != PL_SILICON_OK) return status;
    mapped = region->mapped_base + offset;
    switch (width_bits) {
        case 8u: *(volatile uint8_t *)mapped = (uint8_t)value; break;
        case 16u: *(volatile uint16_t *)mapped = (uint16_t)value; break;
        case 32u: *(volatile uint32_t *)mapped = value; break;
        default: return PL_SILICON_INVALID_ARGUMENT;
    }
    return pl_silicon_barrier_execute(region->barrier_after);
}

static int pl_silicon_isa_arch_matches(pl_silicon_isa isa,
                                       pl_silicon_arch *out_arch,
                                       uint8_t *out_alignment) {
    if (out_arch == NULL || out_alignment == NULL) return 0;
    switch (isa) {
        case PL_SILICON_ISA_THUMB:
            *out_arch = PL_SILICON_ARCH_ARM32;
            *out_alignment = 2u;
            return 1;
        case PL_SILICON_ISA_RISCV32:
            *out_arch = PL_SILICON_ARCH_RISCV32;
            *out_alignment = 4u;
            return 1;
        case PL_SILICON_ISA_RISCV32C:
            *out_arch = PL_SILICON_ARCH_RISCV32;
            *out_alignment = 2u;
            return 1;
        case PL_SILICON_ISA_X86:
            *out_arch = PL_SILICON_ARCH_X86;
            *out_alignment = 1u;
            return 1;
        case PL_SILICON_ISA_X86_64:
            *out_arch = PL_SILICON_ARCH_X86_64;
            *out_alignment = 1u;
            return 1;
        default:
            return 0;
    }
}

int pl_silicon_isa_image_validate(const pl_silicon_isa_image *image,
                                  pl_silicon_isa_report *out_report) {
    pl_silicon_arch target_arch;
    uint8_t alignment;
    uintptr_t entry_address;
    if (image == NULL || out_report == NULL || image->bytes == NULL ||
        image->byte_count == 0u || image->entry_offset >= image->byte_count)
        return PL_SILICON_INVALID_ARGUMENT;
    if (!pl_silicon_isa_arch_matches(image->isa, &target_arch, &alignment))
        return PL_SILICON_UNSUPPORTED;
    if (image->load_address > UINTPTR_MAX - image->byte_count ||
        image->load_address > UINTPTR_MAX - image->entry_offset)
        return PL_SILICON_OVERFLOW;
    entry_address = image->load_address + image->entry_offset;
    if ((image->load_address % alignment) != 0u ||
        (entry_address % alignment) != 0u ||
        ((uintptr_t)image->bytes % alignment) != 0u)
        return PL_SILICON_INVALID_ARGUMENT;
    memset(out_report, 0, sizeof(*out_report));
    out_report->byte_count = image->byte_count;
    out_report->entry_offset = image->entry_offset;
    out_report->required_alignment = alignment;
    out_report->layout_valid = true;
    /* No decoder is linked here: byte layout is not proof of valid instructions. */
    out_report->instruction_decode_performed = false;
    (void)target_arch;
    return PL_SILICON_OK;
}

int pl_silicon_fault_snapshot_validate(const pl_silicon_fault_snapshot *snapshot) {
    if (snapshot == NULL ||
        (int)snapshot->architecture < (int)PL_SILICON_ARCH_UNKNOWN ||
        snapshot->architecture > PL_SILICON_ARCH_X86_64)
        return PL_SILICON_INVALID_ARGUMENT;
    return PL_SILICON_OK;
}

void pl_silicon_trace_ring_init(pl_silicon_trace_ring *ring) {
    if (ring == NULL) return;
    memset(ring->events, 0, sizeof(ring->events));
    atomic_init(&ring->head, 0u);
    atomic_init(&ring->tail, 0u);
    atomic_init(&ring->dropped, 0u);
}

bool pl_silicon_trace_ring_is_lock_free(const pl_silicon_trace_ring *ring) {
    if (ring == NULL) return false;
    return atomic_is_lock_free(&ring->head) &&
           atomic_is_lock_free(&ring->tail) &&
           atomic_is_lock_free(&ring->dropped);
}

int pl_silicon_trace_ring_push(pl_silicon_trace_ring *ring,
                               const pl_silicon_trace_event *event) {
    uint_least32_t head;
    uint_least32_t tail;
    if (ring == NULL || event == NULL) return PL_SILICON_INVALID_ARGUMENT;
    head = atomic_load_explicit(&ring->head, memory_order_relaxed);
    tail = atomic_load_explicit(&ring->tail, memory_order_acquire);
    if ((uint_least32_t)(head - tail) >= PL_SILICON_TRACE_CAPACITY) {
        (void)atomic_fetch_add_explicit(&ring->dropped, 1u, memory_order_relaxed);
        return PL_SILICON_FULL;
    }
    ring->events[head & PL_SILICON_TRACE_MASK] = *event;
    atomic_store_explicit(&ring->head, head + 1u, memory_order_release);
    return PL_SILICON_OK;
}

int pl_silicon_trace_ring_pop(pl_silicon_trace_ring *ring,
                              pl_silicon_trace_event *out_event) {
    uint_least32_t head;
    uint_least32_t tail;
    if (ring == NULL || out_event == NULL) return PL_SILICON_INVALID_ARGUMENT;
    tail = atomic_load_explicit(&ring->tail, memory_order_relaxed);
    head = atomic_load_explicit(&ring->head, memory_order_acquire);
    if (tail == head) return PL_SILICON_EMPTY;
    *out_event = ring->events[tail & PL_SILICON_TRACE_MASK];
    atomic_store_explicit(&ring->tail, tail + 1u, memory_order_release);
    return PL_SILICON_OK;
}

uint32_t pl_silicon_trace_ring_dropped(const pl_silicon_trace_ring *ring) {
    if (ring == NULL) return 0u;
    return (uint32_t)atomic_load_explicit(&ring->dropped, memory_order_relaxed);
}
