#include "system_sigil.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const struct {
    const char *token;
    const char *semantic_id;
    unsigned marker_id;
} cases[PL_SYSTEM_SIGIL_COUNT] = {
    {"^>", "bits.rotate_left", 1u},
    {"!==", "compare.not_equal", 2u},
    {">^", "bits.rotate_right", 3u},
    {"!!==", "bits.pattern_not_equal", 4u},
    {"^^>>", "vector.lane_shift_left_zero_fill", 5u},
    {">>^^", "vector.lane_shift_right_zero_fill", 6u},
    {"!^>", "bits.any_masked_bit_set", 7u},
    {"^!>", "bits.all_masked_bits_match", 8u},
    {">!^", "bits.xor", 9u},
    {">^=", "bits.rotate_right_assign", 10u},
    {"^>>", "bits.arithmetic_shift_right", 11u},
    {">>^", "bits.logical_shift_right", 12u},
    {"^>==!", "logic.equal_with_validity", 13u}
};

static void test_catalog_is_complete_and_stable(void) {
    const pl_system_sigil_info *items = NULL;
    size_t count = pl_system_sigil_catalog(&items);
    size_t index;

    assert(count == PL_SYSTEM_SIGIL_COUNT);
    assert(items != NULL);
    for (index = 0u; index < count; ++index) {
        assert((unsigned)items[index].kind == cases[index].marker_id);
        assert(strcmp(items[index].token, cases[index].token) == 0);
        assert(strcmp(items[index].semantic_id, cases[index].semantic_id) == 0);
        assert(items[index].scalar_signature[0] != '\0');
        assert(items[index].system_array_signature[0] != '\0');
        assert(items[index].effect_class[0] != '\0');
        assert(items[index].applicable_domains[0] != '\0');
        assert(items[index].safety_contract[0] != '\0');
    }
}

static void test_lookup_and_parse_all_operators(void) {
    size_t index;
    for (index = 0u; index < PL_SYSTEM_SIGIL_COUNT; ++index) {
        char input[96];
        pl_system_sigil_directive directive;
        pl_system_array_error error;
        const pl_system_sigil_info *info = pl_system_sigil_lookup(cases[index].token);

        assert(info != NULL);
        assert(info->kind == (pl_system_sigil_kind)cases[index].marker_id);
        (void)snprintf(input, sizeof(input), "{]+/* systemArray %s", cases[index].token);
        assert(pl_system_sigil_parse(input, &directive, &error));
        assert(directive.kind == info->kind);
        assert(directive.system_array.suffix_marker == cases[index].marker_id);
        assert(strcmp(pl_system_array_canonical_name(), "system(array)") == 0);
    }
}

static void test_requires_operator_and_rejects_unknown_sigil(void) {
    pl_system_sigil_directive directive;
    pl_system_array_error error;

    assert(!pl_system_sigil_parse("system(array)", &directive, &error));
    assert(strstr(error.message, "operator is required") != NULL);

    assert(!pl_system_sigil_parse("system(array) ???", &directive, &error));
    assert(error.message[0] != '\0');

    assert(pl_system_sigil_lookup("??") == NULL);
    assert(pl_system_sigil_info_for_kind((pl_system_sigil_kind)0) == NULL);
    assert(pl_system_sigil_info_for_kind((pl_system_sigil_kind)99) == NULL);
}

static void test_mutation_operator_requires_transaction(void) {
    const pl_system_sigil_info *info = pl_system_sigil_lookup(">^=");
    assert(info != NULL);
    assert(info->requires_active_mutation_transaction == 1);
    assert(strcmp(info->effect_class, "StagedMutation") == 0);
    assert(strstr(info->safety_contract, "active explicit") != NULL);
}

static void test_logic_operator_is_not_two_state_bool(void) {
    const pl_system_sigil_info *info = pl_system_sigil_lookup("^>==!");
    assert(info != NULL);
    assert(strcmp(info->effect_class, "Pure") == 0);
    assert(strcmp(info->scalar_signature, "(Logic<N>, Logic<N>) -> LogicBit") == 0);
    assert(strstr(info->safety_contract, "X") != NULL);
}

static void test_ast_reports_no_execution_or_hardware_access(void) {
    pl_system_sigil_directive directive;
    pl_system_array_error error;

    assert(pl_system_sigil_parse("system_array >^=", &directive, &error));
    /* The CLI CTest inspects the emitted JSON; this native test verifies the
       serializer accepts the parsed AST without relying on non-portable streams. */
    assert(pl_system_sigil_write_json(&directive, stdout));
}

static void test_catalog_json(void) {
    /* The catalog CLI CTest verifies the serialized schema and registry content. */
    assert(pl_system_sigil_write_catalog_json(stdout));
}

int main(void) {
    test_catalog_is_complete_and_stable();
    test_lookup_and_parse_all_operators();
    test_requires_operator_and_rejects_unknown_sigil();
    test_mutation_operator_requires_transaction();
    test_logic_operator_is_not_two_state_bool();
    test_ast_reports_no_execution_or_hardware_access();
    test_catalog_json();
    puts("Pytyliki SystemArray sigil registry tests passed");
    return 0;
}
