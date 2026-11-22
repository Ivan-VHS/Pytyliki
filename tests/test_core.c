#include "core.h"
#include "processor.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(void) {
    pl_cpu_info cpu;
    assert(pl_cpu_profile(&cpu) == 0);
    assert(cpu.logical_cpus >= 1);
    assert(cpu.vendor[0] != '\0');

    pl_processor_snapshot snapshot;
    assert(pl_processor_scan(&snapshot) == 0);
    assert(snapshot.architecture[0] != '\0');
    /* AVX usability includes hardware and OS-managed vector-state prerequisites. */
    assert(!snapshot.avx || (snapshot.avx_hw && snapshot.osxsave && snapshot.avx_os_state));
    assert(!snapshot.avx2 || snapshot.avx);
    assert(!snapshot.fma || snapshot.avx);
    assert(!snapshot.avx512f || (snapshot.avx && snapshot.avx512_os_state));
    assert(!snapshot.avx512_os_state || snapshot.avx_os_state);

    pl_benchmark b;
    assert(pl_run_benchmark(10000, &b) == 0);
    assert(b.iterations == 10000);
    assert(b.elapsed_ms >= 0);
    assert(pl_run_benchmark(0, &b) != 0);
    assert(pl_run_benchmark(UINT64_C(500000001), &b) != 0);

    pl_suite_result suite;
    assert(pl_processor_benchmark_suite(50000, 1, 2, &suite) == 0);
    assert(suite.integrity_ok);
    assert(suite.checksum != 0);
    assert(pl_processor_benchmark_suite(50000, 0, 2, &suite) != 0);

    const char *source_path = "pytyliki_test_input.c";
    FILE *source = fopen(source_path, "wb");
    assert(source);
    fputs("void kernel(void){\n __cpuid(regs, 1);\n __m256 v;\n _mm_prefetch(ptr, 0);\n atomic_thread_fence(memory_order_relaxed);\n}\n", source);
    fclose(source);

    pl_analysis analysis = {0};
    char details[1024];
    assert(pl_analyze_file(source_path, &analysis, details, sizeof(details)) == 0);
    assert(analysis.files_scanned == 1);
    assert(analysis.findings == 5);
    assert(strstr(details, "x86 SIMD intrinsic") == NULL);
    assert(strstr(details, "CPUID feature discovery") != NULL);
    assert(strstr(details, "cache prefetch hint") != NULL);
    remove(source_path);

    pl_benchmark invalid_benchmark = b;
    invalid_benchmark.elapsed_ms = NAN;
    pl_analysis empty_analysis = {0};
    assert(pl_write_json("pytyliki_invalid_report.json", &cpu, &invalid_benchmark, &empty_analysis) != 0);
    const char *json_path = "pytyliki_test_report.json";
    assert(pl_write_json(json_path, &cpu, &b, &analysis) == 0);
    FILE *json_file = fopen(json_path, "rb");
    assert(json_file != NULL);
    char json_text[2048] = {0};
    size_t json_size = fread(json_text, 1, sizeof(json_text) - 1u, json_file);
    assert(!ferror(json_file));
    fclose(json_file);
    assert(json_size > 0u);
    assert(strstr(json_text, "\"version\": \"0.7.0\"") != NULL);
    remove(json_path);

    puts("All processor and core tests passed.");
    return 0;
}
