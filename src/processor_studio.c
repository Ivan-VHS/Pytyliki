#include "processor_studio.h"

#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define STUDIO_ARENA_BASE UINT64_C(0x00000000C0000000)
#define STUDIO_ARENA_SIZE UINT64_C(0x00400000)
#define STUDIO_TRAY_SLOT UINT64_C(0x00000000C0010000)
#define STUDIO_TRAY_SIZE UINT64_C(0x0000000000001000)
#define STUDIO_OVERLAY_SLOT UINT64_C(0x00000000C0014000)
#define STUDIO_OVERLAY_SIZE UINT64_C(0x0000000000004000)
#define STUDIO_DOMAIN_TABLE UINT64_C(0x00000000C0020000)
#define STUDIO_DOMAIN_STRIDE UINT32_C(64)
#define STUDIO_DOMAIN_TABLE_SIZE ((uint64_t)PL_PROCESSOR_STUDIO_DOMAIN_COUNT * (uint64_t)STUDIO_DOMAIN_STRIDE)

static const pl_processor_studio_domain_descriptor domain_catalog[PL_PROCESSOR_STUDIO_DOMAIN_COUNT] = {
    { 1u,  PL_PROCESSOR_STUDIO_XOR,      0u,  1u, UINT64_C(0x0101), UINT64_C(0x0008), "Environment", "Machine-oriented", "Processor development studio", "Workspace" },
    { 2u,  PL_PROCESSOR_STUDIO_ROTL_XOR,  3u,  2u, UINT64_C(0x0202), UINT64_C(0x0010), "Space", "Hardware-oriented", "Processor engineering laboratory", "Hardware model" },
    { 3u,  PL_PROCESSOR_STUDIO_ADD,      0u,  3u, UINT64_C(0x0303), UINT64_C(0x0018), "Platform", "Machine-level", "Processor design center", "Architecture" },
    { 4u,  PL_PROCESSOR_STUDIO_XOR,      0u,  4u, UINT64_C(0x0404), UINT64_C(0x0020), "Ecosystem", "Hardware", "Processor technology workshop", "Ecosystem" },
    { 5u,  PL_PROCESSOR_STUDIO_ROTL_XOR,  5u,  5u, UINT64_C(0x0505), UINT64_C(0x0028), "Infrastructure", "Native", "Microprocessor development bureau", "Toolchain" },
    { 6u,  PL_PROCESSOR_STUDIO_ADD,      0u,  6u, UINT64_C(0x0606), UINT64_C(0x0030), "Shell", "Assembler", "Processor systems institute", "Assembly" },
    { 7u,  PL_PROCESSOR_STUDIO_XOR,      0u,  7u, UINT64_C(0x0707), UINT64_C(0x0038), "System", "Processor", "Processor development complex", "System integration" },
    { 8u,  PL_PROCESSOR_STUDIO_ROTL_XOR,  7u,  8u, UINT64_C(0x0808), UINT64_C(0x0040), "Context", "Microprocessor", "Microarchitecture atelier", "Microarchitecture" },
    { 9u,  PL_PROCESSOR_STUDIO_ADD,      0u,  9u, UINT64_C(0x0909), UINT64_C(0x0048), "Layer", "Register-level", "CPU design laboratory", "Register model" },
    { 10u, PL_PROCESSOR_STUDIO_XOR,      0u, 10u, UINT64_C(0x0A0A), UINT64_C(0x0050), "Interface", "Bit-level", "Processor construction center", "Bit operations" },
    { 11u, PL_PROCESSOR_STUDIO_ROTL_XOR,  9u, 11u, UINT64_C(0x0B0B), UINT64_C(0x0058), "Workspace", "Instruction-level", "Microelectronics engineering studio", "Instruction model" },
    { 12u, PL_PROCESSOR_STUDIO_ADD,      0u, 12u, UINT64_C(0x0C0C), UINT64_C(0x0060), "Execution platform", "Address-level", "Hardware architecture workshop", "Address spaces" },
    { 13u, PL_PROCESSOR_STUDIO_XOR,      0u, 13u, UINT64_C(0x0D0D), UINT64_C(0x0068), "Technology base", "Microarchitectural", "Microprocessor systems center", "Technology model" },
    { 14u, PL_PROCESSOR_STUDIO_ROTL_XOR, 11u, 14u, UINT64_C(0x0E0E), UINT64_C(0x0070), "Software platform", "System-level", "Processor modeling laboratory", "Simulation" },
    { 15u, PL_PROCESSOR_STUDIO_ADD,      0u, 15u, UINT64_C(0x0F0F), UINT64_C(0x0078), "Operating environment", "Foundational", "Computing architecture bureau", "System architecture" },
    { 16u, PL_PROCESSOR_STUDIO_XOR,      0u, 16u, UINT64_C(0x1010), UINT64_C(0x0080), "Tooling system", "Foundational", "Hardware programming studio", "Tooling" },
    { 17u, PL_PROCESSOR_STUDIO_ROTL_XOR, 13u, 17u, UINT64_C(0x1111), UINT64_C(0x0088), "Compute environment", "Hardware-oriented", "Computing core engineering center", "Compute core" },
    { 18u, PL_PROCESSOR_STUDIO_ADD,      0u, 18u, UINT64_C(0x1212), UINT64_C(0x0090), "Functional space", "Machine-approximate", "Digital logic laboratory", "Digital logic" },
    { 19u, PL_PROCESSOR_STUDIO_XOR,      0u, 19u, UINT64_C(0x1313), UINT64_C(0x0098), "Hardware/software complex", "Hardware-dependent", "CPU development workshop", "Hardware/software boundary" },
    { 20u, PL_PROCESSOR_STUDIO_ROTL_XOR, 15u, 20u, UINT64_C(0x1414), UINT64_C(0x00A0), "Runtime environment", "Machine-code-adjacent", "Processor technology engineering center", "Runtime" }
};

static void clear_error(pl_processor_studio_error *error) {
    if (error != NULL) {
        error->status = PL_PROCESSOR_STUDIO_OK;
        error->domain_id = 0u;
        error->address = 0u;
        error->message[0] = '\0';
    }
}

static pl_processor_studio_status fail(
    pl_processor_studio_error *error,
    pl_processor_studio_status status,
    uint32_t domain_id,
    uint64_t address,
    const char *message) {
    if (error != NULL) {
        error->status = status;
        error->domain_id = domain_id;
        error->address = address;
        (void)snprintf(error->message, sizeof(error->message), "%s",
                       message != NULL ? message : "processor studio error");
    }
    return status;
}

static int checked_end(uint64_t base, uint64_t size, uint64_t *end) {
    if (size == 0u || base > UINT64_MAX - size) {
        return 0;
    }
    *end = base + size;
    return 1;
}

static int overlaps(uint64_t left_base, uint64_t left_size,
                    uint64_t right_base, uint64_t right_size) {
    uint64_t left_end = 0u;
    uint64_t right_end = 0u;
    if (!checked_end(left_base, left_size, &left_end) ||
        !checked_end(right_base, right_size, &right_end)) {
        return 1;
    }
    return left_base < right_end && right_base < left_end;
}

static int range_inside(uint64_t outer_base, uint64_t outer_size,
                        uint64_t inner_base, uint64_t inner_size) {
    uint64_t outer_end = 0u;
    uint64_t inner_end = 0u;
    return checked_end(outer_base, outer_size, &outer_end) &&
           checked_end(inner_base, inner_size, &inner_end) &&
           inner_base >= outer_base && inner_end <= outer_end;
}

static uint64_t rotate_left_u64(uint64_t value, unsigned int count) {
    count &= 63u;
    if (count == 0u) return value;
    return (value << count) | (value >> (64u - count));
}

static uint8_t bit_parity_u64(uint64_t value) {
    value ^= value >> 32;
    value ^= value >> 16;
    value ^= value >> 8;
    value ^= value >> 4;
    value &= UINT64_C(0xF);
    return (uint8_t)((UINT16_C(0x6996) >> value) & 1u);
}

static uint32_t domain_bit(uint8_t domain_id) {
    return (domain_id >= 1u && domain_id <= PL_PROCESSOR_STUDIO_DOMAIN_COUNT)
        ? (UINT32_C(1) << (domain_id - 1u)) : 0u;
}

static int rights_allowed(const pl_processor_studio *studio,
                          uint32_t rights, uint32_t required) {
    return studio != NULL && (rights & required) == required &&
           (rights & ~studio->config.root_rights) == 0u &&
           (rights & ~PL_PROCESSOR_STUDIO_RIGHT_ALL) == 0u;
}

static void trace_event(pl_processor_studio *studio,
                        pl_processor_studio_trace_event event,
                        uint8_t domain_id, uint32_t source_offset,
                        pl_processor_studio_status status) {
    pl_processor_studio_trace_record *record;
    if (studio == NULL) return;
    record = &studio->trace[studio->trace_head];
    if (studio->trace_sequence < UINT64_MAX) studio->trace_sequence++;
    record->sequence = studio->trace_sequence;
    record->generation = studio->generation;
    record->source_offset = source_offset;
    record->domain_id = domain_id;
    record->event = (uint8_t)event;
    record->status = (int16_t)status;
    studio->trace_head = (studio->trace_head + 1u) % PL_PROCESSOR_STUDIO_MAX_TRACE;
    if (studio->trace_count < PL_PROCESSOR_STUDIO_MAX_TRACE) {
        studio->trace_count++;
    } else if (studio->trace_dropped < UINT32_MAX) {
        studio->trace_dropped++;
    }
}

static void copy_committed_to_pending(pl_processor_studio *studio) {
    memcpy(studio->pending, studio->committed, sizeof(studio->pending));
    studio->pending_registers = studio->committed_registers;
    studio->pending_fold = 0u;
    studio->pending_matrix_fold = 0u;
    studio->pending_sweep_digest = 0u;
    studio->pending_tournament_value = 0u;
    studio->pending_tournament_domain = 0u;
    studio->pending_executed = 0u;
    studio->assignment_count = 0u;
    memset(studio->assignments, 0, sizeof(studio->assignments));
    studio->completed_mask = 0u;
    studio->transaction_mask = 0u;
    studio->planned_mask = 0u;
}

static pl_processor_studio_status append_format(
    char *buffer, size_t capacity, size_t *used, const char *format, ...) {
    va_list args;
    int length;
    if (buffer == NULL || used == NULL || *used >= capacity) {
        return PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL;
    }
    va_start(args, format);
    length = vsnprintf(buffer + *used, capacity - *used, format, args);
    va_end(args);
    if (length < 0 || (size_t)length >= capacity - *used) {
        return PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL;
    }
    *used += (size_t)length;
    return PL_PROCESSOR_STUDIO_OK;
}

static pl_processor_studio_status append_json_string(
    char *buffer, size_t capacity, size_t *used, const char *value) {
    const unsigned char *cursor = (const unsigned char *)(value != NULL ? value : "");
    pl_processor_studio_status status = append_format(buffer, capacity, used, "\"");
    if (status != PL_PROCESSOR_STUDIO_OK) return status;
    while (*cursor != '\0') {
        char escaped[7];
        const char *part = (const char *)cursor;
        size_t length = 1u;
        if (*cursor == '"' || *cursor == '\\') {
            escaped[0] = '\\';
            escaped[1] = (char)*cursor;
            escaped[2] = '\0';
            part = escaped;
            length = 2u;
        } else if (*cursor < 0x20u) {
            (void)snprintf(escaped, sizeof(escaped), "\\u%04x", (unsigned int)*cursor);
            part = escaped;
            length = 6u;
        }
        if (*used + length >= capacity) return PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL;
        memcpy(buffer + *used, part, length);
        *used += length;
        buffer[*used] = '\0';
        cursor++;
    }
    return append_format(buffer, capacity, used, "\"");
}

const pl_processor_studio_domain_descriptor *pl_processor_studio_catalog(size_t *count) {
    if (count != NULL) *count = PL_PROCESSOR_STUDIO_DOMAIN_COUNT;
    return domain_catalog;
}

void pl_processor_studio_default_config(pl_processor_studio_config *config) {
    if (config == NULL) return;
    memset(config, 0, sizeof(*config));
    config->core_id = UINT32_C(0xC020);
    config->root_rights = PL_PROCESSOR_STUDIO_RIGHT_ALL;
    config->worker_count = 4u;
    config->queue_id = PL_PROCESSOR_STUDIO_QUEUE_AXI_BUS;
    config->layout.arena_base = STUDIO_ARENA_BASE;
    config->layout.arena_size = STUDIO_ARENA_SIZE;
    config->layout.tray_slot = STUDIO_TRAY_SLOT;
    config->layout.tray_size = STUDIO_TRAY_SIZE;
    config->layout.overlay_slot = STUDIO_OVERLAY_SLOT;
    config->layout.overlay_size = STUDIO_OVERLAY_SIZE;
    config->layout.domain_table = STUDIO_DOMAIN_TABLE;
    config->layout.domain_table_size = STUDIO_DOMAIN_TABLE_SIZE;
    config->layout.domain_stride = STUDIO_DOMAIN_STRIDE;
}

pl_processor_studio_status pl_processor_studio_validate_layout(
    const pl_processor_studio_layout *layout, pl_processor_studio_error *error) {
    uint64_t arena_end = 0u;
    uint64_t table_required;
    clear_error(error);
    if (layout == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u, "layout is required");
    }
    if ((layout->arena_base & UINT64_C(0xFFF)) != 0u ||
        (layout->tray_slot & UINT64_C(0xFFF)) != 0u ||
        (layout->overlay_slot & UINT64_C(0xFFF)) != 0u ||
        (layout->domain_table & UINT64_C(0x7)) != 0u ||
        layout->domain_stride < sizeof(pl_processor_studio_domain_state_record) ||
        (layout->domain_stride & 7u) != 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_LAYOUT, 0u, 0u,
                    "arena and windows must be aligned; domain stride must be 8-byte aligned and fit a state record");
    }
    if (!checked_end(layout->arena_base, layout->arena_size, &arena_end)) {
        return fail(error, PL_PROCESSOR_STUDIO_ADDRESS_OVERFLOW, 0u, layout->arena_base,
                    "arena range is empty or overflows the 64-bit address space");
    }
    (void)arena_end;
    table_required = (uint64_t)layout->domain_stride *
                     (uint64_t)PL_PROCESSOR_STUDIO_DOMAIN_COUNT;
    if (layout->domain_table_size < table_required) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_LAYOUT, 0u, layout->domain_table,
                    "domain table is smaller than 20 complete slots");
    }
    if (!range_inside(layout->arena_base, layout->arena_size, layout->tray_slot, layout->tray_size) ||
        !range_inside(layout->arena_base, layout->arena_size, layout->overlay_slot, layout->overlay_size) ||
        !range_inside(layout->arena_base, layout->arena_size, layout->domain_table, layout->domain_table_size)) {
        return fail(error, PL_PROCESSOR_STUDIO_OUT_OF_RANGE, 0u, layout->domain_table,
                    "tray, overlay and domain table must be contained in the logical arena");
    }
    if (overlaps(layout->tray_slot, layout->tray_size, layout->overlay_slot, layout->overlay_size) ||
        overlaps(layout->tray_slot, layout->tray_size, layout->domain_table, layout->domain_table_size) ||
        overlaps(layout->overlay_slot, layout->overlay_size, layout->domain_table, layout->domain_table_size)) {
        return fail(error, PL_PROCESSOR_STUDIO_REGION_OVERLAP, 0u, layout->domain_table,
                    "tray, overlay and domain table regions must not overlap");
    }
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_init(
    pl_processor_studio *studio, const pl_processor_studio_config *config,
    pl_processor_studio_error *error) {
    size_t index;
    pl_processor_studio_status status;
    clear_error(error);
    if (studio == NULL || config == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u, "studio and config are required");
    }
    status = pl_processor_studio_validate_layout(&config->layout, error);
    if (status != PL_PROCESSOR_STUDIO_OK) return status;
    if (config->core_id == 0u || config->worker_count == 0u ||
        config->worker_count > PL_PROCESSOR_STUDIO_MAX_WORKERS ||
        config->queue_id != PL_PROCESSOR_STUDIO_QUEUE_AXI_BUS ||
        config->reserved != 0u ||
        (config->root_rights & ~PL_PROCESSOR_STUDIO_RIGHT_ALL) != 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "core id, worker count, logical queue or root rights are invalid");
    }
    memset(studio, 0, sizeof(*studio));
    studio->config = *config;
    studio->generation = 1u;
    for (index = 0u; index < PL_PROCESSOR_STUDIO_DOMAIN_COUNT; index++) {
        studio->committed[index].domain_id = domain_catalog[index].id;
        studio->committed[index].state = PL_PROCESSOR_STUDIO_DOMAIN_READY;
        studio->committed[index].generation = studio->generation;
        studio->pending[index] = studio->committed[index];
    }
    studio->committed_registers.r[0] = 1u;
    studio->committed_registers.r[1] = PL_PROCESSOR_STUDIO_DOMAIN_COUNT;
    studio->committed_registers.planned_jump_target = PL_PROCESSOR_STUDIO_PLANNED_JUMP_TARGET;
    studio->pending_registers = studio->committed_registers;
    studio->committed_report.generation = studio->generation;
    studio->committed_report.worker_count = config->worker_count;
    studio->committed_report.registers = studio->committed_registers;
    studio->committed_report.queue_id = config->queue_id;
    trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_INIT, 0u, 0u, PL_PROCESSOR_STUDIO_OK);
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_begin(
    pl_processor_studio *studio, uint32_t expected_generation,
    uint32_t rights, pl_processor_studio_error *error) {
    clear_error(error);
    if (studio == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u, "studio is required");
    }
    if (!rights_allowed(studio, rights, PL_PROCESSOR_STUDIO_RIGHT_EXECUTE | PL_PROCESSOR_STUDIO_RIGHT_COMMIT)) {
        trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_REJECT, 0u, 0u, PL_PROCESSOR_STUDIO_PERMISSION_DENIED);
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "transaction requires granted execute and commit rights");
    }
    if (studio->transaction_open != 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_TRANSACTION_ALREADY_OPEN, 0u, 0u,
                    "a studio transaction is already open");
    }
    if (expected_generation != studio->generation) {
        return fail(error, PL_PROCESSOR_STUDIO_GENERATION_CONFLICT, 0u, 0u,
                    "expected generation does not match committed studio state");
    }
    if (studio->generation == UINT32_MAX) {
        return fail(error, PL_PROCESSOR_STUDIO_GENERATION_EXHAUSTED, 0u, 0u,
                    "studio generation counter is exhausted");
    }
    copy_committed_to_pending(studio);
    studio->transaction_expected_generation = expected_generation;
    studio->transaction_open = 1u;
    trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_BEGIN, 0u, 0u, PL_PROCESSOR_STUDIO_OK);
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_execute(
    pl_processor_studio *studio, uint32_t domain_mask, uint32_t rights,
    pl_processor_studio_error *error) {
    uint64_t fold = 0u;
    uint64_t matrix_fold = 0u;
    uint64_t sweep_digest;
    uint64_t tournament_value = 0u;
    uint8_t tournament_domain = 0u;
    size_t index;
    clear_error(error);
    if (studio == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u, "studio is required");
    }
    if (!rights_allowed(studio, rights, PL_PROCESSOR_STUDIO_RIGHT_EXECUTE)) {
        trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_REJECT, 0u, 0u, PL_PROCESSOR_STUDIO_PERMISSION_DENIED);
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "execute right was not granted by the root capability ceiling");
    }
    if (studio->transaction_open == 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_NO_TRANSACTION, 0u, 0u,
                    "begin a transaction before executing domains");
    }
    if (studio->pending_executed != 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "this transaction already has an execution result");
    }
    if (studio->planned_mask != 0u && studio->planned_mask != domain_mask) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_DOMAIN_MASK, 0u, 0u,
                    "execution mask must match the transaction's advisory plan");
    }
    if (domain_mask == 0u || (domain_mask & ~PL_PROCESSOR_STUDIO_DOMAIN_MASK) != 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_DOMAIN_MASK, 0u, 0u,
                    "domain mask must select one or more of the 20 known domains");
    }

    memset(&studio->pending_registers, 0, sizeof(studio->pending_registers));
    studio->pending_registers.r[0] = 1u;
    studio->pending_registers.r[1] = PL_PROCESSOR_STUDIO_DOMAIN_COUNT;
    studio->pending_registers.planned_jump_target = PL_PROCESSOR_STUDIO_PLANNED_JUMP_TARGET;

    for (index = 0u; index < PL_PROCESSOR_STUDIO_DOMAIN_COUNT; index++) {
        const pl_processor_studio_domain_descriptor *descriptor = &domain_catalog[index];
        pl_processor_studio_domain_state_record *state = &studio->pending[index];
        uint64_t transformed;
        if ((domain_mask & domain_bit(descriptor->id)) == 0u) continue;

        studio->pending_registers.r[0] = descriptor->id;
        studio->pending_registers.r[2] = descriptor->seed;
        transformed = descriptor->transform == PL_PROCESSOR_STUDIO_ROTL_XOR
            ? rotate_left_u64(descriptor->seed, descriptor->rotation)
            : descriptor->seed;
        if (descriptor->transform == PL_PROCESSOR_STUDIO_ADD) {
            fold += transformed;
        } else {
            fold ^= transformed;
        }
        studio->pending_registers.regit = fold;
        if (descriptor->transform == PL_PROCESSOR_STUDIO_ROTL_XOR) {
            switch (descriptor->id) {
                case 2u: studio->pending_registers.r[3] = transformed; break;
                case 5u: studio->pending_registers.r[4] = transformed; break;
                case 8u: studio->pending_registers.r[5] = transformed; break;
                case 11u: studio->pending_registers.r[6] = transformed; break;
                case 14u: studio->pending_registers.r[7] = transformed; break;
                case 17u: studio->pending_registers.r[3] = transformed; break;
                case 20u: studio->pending_registers.r[4] = transformed; break;
                default: break;
            }
        }
        state->domain_id = descriptor->id;
        state->state = PL_PROCESSOR_STUDIO_DOMAIN_EXECUTED;
        state->generation = studio->transaction_expected_generation + 1u;
        state->transformed_value = transformed;
        state->accumulated_value = studio->pending_registers.regit;
        state->parity = bit_parity_u64(studio->pending_registers.regit);
        state->reserved = 0u;
        state->reserved2 = 0u;
        trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_DOMAIN, descriptor->id,
                    (uint32_t)descriptor->state_slot_offset, PL_PROCESSOR_STUDIO_OK);
    }

    studio->pending_registers.r[5] = studio->pending_registers.r[3] + studio->pending_registers.r[4];
    studio->pending_registers.regit ^= studio->pending_registers.r[5];
    matrix_fold = studio->pending_registers.regit;

    sweep_digest = matrix_fold;
    for (index = 0u; index < PL_PROCESSOR_STUDIO_DOMAIN_COUNT; index++) {
        sweep_digest ^= studio->pending[index].accumulated_value;
    }
    studio->pending_registers.regit = sweep_digest;

    {
        uint64_t round_one = studio->pending_registers.r[3] >= studio->pending_registers.r[4]
            ? studio->pending_registers.r[3] : studio->pending_registers.r[4];
        uint8_t round_one_domain = studio->pending_registers.r[3] >= studio->pending_registers.r[4]
            ? 17u : 20u;
        studio->pending_registers.r[6] = round_one;
        if (studio->pending_registers.r[5] >= round_one) {
            tournament_value = studio->pending_registers.r[5];
            tournament_domain = 0u;
        } else {
            tournament_value = round_one;
            tournament_domain = round_one_domain;
        }
        studio->pending_registers.regit = tournament_value;
    }
    studio->pending_registers.r[1] = studio->pending_registers.regit & UINT64_C(0xFF);
    studio->pending_registers.r[7] = studio->pending_registers.pc;
    studio->pending_registers.flags = studio->pending_registers.regit != 0u ? UINT64_C(1) : UINT64_C(0);

    studio->pending_fold = fold;
    studio->pending_matrix_fold = matrix_fold;
    studio->pending_sweep_digest = sweep_digest;
    studio->pending_tournament_value = tournament_value;
    studio->pending_tournament_domain = tournament_domain;
    studio->transaction_mask = domain_mask;
    studio->completed_mask = domain_mask;
    if (domain_mask == PL_PROCESSOR_STUDIO_DOMAIN_MASK && studio->pending_registers.regit == 0u) {
        studio->pending_executed = 0u;
        studio->completed_mask = 0u;
        studio->transaction_mask = 0u;
        trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_REJECT, 0u, 0u, PL_PROCESSOR_STUDIO_ASSERTION_FAILED);
        return fail(error, PL_PROCESSOR_STUDIO_ASSERTION_FAILED, 0u, 0u,
                    "the full-domain non-zero predicate failed");
    }
    studio->pending_executed = 1u;
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_plan(
    pl_processor_studio *studio, uint32_t domain_mask, uint32_t rights,
    size_t *assignment_count, pl_processor_studio_error *error) {
    size_t index;
    size_t count = 0u;
    clear_error(error);
    if (assignment_count != NULL) *assignment_count = 0u;
    if (studio == NULL || assignment_count == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "studio and assignment_count are required");
    }
    if (!rights_allowed(studio, rights, PL_PROCESSOR_STUDIO_RIGHT_PLAN)) {
        trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_REJECT, 0u, 0u, PL_PROCESSOR_STUDIO_PERMISSION_DENIED);
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "plan right was not granted by the root capability ceiling");
    }
    if (studio->transaction_open == 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_NO_TRANSACTION, 0u, 0u,
                    "begin a transaction before planning work");
    }
    if (domain_mask == 0u || (domain_mask & ~PL_PROCESSOR_STUDIO_DOMAIN_MASK) != 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_DOMAIN_MASK, 0u, 0u,
                    "plan mask must select one or more of the 20 known domains");
    }
    memset(studio->assignments, 0, sizeof(studio->assignments));
    for (index = 0u; index < PL_PROCESSOR_STUDIO_DOMAIN_COUNT; index++) {
        uint8_t domain_id = domain_catalog[index].id;
        if ((domain_mask & domain_bit(domain_id)) == 0u) continue;
        studio->assignments[count].worker_id = (uint8_t)(count % studio->config.worker_count);
        studio->assignments[count].domain_id = domain_id;
        studio->assignments[count].flags = 0u;
        studio->assignments[count].estimated_cost = 1u;
        count++;
    }
    studio->assignment_count = (uint32_t)count;
    studio->planned_mask = domain_mask;
    *assignment_count = count;
    trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_PLAN, 0u, (uint32_t)count, PL_PROCESSOR_STUDIO_OK);
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_commit(
    pl_processor_studio *studio, uint32_t expected_generation,
    uint32_t rights, pl_processor_studio_error *error) {
    size_t index;
    uint32_t next_generation;
    clear_error(error);
    if (studio == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u, "studio is required");
    }
    if (!rights_allowed(studio, rights, PL_PROCESSOR_STUDIO_RIGHT_COMMIT)) {
        trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_REJECT, 0u, 0u, PL_PROCESSOR_STUDIO_PERMISSION_DENIED);
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "commit right was not granted by the root capability ceiling");
    }
    if (studio->transaction_open == 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_NO_TRANSACTION, 0u, 0u,
                    "there is no open transaction to commit");
    }
    if (expected_generation != studio->generation ||
        expected_generation != studio->transaction_expected_generation) {
        return fail(error, PL_PROCESSOR_STUDIO_GENERATION_CONFLICT, 0u, 0u,
                    "commit generation changed while the transaction was open");
    }
    if (studio->pending_executed == 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "execute at least one domain before committing");
    }
    if (studio->generation == UINT32_MAX) {
        return fail(error, PL_PROCESSOR_STUDIO_GENERATION_EXHAUSTED, 0u, 0u,
                    "studio generation counter is exhausted");
    }
    next_generation = studio->generation + 1u;
    for (index = 0u; index < PL_PROCESSOR_STUDIO_DOMAIN_COUNT; index++) {
        if ((studio->transaction_mask & domain_bit(domain_catalog[index].id)) != 0u) {
            studio->pending[index].generation = next_generation;
            studio->pending[index].state = PL_PROCESSOR_STUDIO_DOMAIN_COMMITTED;
            if (studio->pending[index].commit_count < UINT32_MAX) {
                studio->pending[index].commit_count++;
            }
        }
    }
    memcpy(studio->committed, studio->pending, sizeof(studio->committed));
    studio->committed_registers = studio->pending_registers;
    memcpy(studio->committed_assignments, studio->assignments, sizeof(studio->committed_assignments));
    studio->committed_assignment_count = studio->assignment_count;
    studio->generation = next_generation;
    memset(&studio->committed_report, 0, sizeof(studio->committed_report));
    studio->committed_report.generation = next_generation;
    studio->committed_report.requested_domain_mask = studio->transaction_mask;
    studio->committed_report.completed_domain_mask = studio->completed_mask;
    studio->committed_report.committed_domain_mask = studio->transaction_mask;
    studio->committed_report.worker_count = studio->config.worker_count;
    studio->committed_report.queue_id = studio->config.queue_id;
    studio->committed_report.domain_fold = studio->pending_fold;
    studio->committed_report.matrix_fold = studio->pending_matrix_fold;
    studio->committed_report.sweep_digest = studio->pending_sweep_digest;
    studio->committed_report.tournament_value = studio->pending_tournament_value;
    studio->committed_report.tournament_domain = studio->pending_tournament_domain;
    studio->committed_report.low_byte_signature = (uint8_t)studio->pending_tournament_value;
    studio->committed_report.bit_parity = bit_parity_u64(studio->committed_report.low_byte_signature);
    studio->committed_report.predicate_nonzero = (uint8_t)(studio->pending_tournament_value != 0u);
    studio->committed_report.trace_dropped = studio->trace_dropped;
    studio->committed_report.planned_assignment_count = studio->committed_assignment_count;
    studio->committed_report.registers = studio->committed_registers;
    studio->committed_report.physical_memory_accessed = 0u;
    studio->committed_report.mmio_accessed = 0u;
    studio->committed_report.dma_submitted = 0u;
    studio->committed_report.threads_spawned = 0u;
    studio->transaction_open = 0u;
    studio->pending_executed = 0u;
    studio->transaction_expected_generation = 0u;
    trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_COMMIT, 0u, 0u, PL_PROCESSOR_STUDIO_OK);
    studio->committed_report.trace_dropped = studio->trace_dropped;
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_rollback(
    pl_processor_studio *studio, uint32_t rights, pl_processor_studio_error *error) {
    clear_error(error);
    if (studio == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u, "studio is required");
    }
    if (!rights_allowed(studio, rights, PL_PROCESSOR_STUDIO_RIGHT_COMMIT)) {
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "rollback requires commit right");
    }
    if (studio->transaction_open == 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_NO_TRANSACTION, 0u, 0u,
                    "there is no open transaction to roll back");
    }
    copy_committed_to_pending(studio);
    studio->transaction_open = 0u;
    studio->transaction_expected_generation = 0u;
    trace_event(studio, PL_PROCESSOR_STUDIO_TRACE_ROLLBACK, 0u, 0u, PL_PROCESSOR_STUDIO_OK);
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_get_report(
    const pl_processor_studio *studio, uint32_t rights,
    pl_processor_studio_report *report, pl_processor_studio_error *error) {
    clear_error(error);
    if (studio == NULL || report == NULL) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "studio and report are required");
    }
    if (!rights_allowed(studio, rights, PL_PROCESSOR_STUDIO_RIGHT_INSPECT)) {
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "inspect right was not granted by the root capability ceiling");
    }
    *report = studio->committed_report;
    report->trace_dropped = studio->trace_dropped;
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_copy_plan(
    const pl_processor_studio *studio, uint32_t rights,
    pl_processor_studio_assignment *assignments, size_t capacity,
    size_t *assignment_count, pl_processor_studio_error *error) {
    clear_error(error);
    if (studio == NULL || assignment_count == NULL ||
        (assignments == NULL && capacity != 0u)) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "studio, assignment_count and a valid output buffer are required");
    }
    if (!rights_allowed(studio, rights,
                        PL_PROCESSOR_STUDIO_RIGHT_INSPECT | PL_PROCESSOR_STUDIO_RIGHT_PLAN)) {
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "plan inspection requires inspect and plan rights");
    }
    *assignment_count = studio->committed_assignment_count;
    if (capacity < studio->committed_assignment_count) {
        return fail(error, PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL, 0u, 0u,
                    "plan output buffer is smaller than the committed assignment set");
    }
    if (studio->committed_assignment_count != 0u) {
        memcpy(assignments, studio->committed_assignments,
               studio->committed_assignment_count * sizeof(*assignments));
    }
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_copy_trace(
    const pl_processor_studio *studio, uint32_t rights,
    pl_processor_studio_trace_record *records, size_t capacity,
    size_t *record_count, pl_processor_studio_error *error) {
    size_t index;
    uint32_t oldest;
    clear_error(error);
    if (studio == NULL || record_count == NULL ||
        (records == NULL && capacity != 0u)) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "studio, record_count and a valid output buffer are required");
    }
    if (!rights_allowed(studio, rights,
                        PL_PROCESSOR_STUDIO_RIGHT_TRACE | PL_PROCESSOR_STUDIO_RIGHT_INSPECT)) {
        return fail(error, PL_PROCESSOR_STUDIO_PERMISSION_DENIED, 0u, 0u,
                    "trace inspection requires trace and inspect rights");
    }
    *record_count = studio->trace_count;
    if (capacity < studio->trace_count) {
        return fail(error, PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL, 0u, 0u,
                    "trace output buffer is smaller than the retained trace");
    }
    oldest = (studio->trace_head + PL_PROCESSOR_STUDIO_MAX_TRACE -
              studio->trace_count) % PL_PROCESSOR_STUDIO_MAX_TRACE;
    for (index = 0u; index < studio->trace_count; index++) {
        records[index] = studio->trace[(oldest + (uint32_t)index) %
                                       PL_PROCESSOR_STUDIO_MAX_TRACE];
    }
    return PL_PROCESSOR_STUDIO_OK;
}

pl_processor_studio_status pl_processor_studio_report_json(
    const pl_processor_studio *studio, uint32_t rights,
    char *buffer, size_t capacity, size_t *written,
    pl_processor_studio_error *error) {
    pl_processor_studio_report report;
    pl_processor_studio_status status;
    size_t used = 0u;
    size_t index;
    clear_error(error);
    if (buffer == NULL || written == NULL || capacity == 0u) {
        return fail(error, PL_PROCESSOR_STUDIO_INVALID_ARGUMENT, 0u, 0u,
                    "buffer, capacity and written are required");
    }
    *written = 0u;
    buffer[0] = '\0';
    status = pl_processor_studio_get_report(studio, rights, &report, error);
    if (status != PL_PROCESSOR_STUDIO_OK) return status;
#define APPEND(...) do { status = append_format(buffer, capacity, &used, __VA_ARGS__); if (status != PL_PROCESSOR_STUDIO_OK) goto too_small; } while (0)
    APPEND("{\"schema\":\"");
    APPEND("%s", PL_PROCESSOR_STUDIO_SCHEMA);
    APPEND("\",\"contract\":\"");
    APPEND("%s", PL_PROCESSOR_STUDIO_REAPER_CONTRACT);
    APPEND("\",\"abi_major\":%u,\"generation\":%" PRIu32,
           PL_PROCESSOR_STUDIO_ABI_MAJOR, report.generation);
    APPEND(",\"core_id\":%" PRIu32 ",\"domain_count\":%u,\"requested_domain_mask\":%" PRIu32,
           studio->config.core_id, PL_PROCESSOR_STUDIO_DOMAIN_COUNT, report.requested_domain_mask);
    APPEND(",\"completed_domain_mask\":%" PRIu32 ",\"committed_domain_mask\":%" PRIu32,
           report.completed_domain_mask, report.committed_domain_mask);
    APPEND(",\"registers\":{\"r0\":\"0x%016" PRIx64 "\",\"r1\":\"0x%016" PRIx64 "\",\"r2\":\"0x%016" PRIx64 "\",\"r3\":\"0x%016" PRIx64 "\",\"r4\":\"0x%016" PRIx64 "\",\"r5\":\"0x%016" PRIx64 "\",\"r6\":\"0x%016" PRIx64 "\",\"r7\":\"0x%016" PRIx64 "\",\"regit\":\"0x%016" PRIx64 "\",\"pc\":\"0x%016" PRIx64 "\",\"planned_jump_target\":\"0x%016" PRIx64 "\",\"flags\":\"0x%016" PRIx64 "\"},\"worker_plan\":{\"workers\":%" PRIu32 ",\"queue\":\"AXI_BUS\",\"threads_spawned\":false,\"assignments\":[",
           report.registers.r[0], report.registers.r[1], report.registers.r[2], report.registers.r[3],
           report.registers.r[4], report.registers.r[5], report.registers.r[6], report.registers.r[7],
           report.registers.regit, report.registers.pc, report.registers.planned_jump_target,
           report.registers.flags, report.worker_count);
    for (index = 0u; index < studio->committed_assignment_count; index++) {
        const pl_processor_studio_assignment *assignment = &studio->committed_assignments[index];
        if (index != 0u) APPEND(",");
        APPEND("{\"domain_id\":%u,\"worker_id\":%u,\"estimated_cost\":%" PRIu32 "}",
               assignment->domain_id, assignment->worker_id, assignment->estimated_cost);
    }
    APPEND("]},\"fold\":{\"domain\":\"0x%016" PRIx64 "\",\"matrix\":\"0x%016" PRIx64 "\",\"sweep\":\"0x%016" PRIx64 "\",\"tournament\":\"0x%016" PRIx64 "\",\"tournament_domain\":%u,\"low_byte_signature\":%u,\"bit_parity\":%u,\"predicate_nonzero\":%s},\"hardware_effects\":{\"physical_memory_accessed\":false,\"mmio_accessed\":false,\"dma_submitted\":false},\"trace_dropped\":%" PRIu32,
           report.domain_fold, report.matrix_fold, report.sweep_digest,
           report.tournament_value, report.tournament_domain, report.low_byte_signature,
           report.bit_parity, report.predicate_nonzero != 0u ? "true" : "false",
           report.trace_dropped);
    APPEND(",\"domains\":[");
    for (index = 0u; index < PL_PROCESSOR_STUDIO_DOMAIN_COUNT; index++) {
        const pl_processor_studio_domain_descriptor *descriptor = &domain_catalog[index];
        const pl_processor_studio_domain_state_record *state_record = &studio->committed[index];
        if (index != 0u) APPEND(",");
        APPEND("{\"id\":%u,\"seed\":\"0x%04" PRIx64 "\",\"layer\":%u,\"transform\":",
               descriptor->id, descriptor->seed, descriptor->layer);
        status = append_json_string(buffer, capacity, &used,
            pl_processor_studio_transform_name((pl_processor_studio_transform)descriptor->transform));
        if (status != PL_PROCESSOR_STUDIO_OK) goto too_small;
        APPEND(",\"state\":");
        status = append_json_string(buffer, capacity, &used,
            pl_processor_studio_domain_state_name((pl_processor_studio_domain_state)state_record->state));
        if (status != PL_PROCESSOR_STUDIO_OK) goto too_small;
        APPEND(",\"generation\":%" PRIu32 ",\"transformed\":\"0x%016" PRIx64 "\",\"accumulated\":\"0x%016" PRIx64 "\",\"commit_count\":%" PRIu32,
               state_record->generation, state_record->transformed_value,
               state_record->accumulated_value, state_record->commit_count);
        APPEND(",\"environment\":");
        status = append_json_string(buffer, capacity, &used, descriptor->environment);
        if (status != PL_PROCESSOR_STUDIO_OK) goto too_small;
        APPEND(",\"engineering_focus\":");
        status = append_json_string(buffer, capacity, &used, descriptor->engineering_focus);
        if (status != PL_PROCESSOR_STUDIO_OK) goto too_small;
        APPEND(",\"organization\":");
        status = append_json_string(buffer, capacity, &used, descriptor->organization);
        if (status != PL_PROCESSOR_STUDIO_OK) goto too_small;
        APPEND(",\"layer_name\":");
        status = append_json_string(buffer, capacity, &used, descriptor->layer_name);
        if (status != PL_PROCESSOR_STUDIO_OK) goto too_small;
        APPEND("}");
    }
    APPEND("]}");
    *written = used;
#undef APPEND
    return PL_PROCESSOR_STUDIO_OK;
too_small:
#undef APPEND
    buffer[0] = '\0';
    *written = 0u;
    return fail(error, PL_PROCESSOR_STUDIO_OUTPUT_TOO_SMALL, 0u, 0u,
                "JSON output buffer is too small");
}

const char *pl_processor_studio_status_name(pl_processor_studio_status status) {
    static const char *const names[] = {
        "ok", "invalid_argument", "invalid_layout", "address_overflow", "region_overlap",
        "out_of_range", "permission_denied", "invalid_domain_mask", "transaction_already_open",
        "no_transaction", "generation_conflict", "generation_exhausted", "output_too_small",
        "assertion_failed", "internal_error"
    };
    return (unsigned int)status < (sizeof(names) / sizeof(names[0])) ? names[status] : "unknown_status";
}

const char *pl_processor_studio_transform_name(pl_processor_studio_transform transform) {
    switch (transform) {
        case PL_PROCESSOR_STUDIO_XOR: return "xor_u64";
        case PL_PROCESSOR_STUDIO_ADD: return "add_u64";
        case PL_PROCESSOR_STUDIO_ROTL_XOR: return "rotl_u64_then_xor_u64";
        default: return "unknown_transform";
    }
}

const char *pl_processor_studio_domain_state_name(pl_processor_studio_domain_state state) {
    switch (state) {
        case PL_PROCESSOR_STUDIO_DOMAIN_DECLARED: return "declared";
        case PL_PROCESSOR_STUDIO_DOMAIN_READY: return "ready";
        case PL_PROCESSOR_STUDIO_DOMAIN_EXECUTED: return "executed";
        case PL_PROCESSOR_STUDIO_DOMAIN_COMMITTED: return "committed";
        case PL_PROCESSOR_STUDIO_DOMAIN_DISABLED: return "disabled";
        case PL_PROCESSOR_STUDIO_DOMAIN_FAILED: return "failed";
        default: return "unknown_state";
    }
}
