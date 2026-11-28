#include "system_sigil.h"

#include <string.h>

#define ARRAY_COUNT(items) (sizeof(items) / sizeof((items)[0]))

/*
 * These are semantic declarations, not executable implementations.
 * Keep token IDs aligned with the suffix-marker IDs in system_array.c.
 */
static const pl_system_sigil_info sigils[PL_SYSTEM_SIGIL_COUNT] = {
    {PL_SYSTEM_SIGIL_ROTATE_LEFT, "^>", "bits.rotate_left",
     "(Bit<N>, u32) -> Bit<N>; (Logic<N>, Logic<S>) -> Logic<N>",
     "(SystemArray<W,N,S>, u32) -> SystemArrayPlan<W,N,S,RotateLeft>; "
     "(LogicSystemArray<W,N,S>, Logic<S>) -> LogicSystemArrayPlan<RotateLeft>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Rotate within the declared bit width N; N must be nonzero; count is reduced modulo N.",
     0},
    {PL_SYSTEM_SIGIL_NOT_EQUAL, "!==", "compare.not_equal",
     "(T, T) -> bool where T: HardwareComparable",
     "(SystemArray<T>, SystemArray<T>) -> SystemArrayComparePlan<AnyNotEqual>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Operands must have the same comparable type and compatible shape; this is value inequality.",
     0},
    {PL_SYSTEM_SIGIL_ROTATE_RIGHT, ">^", "bits.rotate_right",
     "(Bit<N>, u32) -> Bit<N>; (Logic<N>, Logic<S>) -> Logic<N>",
     "(SystemArray<W,N,S>, u32) -> SystemArrayPlan<W,N,S,RotateRight>; "
     "(LogicSystemArray<W,N,S>, Logic<S>) -> LogicSystemArrayPlan<RotateRight>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Rotate within the declared bit width N; N must be nonzero; count is reduced modulo N.",
     0},
    {PL_SYSTEM_SIGIL_PATTERN_NOT_EQUAL, "!!==", "bits.pattern_not_equal",
     "(Bit<N>, Bit<N>) -> bool",
     "(SystemArray<Bit<N>>, SystemArray<Bit<N>>) -> SystemArrayComparePlan<BitPatternNotEqual>",
     "Pure", "ArchitecturalModel|StructuralLogic|Verification",
     "Compare fixed-width bit patterns; for two-state Bit<N> this has the truth value of !== but preserves pattern intent.",
     0},
    {PL_SYSTEM_SIGIL_VECTOR_LANE_SHIFT_LEFT, "^^>>", "vector.lane_shift_left_zero_fill",
     "(Vector<W,L>, u32) -> Vector<W,L>",
     "(SystemArray<Vector<W,L>>, u32) -> SystemArrayPlan<LaneShiftLeftZeroFill>",
     "Pure", "ArchitecturalModel|StructuralLogic|Verification",
     "Shift each lane independently and fill vacated bits with zero; bits never carry across lane boundaries.",
     0},
    {PL_SYSTEM_SIGIL_VECTOR_LANE_SHIFT_RIGHT, ">>^^", "vector.lane_shift_right_zero_fill",
     "(Vector<W,L>, u32) -> Vector<W,L>",
     "(SystemArray<Vector<W,L>>, u32) -> SystemArrayPlan<LaneShiftRightZeroFill>",
     "Pure", "ArchitecturalModel|StructuralLogic|Verification",
     "Shift each lane independently and fill vacated bits with zero; bits never carry across lane boundaries.",
     0},
    {PL_SYSTEM_SIGIL_ANY_MASKED_BIT_SET, "!^>", "bits.any_masked_bit_set",
     "(Bit<N>, Mask<N>) -> bool",
     "(SystemArray<Bit<N>>, Mask<N>) -> SystemArrayComparePlan<AnyMaskedBitSet>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Return true iff at least one bit selected by the mask is set; widths must match.",
     0},
    {PL_SYSTEM_SIGIL_ALL_MASKED_BITS_MATCH, "^!>", "bits.all_masked_bits_match",
     "(Bit<N>, Mask<N>) -> bool",
     "(SystemArray<Bit<N>>, Mask<N>) -> SystemArrayComparePlan<AllMaskedBitsMatch>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Return true iff every selected bit is set; widths must match; an empty mask is true by definition.",
     0},
    {PL_SYSTEM_SIGIL_XOR, ">!^", "bits.xor",
     "(Bit<N>, Bit<N>) -> Bit<N>",
     "(SystemArray<Bit<N>>, SystemArray<Bit<N>>) -> SystemArrayPlan<Xor>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Bitwise XOR; operands must have equal widths and compatible SystemArray shapes.",
     0},
    {PL_SYSTEM_SIGIL_ROTATE_RIGHT_ASSIGN, ">^=", "bits.rotate_right_assign",
     "(mut Bit<N>, u32) -> Unit",
     "(SystemArrayMutationTransaction<W,N,S>, u32) -> Result<Unit, TransactionFault>",
     "StagedMutation", "Firmware|ArchitecturalModel|Verification",
     "Only valid for a mutable place inside an active explicit SystemArray mutation transaction holding both read and write capabilities; never an implicit device write.",
     1},
    {PL_SYSTEM_SIGIL_ARITHMETIC_SHIFT_RIGHT, "^>>", "bits.arithmetic_shift_right",
     "(SBit<N>, u32) -> SBit<N>; (LogicSigned<N>, Logic<S>) -> LogicSigned<N>",
     "(SystemArray<SBit<N>>, u32) -> SystemArrayPlan<ArithmeticShiftRight>; "
     "(LogicSystemArray<W,N,S>, Logic<S>) -> LogicSystemArrayPlan<ArithmeticShiftRight>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Signed two's-complement shift with sign extension; for count >= N the result is all sign bits.",
     0},
    {PL_SYSTEM_SIGIL_LOGICAL_SHIFT_RIGHT, ">>^", "bits.logical_shift_right",
     "(Bit<N>, u32) -> Bit<N>; (Logic<N>, Logic<S>) -> Logic<N>",
     "(SystemArray<Bit<N>>, u32) -> SystemArrayPlan<LogicalShiftRight>; "
     "(LogicSystemArray<W,N,S>, Logic<S>) -> LogicSystemArrayPlan<LogicalShiftRight>",
     "Pure", "Firmware|ArchitecturalModel|StructuralLogic|Verification",
     "Unsigned logical shift with zero fill; for count >= N the result is zero.",
     0},
    {PL_SYSTEM_SIGIL_LOGIC_EQUAL_WITH_VALIDITY, "^>==!", "logic.equal_with_validity",
     "(Logic<N>, Logic<N>) -> LogicBit",
     "(LogicSystemArray<N>, LogicSystemArray<N>) -> LogicSystemArrayComparePlan<EqualWithValidity>",
     "Pure", "StructuralLogic|Verification",
     "Four-state equality: known equal yields 1, known unequal yields 0, and any unresolved equality yields X; it is not a two-state bool comparison.",
     0}
};

static int set_error(pl_system_array_error *error, size_t offset,
                     const char *message) {
    if (error != NULL) {
        (void)snprintf(error->message, sizeof(error->message), "%s", message);
        error->offset = offset;
    }
    return 0;
}

static const pl_system_sigil_info *info_at(unsigned marker_id) {
    if (marker_id == 0u || marker_id > PL_SYSTEM_SIGIL_COUNT) return NULL;
    return &sigils[marker_id - 1u];
}

size_t pl_system_sigil_catalog(const pl_system_sigil_info **out) {
    if (out == NULL) return 0u;
    *out = sigils;
    return ARRAY_COUNT(sigils);
}

const pl_system_sigil_info *pl_system_sigil_lookup(const char *token) {
    size_t index;
    if (token == NULL) return NULL;
    for (index = 0u; index < ARRAY_COUNT(sigils); ++index) {
        if (strcmp(sigils[index].token, token) == 0) return &sigils[index];
    }
    return NULL;
}

const pl_system_sigil_info *pl_system_sigil_info_for_kind(pl_system_sigil_kind kind) {
    if ((unsigned)kind < 1u || (unsigned)kind > PL_SYSTEM_SIGIL_COUNT)
        return NULL;
    return &sigils[(unsigned)kind - 1u];
}

const char *pl_system_sigil_kind_name(pl_system_sigil_kind kind) {
    const pl_system_sigil_info *info = pl_system_sigil_info_for_kind(kind);
    return info == NULL ? "invalid" : info->token;
}

int pl_system_sigil_parse(const char *source,
                          pl_system_sigil_directive *out,
                          pl_system_array_error *error) {
    const pl_system_sigil_info *info;

    if (out == NULL)
        return set_error(error, 0u, "output directive is required");
    memset(out, 0, sizeof(*out));

    if (!pl_system_array_parse(source, &out->system_array, error)) return 0;
    if (out->system_array.suffix_marker == 0u)
        return set_error(error, out->system_array.source_offset +
                         out->system_array.source_length,
                         "system sigil operator is required as a suffix marker");

    info = info_at(out->system_array.suffix_marker);
    if (info == NULL)
        return set_error(error, out->system_array.source_offset,
                         "system sigil operator is not registered");

    out->kind = info->kind;
    return 1;
}

static int write_json_string(FILE *output, const char *value) {
    const unsigned char *cursor = (const unsigned char *)value;
    if (fputc('"', output) == EOF) return 0;
    while (*cursor != 0u) {
        unsigned char ch = *cursor++;
        if (ch == '"' || ch == '\\') {
            if (fputc('\\', output) == EOF || fputc((int)ch, output) == EOF)
                return 0;
        } else if (ch < 0x20u) {
            if (fprintf(output, "\\u%04x", (unsigned)ch) < 0) return 0;
        } else if (fputc((int)ch, output) == EOF) {
            return 0;
        }
    }
    return fputc('"', output) != EOF;
}

static int write_info_json(const pl_system_sigil_info *info, FILE *output) {
    if (fprintf(output, "{\"id\":%u,\"token\":",
                (unsigned)info->kind) < 0) return 0;
    if (!write_json_string(output, info->token)) return 0;
    if (fputs(",\"semantic_id\":", output) == EOF ||
        !write_json_string(output, info->semantic_id) ||
        fputs(",\"scalar_signature\":", output) == EOF ||
        !write_json_string(output, info->scalar_signature) ||
        fputs(",\"system_array_signature\":", output) == EOF ||
        !write_json_string(output, info->system_array_signature) ||
        fputs(",\"effect_class\":", output) == EOF ||
        !write_json_string(output, info->effect_class) ||
        fputs(",\"applicable_domains\":", output) == EOF ||
        !write_json_string(output, info->applicable_domains) ||
        fputs(",\"safety_contract\":", output) == EOF ||
        !write_json_string(output, info->safety_contract) ||
        fprintf(output, ",\"requires_active_mutation_transaction\":%s}",
                info->requires_active_mutation_transaction ? "true" : "false") < 0)
        return 0;
    return 1;
}

int pl_system_sigil_write_json(const pl_system_sigil_directive *directive,
                               FILE *output) {
    const pl_system_sigil_info *info;
    if (directive == NULL || output == NULL) return 0;
    info = pl_system_sigil_info_for_kind(directive->kind);
    if (info == NULL ||
        directive->system_array.suffix_marker != (unsigned)info->kind ||
        (unsigned)directive->system_array.alias >= PL_SYSTEM_ARRAY_ALIAS_COUNT ||
        memchr(directive->system_array.spelling, '\0',
               sizeof(directive->system_array.spelling)) == NULL)
        return 0;

    if (fprintf(output, "{\"schema\":\"%s\",\"canonical\":",
                PL_SYSTEM_SIGIL_SCHEMA) < 0) return 0;
    if (!write_json_string(output, PL_SYSTEM_SIGIL_CANONICAL_SOURCE)) return 0;
    if (fputs(",\"legacy_ast_name\":", output) == EOF ||
        !write_json_string(output, pl_system_array_canonical_name()) ||
        fputs(",\"alias\":", output) == EOF ||
        !write_json_string(output, pl_system_array_alias_name(directive->system_array.alias)) ||
        fputs(",\"spelling\":", output) == EOF ||
        !write_json_string(output, directive->system_array.spelling) ||
        fputs(",\"operator\":", output) == EOF ||
        !write_info_json(info, output))
        return 0;
    return fputs(",\"overload_resolution_performed\":false,"
                 "\"operator_execution_performed\":false,"
                 "\"hardware_access_performed\":false,"
                 "\"source_execution_performed\":false}\n", output) != EOF;
}

int pl_system_sigil_write_catalog_json(FILE *output) {
    size_t index;
    if (output == NULL) return 0;
    if (fprintf(output, "{\"schema\":\"%s\",\"count\":%u,\"operators\":[",
                PL_SYSTEM_SIGIL_CATALOG_SCHEMA, PL_SYSTEM_SIGIL_COUNT) < 0)
        return 0;
    for (index = 0u; index < ARRAY_COUNT(sigils); ++index) {
        if (index != 0u && fputc(',', output) == EOF) return 0;
        if (!write_info_json(&sigils[index], output)) return 0;
    }
    return fputs("],\"operator_execution_performed\":false,"
                 "\"hardware_access_performed\":false}\n", output) != EOF;
}
