#include "processor_silicon.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t value;
} fake_counter;

typedef struct {
    uint64_t value;
} fake_counter_64;

static int read_fake_counter(void *context, uint64_t *out_value) {
    fake_counter *counter = (fake_counter *)context;
    if (counter == NULL || out_value == NULL) return PL_SILICON_INVALID_ARGUMENT;
    *out_value = counter->value;
    return PL_SILICON_OK;
}

static int read_fake_counter_64(void *context, uint64_t *out_value) {
    fake_counter_64 *counter = (fake_counter_64 *)context;
    if (counter == NULL || out_value == NULL) return PL_SILICON_INVALID_ARGUMENT;
    *out_value = counter->value;
    return PL_SILICON_OK;
}

static void test_architecture_and_barriers(void) {
    pl_silicon_arch arch = pl_silicon_architecture_current();
    assert(pl_silicon_architecture_name(arch) != NULL);
    assert(strcmp(pl_silicon_architecture_name(PL_SILICON_ARCH_UNKNOWN), "unknown") == 0);
    assert(pl_silicon_barrier_execute(PL_SILICON_BARRIER_DATA) == PL_SILICON_OK);
    assert(pl_silicon_barrier_execute((pl_silicon_barrier)99) == PL_SILICON_INVALID_ARGUMENT);
#if defined(__x86_64__) || defined(_M_X64)
    assert(pl_silicon_barrier_execute(PL_SILICON_BARRIER_INSTRUCTION) == PL_SILICON_OK);
    assert(pl_silicon_barrier_execute(PL_SILICON_BARRIER_COMPLETION) == PL_SILICON_UNSUPPORTED);
#endif
}

static void test_native_cycle_source(void) {
    pl_silicon_cycle_source source;
    uint64_t first = 0u;
    uint64_t second = 0u;
    int status = pl_silicon_cycle_source_native(&source);
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
    assert(status == PL_SILICON_OK);
    assert(source.kind == PL_SILICON_COUNTER_X86_TSC);
    assert(source.counter_bits == 64u);
    assert(source.frequency_hz == 0u);
    assert(pl_silicon_cycle_read(&source, &first) == PL_SILICON_OK);
    assert(pl_silicon_cycle_read(&source, &second) == PL_SILICON_OK);
    assert(second >= first);
#else
    assert(status == PL_SILICON_OK || status == PL_SILICON_UNSUPPORTED);
#endif
}

static void test_cycle_source_and_wrap(void) {
    fake_counter counter = {UINT32_C(0xFFFFFFF0)};
    pl_silicon_cycle_source source;
    pl_silicon_cycle_accumulator accumulator;
    uint64_t delta = 0u;

    memset(&source, 0, sizeof(source));
    source.read = read_fake_counter;
    source.context = &counter;
    source.counter_bits = 32u;
    source.kind = PL_SILICON_COUNTER_CUSTOM;
    pl_silicon_cycle_accumulator_init(&accumulator);

    assert(pl_silicon_cycle_accumulator_sample(&source, &accumulator, &delta) == PL_SILICON_OK);
    assert(delta == 0u && accumulator.initialized);
    counter.value = UINT32_C(0x00000010);
    assert(pl_silicon_cycle_accumulator_sample(&source, &accumulator, &delta) == PL_SILICON_OK);
    assert(delta == 32u);
    assert(accumulator.total == 32u && accumulator.peak_delta == 32u);
    assert(accumulator.samples == 1u);
    assert(pl_silicon_cycle_read(NULL, &delta) == PL_SILICON_INVALID_ARGUMENT);

#if defined(__arm__) || defined(__thumb__) || defined(_M_ARM)
    assert(pl_silicon_cycle_source_dwt(&source, (volatile uint32_t *)&counter.value,
                                       UINT64_C(84000000)) == PL_SILICON_OK);
    assert(source.counter_bits == 32u && source.frequency_hz == UINT64_C(84000000));
    assert(pl_silicon_cycle_read(&source, &delta) == PL_SILICON_OK);
    assert(delta == UINT32_C(0x10));
#else
    assert(pl_silicon_cycle_source_dwt(&source, (volatile uint32_t *)&counter.value,
                                       UINT64_C(84000000)) == PL_SILICON_UNSUPPORTED);
#endif
}

static void test_cycle_accumulator_64bit(void) {
    fake_counter_64 counter = {UINT64_MAX - 5u};
    pl_silicon_cycle_source source;
    pl_silicon_cycle_accumulator accumulator;
    uint64_t delta = 0u;

    memset(&source, 0, sizeof(source));
    source.read = read_fake_counter_64;
    source.context = &counter;
    source.counter_bits = 64u;
    source.kind = PL_SILICON_COUNTER_CUSTOM;
    pl_silicon_cycle_accumulator_init(&accumulator);
    assert(pl_silicon_cycle_accumulator_sample(&source, &accumulator, &delta) == PL_SILICON_OK);
    assert(delta == 0u);
    counter.value = 3u;
    assert(pl_silicon_cycle_accumulator_sample(&source, &accumulator, &delta) == PL_SILICON_OK);
    assert(delta == 9u && accumulator.total == 9u);
    accumulator.total = UINT64_MAX - 2u;
    counter.value = 8u;
    assert(pl_silicon_cycle_accumulator_sample(&source, &accumulator, &delta) == PL_SILICON_OK);
    assert(delta == 5u && accumulator.total == UINT64_MAX);
}

static void test_dwt_setup(void) {
    uint32_t demcr = 0u;
    uint32_t ctrl = 0u;
    uint32_t cyccnt = 123u;
    pl_silicon_dwt_registers regs;

    regs.demcr = &demcr;
    regs.dwt_ctrl = &ctrl;
    regs.cyccnt = &cyccnt;
#if defined(__x86_64__) || defined(_M_X64)
    assert(pl_silicon_dwt_enable(&regs, true) == PL_SILICON_UNSUPPORTED);
    /* Unsupported host access must fail before touching the supplied registers. */
    assert(demcr == 0u && ctrl == 0u && cyccnt == 123u);
#endif
    assert(pl_silicon_dwt_enable(NULL, true) == PL_SILICON_INVALID_ARGUMENT);
}

static void test_mmio_region(void) {
    _Alignas(4) uint32_t registers[2] = {0u, 0u};
    pl_silicon_mmio_region region;
    uint32_t value = 0u;

    assert(pl_silicon_mmio_region_init(
        &region, (uintptr_t)0x40020000u, registers, sizeof(registers),
        PL_SILICON_WIDTH_8 | PL_SILICON_WIDTH_16 | PL_SILICON_WIDTH_32,
        PL_SILICON_MMIO_READ | PL_SILICON_MMIO_WRITE,
        PL_SILICON_BARRIER_DATA, PL_SILICON_BARRIER_DATA) == PL_SILICON_OK);
    assert(pl_silicon_mmio_write(&region, (uintptr_t)0x40020004u, 32u,
                                 UINT32_C(0x12345678)) == PL_SILICON_OK);
    assert(pl_silicon_mmio_read(&region, (uintptr_t)0x40020004u, 32u,
                                &value) == PL_SILICON_OK);
    assert(value == UINT32_C(0x12345678));
    assert(pl_silicon_mmio_write(&region, (uintptr_t)0x40020001u, 32u, 1u) ==
           PL_SILICON_INVALID_ARGUMENT);
    assert(pl_silicon_mmio_read(&region, (uintptr_t)0x40020008u, 8u, &value) ==
           PL_SILICON_OUT_OF_RANGE);
    assert(pl_silicon_mmio_write(&region, (uintptr_t)0x40020000u, 8u, 256u) ==
           PL_SILICON_OUT_OF_RANGE);

    assert(pl_silicon_mmio_region_init(
        &region, (uintptr_t)0x1000u, registers, sizeof(registers),
        PL_SILICON_WIDTH_32, PL_SILICON_MMIO_READ,
        PL_SILICON_BARRIER_DATA, PL_SILICON_BARRIER_DATA) == PL_SILICON_OK);
    assert(pl_silicon_mmio_write(&region, (uintptr_t)0x1000u, 32u, 7u) ==
           PL_SILICON_PERMISSION_DENIED);
    assert(pl_silicon_mmio_region_init(
        &region, UINTPTR_MAX - 3u, registers, sizeof(registers),
        PL_SILICON_WIDTH_8, PL_SILICON_MMIO_READ,
        PL_SILICON_BARRIER_DATA, PL_SILICON_BARRIER_DATA) ==
           PL_SILICON_INVALID_ARGUMENT);
}

static void test_isa_layout_only(void) {
    _Alignas(4) const uint8_t code[8] = {0u};
    pl_silicon_isa_image image;
    pl_silicon_isa_report report;

    image.isa = PL_SILICON_ISA_THUMB;
    image.bytes = code;
    image.byte_count = sizeof(code);
    image.load_address = (uintptr_t)0x08000000u;
    image.entry_offset = 2u;
    assert(pl_silicon_isa_image_validate(&image, &report) == PL_SILICON_OK);
    assert(report.layout_valid && !report.instruction_decode_performed);
    assert(report.required_alignment == 2u && report.entry_offset == 2u);

    image.entry_offset = 1u;
    assert(pl_silicon_isa_image_validate(&image, &report) == PL_SILICON_INVALID_ARGUMENT);
    image.isa = PL_SILICON_ISA_RISCV32;
    image.entry_offset = 2u;
    assert(pl_silicon_isa_image_validate(&image, &report) == PL_SILICON_INVALID_ARGUMENT);
    image.isa = PL_SILICON_ISA_RISCV32C;
    assert(pl_silicon_isa_image_validate(&image, &report) == PL_SILICON_OK);
}

static void test_fault_snapshot_and_trace_ring(void) {
    pl_silicon_fault_snapshot fault;
    pl_silicon_trace_ring ring;
    pl_silicon_trace_event event;
    pl_silicon_trace_event received;
    uint32_t i;

    memset(&fault, 0, sizeof(fault));
    fault.architecture = PL_SILICON_ARCH_ARM32;
    fault.exception_number = 3u;
    fault.fault_address_valid = true;
    fault.fault_address = (uintptr_t)0x20000000u;
    assert(pl_silicon_fault_snapshot_validate(&fault) == PL_SILICON_OK);
    /* Address zero can be a valid faulting address; validity is explicit. */
    fault.fault_address = 0u;
    assert(pl_silicon_fault_snapshot_validate(&fault) == PL_SILICON_OK);
    fault.architecture = (pl_silicon_arch)99;
    assert(pl_silicon_fault_snapshot_validate(&fault) == PL_SILICON_INVALID_ARGUMENT);
    fault.architecture = (pl_silicon_arch)-1;
    assert(pl_silicon_fault_snapshot_validate(&fault) == PL_SILICON_INVALID_ARGUMENT);

    pl_silicon_trace_ring_init(&ring);
    (void)pl_silicon_trace_ring_is_lock_free(&ring);
    memset(&event, 0, sizeof(event));
    for (i = 0u; i < PL_SILICON_TRACE_CAPACITY; ++i) {
        event.cycle_stamp = i;
        event.event_code = i + 100u;
        assert(pl_silicon_trace_ring_push(&ring, &event) == PL_SILICON_OK);
    }
    assert(pl_silicon_trace_ring_push(&ring, &event) == PL_SILICON_FULL);
    assert(pl_silicon_trace_ring_dropped(&ring) == 1u);
    for (i = 0u; i < PL_SILICON_TRACE_CAPACITY; ++i) {
        assert(pl_silicon_trace_ring_pop(&ring, &received) == PL_SILICON_OK);
        assert(received.cycle_stamp == i && received.event_code == i + 100u);
    }
    assert(pl_silicon_trace_ring_pop(&ring, &received) == PL_SILICON_EMPTY);
}

int main(void) {
    test_architecture_and_barriers();
    test_native_cycle_source();
    test_cycle_source_and_wrap();
    test_cycle_accumulator_64bit();
    test_dwt_setup();
    test_mmio_region();
    test_isa_layout_only();
    test_fault_snapshot_and_trace_ring();
    puts("Pytyliki processor silicon tests passed.");
    return 0;
}
