#include "silicon_runtime.h"

#include <limits.h>
#include <string.h>

uint16_t pl_silicon_crc16_ccitt(const uint8_t *bytes, size_t byte_count) {
    uint16_t crc = UINT16_C(0xFFFF);
    size_t i;
    if (!bytes && byte_count != 0u) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        unsigned bit;
        crc ^= (uint16_t)((uint16_t)bytes[i] << 8u);
        for (bit = 0u; bit < 8u; ++bit)
            crc = (crc & UINT16_C(0x8000))
                ? (uint16_t)((uint16_t)(crc << 1u) ^ UINT16_C(0x1021))
                : (uint16_t)(crc << 1u);
    }
    return crc;
}

void pl_silicon_patch_bank_init(pl_silicon_patch_bank *bank) {
    if (bank) memset(bank, 0, sizeof(*bank));
}

int pl_silicon_patch_stage(pl_silicon_patch_bank *bank, size_t slot_index,
                           uint32_t patch_id, const uint8_t *bytes,
                           size_t byte_count) {
    pl_silicon_patch_slot *slot;
    if (!bank || !bytes || slot_index >= PL_SILICON_PATCH_SLOT_COUNT ||
        byte_count == 0u || byte_count > PL_SILICON_PATCH_SLOT_BYTES) {
        if (bank) ++bank->rejected_count;
        return PL_SILICON_EINVAL;
    }
    slot = &bank->slots[slot_index];
    slot->valid = 0u;
    memset(slot->bytes, 0, sizeof(slot->bytes));
    memcpy(slot->bytes, bytes, byte_count);
    slot->patch_id = patch_id;
    slot->byte_length = (uint16_t)byte_count;
    slot->crc16 = pl_silicon_crc16_ccitt(slot->bytes, byte_count);
    slot->valid = 1u;
    ++bank->staged_count;
    return PL_SILICON_OK;
}

int pl_silicon_patch_verify(const pl_silicon_patch_bank *bank,
                            size_t slot_index) {
    const pl_silicon_patch_slot *slot;
    if (!bank || slot_index >= PL_SILICON_PATCH_SLOT_COUNT) return PL_SILICON_EINVAL;
    slot = &bank->slots[slot_index];
    if (!slot->valid || slot->byte_length == 0u ||
        slot->byte_length > PL_SILICON_PATCH_SLOT_BYTES) return PL_SILICON_ESTATE;
    return pl_silicon_crc16_ccitt(slot->bytes, slot->byte_length) == slot->crc16
        ? PL_SILICON_OK : PL_SILICON_ECRC;
}

static void write_u16_le(uint8_t *destination, uint16_t value) {
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8u);
}

static void write_u32_le(uint8_t *destination, uint32_t value) {
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8u);
    destination[2] = (uint8_t)(value >> 16u);
    destination[3] = (uint8_t)(value >> 24u);
}

static uint32_t read_u32_le(const uint8_t *source) {
    return (uint32_t)source[0] |
           ((uint32_t)source[1] << 8u) |
           ((uint32_t)source[2] << 16u) |
           ((uint32_t)source[3] << 24u);
}

size_t pl_silicon_table_export(const pl_silicon_table_entry *entries,
                               size_t entry_count, uint32_t version,
                               uint8_t *destination, size_t destination_capacity) {
    const size_t header_bytes = 16u;
    size_t body_bytes, total_bytes, i;
    uint16_t crc;
    if (!entries || !destination || entry_count == 0u || entry_count > UINT16_MAX ||
        entry_count > (SIZE_MAX - header_bytes) / 8u) return 0u;
    body_bytes = entry_count * 8u;
    total_bytes = header_bytes + body_bytes;
    if (destination_capacity < total_bytes || body_bytes > UINT32_MAX) return 0u;
    for (i = 0u; i < entry_count; ++i) {
        write_u32_le(destination + header_bytes + i * 8u, entries[i].id);
        write_u32_le(destination + header_bytes + i * 8u + 4u, (uint32_t)entries[i].parameter);
    }
    crc = pl_silicon_crc16_ccitt(destination + header_bytes, body_bytes);
    write_u32_le(destination, PL_SILICON_TABLE_MAGIC);
    write_u32_le(destination + 4u, version);
    write_u16_le(destination + 8u, (uint16_t)entry_count);
    write_u16_le(destination + 10u, crc);
    write_u32_le(destination + 12u, (uint32_t)body_bytes);
    return total_bytes;
}

int pl_silicon_improvise_cycles(int32_t base_cycles, int32_t raw_delta_cycles,
                                int32_t maximum_span_cycles, int32_t *out_cycles) {
    int64_t span, delta, result;
    if (!out_cycles) return PL_SILICON_EINVAL;
    span = maximum_span_cycles < 0 ? -(int64_t)maximum_span_cycles : (int64_t)maximum_span_cycles;
    delta = raw_delta_cycles;
    if (delta > span) delta = span;
    if (delta < -span) delta = -span;
    result = (int64_t)base_cycles + delta;
    if (result > INT32_MAX) result = INT32_MAX;
    if (result < INT32_MIN) result = INT32_MIN;
    *out_cycles = (int32_t)result;
    return PL_SILICON_OK;
}

int32_t pl_silicon_qmul_saturating(int32_t value, int32_t coefficient,
                                  uint8_t shift_right) {
    int64_t product, scaled;
    if (shift_right >= 63u) return 0;
    product = (int64_t)value * (int64_t)coefficient;
    scaled = product / (INT64_C(1) << shift_right);
    if (scaled > INT32_MAX) return INT32_MAX;
    if (scaled < INT32_MIN) return INT32_MIN;
    return (int32_t)scaled;
}

uint16_t pl_silicon_graph_advance(const pl_silicon_graph_node *table,
                                  size_t count, uint16_t node_index,
                                  uint16_t signal_mask,
                                  uint32_t active_kernel_version) {
    const pl_silicon_graph_node *node;
    if (!table || node_index >= count) return 0u;
    node = &table[node_index];
    if (node->kernel_version != active_kernel_version ||
        (node->required_mask & signal_mask) != node->required_mask ||
        node->next_node >= count) return node_index;
    return node->next_node;
}

static int thumb16_is_whitelisted(uint16_t instruction) {
    const uint16_t top5 = (uint16_t)(instruction & UINT16_C(0xF800));
    if (instruction == UINT16_C(0xBF00)) return 1;
    if (top5 == UINT16_C(0x2000) || top5 == UINT16_C(0x2800) ||
        top5 == UINT16_C(0x3000) || top5 == UINT16_C(0x3800) ||
        top5 == UINT16_C(0x0000) || top5 == UINT16_C(0x0800) ||
        top5 == UINT16_C(0x1000)) return 1;
    if ((instruction & UINT16_C(0xFC00)) == UINT16_C(0x4000)) return 1;
    if ((instruction & UINT16_C(0xFE00)) == UINT16_C(0x1800)) return 1;
    return 0;
}

static int rv32i_is_whitelisted(uint32_t instruction) {
    const uint32_t opcode = instruction & UINT32_C(0x7F);
    const uint32_t funct3 = (instruction >> 12u) & UINT32_C(0x7);
    const uint32_t funct7 = (instruction >> 25u) & UINT32_C(0x7F);
    if (opcode == UINT32_C(0x37) || opcode == UINT32_C(0x17)) return 1;
    if (opcode == UINT32_C(0x13)) {
        if (funct3 == 1u) return funct7 == 0u;
        if (funct3 == 5u) return funct7 == 0u || funct7 == UINT32_C(0x20);
        return funct3 == 0u || funct3 == 2u || funct3 == 3u ||
               funct3 == 4u || funct3 == 6u || funct3 == 7u;
    }
    if (opcode == UINT32_C(0x33)) {
        if (funct7 == 0u) return 1;
        if (funct7 == UINT32_C(0x20)) return funct3 == 0u || funct3 == 5u;
    }
    return 0;
}

int pl_silicon_verify_native_stream(pl_silicon_arch architecture,
                                    pl_silicon_isa isa,
                                    uint32_t load_address,
                                    const uint8_t *bytes,
                                    size_t byte_count,
                                    pl_silicon_isa_report *out_report) {
    size_t i, instruction_count;
    if (!bytes || !out_report || byte_count == 0u) return PL_SILICON_EINVAL;
    memset(out_report, 0, sizeof(*out_report));
    if (architecture == PL_SILICON_ARCH_ARM_CORTEX_M && isa == PL_SILICON_ISA_THUMB16) {
        if ((load_address & 1u) != 0u || (byte_count & 1u) != 0u) return PL_SILICON_ERANGE;
        instruction_count = byte_count / 2u;
        for (i = 0u; i < instruction_count; ++i) {
            const uint16_t instruction = (uint16_t)bytes[i * 2u] |
                (uint16_t)((uint16_t)bytes[i * 2u + 1u] << 8u);
            if (!thumb16_is_whitelisted(instruction)) {
                out_report->rejected_instruction = i;
                out_report->instruction_count = i;
                out_report->bytes_verified = i * 2u;
                return PL_SILICON_EVERIFY;
            }
        }
        out_report->instruction_count = instruction_count;
        out_report->bytes_verified = byte_count;
        return PL_SILICON_OK;
    }
    if (architecture == PL_SILICON_ARCH_RISCV32 && isa == PL_SILICON_ISA_RV32I) {
        if ((load_address & 3u) != 0u || (byte_count & 3u) != 0u) return PL_SILICON_ERANGE;
        instruction_count = byte_count / 4u;
        for (i = 0u; i < instruction_count; ++i) {
            const uint32_t instruction = read_u32_le(bytes + i * 4u);
            if (!rv32i_is_whitelisted(instruction)) {
                out_report->rejected_instruction = i;
                out_report->instruction_count = i;
                out_report->bytes_verified = i * 4u;
                return PL_SILICON_EVERIFY;
            }
        }
        out_report->instruction_count = instruction_count;
        out_report->bytes_verified = byte_count;
        return PL_SILICON_OK;
    }
    if ((architecture == PL_SILICON_ARCH_X86 && isa == PL_SILICON_ISA_X86) ||
        (architecture == PL_SILICON_ARCH_X86_64 && isa == PL_SILICON_ISA_X86_64))
        return PL_SILICON_EUNSUPPORTED;
    return PL_SILICON_EINVAL;
}

static const pl_silicon_mmio_desc *find_mmio_address(const pl_silicon_mmio_bank *bank,
                                                      uint32_t address,
                                                      uint8_t width_bits,
                                                      uint8_t required_access) {
    size_t i;
    if (!bank || bank->count > PL_SILICON_MAX_MMIO) return NULL;
    for (i = 0u; i < bank->count; ++i) {
        const pl_silicon_mmio_desc *entry = &bank->entries[i];
        if (entry->address == address && entry->width_bits == width_bits &&
            (entry->access & required_access) == required_access) return entry;
    }
    return NULL;
}

static int rpr32_resolve_mmio(const pl_silicon_mmio_bank *bank,
                              uint32_t base, uint16_t offset,
                              uint8_t access, const pl_silicon_mmio_desc **out_entry) {
    const uint64_t address = (uint64_t)base + (uint64_t)offset;
    const pl_silicon_mmio_desc *entry;
    if (address > UINT32_MAX) return PL_SILICON_ERANGE;
    entry = find_mmio_address(bank, (uint32_t)address, 32u, access);
    if (!entry) return PL_SILICON_EPERMISSION;
    *out_entry = entry;
    return PL_SILICON_OK;
}

int pl_silicon_rpr32_step(pl_silicon_runtime *runtime,
                          const pl_silicon_mmio_bank *bank,
                          pl_silicon_rpr32_program *program,
                          pl_silicon_exec_mode mode) {
    const uint32_t raw = program && program->words && program->pc < program->word_count
        ? program->words[program->pc] : 0u;
    const uint8_t opcode = (uint8_t)(raw >> 24u);
    const uint8_t rd = (uint8_t)((raw >> 20u) & 0x0Fu);
    const uint8_t rs = (uint8_t)((raw >> 16u) & 0x0Fu);
    const uint16_t immediate = (uint16_t)(raw & UINT32_C(0xFFFF));
    const pl_silicon_mmio_desc *mmio = NULL;
    uint32_t address = 0u;
    int status = PL_SILICON_OK;
    if (!program || !program->words || program->pc >= program->word_count)
        return PL_SILICON_EINVAL;
    if (mode == PL_SILICON_EXEC_NATIVE_ASM || mode == PL_SILICON_EXEC_JIT_TRAMPOLINE)
        return PL_SILICON_EUNSUPPORTED;
    if (mode != PL_SILICON_EXEC_VERIFY_ONLY && mode != PL_SILICON_EXEC_RPR32_INTERPRET)
        return PL_SILICON_EINVAL;
    switch (opcode) {
    case 0x10u: /* MOVI */
    case 0x11u: /* ADD */
    case 0x12u: /* SUB */
    case 0x13u: /* ANDI */
    case 0x14u: /* ORI */
    case 0x15u: /* NOP */
        break;
    case 0x16u: /* LD32_MMIO */
        if (!runtime || !bank || (uint64_t)program->registers[rs] + immediate > UINT32_MAX)
            status = PL_SILICON_ERANGE;
        else {
            address = program->registers[rs] + (uint32_t)immediate;
            status = rpr32_resolve_mmio(bank, program->registers[rs], immediate,
                                        PL_SILICON_ACCESS_READ, &mmio);
        }
        break;
    case 0x17u: /* ST32_MMIO */
        if (!runtime || !bank || (uint64_t)program->registers[rd] + immediate > UINT32_MAX)
            status = PL_SILICON_ERANGE;
        else {
            address = program->registers[rd] + (uint32_t)immediate;
            status = rpr32_resolve_mmio(bank, program->registers[rd], immediate,
                                        PL_SILICON_ACCESS_WRITE, &mmio);
            if (status == PL_SILICON_OK &&
                (program->registers[rs] & ~mmio->writable_mask) != 0u)
                status = PL_SILICON_EPERMISSION;
        }
        break;
    default:
        status = PL_SILICON_EVERIFY;
        break;
    }
    if (status != PL_SILICON_OK) {
        ++program->rejected_steps;
        return status;
    }
    if (mode == PL_SILICON_EXEC_RPR32_INTERPRET) {
        switch (opcode) {
        case 0x10u: program->registers[rd] = (uint32_t)immediate; break;
        case 0x11u: program->registers[rd] += program->registers[rs]; break;
        case 0x12u: program->registers[rd] -= program->registers[rs]; break;
        case 0x13u: program->registers[rd] &= (uint32_t)immediate; break;
        case 0x14u: program->registers[rd] |= (uint32_t)immediate; break;
        case 0x15u: break;
        case 0x16u:
            status = pl_silicon_mmio_read(runtime, bank, mmio->name,
                                          &program->registers[rd]);
            break;
        case 0x17u:
            status = pl_silicon_mmio_write(runtime, bank, mmio->name,
                                           program->registers[rs]);
            break;
        default: status = PL_SILICON_EVERIFY; break;
        }
        if (status != PL_SILICON_OK) {
            ++program->rejected_steps;
            return status;
        }
    }
    (void)address;
    ++program->pc;
    ++program->verified_steps;
    return PL_SILICON_OK;
}
