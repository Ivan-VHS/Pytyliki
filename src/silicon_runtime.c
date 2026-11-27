#include "silicon_runtime.h"

#include <limits.h>
#include <string.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

static uint64_t width_mask(uint8_t bits) {
    if (bits >= 64u) return UINT64_MAX;
    if (bits == 0u) return 0u;
    return (UINT64_C(1) << bits) - UINT64_C(1);
}

static uint64_t cycle_delta(uint64_t now, uint64_t previous, uint8_t width_bits) {
    return (now - previous) & width_mask(width_bits);
}

static uint64_t saturating_add_u64(uint64_t left, uint64_t right) {
    return UINT64_MAX - left < right ? UINT64_MAX : left + right;
}

static uint32_t width_mask_u32(uint8_t bits) {
    return bits == 32u ? UINT32_MAX : (uint32_t)((UINT32_C(1) << bits) - 1u);
}

static int valid_cycle_width(uint8_t bits) {
    return bits == 16u || bits == 32u || bits == 64u;
}

static int valid_barrier(pl_silicon_barrier kind) {
    return kind >= PL_SILICON_BARRIER_NONE && kind <= PL_SILICON_BARRIER_FENCE;
}

static int apply_barrier(pl_silicon_runtime *runtime, pl_silicon_barrier kind) {
    if (kind == PL_SILICON_BARRIER_NONE) return PL_SILICON_OK;
    if (!runtime || !runtime->hal.barrier) return PL_SILICON_EUNSUPPORTED;
    return runtime->hal.barrier(runtime->hal.context, kind) == 0
        ? PL_SILICON_OK : PL_SILICON_EUNSUPPORTED;
}

int pl_silicon_runtime_init(pl_silicon_runtime *runtime,
                            const pl_silicon_config *config,
                            const pl_silicon_hal *hal) {
    if (!runtime || !config || !hal ||
        config->architecture < PL_SILICON_ARCH_UNKNOWN ||
        config->architecture > PL_SILICON_ARCH_SIMULATOR ||
        !valid_cycle_width(config->cycle_width_bits)) {
        return PL_SILICON_EINVAL;
    }
    memset(runtime, 0, sizeof(*runtime));
    runtime->config = *config;
    runtime->hal = *hal;
    runtime->portrait.kernel_version = config->kernel_version;
    runtime->portrait.architecture = config->architecture;
    runtime->portrait.cpuid_or_marchid = config->cpuid_or_marchid;
    runtime->portrait.state = PL_SILICON_STATE_RESET_BOOT;
    runtime->portrait.cycle_hz = config->cycle_hz;
    return PL_SILICON_OK;
}

pl_silicon_arch pl_silicon_native_architecture(void) {
#if defined(__arm__) && defined(__ARM_ARCH_PROFILE) && (__ARM_ARCH_PROFILE == 'M')
    return PL_SILICON_ARCH_ARM_CORTEX_M;
#elif defined(__riscv) && (__riscv_xlen == 32)
    return PL_SILICON_ARCH_RISCV32;
#elif defined(_M_X64) || defined(__x86_64__)
    return PL_SILICON_ARCH_X86_64;
#elif defined(_M_IX86) || defined(__i386__)
    return PL_SILICON_ARCH_X86;
#else
    return PL_SILICON_ARCH_UNKNOWN;
#endif
}

int pl_silicon_native_barrier(pl_silicon_barrier kind) {
    if (!valid_barrier(kind)) return PL_SILICON_EINVAL;
    if (kind == PL_SILICON_BARRIER_NONE) return PL_SILICON_OK;
#if defined(__arm__) && (defined(__GNUC__) || defined(__clang__))
    switch (kind) {
    case PL_SILICON_BARRIER_DMB: __asm__ volatile("dmb sy" ::: "memory"); return PL_SILICON_OK;
    case PL_SILICON_BARRIER_DSB: __asm__ volatile("dsb sy" ::: "memory"); return PL_SILICON_OK;
    case PL_SILICON_BARRIER_ISB: __asm__ volatile("isb sy" ::: "memory"); return PL_SILICON_OK;
    case PL_SILICON_BARRIER_FENCE: __asm__ volatile("dmb sy" ::: "memory"); return PL_SILICON_OK;
    default: break;
    }
#elif defined(__riscv) && (defined(__GNUC__) || defined(__clang__))
    switch (kind) {
    case PL_SILICON_BARRIER_DMB: __asm__ volatile("fence rw, rw" ::: "memory"); return PL_SILICON_OK;
    case PL_SILICON_BARRIER_DSB: __asm__ volatile("fence iorw, iorw" ::: "memory"); return PL_SILICON_OK;
    case PL_SILICON_BARRIER_FENCE: __asm__ volatile("fence iorw, iorw" ::: "memory"); return PL_SILICON_OK;
    case PL_SILICON_BARRIER_ISB:
#if defined(__riscv_zifencei)
        __asm__ volatile("fence.i" ::: "memory"); return PL_SILICON_OK;
#else
        return PL_SILICON_EUNSUPPORTED;
#endif
    default: break;
    }
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    if (kind == PL_SILICON_BARRIER_DMB || kind == PL_SILICON_BARRIER_DSB ||
        kind == PL_SILICON_BARRIER_FENCE) {
        _mm_mfence();
        _ReadWriteBarrier();
        return PL_SILICON_OK;
    }
    if (kind == PL_SILICON_BARRIER_ISB) {
        int registers[4];
        __cpuid(registers, 0);
        _ReadWriteBarrier();
        return PL_SILICON_OK;
    }
#elif (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
    if (kind == PL_SILICON_BARRIER_DMB || kind == PL_SILICON_BARRIER_DSB ||
        kind == PL_SILICON_BARRIER_FENCE) {
        __asm__ volatile("mfence" ::: "memory");
        return PL_SILICON_OK;
    }
    if (kind == PL_SILICON_BARRIER_ISB) {
        uint32_t eax = 0u, ebx = 0u, ecx = 0u, edx = 0u;
        __asm__ volatile("cpuid" : "+a"(eax), "+b"(ebx),
                         "+c"(ecx), "+d"(edx) :: "memory");
        return PL_SILICON_OK;
    }
#endif
    return PL_SILICON_EUNSUPPORTED;
}

int pl_silicon_native_cycle_read(uint64_t *out_cycles) {
    if (!out_cycles) return PL_SILICON_EINVAL;
#if defined(PL_SILICON_ENABLE_CORTEX_M_DWT) && defined(__arm__) && \
    defined(__ARM_ARCH_PROFILE) && (__ARM_ARCH_PROFILE == 'M')
    *out_cycles = (uint64_t)(*(volatile uint32_t *)(uintptr_t)UINT32_C(0xE0001004));
    return PL_SILICON_OK;
#elif defined(__riscv) && (defined(__GNUC__) || defined(__clang__))
# if __riscv_xlen == 32
    uint32_t high_before, low, high_after;
    do {
        __asm__ volatile("rdcycleh %0" : "=r"(high_before) :: "memory");
        __asm__ volatile("rdcycle %0" : "=r"(low) :: "memory");
        __asm__ volatile("rdcycleh %0" : "=r"(high_after) :: "memory");
    } while (high_before != high_after);
    *out_cycles = ((uint64_t)high_after << 32u) | (uint64_t)low;
# elif __riscv_xlen == 64
    uint64_t cycles;
    __asm__ volatile("rdcycle %0" : "=r"(cycles) :: "memory");
    *out_cycles = cycles;
# else
    return PL_SILICON_EUNSUPPORTED;
# endif
    return PL_SILICON_OK;
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    _mm_lfence();
    *out_cycles = (uint64_t)__rdtsc();
    _mm_lfence();
    return PL_SILICON_OK;
#elif (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
    uint32_t low, high;
    __asm__ volatile("lfence\n\trdtsc\n\tlfence"
                     : "=a"(low), "=d"(high) :: "memory");
    *out_cycles = ((uint64_t)high << 32u) | (uint64_t)low;
    return PL_SILICON_OK;
#else
    return PL_SILICON_EUNSUPPORTED;
#endif
}

int pl_silicon_cortex_m_dwt_enable(void) {
#if defined(PL_SILICON_ENABLE_CORTEX_M_DWT) && defined(__arm__) && \
    defined(__ARM_ARCH_PROFILE) && (__ARM_ARCH_PROFILE == 'M')
    volatile uint32_t *demcr = (volatile uint32_t *)(uintptr_t)UINT32_C(0xE000EDFC);
    volatile uint32_t *dwt_ctrl = (volatile uint32_t *)(uintptr_t)UINT32_C(0xE0001000);
    volatile uint32_t *dwt_cyccnt = (volatile uint32_t *)(uintptr_t)UINT32_C(0xE0001004);
    const uint32_t trace_enable = UINT32_C(1) << 24u;
    const uint32_t cycle_enable = UINT32_C(1);
    const uint32_t no_cycle_counter = UINT32_C(1) << 25u;
    if ((*dwt_ctrl & no_cycle_counter) != 0u) return PL_SILICON_EUNSUPPORTED;
    *demcr |= trace_enable;
    *dwt_cyccnt = 0u;
    *dwt_ctrl |= cycle_enable;
    if (pl_silicon_native_barrier(PL_SILICON_BARRIER_DSB) != PL_SILICON_OK ||
        pl_silicon_native_barrier(PL_SILICON_BARRIER_ISB) != PL_SILICON_OK) {
        return PL_SILICON_EUNSUPPORTED;
    }
    return (*dwt_ctrl & cycle_enable) != 0u ? PL_SILICON_OK : PL_SILICON_EIO;
#else
    return PL_SILICON_EUNSUPPORTED;
#endif
}

int pl_silicon_native_wait_for_interrupt(void) {
#if defined(__arm__) && (defined(__GNUC__) || defined(__clang__))
    __asm__ volatile("dsb sy\n\twfi" ::: "memory");
    return PL_SILICON_OK;
#elif defined(__riscv) && (defined(__GNUC__) || defined(__clang__))
    __asm__ volatile("wfi" ::: "memory");
    return PL_SILICON_OK;
#else
    return PL_SILICON_EUNSUPPORTED;
#endif
}

static int native_read_cycles_cb(void *context, uint64_t *out_cycles) {
    (void)context;
    return pl_silicon_native_cycle_read(out_cycles) == PL_SILICON_OK ? 0 : -1;
}

static int native_barrier_cb(void *context, pl_silicon_barrier kind) {
    (void)context;
    return pl_silicon_native_barrier(kind) == PL_SILICON_OK ? 0 : -1;
}

#if defined(__arm__) || defined(__riscv)
static int native_wait_cb(void *context) {
    (void)context;
    return pl_silicon_native_wait_for_interrupt() == PL_SILICON_OK ? 0 : -1;
}
#endif

#if defined(PL_SILICON_ENABLE_DIRECT_MMIO)
static int native_mmio_read_cb(void *context, uint32_t address,
                               uint8_t width_bits, uint32_t *out_value) {
    (void)context;
    if (!out_value || address == 0u ||
        (width_bits != 8u && width_bits != 16u && width_bits != 32u) ||
        (address % (uint32_t)(width_bits / 8u)) != 0u) return -1;
    if (width_bits == 8u) *out_value = *(volatile uint8_t *)(uintptr_t)address;
    else if (width_bits == 16u) *out_value = *(volatile uint16_t *)(uintptr_t)address;
    else *out_value = *(volatile uint32_t *)(uintptr_t)address;
    return 0;
}

static int native_mmio_write_cb(void *context, uint32_t address,
                                uint8_t width_bits, uint32_t value) {
    (void)context;
    if (address == 0u ||
        (width_bits != 8u && width_bits != 16u && width_bits != 32u) ||
        (address % (uint32_t)(width_bits / 8u)) != 0u ||
        (width_bits < 32u && (value & ~width_mask_u32(width_bits)) != 0u)) return -1;
    if (width_bits == 8u) *(volatile uint8_t *)(uintptr_t)address = (uint8_t)value;
    else if (width_bits == 16u) *(volatile uint16_t *)(uintptr_t)address = (uint16_t)value;
    else *(volatile uint32_t *)(uintptr_t)address = value;
    return 0;
}
#endif

int pl_silicon_native_hal_init(pl_silicon_hal *hal,
                               pl_silicon_config *config) {
    pl_silicon_arch architecture;
    if (!hal || !config) return PL_SILICON_EINVAL;
    architecture = pl_silicon_native_architecture();
    if (architecture == PL_SILICON_ARCH_UNKNOWN) return PL_SILICON_EUNSUPPORTED;
    memset(hal, 0, sizeof(*hal));
    memset(config, 0, sizeof(*config));
    config->architecture = architecture;
    config->kernel_version = PL_SILICON_ABI_VERSION;
    config->cycle_width_bits = architecture == PL_SILICON_ARCH_ARM_CORTEX_M ? 32u : 64u;
    hal->read_cycles = native_read_cycles_cb;
    hal->barrier = native_barrier_cb;
#if defined(PL_SILICON_ENABLE_DIRECT_MMIO)
    hal->read_mmio = native_mmio_read_cb;
    hal->write_mmio = native_mmio_write_cb;
#endif
#if defined(__arm__) || defined(__riscv)
    hal->wait_for_interrupt = native_wait_cb;
#endif
    return PL_SILICON_OK;
}

void pl_silicon_mmio_bank_init(pl_silicon_mmio_bank *bank) {
    if (bank) memset(bank, 0, sizeof(*bank));
}

static const pl_silicon_mmio_desc *find_mmio_name(const pl_silicon_mmio_bank *bank,
                                                   const char *name) {
    size_t i;
    if (!bank || !name || bank->count > PL_SILICON_MAX_MMIO) return NULL;
    for (i = 0u; i < bank->count; ++i)
        if (strcmp(bank->entries[i].name, name) == 0) return &bank->entries[i];
    return NULL;
}


int pl_silicon_mmio_add(pl_silicon_mmio_bank *bank, const char *name,
                        uint32_t address, uint8_t width_bits,
                        uint8_t access, uint32_t writable_mask,
                        pl_silicon_barrier barrier) {
    size_t length;
    uint32_t mask;
    size_t i;
    uint64_t new_start, new_end;
    pl_silicon_mmio_desc *entry;
    if (!bank || !name || !name[0] || bank->count >= PL_SILICON_MAX_MMIO ||
        (width_bits != 8u && width_bits != 16u && width_bits != 32u) ||
        (access == 0u) || (access & ~(uint8_t)(PL_SILICON_ACCESS_READ | PL_SILICON_ACCESS_WRITE)) != 0u ||
        !valid_barrier(barrier) || address == 0u ||
        (address % (uint32_t)(width_bits / 8u)) != 0u) return PL_SILICON_EINVAL;
    length = strlen(name);
    if (length >= PL_SILICON_NAME_MAX || find_mmio_name(bank, name)) return PL_SILICON_EINVAL;
    mask = width_mask_u32(width_bits);
    if ((writable_mask & ~mask) != 0u ||
        ((access & PL_SILICON_ACCESS_WRITE) == 0u && writable_mask != 0u) ||
        ((access & PL_SILICON_ACCESS_WRITE) != 0u && writable_mask == 0u)) {
        return PL_SILICON_EPERMISSION;
    }
    new_start = address;
    new_end = new_start + (uint64_t)(width_bits / 8u);
    for (i = 0u; i < bank->count; ++i) {
        const pl_silicon_mmio_desc *existing = &bank->entries[i];
        const uint64_t old_start = existing->address;
        const uint64_t old_end = old_start + (uint64_t)(existing->width_bits / 8u);
        if (new_start < old_end && old_start < new_end) return PL_SILICON_EINVAL;
    }
    entry = &bank->entries[bank->count++];
    memset(entry, 0, sizeof(*entry));
    memcpy(entry->name, name, length + 1u);
    entry->address = address;
    entry->width_bits = width_bits;
    entry->access = access;
    entry->writable_mask = writable_mask;
    entry->barrier = barrier;
    return PL_SILICON_OK;
}

int pl_silicon_mmio_read(pl_silicon_runtime *runtime,
                         const pl_silicon_mmio_bank *bank,
                         const char *name, uint32_t *out_value) {
    const pl_silicon_mmio_desc *entry = find_mmio_name(bank, name);
    uint32_t value = 0u;
    if (!runtime || !entry || !out_value) return PL_SILICON_EINVAL;
    if ((entry->access & PL_SILICON_ACCESS_READ) == 0u) return PL_SILICON_EPERMISSION;
    if (!runtime->hal.read_mmio) return PL_SILICON_EUNSUPPORTED;
    if (apply_barrier(runtime, entry->barrier) != PL_SILICON_OK) return PL_SILICON_EUNSUPPORTED;
    if (runtime->hal.read_mmio(runtime->hal.context, entry->address,
                               entry->width_bits, &value) != 0) return PL_SILICON_EIO;
    if (apply_barrier(runtime, entry->barrier) != PL_SILICON_OK) return PL_SILICON_EUNSUPPORTED;
    *out_value = value & width_mask_u32(entry->width_bits);
    return PL_SILICON_OK;
}

int pl_silicon_mmio_write(pl_silicon_runtime *runtime,
                          const pl_silicon_mmio_bank *bank,
                          const char *name, uint32_t value) {
    const pl_silicon_mmio_desc *entry = find_mmio_name(bank, name);
    if (!runtime || !entry) return PL_SILICON_EINVAL;
    if ((entry->access & PL_SILICON_ACCESS_WRITE) == 0u ||
        (value & ~entry->writable_mask) != 0u ||
        (value & ~width_mask_u32(entry->width_bits)) != 0u) return PL_SILICON_EPERMISSION;
    if (!runtime->hal.write_mmio) return PL_SILICON_EUNSUPPORTED;
    if (apply_barrier(runtime, entry->barrier) != PL_SILICON_OK) return PL_SILICON_EUNSUPPORTED;
    if (runtime->hal.write_mmio(runtime->hal.context, entry->address,
                                entry->width_bits, value) != 0) return PL_SILICON_EIO;
    if (apply_barrier(runtime, entry->barrier) != PL_SILICON_OK) return PL_SILICON_EUNSUPPORTED;
    return PL_SILICON_OK;
}

int pl_silicon_hardware_guard_check(pl_silicon_runtime *runtime,
                                    const pl_silicon_mmio_bank *bank,
                                    const char *name,
                                    uint32_t required_ready_mask,
                                    uint32_t forbidden_error_mask,
                                    uint32_t *out_register_value) {
    uint32_t value = 0u;
    int status = pl_silicon_mmio_read(runtime, bank, name, &value);
    if (status != PL_SILICON_OK) return status;
    if (out_register_value) *out_register_value = value;
    return ((value & required_ready_mask) == required_ready_mask &&
            (value & forbidden_error_mask) == 0u)
        ? PL_SILICON_OK : PL_SILICON_ESTATE;
}

int pl_silicon_state_transition(pl_silicon_runtime *runtime,
                                pl_silicon_state_program *program,
                                pl_silicon_state target,
                                uint32_t conditions) {
    size_t i;
    if (!runtime || !program || !program->rules ||
        program->rule_count == 0u || program->rule_count > PL_SILICON_MAX_RULES ||
        target < PL_SILICON_STATE_RESET_BOOT || target > PL_SILICON_STATE_SAFE_HALT) {
        return PL_SILICON_EINVAL;
    }
    if (target == PL_SILICON_STATE_SAFE_HALT &&
        (conditions & PL_SILICON_CONDITION_UNRECOVERABLE) == 0u) return PL_SILICON_EPERMISSION;
    if (target == PL_SILICON_STATE_STANDBY_WFI &&
        (conditions & PL_SILICON_CONDITION_IDLE_READY) == 0u) return PL_SILICON_EPERMISSION;
    for (i = 0u; i < program->rule_count; ++i) {
        const pl_silicon_state_rule *rule = &program->rules[i];
        const int from_matches = rule->from == PL_SILICON_STATE_ANY ||
                                 rule->from == runtime->portrait.state;
        const int to_matches = rule->to == target;
        const int faults_match =
            (runtime->portrait.fault_flags & rule->required_faults) == rule->required_faults &&
            (runtime->portrait.fault_flags & rule->forbidden_faults) == 0u;
        const int pain_matches = runtime->portrait.pain_cycles >= rule->minimum_pain_cycles;
        const int conditions_match =
            (conditions & rule->required_conditions) == rule->required_conditions;
        if (from_matches && to_matches && faults_match && pain_matches && conditions_match) {
            runtime->portrait.state = target;
            ++program->transitions;
            return PL_SILICON_OK;
        }
    }
    return PL_SILICON_ESTATE;
}

int pl_silicon_wait(pl_silicon_runtime *runtime) {
    if (!runtime) return PL_SILICON_EINVAL;
    if (runtime->portrait.state != PL_SILICON_STATE_STANDBY_WFI)
        return PL_SILICON_ESTATE;
    if (!runtime->hal.wait_for_interrupt) return PL_SILICON_EUNSUPPORTED;
    return runtime->hal.wait_for_interrupt(runtime->hal.context) == 0
        ? PL_SILICON_OK : PL_SILICON_EUNSUPPORTED;
}

int pl_silicon_cycle_engine_init(pl_silicon_cycle_engine *engine,
                                 pl_silicon_runtime *runtime,
                                 uint64_t slowmo_threshold_cycles,
                                 uint32_t slowmo_stride,
                                 uint64_t decay_cycles_per_step) {
    uint64_t now;
    if (!engine || !runtime || !runtime->hal.read_cycles || slowmo_stride == 0u ||
        slowmo_threshold_cycles == 0u) return PL_SILICON_EINVAL;
    if (runtime->hal.read_cycles(runtime->hal.context, &now) != 0) return PL_SILICON_EIO;
    memset(engine, 0, sizeof(*engine));
    engine->runtime = runtime;
    engine->previous_cycle = now & width_mask(runtime->config.cycle_width_bits);
    engine->slowmo_threshold_cycles = slowmo_threshold_cycles;
    engine->slowmo_stride = slowmo_stride;
    engine->decay_cycles_per_step = decay_cycles_per_step;
    engine->pain_cycles = runtime->portrait.pain_cycles;
    engine->peak_pain_cycles = runtime->portrait.peak_pain_cycles;
    runtime->portrait.last_cycle = engine->previous_cycle;
    return PL_SILICON_OK;
}

int pl_silicon_cycle_tick(pl_silicon_cycle_engine *engine,
                          uint64_t *out_delta_cycles,
                          int *out_full_step) {
    pl_silicon_runtime *runtime;
    uint64_t now, delta;
    int full_step = 1;
    if (!engine || !engine->runtime) return PL_SILICON_EINVAL;
    runtime = engine->runtime;
    if (!runtime->hal.read_cycles) return PL_SILICON_EUNSUPPORTED;
    if (runtime->hal.read_cycles(runtime->hal.context, &now) != 0) return PL_SILICON_EIO;
    now &= width_mask(runtime->config.cycle_width_bits);
    delta = cycle_delta(now, engine->previous_cycle, runtime->config.cycle_width_bits);
    engine->previous_cycle = now;
    engine->committed_steps = saturating_add_u64(engine->committed_steps, 1u);
    engine->total_cycles = saturating_add_u64(engine->total_cycles, delta);
    if (engine->pain_cycles > engine->decay_cycles_per_step)
        engine->pain_cycles -= engine->decay_cycles_per_step;
    else
        engine->pain_cycles = 0u;
    ++engine->trace_sample_index;
    if (engine->slowmo_remaining_steps > 0u) {
        --engine->slowmo_remaining_steps;
        full_step = (engine->trace_sample_index % engine->slowmo_stride) == 0u;
    }
    runtime->portrait.last_cycle = now;
    runtime->portrait.pain_cycles = engine->pain_cycles;
    if (engine->peak_pain_cycles > runtime->portrait.peak_pain_cycles)
        runtime->portrait.peak_pain_cycles = engine->peak_pain_cycles;
    if (out_delta_cycles) *out_delta_cycles = delta;
    if (out_full_step) *out_full_step = full_step;
    return PL_SILICON_OK;
}

void pl_silicon_add_pain(pl_silicon_runtime *runtime,
                         pl_silicon_cycle_engine *engine,
                         pl_silicon_pain_cause cause,
                         uint64_t cost_cycles) {
    if (!runtime) return;
    runtime->portrait.last_pain_cause = (uint32_t)cause;
    ++runtime->portrait.pain_incidents;
    runtime->portrait.pain_cycles = saturating_add_u64(runtime->portrait.pain_cycles, cost_cycles);
    if (runtime->portrait.pain_cycles > runtime->portrait.peak_pain_cycles)
        runtime->portrait.peak_pain_cycles = runtime->portrait.pain_cycles;
    if (engine) {
        engine->pain_cycles = saturating_add_u64(engine->pain_cycles, cost_cycles);
        if (engine->pain_cycles > engine->peak_pain_cycles)
            engine->peak_pain_cycles = engine->pain_cycles;
        if (engine->pain_cycles >= engine->slowmo_threshold_cycles)
            engine->slowmo_remaining_steps = 32u;
        runtime->portrait.pain_cycles = engine->pain_cycles;
        runtime->portrait.peak_pain_cycles = engine->peak_pain_cycles;
    }
}

void pl_silicon_fault_ring_init(pl_silicon_fault_ring *ring) {
    if (!ring) return;
    memset(ring->events, 0, sizeof(ring->events));
    atomic_init(&ring->head, 0u);
    atomic_init(&ring->tail, 0u);
    atomic_init(&ring->dropped, 0u);
}

int pl_silicon_fault_ring_push(pl_silicon_fault_ring *ring,
                               const pl_silicon_fault_event *event) {
    unsigned head, tail;
    if (!ring || !event) return PL_SILICON_EINVAL;
    head = atomic_load_explicit(&ring->head, memory_order_relaxed);
    tail = atomic_load_explicit(&ring->tail, memory_order_acquire);
    if ((unsigned)(head - tail) >= PL_SILICON_FAULT_RING_CAP) {
        atomic_fetch_add_explicit(&ring->dropped, 1u, memory_order_relaxed);
        return PL_SILICON_EFULL;
    }
    ring->events[head & (PL_SILICON_FAULT_RING_CAP - 1u)] = *event;
    atomic_store_explicit(&ring->head, head + 1u, memory_order_release);
    return PL_SILICON_OK;
}

int pl_silicon_fault_ring_pop(pl_silicon_fault_ring *ring,
                              pl_silicon_fault_event *out_event) {
    unsigned head, tail;
    if (!ring || !out_event) return PL_SILICON_EINVAL;
    tail = atomic_load_explicit(&ring->tail, memory_order_relaxed);
    head = atomic_load_explicit(&ring->head, memory_order_acquire);
    if (tail == head) return PL_SILICON_EMPTY;
    *out_event = ring->events[tail & (PL_SILICON_FAULT_RING_CAP - 1u)];
    atomic_store_explicit(&ring->tail, tail + 1u, memory_order_release);
    return PL_SILICON_OK;
}

int pl_silicon_fault_record(pl_silicon_runtime *runtime,
                            pl_silicon_fault_ring *ring,
                            uint32_t vector, uint32_t event_code,
                            uint32_t payload, uint32_t fault_flags,
                            uint64_t pain_cost_cycles) {
    pl_silicon_fault_event event;
    uint64_t now = 0u;
    if (!runtime || !ring) return PL_SILICON_EINVAL;
    if (runtime->hal.read_cycles && runtime->hal.read_cycles(runtime->hal.context, &now) == 0)
        runtime->portrait.last_cycle = now & width_mask(runtime->config.cycle_width_bits);
    runtime->portrait.fault_flags |= fault_flags;
    runtime->portrait.pain_cycles = saturating_add_u64(runtime->portrait.pain_cycles,
                                                       pain_cost_cycles);
    if (runtime->portrait.pain_cycles > runtime->portrait.peak_pain_cycles)
        runtime->portrait.peak_pain_cycles = runtime->portrait.pain_cycles;
    event.cycle_low = (uint32_t)runtime->portrait.last_cycle;
    event.kernel_version = runtime->portrait.kernel_version;
    event.vector = vector;
    event.event_code = event_code;
    event.payload = payload;
    return pl_silicon_fault_ring_push(ring, &event);
}

int pl_silicon_thread_budget_init(pl_silicon_thread_budget *thread,
                                  uint32_t thread_id,
                                  uint64_t quota_cycles,
                                  uint64_t breath_cycles,
                                  uint8_t cycle_width_bits) {
    if (!thread || quota_cycles == 0u || !valid_cycle_width(cycle_width_bits))
        return PL_SILICON_EINVAL;
    memset(thread, 0, sizeof(*thread));
    thread->thread_id = thread_id;
    thread->quota_cycles = quota_cycles;
    thread->breath_cycles = breath_cycles;
    thread->cycle_width_bits = cycle_width_bits;
    return PL_SILICON_OK;
}

int pl_silicon_thread_budget_acquire(pl_silicon_thread_budget *thread,
                                     uint64_t now_cycle,
                                     uint64_t step_cost_cycles) {
    if (!thread || step_cost_cycles == 0u || step_cost_cycles > thread->quota_cycles)
        return PL_SILICON_EINVAL;
    now_cycle &= width_mask(thread->cycle_width_bits);
    if (thread->breathing) {
        const uint64_t elapsed = cycle_delta(now_cycle, thread->previous_cycle,
                                             thread->cycle_width_bits);
        thread->previous_cycle = now_cycle;
        if (elapsed < thread->breath_cycles_left) {
            thread->breath_cycles_left -= elapsed;
            return PL_SILICON_ESTATE;
        }
        thread->breath_cycles_left = 0u;
        thread->breathing = 0u;
        thread->spent_cycles = 0u;
    }
    if (step_cost_cycles > thread->quota_cycles - thread->spent_cycles) {
        thread->breathing = thread->breath_cycles != 0u;
        thread->breath_cycles_left = thread->breath_cycles;
        thread->previous_cycle = now_cycle;
        if (!thread->breathing) thread->spent_cycles = 0u;
        return PL_SILICON_ESTATE;
    }
    thread->spent_cycles += step_cost_cycles;
    thread->previous_cycle = now_cycle;
    return PL_SILICON_OK;
}

void pl_silicon_determinism_guard_init(pl_silicon_determinism_guard *guard) {
    if (guard) memset(guard, 0, sizeof(*guard));
}

int pl_silicon_determinism_route(pl_silicon_determinism_guard *guard,
                                 uint32_t input_hash,
                                 uint32_t state_hash,
                                 uint32_t proposed_route,
                                 uint32_t *out_route) {
    if (!guard || !out_route) return PL_SILICON_EINVAL;
    ++guard->evaluations;
    if (!guard->initialized || guard->input_hash != input_hash ||
        guard->state_hash != state_hash) {
        guard->input_hash = input_hash;
        guard->state_hash = state_hash;
        guard->route_id = proposed_route;
        guard->initialized = 1u;
        *out_route = proposed_route;
        return PL_SILICON_OK;
    }
    if (guard->route_id != proposed_route) {
        ++guard->nondeterministic_events;
        *out_route = guard->route_id;
        return PL_SILICON_EVERIFY;
    }
    *out_route = guard->route_id;
    return PL_SILICON_OK;
}
