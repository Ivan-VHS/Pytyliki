#ifndef PYTYLIKI_SILICON_RUNTIME_H
#define PYTYLIKI_SILICON_RUNTIME_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PL_SILICON_ABI_VERSION UINT32_C(0x00010000)
#define PL_SILICON_NAME_MAX 40u
#define PL_SILICON_MAX_MMIO 32u
#define PL_SILICON_MAX_RULES 16u
#define PL_SILICON_GPR_COUNT 16u
#define PL_SILICON_FAULT_RING_CAP 32u
#define PL_SILICON_PATCH_SLOT_COUNT 4u
#define PL_SILICON_PATCH_SLOT_BYTES 128u
#define PL_SILICON_TABLE_MAGIC UINT32_C(0x52504354)

typedef enum {
    PL_SILICON_OK = 0,
    PL_SILICON_EMPTY = 1,
    PL_SILICON_EINVAL = -1,
    PL_SILICON_EUNSUPPORTED = -2,
    PL_SILICON_EPERMISSION = -3,
    PL_SILICON_ERANGE = -4,
    PL_SILICON_EIO = -5,
    PL_SILICON_ECRC = -6,
    PL_SILICON_EFULL = -7,
    PL_SILICON_ESTATE = -8,
    PL_SILICON_EVERIFY = -9
} pl_silicon_status;

typedef enum {
    PL_SILICON_ARCH_UNKNOWN = 0,
    PL_SILICON_ARCH_ARM_CORTEX_M,
    PL_SILICON_ARCH_RISCV32,
    PL_SILICON_ARCH_X86,
    PL_SILICON_ARCH_X86_64,
    PL_SILICON_ARCH_SIMULATOR
} pl_silicon_arch;

typedef enum {
    PL_SILICON_BARRIER_NONE = 0,
    PL_SILICON_BARRIER_DMB,
    PL_SILICON_BARRIER_DSB,
    PL_SILICON_BARRIER_ISB,
    PL_SILICON_BARRIER_FENCE
} pl_silicon_barrier;

typedef enum {
    PL_SILICON_STATE_RESET_BOOT = 0,
    PL_SILICON_STATE_STANDBY_WFI,
    PL_SILICON_STATE_RUN_HOT,
    PL_SILICON_STATE_SLOWMO_TRACE,
    PL_SILICON_STATE_FAULT_LOCK,
    PL_SILICON_STATE_SAFE_HALT,
    PL_SILICON_STATE_ANY = 255
} pl_silicon_state;

#define PL_SILICON_FAULT_BUS      (UINT32_C(1) << 0)
#define PL_SILICON_FAULT_MEMORY   (UINT32_C(1) << 1)
#define PL_SILICON_FAULT_WATCHDOG (UINT32_C(1) << 2)
#define PL_SILICON_FAULT_THERMAL  (UINT32_C(1) << 3)
#define PL_SILICON_FAULT_CRITICAL (UINT32_C(1) << 31)

typedef enum {
    PL_SILICON_PAIN_NONE = 0,
    PL_SILICON_PAIN_BUS_WAIT,
    PL_SILICON_PAIN_IRQ_JITTER,
    PL_SILICON_PAIN_DMA_QUEUE,
    PL_SILICON_PAIN_DIE_TEMP
} pl_silicon_pain_cause;

typedef enum {
    PL_SILICON_ACCESS_READ = 1u,
    PL_SILICON_ACCESS_WRITE = 2u
} pl_silicon_access;

typedef enum {
    PL_SILICON_ISA_THUMB16 = 1u,
    PL_SILICON_ISA_RV32I = 2u,
    PL_SILICON_ISA_X86 = 3u,
    PL_SILICON_ISA_X86_64 = 4u
} pl_silicon_isa;

typedef enum {
    PL_SILICON_EXEC_VERIFY_ONLY = 0,
    PL_SILICON_EXEC_RPR32_INTERPRET,
    PL_SILICON_EXEC_JIT_TRAMPOLINE,
    PL_SILICON_EXEC_NATIVE_ASM
} pl_silicon_exec_mode;

typedef struct {
    void *context;
    int (*read_cycles)(void *context, uint64_t *out_cycles);
    int (*read_mmio)(void *context, uint32_t address, uint8_t width_bits,
                     uint32_t *out_value);
    int (*write_mmio)(void *context, uint32_t address, uint8_t width_bits,
                      uint32_t value);
    int (*barrier)(void *context, pl_silicon_barrier kind);
    int (*wait_for_interrupt)(void *context);
} pl_silicon_hal;

typedef struct {
    pl_silicon_arch architecture;
    uint32_t kernel_version;
    uint32_t cpuid_or_marchid;
    uint64_t cycle_hz;
    uint8_t cycle_width_bits;
} pl_silicon_config;

typedef struct {
    uint32_t kernel_version;
    pl_silicon_arch architecture;
    uint32_t cpuid_or_marchid;
    pl_silicon_state state;
    uint64_t last_cycle;
    uint64_t cycle_hz;
    uint64_t pain_cycles;
    uint64_t peak_pain_cycles;
    uint32_t fault_flags;
    uint32_t last_pain_cause;
    uint32_t pain_incidents;
} pl_silicon_portrait;

typedef struct {
    pl_silicon_config config;
    pl_silicon_hal hal;
    pl_silicon_portrait portrait;
} pl_silicon_runtime;

typedef struct {
    char name[PL_SILICON_NAME_MAX];
    uint32_t address;
    uint32_t writable_mask;
    uint8_t width_bits;
    uint8_t access;
    pl_silicon_barrier barrier;
} pl_silicon_mmio_desc;

typedef struct {
    pl_silicon_mmio_desc entries[PL_SILICON_MAX_MMIO];
    size_t count;
} pl_silicon_mmio_bank;

typedef struct {
    pl_silicon_state from;
    pl_silicon_state to;
    uint32_t required_faults;
    uint32_t forbidden_faults;
    uint64_t minimum_pain_cycles;
    uint32_t required_conditions;
} pl_silicon_state_rule;

enum {
    PL_SILICON_CONDITION_IDLE_READY = 1u << 0,
    PL_SILICON_CONDITION_HARDWARE_READY = 1u << 1,
    PL_SILICON_CONDITION_UNRECOVERABLE = 1u << 2
};

typedef struct {
    const pl_silicon_state_rule *rules;
    size_t rule_count;
    uint32_t transitions;
} pl_silicon_state_program;

typedef struct {
    pl_silicon_runtime *runtime;
    uint64_t previous_cycle;
    uint64_t committed_steps;
    uint64_t total_cycles;
    uint64_t pain_cycles;
    uint64_t peak_pain_cycles;
    uint64_t slowmo_threshold_cycles;
    uint64_t decay_cycles_per_step;
    uint32_t slowmo_stride;
    uint32_t slowmo_remaining_steps;
    uint32_t trace_sample_index;
} pl_silicon_cycle_engine;

typedef struct {
    uint32_t cycle_low;
    uint32_t kernel_version;
    uint32_t vector;
    uint32_t event_code;
    uint32_t payload;
} pl_silicon_fault_event;

typedef struct {
    pl_silicon_fault_event events[PL_SILICON_FAULT_RING_CAP];
    atomic_uint head;
    atomic_uint tail;
    atomic_uint dropped;
} pl_silicon_fault_ring;

typedef struct {
    uint32_t thread_id;
    uint64_t quota_cycles;
    uint64_t breath_cycles;
    uint64_t spent_cycles;
    uint64_t breath_cycles_left;
    uint64_t previous_cycle;
    uint8_t cycle_width_bits;
    uint8_t breathing;
} pl_silicon_thread_budget;

typedef struct {
    uint32_t input_hash;
    uint32_t state_hash;
    uint32_t route_id;
    uint32_t evaluations;
    uint32_t nondeterministic_events;
    uint8_t initialized;
} pl_silicon_determinism_guard;

typedef struct {
    uint32_t id;
    int32_t parameter;
} pl_silicon_table_entry;

typedef struct {
    uint32_t patch_id;
    uint16_t byte_length;
    uint16_t crc16;
    uint8_t valid;
    uint8_t bytes[PL_SILICON_PATCH_SLOT_BYTES];
} pl_silicon_patch_slot;

typedef struct {
    pl_silicon_patch_slot slots[PL_SILICON_PATCH_SLOT_COUNT];
    uint32_t staged_count;
    uint32_t rejected_count;
} pl_silicon_patch_bank;

typedef struct {
    size_t instruction_count;
    size_t bytes_verified;
    size_t rejected_instruction;
} pl_silicon_isa_report;

typedef struct {
    const uint32_t *words;
    size_t word_count;
    size_t pc;
    uint32_t registers[PL_SILICON_GPR_COUNT];
    uint32_t verified_steps;
    uint32_t rejected_steps;
} pl_silicon_rpr32_program;

typedef struct {
    uint16_t required_mask;
    uint16_t next_node;
    int32_t coefficient_q;
    uint8_t shift_right;
    uint32_t kernel_version;
} pl_silicon_graph_node;

int pl_silicon_runtime_init(pl_silicon_runtime *runtime,
                            const pl_silicon_config *config,
                            const pl_silicon_hal *hal);
int pl_silicon_native_hal_init(pl_silicon_hal *hal,
                               pl_silicon_config *config);
pl_silicon_arch pl_silicon_native_architecture(void);
int pl_silicon_native_barrier(pl_silicon_barrier kind);
int pl_silicon_native_cycle_read(uint64_t *out_cycles);
int pl_silicon_cortex_m_dwt_enable(void);
int pl_silicon_native_wait_for_interrupt(void);

void pl_silicon_mmio_bank_init(pl_silicon_mmio_bank *bank);
int pl_silicon_mmio_add(pl_silicon_mmio_bank *bank, const char *name,
                        uint32_t address, uint8_t width_bits,
                        uint8_t access, uint32_t writable_mask,
                        pl_silicon_barrier barrier);
int pl_silicon_mmio_read(pl_silicon_runtime *runtime,
                         const pl_silicon_mmio_bank *bank,
                         const char *name, uint32_t *out_value);
int pl_silicon_mmio_write(pl_silicon_runtime *runtime,
                          const pl_silicon_mmio_bank *bank,
                          const char *name, uint32_t value);
int pl_silicon_hardware_guard_check(pl_silicon_runtime *runtime,
                                    const pl_silicon_mmio_bank *bank,
                                    const char *name,
                                    uint32_t required_ready_mask,
                                    uint32_t forbidden_error_mask,
                                    uint32_t *out_register_value);

int pl_silicon_state_transition(pl_silicon_runtime *runtime,
                                pl_silicon_state_program *program,
                                pl_silicon_state target,
                                uint32_t conditions);
int pl_silicon_wait(pl_silicon_runtime *runtime);

int pl_silicon_cycle_engine_init(pl_silicon_cycle_engine *engine,
                                 pl_silicon_runtime *runtime,
                                 uint64_t slowmo_threshold_cycles,
                                 uint32_t slowmo_stride,
                                 uint64_t decay_cycles_per_step);
int pl_silicon_cycle_tick(pl_silicon_cycle_engine *engine,
                          uint64_t *out_delta_cycles,
                          int *out_full_step);
void pl_silicon_add_pain(pl_silicon_runtime *runtime,
                         pl_silicon_cycle_engine *engine,
                         pl_silicon_pain_cause cause,
                         uint64_t cost_cycles);

void pl_silicon_fault_ring_init(pl_silicon_fault_ring *ring);
int pl_silicon_fault_ring_push(pl_silicon_fault_ring *ring,
                               const pl_silicon_fault_event *event);
int pl_silicon_fault_ring_pop(pl_silicon_fault_ring *ring,
                              pl_silicon_fault_event *out_event);
int pl_silicon_fault_record(pl_silicon_runtime *runtime,
                            pl_silicon_fault_ring *ring,
                            uint32_t vector, uint32_t event_code,
                            uint32_t payload, uint32_t fault_flags,
                            uint64_t pain_cost_cycles);

int pl_silicon_thread_budget_init(pl_silicon_thread_budget *thread,
                                  uint32_t thread_id,
                                  uint64_t quota_cycles,
                                  uint64_t breath_cycles,
                                  uint8_t cycle_width_bits);
int pl_silicon_thread_budget_acquire(pl_silicon_thread_budget *thread,
                                     uint64_t now_cycle,
                                     uint64_t step_cost_cycles);

void pl_silicon_determinism_guard_init(pl_silicon_determinism_guard *guard);
int pl_silicon_determinism_route(pl_silicon_determinism_guard *guard,
                                 uint32_t input_hash,
                                 uint32_t state_hash,
                                 uint32_t proposed_route,
                                 uint32_t *out_route);

uint16_t pl_silicon_crc16_ccitt(const uint8_t *bytes, size_t byte_count);
void pl_silicon_patch_bank_init(pl_silicon_patch_bank *bank);
int pl_silicon_patch_stage(pl_silicon_patch_bank *bank, size_t slot_index,
                           uint32_t patch_id, const uint8_t *bytes,
                           size_t byte_count);
int pl_silicon_patch_verify(const pl_silicon_patch_bank *bank,
                            size_t slot_index);
size_t pl_silicon_table_export(const pl_silicon_table_entry *entries,
                               size_t entry_count, uint32_t version,
                               uint8_t *destination, size_t destination_capacity);
int pl_silicon_improvise_cycles(int32_t base_cycles, int32_t raw_delta_cycles,
                                int32_t maximum_span_cycles, int32_t *out_cycles);
int32_t pl_silicon_qmul_saturating(int32_t value, int32_t coefficient,
                                  uint8_t shift_right);
uint16_t pl_silicon_graph_advance(const pl_silicon_graph_node *table,
                                  size_t count, uint16_t node_index,
                                  uint16_t signal_mask,
                                  uint32_t active_kernel_version);

int pl_silicon_verify_native_stream(pl_silicon_arch architecture,
                                    pl_silicon_isa isa,
                                    uint32_t load_address,
                                    const uint8_t *bytes,
                                    size_t byte_count,
                                    pl_silicon_isa_report *out_report);
int pl_silicon_rpr32_step(pl_silicon_runtime *runtime,
                          const pl_silicon_mmio_bank *bank,
                          pl_silicon_rpr32_program *program,
                          pl_silicon_exec_mode mode);

#ifdef __cplusplus
}
#endif

#endif
