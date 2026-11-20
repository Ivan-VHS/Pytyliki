#include "qcs.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

static const char *valid_source =
    "[@ domain:irq(vector: 32), budget:120ns, state:! @]\n"
    "func timerTick(counter: UInt64?) {\n"
    "  let next = counter &+ 1\n"
    "  next send IRQQueue.enqueue()\n"
    "}\n"
    "[@ end @]\n";

static void test_valid_frame_and_flow(void) {
    pl_qcs_ast ast = {0};
    pl_qcs_error error = {{0}, 0u, 0u};
    assert(pl_qcs_parse_source(valid_source, &ast, &error));
    assert(ast.frame_count == 1u);
    assert(ast.function_count == 1u);
    assert(ast.flow_count == 1u);
    assert(pl_qcs_domain_known("irq"));
    assert(!pl_qcs_domain_known("madeUp"));
    assert(pl_qcs_domain_count() == 65u);
    pl_qcs_ast_free(&ast);
}

static void test_nested_frames(void) {
    const char *source =
        "[@ domain:irq, state:! @]\n"
        "[@ domain:dma, state:deferred @]\n"
        "func transfer() {}\n"
        "[@ end @]\n"
        "[@ end @]\n";
    pl_qcs_ast ast = {0};
    pl_qcs_error error = {{0}, 0u, 0u};
    assert(pl_qcs_parse_source(source, &ast, &error));
    assert(ast.frame_count == 2u);
    assert(ast.function_count == 1u);
    pl_qcs_ast_free(&ast);
}

static void test_forbidden_characters(void) {
    const char *bad[] = {"let x = a < b", "let x = a > b", "let x = a | b", "let x = a ~ b"};
    size_t i;
    for (i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        pl_qcs_tokens tokens = {0};
        pl_qcs_error error = {{0}, 0u, 0u};
        assert(!pl_qcs_lex(bad[i], &tokens, &error));
        assert(strstr(error.message, "forbidden") != NULL);
        pl_qcs_tokens_free(&tokens);
    }
}

static void test_missing_domain_and_end(void) {
    pl_qcs_ast ast = {0};
    pl_qcs_error error = {{0}, 0u, 0u};
    assert(!pl_qcs_parse_source("[@ state:! @]\n[@ end @]", &ast, &error));
    assert(strstr(error.message, "requires domain") != NULL);
    assert(!pl_qcs_parse_source("[@ domain:mmio @]\nfunc f() {}", &ast, &error));
    assert(strstr(error.message, "unterminated frame") != NULL);
}

static void test_duplicate_domain_attribute(void) {
    pl_qcs_ast ast = {0};
    pl_qcs_error error = {{0}, 0u, 0u};
    assert(!pl_qcs_parse_source("[@ domain:mmio, domain:dma @]\n[@ end @]", &ast, &error));
    assert(strstr(error.message, "duplicate") != NULL);
}

int main(void) {
    test_valid_frame_and_flow();
    test_nested_frames();
    test_forbidden_characters();
    test_missing_domain_and_end();
    test_duplicate_domain_attribute();
    return 0;
}
