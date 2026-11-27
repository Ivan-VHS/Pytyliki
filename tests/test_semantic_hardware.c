#include "semantic_hardware.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static FILE *open_temporary_stream(void) {
#if defined(_MSC_VER)
    FILE *stream = NULL;
    if (tmpfile_s(&stream) != 0) return NULL;
    return stream;
#else
    return tmpfile();
#endif
}

static const char kinematics_source[] =
    "module kinematics;\n"
    "target host;\n"
    "input distance: cm = 120;\n"
    "input elapsed: s = 4;\n"
    "derive speed: m/s = distance / elapsed;\n"
    "derive travelled: m = speed * elapsed;\n";

static void test_dimensioned_semantics_and_scale_normalization(void) {
    pl_sh_module module;
    pl_sh_error error;
    assert(pl_sh_compile(kinematics_source, "kinematics.shw", &module, &error));
    assert(module.value_count == 8u);
    assert(module.symbol_count == 4u);
    assert(module.last_value == 7u);
    assert(module.values[0].opcode == PL_SH_OP_CONST_F64);
    assert(module.values[0].immediate > 1.199999999 && module.values[0].immediate < 1.200000001);
    assert(module.types[module.values[module.symbols[2].value_index].type_index].dimensions[0] == 1);
    assert(module.types[module.values[module.symbols[2].value_index].type_index].dimensions[2] == -1);
    assert(module.types[module.values[module.symbols[3].value_index].type_index].dimensions[0] == 1);
    assert(module.types[module.values[module.symbols[3].value_index].type_index].dimensions[2] == 0);
}

static void test_dimension_mismatch_is_rejected(void) {
    const char *source =
        "module invalid; target host;\n"
        "input distance: m = 10;\n"
        "input elapsed: s = 2;\n"
        "derive speed: m/s = distance + elapsed;\n";
    pl_sh_module module; pl_sh_error error;
    assert(!pl_sh_compile(source, "invalid.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-E044") == 0);
    assert(error.line == 4u && error.column > 0u);
}

static void test_undeclared_symbol_and_duplicate_name_are_rejected(void) {
    const char *unknown = "module bad; target host; derive x: m = missing;";
    const char *duplicate = "module bad; target host; input x: m = 1; input x: m = 2;";
    pl_sh_module module; pl_sh_error error;
    assert(!pl_sh_compile(unknown, "unknown.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-E032") == 0);
    assert(!pl_sh_compile(duplicate, "duplicate.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-E030") == 0);
}

static void test_effect_allowlist_and_register_access(void) {
    const char *read_ok =
        "module probe; target sim; register status: u32 access read_only;"
        "allow mmio.read; operation status_value: u32 = mmio.read(status);";
    const char *read_denied =
        "module probe; target sim; register status: u32 access read_only;"
        "operation status_value: u32 = mmio.read(status);";
    const char *write_readonly =
        "module probe; target sim; register status: u32 access read_only;"
        "input mask: u32 = 1; allow mmio.write;"
        "operation ack: unit = mmio.write(status, mask);";
    pl_sh_module module; pl_sh_error error;
    assert(pl_sh_compile(read_ok, "read.shw", &module, &error));
    assert(module.inferred_effects == PL_SH_EFFECT_MMIO_READ);
    assert(module.allowed_effects == PL_SH_EFFECT_MMIO_READ);
    FILE *blocked = open_temporary_stream();
    assert(blocked != NULL);
    assert(!pl_sh_write_c17(&module, blocked, &error));
    assert(strcmp(error.code, "RPR-SH-C001") == 0 || strcmp(error.code, "RPR-SH-C003") == 0);
    fclose(blocked);
    assert(!pl_sh_compile(read_denied, "denied.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-E056") == 0);
    assert(!pl_sh_compile(write_readonly, "readonly.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-E053") == 0);
}

static void test_ir_and_reaper_envelopes(void) {
    pl_sh_module module; pl_sh_error error; FILE *stream; char buffer[16384]; size_t count;
    assert(pl_sh_compile(kinematics_source, "kinematics.shw", &module, &error));
    stream = open_temporary_stream(); assert(stream != NULL);
    assert(pl_sh_write_ir_json(&module, stream));
    rewind(stream); count=fread(buffer,1u,sizeof(buffer)-1u,stream); buffer[count]='\0';
    assert(strstr(buffer, PL_SH_SCHEMA) != NULL);
    assert(strstr(buffer, "\"dimensions\": [1, 0, -1") != NULL);
    fclose(stream);
    stream = open_temporary_stream(); assert(stream != NULL);
    assert(pl_sh_write_reaper_ir_json(&module, stream));
    rewind(stream); count=fread(buffer,1u,sizeof(buffer)-1u,stream); buffer[count]='\0';
    assert(strstr(buffer, PL_SH_REAPER_SCHEMA) != NULL);
    assert(strstr(buffer, "\"span\"") != NULL);
    fclose(stream);
}

static void test_c17_emission_is_pure_and_compilable(void) {
    pl_sh_module module; pl_sh_error error; FILE *stream; char buffer[8192]; size_t count;
    assert(pl_sh_compile(kinematics_source, "kinematics.shw", &module, &error));
    stream=open_temporary_stream(); assert(stream != NULL);
    assert(pl_sh_write_c17(&module,stream,&error));
    rewind(stream); count=fread(buffer,1u,sizeof(buffer)-1u,stream); buffer[count]='\0';
    assert(strstr(buffer,"double pytyliki_model(void)")!=NULL);
    assert(strstr(buffer,"v0 = 1.2")!=NULL);
    assert(strstr(buffer,"return v7")!=NULL);
    fclose(stream);
}

static void test_constant_zero_division_and_tampered_hir_are_rejected(void) {
    const char *zero = "module bad; target host; input zero: s = 0; input distance: m = 5; derive bad_rate: m/s = distance / zero;";
    pl_sh_module module; pl_sh_error error;
    assert(!pl_sh_compile(zero, "zero.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-V027") == 0);
    assert(pl_sh_compile(kinematics_source, "kinematics.shw", &module, &error));
    module.values[4].lhs = 4u;
    assert(!pl_sh_verify(&module, &error));
    assert(strcmp(error.code, "RPR-SH-V003") == 0);
}

static void test_fixed_width_integer_and_negative_unit_initializers(void) {
    const char *fractional = "module bad; target host; input mask: u32 = 1.5;";
    const char *overflow = "module bad; target host; input mask: u32 = 4294967296;";
    const char *negative_quantity = "module sample; target host; input offset: cm = -120;";
    pl_sh_module module; pl_sh_error error;
    assert(!pl_sh_compile(fractional, "fractional.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-E049") == 0);
    assert(!pl_sh_compile(overflow, "overflow.shw", &module, &error));
    assert(strcmp(error.code, "RPR-SH-E049") == 0);
    assert(pl_sh_compile(negative_quantity, "negative.shw", &module, &error));
    {
        uint32_t literal = module.last_value;
        if (module.values[literal].opcode == PL_SH_OP_NEG) literal = module.values[literal].lhs;
        assert(module.values[literal].immediate < -1.199999999 &&
               module.values[literal].immediate > -1.200000001);
    }
}

static void test_malformed_source_and_limits(void) {
    pl_sh_module module; pl_sh_error error;
    assert(!pl_sh_compile("module bad target host;", "bad.shw", &module, &error));
    assert(error.code[0] != '\0');
    assert(!pl_sh_compile(NULL, "bad.shw", &module, &error));
    assert(!pl_sh_verify(NULL, &error));
}

int main(void) {
    test_dimensioned_semantics_and_scale_normalization();
    test_dimension_mismatch_is_rejected();
    test_undeclared_symbol_and_duplicate_name_are_rejected();
    test_effect_allowlist_and_register_access();
    test_ir_and_reaper_envelopes();
    test_c17_emission_is_pure_and_compilable();
    test_constant_zero_division_and_tampered_hir_are_rejected();
    test_fixed_width_integer_and_negative_unit_initializers();
    test_malformed_source_and_limits();
    puts("Pytyliki semantic hardware compiler tests passed");
    return 0;
}
