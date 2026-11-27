#include "silicon_runtime.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint64_t cycles;
    uint32_t mmio_address;
    uint32_t mmio_value;
    uint8_t mmio_width;
    unsigned reads;
    unsigned writes;
    unsigned barriers;
    unsigned waits;
    pl_silicon_barrier last_barrier;
} fake_hardware;

static int fake_read_cycles(void *context, uint64_t *out) {
    fake_hardware *hw = (fake_hardware *)context;
    if (!hw || !out) return -1;
    *out = hw->cycles;
    return 0;
}

static int fake_read_mmio(void *context, uint32_t address, uint8_t width,
                          uint32_t *out) {
    fake_hardware *hw = (fake_hardware *)context;
    if (!hw || !out || address != hw->mmio_address || width != hw->mmio_width)
        return -1;
    ++hw->reads;
    *out = hw->mmio_value;
    return 0;
}

static int fake_write_mmio(void *context, uint32_t address, uint8_t width,
                           uint32_t value) {
    fake_hardware *hw = (fake_hardware *)context;
    if (!hw || address != hw->mmio_address || width != hw->mmio_width) return -1;
    ++hw->writes;
    hw->mmio_value = value;
    return 0;
}

static int fake_barrier(void *context, pl_silicon_barrier kind) {
    fake_hardware *hw = (fake_hardware *)context;
    if (!hw || kind == PL_SILICON_BARRIER_NONE) return -1;
    ++hw->barriers;
    hw->last_barrier = kind;
    return 0;
}

static int fake_wait(void *context) {
    fake_hardware *hw = (fake_hardware *)context;
    if (!hw) return -1;
    ++hw->waits;
    return 0;
}

static void test_runtime_and_mmio(fake_hardware *hw, pl_silicon_runtime *runtime,
                                  pl_silicon_mmio_bank *bank) {
    pl_silicon_config config = {
        PL_SILICON_ARCH_SIMULATOR, PL_SILICON_ABI_VERSION,
        UINT32_C(0x1234), UINT64_C(1000000), 16u
    };
    pl_silicon_hal hal = {
        hw, fake_read_cycles, fake_read_mmio, fake_write_mmio,
        fake_barrier, fake_wait
    };
    uint32_t value = 0u;

    assert(pl_silicon_runtime_init(runtime, &config, &hal) == PL_SILICON_OK);
    assert(runtime->portrait.state == PL_SILICON_STATE_RESET_BOOT);
    pl_silicon_mmio_bank_init(bank);
    assert(pl_silicon_mmio_add(bank, "GPIO.OUT", UINT32_C(0x4000), 32u,
        PL_SILICON_ACCESS_READ | PL_SILICON_ACCESS_WRITE,
        UINT32_C(0x0000ffff), PL_SILICON_BARRIER_DMB) == PL_SILICON_OK);
    assert(pl_silicon_mmio_add(bank, "GPIO.BAD", UINT32_C(0x4002), 32u,
        PL_SILICON_ACCESS_READ, 0u, PL_SILICON_BARRIER_NONE) == PL_SILICON_EINVAL);
    assert(pl_silicon_mmio_add(bank, "GPIO.DUP", UINT32_C(0x4000), 32u,
        PL_SILICON_ACCESS_READ, 0u, PL_SILICON_BARRIER_NONE) == PL_SILICON_EINVAL);
    assert(pl_silicon_mmio_add(bank, "GPIO.OVERLAP", UINT32_C(0x4002), 16u,
        PL_SILICON_ACCESS_READ, 0u, PL_SILICON_BARRIER_NONE) == PL_SILICON_EINVAL);
    assert(pl_silicon_mmio_add(bank, "GPIO.RO", UINT32_C(0x4004), 16u,
        PL_SILICON_ACCESS_READ, 1u, PL_SILICON_BARRIER_NONE) == PL_SILICON_EPERMISSION);
    assert(pl_silicon_mmio_add(bank, "GPIO.STATUS", UINT32_C(0x4008), 32u,
        PL_SILICON_ACCESS_READ, 0u, PL_SILICON_BARRIER_DSB) == PL_SILICON_OK);

    hw->mmio_address = UINT32_C(0x4000);
    hw->mmio_width = 32u;
    hw->mmio_value = UINT32_C(0x1234);
    assert(pl_silicon_mmio_read(runtime, bank, "GPIO.OUT", &value) == PL_SILICON_OK);
    assert(value == UINT32_C(0x1234) && hw->reads == 1u && hw->barriers == 2u);
    assert(pl_silicon_mmio_write(runtime, bank, "GPIO.OUT", UINT32_C(0x4321)) == PL_SILICON_OK);
    assert(hw->writes == 1u && hw->mmio_value == UINT32_C(0x4321));
    assert(pl_silicon_mmio_write(runtime, bank, "GPIO.OUT", UINT32_C(0x10000)) == PL_SILICON_EPERMISSION);
    assert(pl_silicon_mmio_read(runtime, bank, "UNKNOWN", &value) == PL_SILICON_EINVAL);

    hw->mmio_address = UINT32_C(0x4008);
    hw->mmio_width = 32u;
    hw->mmio_value = UINT32_C(0x00000005);
    assert(pl_silicon_hardware_guard_check(runtime, bank, "GPIO.STATUS", 5u,
                                           UINT32_C(0x0000f000), &value) == PL_SILICON_OK);
    hw->mmio_value = UINT32_C(0x00001005);
    assert(pl_silicon_hardware_guard_check(runtime, bank, "GPIO.STATUS", 5u,
                                           UINT32_C(0x0000f000), &value) == PL_SILICON_ESTATE);
}

static void test_state_and_cycles(fake_hardware *hw, pl_silicon_runtime *runtime) {
    const pl_silicon_state_rule rules[] = {
        {PL_SILICON_STATE_RESET_BOOT, PL_SILICON_STATE_RUN_HOT, 0u,
         PL_SILICON_FAULT_CRITICAL, 0u, PL_SILICON_CONDITION_HARDWARE_READY},
        {PL_SILICON_STATE_RUN_HOT, PL_SILICON_STATE_STANDBY_WFI, 0u,
         PL_SILICON_FAULT_CRITICAL, 0u, PL_SILICON_CONDITION_IDLE_READY},
        {PL_SILICON_STATE_ANY, PL_SILICON_STATE_SAFE_HALT,
         PL_SILICON_FAULT_CRITICAL, 0u, 0u, PL_SILICON_CONDITION_UNRECOVERABLE}
    };
    pl_silicon_state_program program = {rules, sizeof(rules) / sizeof(rules[0]), 0u};
    pl_silicon_cycle_engine engine;
    uint64_t delta = 0u;
    int full_step = 0;

    assert(pl_silicon_state_transition(runtime, &program, PL_SILICON_STATE_RUN_HOT,
        PL_SILICON_CONDITION_HARDWARE_READY) == PL_SILICON_OK);
    assert(pl_silicon_state_transition(runtime, &program, PL_SILICON_STATE_STANDBY_WFI,
        0u) == PL_SILICON_EPERMISSION);
    assert(pl_silicon_state_transition(runtime, &program, PL_SILICON_STATE_STANDBY_WFI,
        PL_SILICON_CONDITION_IDLE_READY) == PL_SILICON_OK);
    assert(pl_silicon_wait(runtime) == PL_SILICON_OK && hw->waits == 1u);
    assert(pl_silicon_cycle_engine_init(&engine, runtime, 10u, 2u, 1u) == PL_SILICON_OK);
    hw->cycles = 5u;
    engine.previous_cycle = UINT16_C(65530);
    assert(pl_silicon_cycle_tick(&engine, &delta, &full_step) == PL_SILICON_OK);
    assert(delta == 11u && full_step == 1);
    pl_silicon_add_pain(runtime, &engine, PL_SILICON_PAIN_IRQ_JITTER, 20u);
    assert(runtime->portrait.pain_cycles == 20u);
    assert(runtime->portrait.last_pain_cause == PL_SILICON_PAIN_IRQ_JITTER);
    assert(runtime->portrait.pain_incidents == 1u);
    assert(engine.slowmo_remaining_steps == 32u);
    hw->cycles = 6u;
    assert(pl_silicon_cycle_tick(&engine, &delta, &full_step) == PL_SILICON_OK);
    assert(delta == 1u && full_step == 1);
    runtime->portrait.fault_flags |= PL_SILICON_FAULT_CRITICAL;
    assert(pl_silicon_state_transition(runtime, &program, PL_SILICON_STATE_SAFE_HALT,
        PL_SILICON_CONDITION_UNRECOVERABLE) == PL_SILICON_OK);
    assert(pl_silicon_wait(runtime) == PL_SILICON_ESTATE);
}

static void test_faults_and_thread_budget(pl_silicon_runtime *runtime) {
    pl_silicon_fault_ring ring;
    pl_silicon_fault_event event = {0}, received = {0};
    pl_silicon_thread_budget thread;
    unsigned i;
    pl_silicon_fault_ring_init(&ring);
    for (i = 0u; i < PL_SILICON_FAULT_RING_CAP; ++i) {
        event.vector = i;
        assert(pl_silicon_fault_ring_push(&ring, &event) == PL_SILICON_OK);
    }
    assert(pl_silicon_fault_ring_push(&ring, &event) == PL_SILICON_EFULL);
    assert(atomic_load(&ring.dropped) == 1u);
    for (i = 0u; i < PL_SILICON_FAULT_RING_CAP; ++i) {
        assert(pl_silicon_fault_ring_pop(&ring, &received) == PL_SILICON_OK);
        assert(received.vector == i);
    }
    assert(pl_silicon_fault_ring_pop(&ring, &received) == PL_SILICON_EMPTY);
    assert(pl_silicon_fault_record(runtime, &ring, 3u, 0xb1a0u, 7u,
        PL_SILICON_FAULT_BUS, 8u) == PL_SILICON_OK);
    assert(pl_silicon_fault_ring_pop(&ring, &received) == PL_SILICON_OK);
    assert(received.vector == 3u && received.payload == 7u);
    assert((runtime->portrait.fault_flags & PL_SILICON_FAULT_BUS) != 0u);

    assert(pl_silicon_thread_budget_init(&thread, 9u, 10u, 5u, 16u) == PL_SILICON_OK);
    assert(pl_silicon_thread_budget_acquire(&thread, 100u, 6u) == PL_SILICON_OK);
    assert(pl_silicon_thread_budget_acquire(&thread, 101u, 5u) == PL_SILICON_ESTATE);
    assert(thread.breathing && thread.breath_cycles_left == 5u);
    assert(pl_silicon_thread_budget_acquire(&thread, 103u, 1u) == PL_SILICON_ESTATE);
    assert(thread.breath_cycles_left == 3u);
    assert(pl_silicon_thread_budget_acquire(&thread, 106u, 1u) == PL_SILICON_OK);
}

static void test_integrity_and_math(void) {
    pl_silicon_determinism_guard guard;
    uint32_t route = 0u;
    pl_silicon_patch_bank patches;
    const uint8_t payload[] = {1u, 2u, 3u, 4u};
    pl_silicon_table_entry entries[] = {{1u, -2}, {3u, 4}};
    uint8_t table[64];
    size_t bytes;
    int32_t adjusted = 0;
    pl_silicon_graph_node nodes[] = {{1u, 1u, 32768, 1u, 7u}, {0u, 0u, 0, 0u, 7u}};

    pl_silicon_determinism_guard_init(&guard);
    assert(pl_silicon_determinism_route(&guard, 1u, 2u, 3u, &route) == PL_SILICON_OK && route == 3u);
    assert(pl_silicon_determinism_route(&guard, 1u, 2u, 4u, &route) == PL_SILICON_EVERIFY && route == 3u);
    assert(guard.nondeterministic_events == 1u);

    assert(pl_silicon_crc16_ccitt((const uint8_t *)"123456789", 9u) == UINT16_C(0x29B1));
    pl_silicon_patch_bank_init(&patches);
    assert(pl_silicon_patch_stage(&patches, 0u, 44u, payload, sizeof(payload)) == PL_SILICON_OK);
    assert(pl_silicon_patch_verify(&patches, 0u) == PL_SILICON_OK);
    patches.slots[0].bytes[0] ^= 1u;
    assert(pl_silicon_patch_verify(&patches, 0u) == PL_SILICON_ECRC);
    assert(pl_silicon_patch_stage(&patches, 1u, 45u, payload,
        PL_SILICON_PATCH_SLOT_BYTES + 1u) == PL_SILICON_EINVAL);

    memset(table, 0, sizeof(table));
    bytes = pl_silicon_table_export(entries, 2u, 9u, table, sizeof(table));
    assert(bytes == 32u && table[0] == 'T' && table[1] == 'C');
    assert(pl_silicon_table_export(entries, 2u, 9u, table, 31u) == 0u);
    assert(pl_silicon_improvise_cycles(100, 99, 10, &adjusted) == PL_SILICON_OK && adjusted == 110);
    assert(pl_silicon_improvise_cycles(INT32_MAX, 2, 10, &adjusted) == PL_SILICON_OK && adjusted == INT32_MAX);
    assert(pl_silicon_qmul_saturating(10, 32768, 15u) == 10);
    assert(pl_silicon_qmul_saturating(INT32_MAX, INT32_MAX, 0u) == INT32_MAX);
    assert(pl_silicon_graph_advance(nodes, 2u, 0u, 1u, 7u) == 1u);
    assert(pl_silicon_graph_advance(nodes, 2u, 0u, 0u, 7u) == 0u);
}

static void test_machine_streams_and_rpr32(fake_hardware *hw,
                                            pl_silicon_runtime *runtime,
                                            pl_silicon_mmio_bank *bank) {
    const uint8_t thumb_ok[] = {0x01u, 0x20u, 0x00u, 0xBFu}; /* MOVS r0,#1; NOP */
    const uint8_t thumb_bad[] = {0x00u, 0xE0u}; /* B +0: control flow is not whitelisted */
    const uint8_t rv_ok[] = {0x93u, 0x00u, 0x50u, 0x00u}; /* addi x1,x0,5 */
    const uint8_t rv_bad[] = {0x63u, 0x00u, 0x00u, 0x00u}; /* beq: control flow */
    pl_silicon_isa_report report;
    uint32_t words[] = {
        UINT32_C(0x10300000), /* MOVI r3,0 */
        UINT32_C(0x16030000), /* LD32 r0,[r3+0] */
        UINT32_C(0x17300000)  /* ST32 [r3+0],r0 */
    };
    pl_silicon_rpr32_program program;
    unsigned reads_before, writes_before;

    assert(pl_silicon_verify_native_stream(PL_SILICON_ARCH_ARM_CORTEX_M,
        PL_SILICON_ISA_THUMB16, 0x2000u, thumb_ok, sizeof(thumb_ok), &report) == PL_SILICON_OK);
    assert(report.instruction_count == 2u && report.bytes_verified == 4u);
    assert(pl_silicon_verify_native_stream(PL_SILICON_ARCH_ARM_CORTEX_M,
        PL_SILICON_ISA_THUMB16, 0x2000u, thumb_bad, sizeof(thumb_bad), &report) == PL_SILICON_EVERIFY);
    assert(pl_silicon_verify_native_stream(PL_SILICON_ARCH_RISCV32,
        PL_SILICON_ISA_RV32I, 0x80000000u, rv_ok, sizeof(rv_ok), &report) == PL_SILICON_OK);
    assert(pl_silicon_verify_native_stream(PL_SILICON_ARCH_RISCV32,
        PL_SILICON_ISA_RV32I, 0x80000000u, rv_bad, sizeof(rv_bad), &report) == PL_SILICON_EVERIFY);
    assert(pl_silicon_verify_native_stream(PL_SILICON_ARCH_X86_64,
        PL_SILICON_ISA_X86_64, 0x1000u, thumb_ok, sizeof(thumb_ok), &report) == PL_SILICON_EUNSUPPORTED);

    memset(&program, 0, sizeof(program));
    program.words = words;
    program.word_count = sizeof(words) / sizeof(words[0]);
    program.registers[3] = UINT32_C(0x4000);
    hw->mmio_address = UINT32_C(0x4000);
    hw->mmio_width = 32u;
    hw->mmio_value = UINT32_C(0x55aa);
    reads_before = hw->reads;
    writes_before = hw->writes;
    assert(pl_silicon_rpr32_step(runtime, bank, &program,
        PL_SILICON_EXEC_VERIFY_ONLY) == PL_SILICON_OK);
    assert(pl_silicon_rpr32_step(runtime, bank, &program,
        PL_SILICON_EXEC_VERIFY_ONLY) == PL_SILICON_OK);
    assert(hw->reads == reads_before && hw->writes == writes_before);
    assert(program.registers[0] == 0u);
    program.pc = 1u;
    assert(pl_silicon_rpr32_step(runtime, bank, &program,
        PL_SILICON_EXEC_RPR32_INTERPRET) == PL_SILICON_OK);
    assert(program.registers[0] == UINT32_C(0x55aa));
    program.pc = 2u;
    assert(pl_silicon_rpr32_step(runtime, bank, &program,
        PL_SILICON_EXEC_RPR32_INTERPRET) == PL_SILICON_OK);
    assert(hw->mmio_value == UINT32_C(0x55aa));
    program.pc = 1u;
    program.registers[3] = UINT32_C(0x4001);
    assert(pl_silicon_rpr32_step(runtime, bank, &program,
        PL_SILICON_EXEC_RPR32_INTERPRET) == PL_SILICON_EPERMISSION);
    program.pc = 0u;
    assert(pl_silicon_rpr32_step(runtime, bank, &program,
        PL_SILICON_EXEC_NATIVE_ASM) == PL_SILICON_EUNSUPPORTED);
}

int main(void) {
    fake_hardware hw = {0};
    pl_silicon_runtime runtime;
    pl_silicon_mmio_bank bank;
    test_runtime_and_mmio(&hw, &runtime, &bank);
    test_state_and_cycles(&hw, &runtime);
    test_faults_and_thread_budget(&runtime);
    test_integrity_and_math();
    test_machine_streams_and_rpr32(&hw, &runtime, &bank);
    puts("Pytyliki silicon runtime tests passed.");
    return 0;
}
